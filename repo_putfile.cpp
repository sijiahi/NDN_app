#include "repo_putfile.h"
#include <QInputDialog>
#include <QFileDialog>
namespace repo{
//////////////////////////
///////////////////////////// \brief initMessager
void
repo_putfile::initMessager(){
messager->setWindowTitle("Post to DataBase");
messager->addMessage("Start Posting Text to Database");
messager->show();
}
void
repo_putfile::addFile(std::string file_name){
    std::ifstream inputFileStream(file_name, std::ios::in | std::ios::binary);
    //TODO:: Read file and add to files
    fileContainer.insert(std::make_pair(ndn::Name(file_name),&inputFileStream));
    return;
}
//////////////////////////
/// \brief repo_putfile::getEncodedFileName
/// \param filename
/// \return 6 filenames
QStringList
repo_putfile::getEncodedFileName(std::string filename){
    QProcess encoderP;
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
        program = QString::fromStdString(g_Path+std::string("encoder"));
        ///////////////////////////////////////////////
    //TODO: Change Dirname
    //QString program =  "../encoder";
    QStringList arguments;
    QString Infile_name = QString::fromStdString(filename);//convertion
    arguments<<Infile_name;
    encoderP.start(program,arguments);
    while (encoderP.waitForFinished(100) == false) {
        QByteArray qByteRead = encoderP.readAllStandardOutput();
        if (!qByteRead.isEmpty()) {
            QString Outfile_name=QString(qByteRead);
            QStringList Outfile_names = Outfile_name.split("\n");
        }
    }
    QByteArray qByteRead = encoderP.readAllStandardOutput();
    QString Outfile_name=QString(qByteRead);
    QStringList Outfile_names = Outfile_name.split("\n");
    //Remove blank String
    Outfile_names.removeAll("");
    return Outfile_names;
}
///////////////////////////////////////
//Prepare 11 data packets at a time
//referenceSegmentNo = 0:
//      read all file and load into ndn::Data
////////////////////////////////////////
void
repo_putfile::prepareData(uint64_t referenceSegmentNo)
{
  // make sure m_data has [referenceSegmentNo, referenceSegmentNo + PRE_SIGN_DATA_COUNT] Data
  if (m_dataLoaded)
    return;
  size_t nDataToPrepare = PRE_SIGN_DATA_COUNT;
  ///Set How many packet should be set this time
  if (!m_data.empty()) {
    uint64_t maxSegmentNo = m_data.rbegin()->first;
    if (maxSegmentNo - referenceSegmentNo >= nDataToPrepare) {
      // nothing to prepare
      return;
    }
    nDataToPrepare -= maxSegmentNo - referenceSegmentNo;
  }
  isSingle = false;
  for (size_t i = 0; i < nDataToPrepare && !m_dataLoaded; ++i) {
    uint8_t buffer[DEFAULT_BLOCK_SIZE];
    auto readSize = boost::iostreams::read(*insertStream,
                                           reinterpret_cast<char*>(buffer), DEFAULT_BLOCK_SIZE);
    if (readSize <= 0) {
      messager->addMessage(QString::fromStdString("\nError reading from the input stream"));
      messager->repaint();
      return;
    }
    ///Single packet scenario
    if (m_currentSegmentNo == 0&&readSize<DEFAULT_BLOCK_SIZE){
        auto data = make_shared<ndn::Data>(m_dataPrefix);
        data->setContent(buffer, readSize);
        data->setFreshnessPeriod(freshnessPeriod);
        signData(*data);
        m_data.insert(std::make_pair(m_currentSegmentNo, data));
        isSingle = true;
        m_dataLoaded = true;
    }
    //multiple packet scenario
    else{
        auto data = make_shared<ndn::Data>(Name(m_dataPrefix).appendSegment(m_currentSegmentNo));
        if (insertStream->peek() == std::istream::traits_type::eof()) {
          data->setFinalBlock(ndn::name::Component::fromSegment(m_currentSegmentNo));
          m_dataLoaded = true;
        }
        data->setContent(buffer, readSize);
        data->setFreshnessPeriod(freshnessPeriod);
        signData(*data);
        m_data.insert(std::make_pair(m_currentSegmentNo, data));
        ++m_currentSegmentNo;
    }//single or not
  }
  if(m_dataLoaded){
    messager->addMessage(QString::fromStdString("Data Loaded"));
    messager->repaint();
    }
}
void
repo_putfile::setSingle(){
 if (m_data.size() == 1) {
     isSingle = true;}
else{
    isSingle = false;
}}
void
repo_putfile::run()
{
    m_dataLoaded = false;
    m_isFinished = false;
  m_dataPrefix = ndnName;
  if (!isUnversioned)
    m_dataPrefix.appendVersion(m_timestampVersion);
  prepareData(0);
  messager->addMessage(QString::fromStdString("setInterestFilter for "+ m_dataPrefix.toUri()));
 messager->repaint();
  if (isVerbose)
    std::cerr << "setInterestFilter for " << m_dataPrefix << std::endl;
  m_face.setInterestFilter(m_dataPrefix,
                           bind(&repo_putfile::onInterest, this, _1, _2),
                           bind(&repo_putfile::onRegisterSuccess, this, _1),
                           bind(&repo_putfile::onRegisterFailed, this, _1, _2));

  if (hasTimeout)
    m_scheduler.schedule(timeout, [this] { stopProcess(); });
  do{
  m_face.processEvents(timeout = time::seconds(1));
  sleep(0.5);
  //std::cout<<"timeoutonce"<<std::endl;
  }while(m_isFinished == false);
  //TODO
  m_face.removeAllPendingInterests();
  m_data.clear();
  stopProcess();
  sleep(1);
   return;
}

void
repo_putfile::runFiles()
{
 for (fileStreamContainer::iterator it = fileContainer.begin();it!=fileContainer.end();it++){
    m_dataPrefix = it->first;
    insertStream = it->second;
    //TODO
    if (isVerbose){
    messager->addMessage(QString::fromStdString("setInterestFilter for "+ m_dataPrefix.toUri()));
    messager->repaint();}
    std::cerr << "setInterestFilter for " << m_dataPrefix << std::endl;
    //std::ifstream inputFileStreamtt("/home/sijia/Desktop/swap.txt.0", std::ios::in | std::ios::binary);
    if (!isUnversioned)
       m_dataPrefix.appendVersion(m_timestampVersion);
     prepareData(0);
     if (isVerbose)
       std::cerr << "setInterestFilter for " << m_dataPrefix << std::endl;
     m_face.setInterestFilter(m_dataPrefix,
                              bind(&repo_putfile::onInterest, this, _1, _2),
                              bind(&repo_putfile::onRegisterSuccess, this, _1),
                              bind(&repo_putfile::onRegisterFailed, this, _1, _2));

     if (hasTimeout)
       m_scheduler.schedule(timeout, [this] { stopProcess(); });

     m_face.processEvents();
 }
    return;
}

void
repo_putfile::onRegisterSuccess(const Name& prefix)
{
  std::cout<<"Register prefix: "<<prefix<<" Success! "<<std::endl;
  startInsertCommand();
}

void
repo_putfile::startInsertCommand()
{
  //** Insert Command must set 3 arguments:
  //1 PrefixName(setName(String))
  //2 StartBlockId;
  //2 EndblockId;
  RepoCommandParameter parameters;
  parameters.setName(m_dataPrefix);
//****Need to set Start Block ID and End Block Id to enable Data Transmission!!!!
  if (!isSingle) {
      parameters.setStartBlockId(0);
  }
  ndn::Interest commandInterest = generateCommandInterest(repoPrefix, "insert", parameters);
  std::cout<<"sending insert command for "<<m_dataPrefix<<std::endl;
  m_face.expressInterest(commandInterest,
                         bind(&repo_putfile::onInsertCommandResponse, this, _1, _2),
                         bind(&repo_putfile::onInsertCommandNack, this, _1, _2), // Nack
                         bind(&repo_putfile::onInsertCommandTimeout, this, _1));
}

void
repo_putfile::onInsertCommandResponse(const ndn::Interest& interest, const ndn::Data& data)
{
  RepoCommandResponse response(data.getContent().blockFromValue());
  auto statusCode = response.getCode();
  std::string status = response.getInsertionStatus();
  if (isVerbose){
  messager->addMessage(QString::fromStdString("\nInsert Command Response: "+status));
  messager->repaint();}
  if (statusCode >= 400) {
    BOOST_THROW_EXCEPTION(Error("insert command failed with code " +
                                boost::lexical_cast<std::string>(statusCode)));}
    m_processId = response.getProcessId();
    m_scheduler.schedule(m_checkPeriod, [this] { isSingle? startSingleCheckCommand():startCheckCommand(); });
}

void
repo_putfile::onInsertCommandNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
{
    std::stringstream text;
    text << "Received nack: " << nack.getReason() <<"!";
        QMessageBox msg;
        msg.setText(QString::fromStdString(text.str()));
        msg.setWindowTitle("ERROR");
        msg.exec();
    BOOST_THROW_EXCEPTION(Error("insertion command nacked"));
}

void
repo_putfile::onInsertCommandTimeout(const ndn::Interest& interest)
{
  BOOST_THROW_EXCEPTION(Error("command response timeout"));
}

void
repo_putfile::onInterest(const ndn::Name& prefix, const ndn::Interest& interest)
{
if(!isSingle){
  std::cout<<"Received Interest"<<interest<<std::endl;
  if (interest.getName().size() != prefix.size() + 1) {
    if (isVerbose) {
        std::stringstream text;
        //text <<  interest  ;
        //process.addMessage(QString::fromStdString("\nUnrecognized Interest: "+text.str()));
    }
    return;
  }
  uint64_t segmentNo;
  try {
    ndn::Name::Component segmentComponent = interest.getName().get(prefix.size());
    segmentNo = segmentComponent.toSegment();
  }
  catch (const tlv::Error& e) {
    if (isVerbose) {
      std::cerr << "Error processing incoming interest " << interest << ": "
                << e.what() << std::endl;
    }
    return;
  }
  prepareData(segmentNo);
  DataContainer::iterator item = m_data.find(segmentNo);
  if (item == m_data.end()) {
    if (isVerbose) {
      std::cerr << "Requested segment [" << segmentNo << "] does not exist" << std::endl;
    }
    return;
  }
  if (m_isFinished) {
    uint64_t final = m_currentSegmentNo - 1;
    item->second->setFinalBlock(ndn::name::Component::fromSegment(final));
  }
  std::cout<<"Transmitting data: "<<*item->second<<std::endl;
  m_face.put(*item->second);        }
else{
    //single interest
    //BOOST_ASSERT(prefix == m_dataPrefix);
    if (prefix != interest.getName()) {
      if (isVerbose) {
        std::cerr << "Received unexpected interest " << interest << std::endl;
      }
      return;
    }
    /////////////////////////////////////////////
    DataContainer::iterator item = m_data.find(0);
    m_face.put(*item->second);
    std::cout<<"Transmitting data: "<<*item->second<<std::endl;
    //m_isFinished = true;
    //////////////////////////////////////////////
    /////////////////////////////////////////////
}
}


void
repo_putfile::onRegisterFailed(const ndn::Name& prefix, const std::string& reason)
{
  BOOST_THROW_EXCEPTION(Error("onRegisterFailed: " + reason));
}

void
repo_putfile::stopProcess()
{
  m_face.getIoService().stop();
  m_face.shutdown();
}

void
repo_putfile::signData(ndn::Data& data)
{
  if (useDigestSha256) {
    m_keyChain.sign(data, ndn::signingWithSha256());
  }
  else if (identityForData.empty())
    m_keyChain.sign(data);
  else {
    m_keyChain.sign(data, ndn::signingByIdentity(identityForData));
  }
}

void
repo_putfile::startSingleCheckCommand()
{
  //TODO: Set Command Parameters
  Interest fetchInterest(m_dataPrefix);
  fetchInterest.setInterestLifetime(interestLifetime);
  fetchInterest.setCanBePrefix(false);
  m_face.expressInterest(fetchInterest,
                         bind(&repo_putfile::onSingleCheckCommandResponse, this, _1, _2),
                         bind(&repo_putfile::onCheckCommandNack, this, _1), // Nack
                         bind(&repo_putfile::onCheckCommandTimeout, this, _1));
}

void
repo_putfile::startCheckCommand()
{
  //TODO: Set Command Parameters
  RepoCommandParameter parameters;
  parameters.setName(m_dataPrefix);
  parameters.setProcessId(m_processId);
  ndn::Interest checkInterest = generateCommandInterest(repoPrefix, "insertcheck",
                                                        parameters);
  std::cout<<"Expressing Check Command Interest"<<checkInterest<<std::endl;
  std::cout<<"For Process ID "<<m_processId<<std::endl;
  m_face.expressInterest(checkInterest,
                         bind(&repo_putfile::onCheckCommandResponse, this, _1, _2),
                         bind(&repo_putfile::onCheckCommandNack, this, _1), // Nack
                         bind(&repo_putfile::onCheckCommandTimeout, this, _1));
}
void
repo_putfile::onSingleCheckCommandResponse(const ndn::Interest& interest, const ndn::Data& data)
{
    DataContainer::iterator localData = m_data.find(0);

    if(data.getContent() == (*localData->second).getContent()){
        if (isVerbose){
        messager->addMessage(QString::fromStdString("\nInsertion Process Finished !"));
        messager->repaint();}
        m_isFinished = true;
    }else{
             std::stringstream text;
             text<<"Different from what we sent, Received Data: "<<data.getName()<<"\n";
             const auto& content = data.getContent();
             text<<"Data Content is "<<reinterpret_cast<const char*>(content.value());
             if (isVerbose){
              messager->addMessage(QString::fromStdString(text.str()));
              messager->repaint();}
              m_isFinished = true;
          }
    return;

}
void
repo_putfile::onCheckCommandResponse(const ndn::Interest& interest, const ndn::Data& data)
{
    //////////////////////////////////////////////
    /// \brief Controlresponse
    RepoCommandResponse response(data.getContent().blockFromValue());
    std::string status = response.getInsertionStatus();
    if (isVerbose){
    messager->addMessage(QString::fromStdString("\nCheck Insertion Process: "+status));}
    auto statusCode = response.getCode();
    if (statusCode >= 400) {
        messager->addMessage(QString::fromStdString("\nInsertion failed with Message from code: "+statusCode+response.getInsertionStatus()));
        messager->addMessage(QString::fromStdString("\nWith Message: "+response.getText()));
        messager->repaint();
        return;}
//////////////////////////////////////////////
    if (statusCode == 200) {
      //TODO: A Msg Box to notify User Process Has finished
        if (isVerbose){
          messager->addMessage(QString::fromStdString("\nInsertion Process Finished !"));
          messager->repaint();}
          m_isFinished = true;
          return;
      }
  if (m_isFinished) {
    //TODO: A Msg Box to notify User Process Has finished
    uint64_t insertCount = response.getInsertNum();
    if (isSingle) {
      if (insertCount == 1) {
          if (isVerbose){
        messager->addMessage(QString::fromStdString("\nInsertion Process Finished !"));
        messager->repaint();
        m_isFinished = true;
        return;

      }
    }
    // Technically, the check should not infer, but directly has signal from repo that
    // write operation has been finished
    if (insertCount == m_currentSegmentNo) {
      m_face.getIoService().stop();
      if (isVerbose){
      messager->addMessage(QString::fromStdString("\nInsertion Process Finished !"));
      messager->repaint();}
    }
      m_isFinished = true;
      return;
    }
  }
      m_scheduler.schedule(m_checkPeriod, [this] { isSingle? startSingleCheckCommand():startCheckCommand(); });
}

void
repo_putfile::onCheckCommandNack(const ndn::Interest& interest)
{
    messager->addMessage(QString::fromStdString("\nCheck response Nacked! "));
    messager->repaint();
    BOOST_THROW_EXCEPTION(Error("check response Nacked"));
}

void
repo_putfile::onCheckCommandTimeout(const ndn::Interest& interest)
{
    messager->addMessage(QString::fromStdString("\nCheck response timeout! "));
    //messager->exec();
    BOOST_THROW_EXCEPTION(Error("check response timeout"));
}

ndn::Interest
repo_putfile::generateCommandInterest(const ndn::Name& commandPrefix, const std::string& command,
                                    const RepoCommandParameter& commandParameter)
{
  Name cmd = commandPrefix;
  cmd
    .append(command)
    .append(commandParameter.wireEncode());
  ndn::Interest interest;
  if (identityForCommand.empty())
    interest = m_cmdSigner.makeCommandInterest(cmd);
  else {
    interest = m_cmdSigner.makeCommandInterest(cmd, ndn::signingByIdentity(identityForCommand));
  }
  interest.setCanBePrefix(false);
  interest.setInterestLifetime(interestLifetime);
  return interest;
}
} // namespace repo


