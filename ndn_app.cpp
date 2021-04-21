#include "ndn_app.h"
#include "ui_ndn_app.h"
#include "nfd_checker.h"
#include "repo_getfile.h"
//This is to check if the path exist


ndn_app::ndn_app(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ndn_app)
{
    ui->setupUi(this);

}

ndn_app::~ndn_app()
{
    delete ui;
}



void ndn_app::on_nfd_Manager_clicked()
{
    nfd_checker checker;
    checker.check();
    try{
        chatList = new QProcess(this);
        DIR *dir;
        std::string g_Path;
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
             error.setText(QString::fromStdString(std::string("Can not find chat app path from: ")+buffer));
             error.exec();
             return;
        }
        QString strCmd;
        strCmd = QString::fromStdString(g_Path+std::string("nfd-status-http-server"));
        chatList->start("bash", QStringList() <<"-c" << strCmd);
    }
    catch(const std::exception& e){
            std::cout<<"ERROR :"<<e.what()<<std::endl;
            QMessageBox errorAlert;
            errorAlert.setText(e.what());
            errorAlert.show();
    }
}

void ndn_app::on_consumer_app_clicked()
{
try{
    if (manager== NULL){
    manager = new Consumer_manager();
    }
    manager->show();}
    catch(const std::exception& e){
        std::cout<<"ERROR :"<<e.what()<<std::endl;
        QMessageBox errorAlert;
        errorAlert.setText(e.what());
        errorAlert.show();
    }
}

void ndn_app::on_to_DB_clicked()
{
    try {

       repo::putfile();
       //repo::putfile();
    }
    catch (const std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
     }
}

void ndn_app::on_delete_DB_clicked()
{
    try {
       repo::deletefile();
       //repo::putfile();
    }
    catch (const std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
     }
}

void ndn_app::on_chat_app_clicked()
{
    try{
        chatList = new QProcess(this);
        DIR *dir;
        std::string g_Path;
        if ((dir=opendir(std::string("./chat-Release/").c_str())) != NULL)
         {
            g_Path = "./chat-Release/";
        }
        else if((dir=opendir(std::string("../chat-Release/").c_str())) != NULL){
            g_Path = "../chat-Release/";
        }
        if (g_Path == "")
         {
             QMessageBox error;
             char *buffer = getcwd(NULL, 0);
             error.setText(QString::fromStdString(std::string("Can not find chat app path from: ")+buffer));
             error.exec();
             return;
        }
        QString strCmd;
        strCmd = QString::fromStdString(g_Path+std::string("app"));
        chatList->start("bash", QStringList() <<"-c" << strCmd);
    }
    catch(const std::exception& e){
            std::cout<<"ERROR :"<<e.what()<<std::endl;
            QMessageBox errorAlert;
            errorAlert.setText(e.what());
            errorAlert.show();
    }
}

void ndn_app::on_getFiles_clicked()
{
    try {
       repo::getfile();
       //repo::putfile();
    }
    catch (const std::exception& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
     }
}
