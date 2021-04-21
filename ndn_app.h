#ifndef NDN_APP_H
#define NDN_APP_H

#include <QMainWindow>
#include <QMessageBox>
#include "consumer_manager.h"
#include "repo_putfile.h"
#include "repo_deletefile.h"
#include <QProcess>
#include <dirent.h>

namespace Ui {
class ndn_app;
}

class ndn_app : public QMainWindow
{
    Q_OBJECT

public:
    explicit ndn_app(QWidget *parent = 0);
    ~ndn_app();

private slots:

    void on_nfd_Manager_clicked();

    void on_consumer_app_clicked();

    void on_to_DB_clicked();

    void on_delete_DB_clicked();

    void on_chat_app_clicked();

    void on_getFiles_clicked();

private:
    Ui::ndn_app *ui;
    Consumer_manager *manager;
    QProcess *chatList;
};

#endif // NDN_APP_H
