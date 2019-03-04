#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(readyRead()), this,
                    SLOT(readyRead()));

    connect(socket, SIGNAL(connected()), this,
                    SLOT(connected()));
}

void MainWindow::on_loginButton_clicked()
{
    socket->connectToHost(ui->serverLineEdit->text(), ui->portlineEdit->text().toInt());
}

void MainWindow::on_sayButton_clicked()
{
    QString message = ui->sayLineEdit->text().trimmed();

    if(!message.isEmpty())
    {
        socket->write(QString(message + "\n").toUtf8());
    }

    ui->sayLineEdit->clear();
    ui->sayLineEdit->setFocus();
}

void MainWindow::readyRead()
{
    while(socket->canReadLine())
    {
        QString line = QString::fromUtf8(socket->readLine()).trimmed();

        QRegExp messageRegex("^([^:]+):(.*)$");
        QRegExp fileRegex("^file:(.*):([0-9]*)$");

        if (fileRegex.indexIn(line) != -1) {
            QString filename = fileRegex.cap(1).toUtf8();
            qint64 fileSize = fileRegex.cap(2).toInt(nullptr, 10);
            qint64 recvSize = 0;
            // Open File
            QFile file;
            file.setFileName(filename);
            file.open(QIODevice::WriteOnly);
            // Read And Save
            while ( recvSize < fileSize ) {
                qint64 blockSize = 2 * 1024;
                blockSize = qMin(blockSize, fileSize - recvSize);
                QByteArray data;
                data.clear();
                data = socket->read(blockSize);
                recvSize += file.write(data);
            }
            file.close();
            ui->roomTextEdit->append("<b>收到文件：</b>: " + filename);
        }
        else if(messageRegex.indexIn(line) != -1)
        {
            QString user = messageRegex.cap(1);
            QString message = messageRegex.cap(2);

            ui->roomTextEdit->append("<b>"+user+"</b>: "+message);
        }
    }
}

void MainWindow::connected()
{
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
    socket->write(QString("/me:"+ui->userLineEdit->text()
                                +"\n").toUtf8());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_saveButton_clicked()
{
    if(ui->roomTextEdit->document()->isEmpty())
    {
        QMessageBox::warning(0, tr("warning"), tr("ChatMessage is empty!"), QMessageBox::Ok);
    }
    else
    {
        QFile log_file("ChatMessageLog.txt");
        if(log_file.open(QFile::ReadWrite | QFile::Truncate))
        {
            QTextStream out(&log_file);
            out<<ui->roomTextEdit->toPlainText();
            QMessageBox::warning(0, tr("tip"), tr("Chat message save in file: ChatMessage.txt."), QMessageBox::Ok);
        }
    }
}

void MainWindow::on_fileButton_clicked()
{
    //初始化对话框
    QFileDialog *fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(tr("选择文件"));
    //设置默认文件路径
    fileDialog->setDirectory(".");
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QString fileName;
    if(fileDialog->exec())
    {
        fileName = fileDialog->selectedFiles()[0];

        // Open File
        QFile file;
        file.setFileName(fileName);
        bool fileStatus = file.open(QIODevice::ReadOnly);
        if ( fileStatus == false ) {
            QMessageBox::information(NULL, "提示", "文件：" + fileName + "打开失败！", QMessageBox::Yes, QMessageBox::Yes);
            file.close();
            return;
        }


        // Get File Info
        qint64 fileSize = file.size();
        qint64 sendSize = 0;
        QFileInfo Info(file.fileName());

        // Send File Header
        QString HeadInfo = QString("file:%1:%2\n").arg(Info.fileName()).arg(fileSize);
        socket->write(HeadInfo.toUtf8());

        // Read and send File Data
        while ( sendSize < fileSize ) {
            qint64 blockSize = 2 * 1024;
            blockSize = qMin(blockSize, fileSize - sendSize);
            QByteArray data;
            data.clear();
            data = file.read(blockSize);
            sendSize += socket->write(data);
        }

        // Close File
        file.close();
    }
    delete fileDialog;
}
