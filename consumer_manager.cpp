#include "consumer_manager.h"
#include "ui_consumer_manager.h"
#include <QMessageBox>
Consumer_manager::Consumer_manager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Consumer_manager)
{
    ui->setupUi(this);
}

Consumer_manager::~Consumer_manager()
{
    delete ui;
}

void Consumer_manager::on_AddPrefix_clicked()
{
    std::string inputPrefix = ui->PrefixInput->text().toStdString();
    std::cout<<inputPrefix<<std::endl;
    sendInterest(inputPrefix);
}
bool Consumer_manager::sendInterest(std::string inputPrefix){
    try{
        Name interestName(inputPrefix);
        //interestName.appendVersion();
        Interest interest(interestName);
        interest.setCanBePrefix(true);
        interest.setMustBeFresh(true);
        interest.setInterestLifetime(20_s); // The default is 4 seconds
        std::cout << "Sending Interest " << interest << std::endl;
    m_face.expressInterest(interest,
                           bind(&Consumer_manager::onData, this,  _1, _2),
                           bind(&Consumer_manager::onNack, this, _1, _2),
                           bind(&Consumer_manager::onTimeout, this, _1));

    // processEvents will block until the requested data is received or a timeout occurs
    m_face.processEvents();}
        catch (const std::exception& e) {
          std::cerr << "ERROR Generating interest packet: " << e.what() << std::endl;
          return false;
        }
    return true;

}
//The value of this instance does not change before and after this function
void Consumer_manager::onData(const Interest& interest, const Data& data) const
{
    std::stringstream text;
    //Read Data
   text<<"Received Data: "<<data.getName()<<"\n";
    const auto& content = data.getContent();
    text<<"Data Content is "<<reinterpret_cast<const char*>(content.value());
//    Block content = data.getContent();
    //text << "Received data: " << data << "\nFrom Interest: "<< interest<<"\n Data Content is"<<data.getContent();
        this->ui->DataContainer->setText(QString::fromStdString(text.str()));
        Messager msg;
        msg.setMessage(QString::fromStdString(text.str()));
        msg.setWindowTitle("Received Data");
        msg.exec();
        ///////////////////////////////////////////
}

void Consumer_manager::onNack(const Interest& interest, const lp::Nack& nack) const
{
  std::cout << "Received Nack with reason " << nack.getReason() << std::endl;
  std::stringstream text;
  text << "Received nack: " << nack.getReason() <<"!";
      QMessageBox msg;
      msg.setText(QString::fromStdString(text.str()));
      msg.setWindowTitle("ERROR");
      msg.exec();
}

void Consumer_manager::onTimeout(const Interest& interest) const
{
  std::cout << "Timeout for " << interest << std::endl;
  std::stringstream text;
      text << "Timeout for " << interest<<"!";
      QMessageBox msg;
      msg.setText(QString::fromStdString(text.str()));
      msg.setWindowTitle("ERROR");
      msg.exec();
}

