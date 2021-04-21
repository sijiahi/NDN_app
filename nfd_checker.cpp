#include "nfd_checker.h"
#include <QMessageBox>
#include <QTextStream>
#include <message_parser.h>
#include <QDebug>
#include <QProcess>
nfd_checker::nfd_checker()
{
}
bool
nfd_checker::check(){
    try{
        Name interestName("/localhost/nfd/status/general");
        Interest  interest(interestName);
        interest.setCanBePrefix(true);
        //interest.setCanBePrefix(false);
        std::cout << "Sending Interest " << interest << std::endl;
        m_face.expressInterest(interest,
                               bind(&nfd_checker::onData, this,  _1, _2),
                               bind(&nfd_checker::onNack, this, _1, _2),
                               bind(&nfd_checker::onTimeout, this, _1));
        // processEvents will block until the requested data is received or a timeout occurs
        m_face.processEvents();}
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        QMessageBox msgBox;
        QString msg = e.what();
        msgBox.setText(msg);
        msgBox.exec();
      }
}
void
nfd_checker::onData(const Interest& interest, const Data& data){
    //qout<<data<<endl;
       std::cout << "Received Data " << data << std::endl;
       std::cout << "Received Data " << data.getContent() << std::endl;
       message_parser parser;
       parser.wireDecode(data.getContent());
       QMessageBox msgBox;
       msgBox.setText("hh");
       msgBox.exec();
}
void
nfd_checker::onNack(const Interest& interest, const Nack& nack){
    std::cout << "Received Nack with reason " << nack.getReason() << std::endl;
    //QMessageBox msgBox;
    QProcess::execute("nfd-start");
    QMessageBox msgBox;
    msgBox.setText("Nacked");
    msgBox.exec();
    //msgBox.setText(nack.getReason() );
    //msgBox.exec();
}
void
nfd_checker::onTimeout(const Interest& interest){
      std::cout << "Timeout for " << interest << std::endl;
      QMessageBox msgBox;
      msgBox.setText("Timeout for interest");
      msgBox.exec();
}
