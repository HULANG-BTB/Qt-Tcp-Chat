#include "widget.h"
#include "chatServer.h"
#include "ui_widget.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->btnStart, SIGNAL(clicked()), this,
                          SLOT(slots_btnStart()));

    server = new chatServer();
    connect(server, SIGNAL(clients_signal(int)), this,
                    SLOT(slot_clients(int)));
    connect(server, SIGNAL(message_signal(QString)), this,
                    SLOT(slot_message(QString)));
}

void Widget::slots_btnStart()
{
    bool success = server->listen(QHostAddress::Any, ui->lineEdit->text().toInt());

    if(success){
        ui->textEdit->append("Chat Server start");

    }else{
        ui->textEdit->append("Chat Server error");
    }
}

void Widget::slot_clients(int users)
{
    QString str = QString("Clients : %1").arg(users);
    ui->label->setText(str);
}

void Widget::slot_message(QString msg)
{
    ui->textEdit->append(msg);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_btnSave_clicked()
{
    if(ui->textEdit->document()->isEmpty())
    {
        QMessageBox::warning(0, tr("warning"), tr("ChatMessage is empty!"), QMessageBox::Ok);
    }
    else
    {
        QFile log_file("ChatMessageLog.txt");
        if(log_file.open(QFile::ReadWrite | QFile::Truncate))
        {
            QTextStream out(&log_file);
            out<<ui->textEdit->toPlainText();
            QMessageBox::warning(0, tr("tip"), tr("Chat message save in file: ChatMessage.txt."), QMessageBox::Ok);
        }
    }
}
