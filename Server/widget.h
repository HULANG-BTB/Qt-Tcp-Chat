#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "chatServer.h"

namespace Ui {
    class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    chatServer *server;

private slots:
    void slots_btnStart();
    void slot_clients(int users);
    void slot_message(QString msg);
    void on_btnSave_clicked();
};

#endif // WIDGET_H
