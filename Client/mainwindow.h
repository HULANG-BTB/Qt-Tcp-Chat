#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QTcpSocket>
#include <QRegExp>
#include <QStackedWidget>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void on_loginButton_clicked();
    void on_sayButton_clicked();
    void readyRead();
    void connected();

    void on_saveButton_clicked();

    void on_fileButton_clicked();

private:
    QTcpSocket *socket;


};

#endif // MAINWINDOW_H
