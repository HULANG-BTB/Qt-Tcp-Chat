#include <QWidget>
#include <QRegExp>
#include <QFile>
#include <QFileDialog>
#include "chatServer.h"


chatServer::chatServer(QObject *parent) : QTcpServer(parent)
{
}

void chatServer::incomingConnection(qintptr socketfd)
{
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketfd);
    clients.insert(client);

    emit clients_signal(clients.count());

    QString str;
    str = QString("New client from: %1")
            .arg(client->peerAddress().toString());

    emit message_signal(str);

    connect(client, SIGNAL(readyRead()), this,
                    SLOT(readyRead()));
    connect(client, SIGNAL(disconnected()), this,
                    SLOT(disconnected()));
}

void chatServer::readyRead()
{
    QTcpSocket *client = (QTcpSocket*)sender();
    while(client->canReadLine())
    {
        QString line = QString::fromUtf8(client->readLine()).trimmed();

        QString str;
        str = QString("Read line: %1").arg(line);

        emit message_signal(str);

        QRegExp meRegex("^/me:(.*)$");

        if(meRegex.indexIn(line) != -1)
        {
            QString user = meRegex.cap(1);
            users[client] = user;
            foreach(QTcpSocket *client, clients)
            {
                client->write(QString("Server: %1 connect")
                              .arg(user).toUtf8());
            }

            sendUserList();
        }
        else if(users.contains(client))
        {
            QString message = line;
            QString user = users[client];

            QString str;
            str = QString("User: %1, Message: %2").arg(user).arg(message);

            // Check File Header
            QRegExp meRegex("^file:(.*):([0-9]*)$");
            if ( meRegex.indexIn(line) != -1 ) {
                message = meRegex.cap(1);
                // Change Message
                str = QString("User: %1, File: %2").arg(user).arg(message);
                // Get File Info
                QString filename = meRegex.cap(1);
                qint64 fileSize = meRegex.cap(2).toInt(nullptr, 10);
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
                    data = client->read(blockSize);
                    recvSize += file.write(data);
                }
                file.close();
            }

            emit message_signal(str);

            foreach(QTcpSocket *otherClient, clients) {
                otherClient->write(QString(user+":"+message+"\n").toUtf8());
                if ( meRegex.indexIn(line) != -1 ) {
                    // Get File Info
                    QString filename = meRegex.cap(1);
                    qint64 fileSize = meRegex.cap(2).toInt(nullptr, 10);
                    qint64 sendSize = 0;
                    // Open File
                    QFile file;
                    file.setFileName(filename);
                    file.open(QIODevice::ReadOnly);
                    // Send File Header
                    otherClient->write(QString(line+"\n").toUtf8());
                    // Send File Data
                    while ( sendSize < fileSize ) {
                        qint64 blockSize = 2 * 1024;
                        blockSize = qMin(blockSize, fileSize - sendSize);
                        QByteArray data;
                        data.clear();
                        data = file.read(blockSize);
                        sendSize += otherClient->write(data);
                    }
                    file.close();
                }
            }
        }
        else
        {
            qWarning() << "Got bad message from client:"
                                << client->peerAddress().toString() << line;
        }
    }
}

void chatServer::disconnected()
{
    QTcpSocket *client = (QTcpSocket*)sender();

    QString str;
    str = QString("Client disconnected:: %1")
            .arg(client->peerAddress().toString());

    emit message_signal(str);

    clients.remove(client);

    emit clients_signal(clients.count());

    QString user = users[client];
    users.remove(client);

    sendUserList();
    foreach(QTcpSocket *client, clients)
        client->write(QString("Server: %1 disconnect").arg(user).toUtf8());

}

void chatServer::sendUserList()
{
    QStringList userList;
    foreach(QString user, users.values())
        userList << user;

    foreach(QTcpSocket *client, clients)
        client->write(QString("/users:" + userList.join(",") + "\n")
                       .toUtf8());
}
