#ifndef repo_deletefile_H
#define repo_deletefile_H




/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019, Regents of the University of California.
 *
 * This file is part of NDN repo-ng (Next generation of NDN repository).
 * See AUTHORS.md for complete list of repo-ng authors and contributors.
 *
 * repo-ng is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * repo-ng is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * repo-ng, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "repo-ng/repo-command-parameter.hpp"
#include "repo-ng/repo-command-response.hpp"
#include "messager.h"

#include <QObject>
#include <QInputDialog>
#include <QFileDialog>
#include <QDialog>
#include <QMessageBox>
#include <ndn-cxx/face.hpp>

#include <ndn-cxx/security/command-interest-signer.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/signing-helpers.hpp>
#include <ndn-cxx/util/scheduler.hpp>
#include <ndn-cxx/util/random.hpp>

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/operations.hpp>
#include <boost/iostreams/read.hpp>
#include "common.h"
namespace repo {

using namespace ndn::time;

using std::shared_ptr;
using std::make_shared;
using std::bind;


static int deletefile();

class repo_deletefile : public QObject
{
        Q_OBJECT
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

 explicit repo_deletefile(QObject *parent = nullptr)
    : isUnversioned(true)
    , isSingle(false)
    , useDigestSha256(false)
    , freshnessPeriod(DEFAULT_FRESHNESS_PERIOD)
    , interestLifetime(DEFAULT_INTEREST_LIFETIME)
    , hasTimeout(false)
    , timeout(0)
    , insertStream(nullptr)
    , isVerbose(true)
    , m_scheduler(m_face.getIoService())
    , m_timestampVersion(toUnixTimestamp(system_clock::now()).count())
    , m_processId(0)
    , m_checkPeriod(DEFAULT_CHECK_PERIOD)
    , m_currentSegmentNo(0)
    , m_isFinished(false)
    , m_cmdSigner(m_keyChain)
    , process(nullptr)
  {
      process.setWindowTitle("Post to DataBase");
      process.addMessage("Start Posting Text to Database");
      process.show();
  }

  void
  run();
  ////////////////////////
  void
  setInsertStream(std::istream* inputFileStream);
  ///////////////////////////

private:
  //////////////////////
  void
  setSingle();
  ////////////////////////
  void
  prepareNextData(uint64_t referenceSegmentNo);

  void
  startDeleteCommand();

  void
  onDeleteCommandResponse(const ndn::Interest& interest, const ndn::Data& data);
  /////////////////////////////////////
  //void
  //setSingle(const )
  /// /////////////////////////////////
  //////////////////////
  void
  onDeleteCommandNack(const ndn::Interest& interest, const ndn::lp::Nack& nack);
///////////////////////////////
  void
  onDeleteCommandTimeout(const ndn::Interest& interest);
  void
  onInterestSelect(const ndn::Name& prefix, const ndn::Interest& interest);
  void
  onInterest(const ndn::Name& prefix, const ndn::Interest& interest);

  void
  onSingleInterest(const ndn::Name& prefix, const ndn::Interest& interest);

  void
  onRegisterSuccess(const ndn::Name& prefix);

  void
  onRegisterFailed(const ndn::Name& prefix, const std::string& reason);

  void
  stopProcess();

  void
  signData(ndn::Data& data);

  void
  startCheckCommand();

  void
  onCheckCommandResponse(const ndn::Interest& interest, const ndn::Data& data);

void onCheckCommandNack(const ndn::Interest& interest);

  void
  onCheckCommandTimeout(const ndn::Interest& interest);

  ndn::Interest
  generateCommandInterest(const ndn::Name& commandPrefix, const std::string& command,
                          const RepoCommandParameter& commandParameter);


public:
  bool isUnversioned;
  bool isSingle;
  bool useDigestSha256;
  std::string identityForData;
  std::string identityForCommand;
  milliseconds freshnessPeriod;
  milliseconds interestLifetime;
  bool hasTimeout;
  milliseconds timeout;
  ndn::Name repoPrefix;
  ndn::Name ndnName;
  std::istream* insertStream;
  bool isVerbose;
  Messager process;
private:
  ndn::Face m_face;
  ndn::Scheduler m_scheduler;
  ndn::KeyChain m_keyChain;
  uint64_t m_timestampVersion;
  uint64_t m_processId;
  milliseconds m_checkPeriod;
  size_t m_currentSegmentNo;
  bool m_isFinished;
  ndn::Name m_dataPrefix;
  using DataContainer = std::map<uint64_t, shared_ptr<ndn::Data>>;
  DataContainer m_data;
  ndn::security::CommandInterestSigner m_cmdSigner;

signals:

public slots:
    };
////////////////////////////////////
/// \brief putfile
/// This is to post file and build an independent Interface
/// \return Execution Success or not
/////
static int
deletefile(){
repo_deletefile repo_deletefile;
repo_deletefile.isUnversioned = true;
repo_deletefile.isSingle = true;
repo_deletefile.isVerbose = true;
bool nameset = false;
std::string repoPrefix;
std::string ndnName;
while(!nameset){
repoPrefix = QInputDialog::getText(NULL, "repo-ng: repoPrefix",
                                                   "Please Enter repoPrefix",
                                                   QLineEdit::Normal,
                                                   "/example/repo/1",
                                                   &nameset).toStdString();
}

nameset = false;
repo_deletefile.repoPrefix = Name(repoPrefix);
repo_deletefile.process.addMessage(QString::fromStdString("\nSetting DataBase Prefix: "+repoPrefix));
while(!nameset){
ndnName = QInputDialog::getText(NULL, "repo-ng: Data Prefix",
                                                   "Please Enter ndnName",
                                                   QLineEdit::Normal,
                                                   "/sijiahi",
                                                   &nameset).toStdString();}
repo_deletefile.ndnName = Name(ndnName);
repo_deletefile.process.addMessage(QString::fromStdString("\nSetting Data Prefix: "+ndnName));
//bool inputFileStreamReady = false;
//std::string file_name = QFileDialog::getOpenFileName(NULL,"Select file to post on DataBase",".","*.*").toStdString();
//std::ifstream inputFileStream(file_name, std::ios::in | std::ios::binary);
/*while (!inputFileStreamReady) {

    if(inputFileStream.is_open())
    {
        repo_deletefile.process.addMessage(QString::fromStdString("\nReading file: "+file_name));
        inputFileStreamReady = true;}
    else
        repo_deletefile.process.addMessage(QString::fromStdString("\nERROR: cannot open " + file_name));
}*/
    //repo_deletefile.insertStream = &inputFileStream;
    repo_deletefile.run();
}
/////////////////////////////
} //repo

#endif // repo_deletefile_H
