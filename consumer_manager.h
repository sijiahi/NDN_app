#ifndef CONSUMER_MANAGER_H
#define CONSUMER_MANAGER_H

#include <QWidget>
#include <ndn-cxx/face.hpp>
#include <iostream>
#include "messager.h"
namespace Ui {
class Consumer_manager;
}
using namespace ndn;

class Consumer_manager : public QWidget
{
    Q_OBJECT

public:
    explicit Consumer_manager(QWidget *parent = 0);
    ~Consumer_manager();

private slots:
   void on_AddPrefix_clicked();

private:
  bool sendInterest(std::string inputPrefix);
  void onData(const Interest& interest, const Data& data)const;
  void onNack(const Interest& interest, const lp::Nack& nack)const;
  void onTimeout(const Interest& interest)const;

private:
  Ui::Consumer_manager *ui;
  Face m_face;
};

#endif // CONSUMER_MANAGER_H
