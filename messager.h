#ifndef MESSAGER_H
#define MESSAGER_H

#include <QDialog>
#include <QString>

namespace Ui {
class Messager;
}

class Messager : public QDialog
{
    Q_OBJECT

public:
    explicit Messager(QWidget *parent = 0);
    ~Messager();
    void setMessage(QString);
    void addMessage(QString text);
private:
    Ui::Messager *ui;
};

#endif // MESSAGER_H
