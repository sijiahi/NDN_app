#ifndef REPO_GETFILE_H
#define REPO_GETFILE_H


#include <ndn-cxx/data.hpp>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <fstream>
#include <iostream>
#include <unistd.h>


#include <messager.h>
#include "common.h"
#include <boost/lexical_cast.hpp>

#include <QObject>
#include <QProcess>
#include <QInputDialog>
#include<QLineEdit>
#include <QMessageBox>

#include <dirent.h>
namespace repo {
using namespace ndn::time;
using ndn::Name;
using ndn::Interest;
using ndn::Data;


static int getfile();

class repo_getfile : public QObject
{
        Q_OBJECT
public:
  class Error : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };
public:
explicit repo_getfile(std::ostream& os)
        : //m_dataName(dataName)
         m_os(os)
        , m_verbose(true)
        , m_hasVersion(true)
        , m_isSingle(true)
        , m_isFinished(false)
       , m_fetchedAllData(false)
        , m_isFirst(true)
        , m_interestLifetime(4000)
       , hasTimeout(false)
       , timeout(0)
       , m_timeout(0)
        , m_nextSegment(0)
        , m_totalSize(0)
        , m_retryCount(0)
        , m_mustBeFresh(false)
        , m_canBePrefix(true)
        , messager(nullptr)
      {

      }

      void
      initMessager();
      void
      run();
      void
      combineFiles(QStringList filenames);
     Messager* messager;
     ndn::Name m_dataName;
    private:
     void
     stopProcess();
      void
      fetchData(const ndn::Name& name);

      void
      onVersionedData(const ndn::Interest& interest, const ndn::Data& data);

      void
      onUnversionedData(const ndn::Interest& interest, const ndn::Data& data);

      void
      onTimeout(const ndn::Interest& interest);

      void
      readData(const ndn::Data& data);

      void
      fetchNextData(const ndn::Name& name, const ndn::Data& data);

    private:
      ndn::Face m_face;
      bool hasTimeout;
      ndn::time::milliseconds timeout;
      std::ostream& m_os;
      bool m_verbose;
      bool m_hasVersion;
      bool m_isSingle;
      bool m_isFinished;
      bool m_fetchedAllData;
      bool m_isFirst;
      ndn::time::milliseconds m_interestLifetime;
      ndn::time::milliseconds m_timeout;
      uint64_t m_nextSegment;
      int m_totalSize;
      int m_retryCount;
      bool m_mustBeFresh;
      bool m_canBePrefix;

      static constexpr int MAX_RETRY = 3;
    };
static int
getfile()
{

    std::streambuf* buf;
    std::ofstream of;
    const char* outputFile = nullptr;
    std::string filename;
    std::string name;
    Messager messager;
    messager.setWindowTitle("Get file");
    messager.addMessage("\nStart Getting file");
    messager.show();
    bool nameset = false;
    while(!nameset){
    name = QInputDialog::getText(NULL, "ndnPrefix: dataPrefix",
                                                       "Please Enter dataPrefix",
                                                       QLineEdit::Normal,
                                                       "/test/01",
                                                       &nameset).toStdString();
    }
    messager.addMessage(QString::fromStdString("\nSetting data Prefix: "+name));
messager.repaint();
    nameset = false;
    while(!nameset){
    filename = QInputDialog::getText(NULL, "Saving new file",
                                                       "Please Enter new filename",
                                                       QLineEdit::Normal,
                                                       "test.txt",
                                                       &nameset).toStdString();
    }
    messager.addMessage(QString::fromStdString("\nRetrieving file to: "+filename));
    messager.repaint();
    /*
    outputFile = filename.c_str();
    if (outputFile != nullptr) {

      of.open(outputFile, std::ios::out | std::ios::binary | std::ios::trunc);
      if (!of || !of.is_open()) {
        std::cerr << "ERROR: cannot open " << outputFile << std::endl;
        return 2;
      }
      buf = of.rdbuf();
    }
    else {
      buf = std::cout.rdbuf();
    }
    */
   QStringList filenames;
    for (int i = 0; i < 6; i++){
  messager.addMessage(QString::fromStdString(("\nINFO: Fetching data :"+std::to_string(i))));
  messager.repaint();
  filenames<<QString::fromStdString("/files/"+filename+'.'+std::to_string(i));
  outputFile = ("/files/"+filename+'.'+std::to_string(i)).c_str();
  if (outputFile != nullptr) {
    of.open(outputFile, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!of || !of.is_open()) {
      std::cerr << "ERROR: cannot open " << outputFile <<of.is_open()<< std::endl;
      return 2;
    }
    buf = of.rdbuf();
  }
  else {
    buf = std::cout.rdbuf();
  }

std::ostream os(buf);
repo_getfile repo_getfile(os);
repo_getfile.m_dataName = Name(name).append(std::to_string(i));
repo_getfile.messager = &messager;
repo_getfile.initMessager();

try {
  repo_getfile.run();
}
catch (const std::exception& e) {
  std::cerr << "ERROR: " << e.what() << std::endl;
  return 1;
}
of.close();
messager.addMessage(QString::fromStdString(("INFO: Data collected :"+std::to_string(i))));
messager.repaint();
if (i == 6)
    repo_getfile.combineFiles(filenames);

  }
  messager.addMessage(QString("\nAll data has been collected!"));
  messager.repaint();
  messager.exec();
  return 0;
}
} //repo

#endif // REPO_PUTFILE_H
