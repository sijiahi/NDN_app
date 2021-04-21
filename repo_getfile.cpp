#include "repo_getfile.h"
#include <QInputDialog>
#include <QFileDialog>
namespace repo{
void
repo_getfile::initMessager(){
    messager->setWindowTitle("Get file");
    messager->repaint();
}
void
repo_getfile::stopProcess()
{
  m_face.getIoService().stop();
  m_face.shutdown();
}
void
repo_getfile::combineFiles(QStringList filenames){
    return;
    QProcess decoderP;
    std::string g_Path;
    try{
        DIR *dir;

        if ((dir=opendir(std::string("./Dependencies/").c_str())) != NULL)
         {
            g_Path = "./Dependencies/";
        }
        else if((dir=opendir(std::string("../Dependencies/").c_str())) != NULL){
            g_Path = "../Dependencies/";
        }
        if (g_Path == "")
         {
             QMessageBox error;
             char *buffer = getcwd(NULL, 0);
             error.setText(QString::fromStdString(std::string("Can not find encoder path from: ")+buffer));
             error.exec();
        }
    }catch(const std::exception& e){
        std::cout<<"ERROR :"<<e.what()<<std::endl;
        QMessageBox errorAlert;
        errorAlert.setText(e.what());
        errorAlert.show();
    }
        QString program;
        program = QString::fromStdString(g_Path+std::string("decoder"));
        ///////////////////////////////////////////////
    //TODO: Change Dirname
    //QString program =  "../encoder";
    QStringList arguments;
    for(int i = 0;i<=filenames.size();i++){
        arguments<<filenames[i];
    }
    decoderP.start(program,arguments);
    while (decoderP.waitForFinished(100) == false) {
        QByteArray qByteRead = decoderP.readAllStandardOutput();
        if (!qByteRead.isEmpty()) {
            QString Outfile_name=QString(qByteRead);
            QStringList Outfile_names = Outfile_name.split("\n");
        }
    }
    QByteArray qByteRead = decoderP.readAllStandardOutput();
    QString Outfile_name=QString(qByteRead);
    std::cout<<Outfile_name.toStdString()<<std::endl;
    messager->addMessage("\nFile Retrieve Success!"+Outfile_name);
    messager->repaint();
}
void
repo_getfile::fetchData(const Name& name)
{
  Interest interest(name);
  interest.setInterestLifetime(m_interestLifetime);

  interest.setCanBePrefix(m_canBePrefix);

  m_face.expressInterest(interest,
                         m_hasVersion ? std::bind(&repo_getfile::onVersionedData, this, _1, _2)
                                      : std::bind(&repo_getfile::onUnversionedData, this, _1, _2),
                         std::bind(&repo_getfile::onTimeout, this, _1), // Nack
                         std::bind(&repo_getfile::onTimeout, this, _1));
}
void
repo_getfile::run()
{
  // Send the first Interest
  m_isFinished = false;
  Name name(m_dataName);
  m_nextSegment++;
  messager->addMessage("Start Retrieving");
  fetchData(name);
  // processEvents will block until the requested data received or timeout occurs
/*  if (hasTimeout)
    m_scheduler.schedule(timeout, [this] { stopProcess(); });*/
  do{
  m_face.processEvents(timeout = ndn::time::seconds(1));
  sleep(1);
}while(m_isFinished = false);
    stopProcess();
  sleep(2);
}

void
repo_getfile::onVersionedData(const Interest& interest, const Data& data)
{
  const Name& name = data.getName();
  ////DataName = /DataPrefix/version/segmentID
  // Received Data Name = Specified Name(Name + version), send Name + segmentID
  // single data
  if (name.size() == m_dataName.size()) {
    if (!m_isSingle) {
      Name fetchName = name;
      fetchName.appendSegment(0);
      fetchData(fetchName);
    }
  }
  // Received Data Name = Specified Name(Name), send Name + segmentID
  else if (name.size() == m_dataName.size() + 1) {
      if (m_isFirst) {
        uint64_t segment = name[-1].toSegment();
        if (segment != 0) {
          fetchData(Name(m_dataName).appendSegment(0));
          m_isFirst = false;
          return;
        }
        m_isFirst = false;
        m_isSingle = false;
      fetchNextData(name, data);
    }
  }
  else {
    std::cerr << "ERROR: Name size does not match" << std::endl;
    return;
  }
  readData(data);
}

void
repo_getfile::onUnversionedData(const Interest& interest, const Data& data)
{
  ////DataName = /DataPrefix/version/segmentID
  const Name& name = data.getName();
  //Received Data Name = Specified Name + version
  if (name.size() == m_dataName.size() + 1) {
    if (!m_isSingle) {
      Name fetchName = name;
      fetchName.append(name[-1]).appendSegment(0);
      fetchData(fetchName);
    }
  }
  //Received Data Name = Specified Name + version + SegmentID
  else if (name.size() == m_dataName.size() + 2) {
    if (!m_isSingle) {
       if (m_isFirst) {
        uint64_t segment = name[-1].toSegment();
        if (segment != 0) {
          fetchData(Name(m_dataName).append(name[-2]).appendSegment(0));
          m_isFirst = false;
          return;
        }
        m_isFirst = false;
      }
      fetchNextData(name, data);
    }
    else {
      std::cerr << "ERROR: Data is not stored in a single packet" << std::endl;
      return;
    }
  }
  else {
    std::cerr << "ERROR: Name size does not match" << std::endl;
    return;
  }
  readData(data);
}

void
repo_getfile::readData(const Data& data)
{
  const auto& content = data.getContent();
  m_os.write(reinterpret_cast<const char*>(content.value()), content.value_size());
  m_totalSize += content.value_size();
  if (m_verbose) {
    std::cerr << "LOG: received data = " << data.getName() << std::endl;
    messager->addMessage(QString::fromStdString("LOG: received data = "+data.getName().toUri()));
  }
  if (m_fetchedAllData || m_isSingle) {
    std::cerr << "INFO: End of file is reached" << std::endl;
    std::cerr << "INFO: Total # of segments received: " << m_nextSegment  << std::endl;
    std::cerr << "INFO: Total # bytes of content received: " << m_totalSize << std::endl;
    //messager->addMessage(QString("INFO: End of file is reached"));
    //messager->addMessage(QString("INFO: Total # of segments received: "+m_nextSegment));
    messager->addMessage(QString("\nINFO: Total size is: "+m_totalSize));
    messager->repaint();
    m_isFinished = true;
  }
}

void
repo_getfile::fetchNextData(const Name& name, const Data& data)
{
  uint64_t segment = name[-1].toSegment();
  BOOST_VERIFY(segment == (m_nextSegment - 1));

  auto finalBlockId = data.getFinalBlock();
  if (finalBlockId == name[-1]) {
    m_fetchedAllData = true;
  }
  else {
    // Reset retry counter
    m_retryCount = 0;
    if (m_hasVersion)
      fetchData(Name(m_dataName).appendSegment(m_nextSegment++));
    else
      fetchData(Name(m_dataName).append(name[-2]).appendSegment(m_nextSegment++));
  }
}

void
repo_getfile::onTimeout(const Interest& interest)
{
  if (m_retryCount++ < MAX_RETRY) {
    // Retransmit the interest
    fetchData(interest.getName());
    if (m_verbose) {
      std::cerr << "TIMEOUT: retransmit interest for " << interest.getName() << std::endl;
    }
  }
  else {
    std::cerr << "TIMEOUT: last interest sent for segment #" << (m_nextSegment - 1) << std::endl;
    std::cerr << "TIMEOUT: abort fetching after " << MAX_RETRY
              << " times of retry" << std::endl;
  }
}


} // namespace repo
/*
int
main1(int argc, char** argv)
{
  return repo::main(argc, argv);
}
*/
