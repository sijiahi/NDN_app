#include "ndn_app.h"
#include <QApplication>
#include "consumer.cpp"
#include "consumer_manager.h"
//#include "ndnputfile.cpp"
#include "repo_putfile.h"
#include "repo_putfile.cpp"
int main(int argc, char *argv[])
{
    //chatgo(argc,argv);
    //consumerGo(argc,argv);
    QApplication a(argc, argv);
    ndn_app w;
    w.show();
    //mainrepo(0, );
//    qDebug() << file_name;
    return a.exec();
}
