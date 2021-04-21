#include "messager.h"
#include "ui_messager.h"

Messager::Messager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Messager)
{
    ui->setupUi(this);
}

Messager::~Messager()
{
    delete ui;
}
void Messager::setMessage(QString text){
      this->ui->textBrowser->setText(text);
      this->ui->textBrowser->update();
      this->update();
}
void Messager::addMessage(QString text){
      this->ui->textBrowser->append(text);
    this->ui->textBrowser->update();
    this->update();
}
