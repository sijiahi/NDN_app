#include "repo_deletefile.h"
#include <QInputDialog>
#include <QFileDialog>
namespace repo{
void
repo_deletefile::prepareNextData(uint64_t referenceSegmentNo)
{
  // make sure m_data has [referenceSegmentNo, referenceSegmentNo + PRE_SIGN_DATA_COUNT] Data
  if (m_isFinished)
    return;
  size_t nDataToPrepare = PRE_SIGN_DATA_COUNT;
  if (!m_data.empty()) {
    uint64_t maxSegmentNo = m_data.rbegin()->first;
    if (maxSegmentNo - referenceSegmentNo >= nDataToPrepare) {
      // nothing to prepare
      return;
    }
    nDataToPrepare -= maxSegmentNo - referenceSegmentNo;
  }

  for (size_t i = 0; i < nDataToPrepare && !m_isFinished; ++i) {
    uint8_t buffer[DEFAULT_BLOCK_SIZE];
    auto readSize = boost::iostreams::read(*insertStream,
                                           reinterpret_cast<char*>(buffer), DEFAULT_BLOCK_SIZE);
    if (readSize <= 0) {
      BOOST_THROW_EXCEPTION(Error("Error reading from the input stream"));
    }

    auto data = make_shared<ndn::Data>(Name(m_dataPrefix).appendSegment(m_currentSegmentNo));

    if (insertStream->peek() == std::istream::traits_type::eof()) {
      data->setFinalBlock(ndn::name::Component::fromSegment(m_currentSegmentNo));
      m_isFinished = true;
    }
    data->setContent(buffer, readSize);
    data->setFreshnessPeriod(freshnessPeriod);
    signData(*data);
    m_data.insert(std::make_pair(m_currentSegmentNo, data));
    ++m_currentSegmentNo;
  }
}
void
repo_deletefile::setSingle(){
 if (insertStream->peek() != std::istream::traits_type::eof()) {
     isSingle = false;
 }
 else{
     isSingle = true;
 }
}
void
repo_deletefile::run()
{
  m_dataPrefix = ndnName;
startDeleteCommand();
}
void
repo_deletefile::startDeleteCommand()
{  //** Insert Command must set 3 arguments:
  //1 PrefixName(setName(String))
  //2 StartBlockId;
  //2 EndblockId;
  RepoCommandParameter parameters;
  parameters.setName(m_dataPrefix);
//****Need to set Start Block ID and End Block Id to enable Data Transmission!!!!
  if (!isSingle) {
      parameters.setStartBlockId(0);
  }
  m_processId = ndn::random::generateWord64();
  parameters.setProcessId(m_processId);
  ndn::Interest commandInterest = generateCommandInterest(repoPrefix, "delete", parameters);
  m_face.expressInterest(commandInterest,
                         bind(&repo_deletefile::onDeleteCommandResponse, this, _1, _2),
                         bind(&repo_deletefile::onDeleteCommandNack, this, _1, _2), // Nack
                         bind(&repo_deletefile::onDeleteCommandTimeout, this, _1));
  // processEvents will block until the requested data is received or a timeout occurs
  m_face.processEvents();
}

void
repo_deletefile::onDeleteCommandResponse(const ndn::Interest& interest, const ndn::Data& data)
{
  RepoCommandResponse response(data.getContent().blockFromValue());
  if(m_processId == response.getProcessId()){
      auto statusCode = response.getCode();
      std::string status = response.getDeletionStatus();
      process.addMessage(QString::fromStdString("\nInsert Command Response: "+status));
      if (statusCode >= 400) {
          if(response.hasStatusCode())
              process.addMessage(QString::fromStdString("\nInsert Command Failed with Message"+response.getText()));
          else{
              process.addMessage(QString::fromStdString("\nInsert Command Failed with Code"+response.getCode()));
                process.exec();
        BOOST_THROW_EXCEPTION(Error("insert command failed with code " +
                                    boost::lexical_cast<std::string>(statusCode)));}}
      if(statusCode == 300)
        m_scheduler.schedule(m_checkPeriod, [this] { startCheckCommand(); });
      if(statusCode == 200){
          process.addMessage(QString::fromStdString("\nInsert Command Success with Code"+response.getCode()));
            process.exec();
            return;}
  }
  else{
      process.addMessage("Wrong ProcessID!");
      process.exec();
      return;
  }
}

void
repo_deletefile::onDeleteCommandNack(const ndn::Interest& interest, const ndn::lp::Nack& nack)
{
    std::stringstream reason;
    reason << nack.getReason() <<"!";
    process.addMessage(QString::fromStdString("\nDelete Command Nacked with reason: "+reason.str()));
    process.exec();
    BOOST_THROW_EXCEPTION(Error("Delete command nacked"));
}

void
repo_deletefile::onDeleteCommandTimeout(const ndn::Interest& interest)
{
  process.addMessage(QString::fromStdString("\nDelete Command Timeout! "));
  process.exec();
  BOOST_THROW_EXCEPTION(Error("Delete command timeout! "));
}

void
repo_deletefile::onInterestSelect(const ndn::Name& prefix, const ndn::Interest& interest)
{
    uint8_t buffer[DEFAULT_BLOCK_SIZE];
    std::streamsize readSize =
    boost::iostreams::read(*insertStream, reinterpret_cast<char*>(buffer), DEFAULT_BLOCK_SIZE);
  if (!isSingle) {
      ////////////////////////////////////////////////////////
      if (interest.getName().size() != prefix.size() + 1) {
        if (isVerbose) {
          std::cerr << "Error processing incoming interest " << interest << ": "
                    << "Unrecognized Interest" << std::endl;
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
      prepareNextData(segmentNo);
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
      process.addMessage(QString::fromStdString("\nTransmitted data! "));
      std::cout<<"Transmitting data: "<<*item->second<<std::endl;
      m_face.put(*item->second);
      return;
    ////////////////////////////////////////////////////////////////////////////////
  }
  else{
    ////////////////////////////////////////////////////////////////////////////////
      if (readSize <= 0) {
        BOOST_THROW_EXCEPTION(Error("Error reading from the input stream(Single)"));
      }
      std::cout<<"Processing single insert interest"<<std::endl;
      std::cerr << "\nReceived unexpected interest " << interest.getName() <<"\nExpecting Prefix: "<<prefix<<std::endl;
      BOOST_ASSERT(prefix == m_dataPrefix);
      auto data = make_shared<ndn::Data>(m_dataPrefix);
      data->setContent(buffer, readSize);
      data->setFreshnessPeriod(freshnessPeriod);
      signData(*data);
      m_face.put(*data);
      m_isFinished = true;
      process.addMessage(QString::fromStdString("\nTransmitted data !"));
      return;
    ////////////////////////////////////////////////////////
  }
}



void
repo_deletefile::stopProcess()
{
  m_face.getIoService().stop();
}

void
repo_deletefile::signData(ndn::Data& data)
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
repo_deletefile::startCheckCommand()
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
                         bind(&repo_deletefile::onCheckCommandResponse, this, _1, _2),
                         bind(&repo_deletefile::onCheckCommandNack, this, _1), // Nack
                         bind(&repo_deletefile::onCheckCommandTimeout, this, _1));
}

void
repo_deletefile::onCheckCommandResponse(const ndn::Interest& interest, const ndn::Data& data)
{
    //////////////////////////////////////////////
    /// \brief Controlresponse
    RepoCommandResponse response(data.getContent().blockFromValue());
    std::string status = response.getDeletionStatus();
    process.addMessage(QString::fromStdString("\nCheck Insertion Process: "+status));
    auto statusCode = response.getCode();
    if (statusCode >= 400) {
        process.addMessage(QString::fromStdString("\nInsertion failed with Message from code: "+statusCode+response.getDeletionStatus()));
        process.addMessage(QString::fromStdString("\nWith Message: "+response.getText()));
      process.exec();
      BOOST_THROW_EXCEPTION(Error("Insertion failed with code " +
                                  boost::lexical_cast<std::string>(statusCode)));}
    process.addMessage(QString::fromStdString("\nCheck Insertion Process: "+statusCode));
//////////////////////////////////////////////
    if (statusCode == 200) {
      //TODO: A Msg Box to notify User Process Has finished
          m_face.getIoService().stop();
          process.addMessage(QString::fromStdString("\Deletion Process Finished !"));
          process.exec();
          return;
      }
  if (m_isFinished) {
    //TODO: A Msg Box to notify User Process Has finished
    uint64_t insertCount = response.getInsertNum();
    if (isSingle) {
      if (insertCount == 1) {
        m_face.getIoService().stop();
        process.addMessage(QString::fromStdString("\nDeletion Process Finished !"));
        process.exec();
        return;
      }
    }
    // Technically, the check should not infer, but directly has signal from repo that
    // write operation has been finished
    if (insertCount == m_currentSegmentNo) {
      m_face.getIoService().stop();
      process.addMessage(QString::fromStdString("\nInsertion Process Finished !"));
      process.exec();
      return;
    }
  }
      m_scheduler.schedule(m_checkPeriod, [this] { startCheckCommand(); });
}

void
repo_deletefile::onCheckCommandNack(const ndn::Interest& interest)
{
    process.addMessage(QString::fromStdString("\nCheck response Nacked! "));
    process.exec();
    BOOST_THROW_EXCEPTION(Error("check response Nacked"));
}

void
repo_deletefile::onCheckCommandTimeout(const ndn::Interest& interest)
{
    process.addMessage(QString::fromStdString("\nCheck response timeout! "));
    process.exec();
    BOOST_THROW_EXCEPTION(Error("check response timeout"));
}

ndn::Interest
repo_deletefile::generateCommandInterest(const ndn::Name& commandPrefix, const std::string& command,
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
  interest.setInterestLifetime(interestLifetime);
  return interest;
}
} // namespace repo


