#include "message_sender.h"

message_sender::message_sender()
{
    nextBlockSize = 0;
}

void message_sender::sendMessage(QTcpSocket *socket, const QString &message)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState) {

        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_12);

        out << quint16(0) << message.toUtf8();
        out.device()->seek(0);
        out << quint16(unsigned(data.size()) - sizeof(quint16));

        socket->write(data);
    }
}
//просто отправка сообщения
void message_sender::sendMessageToClients(const QVector<QTcpSocket *> &clients, const QString &message)
{
    for (auto *client : clients) {
        sendMessage(client, message);
    }
}
//ф-ция для отправки ника всем, кроме себя
void message_sender::sendAllNicksToNewConnectedSocket(const QStringList allNicks, QTcpSocket *newSocket)
{
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_12);

    QString allNicksStr;

    for (auto nick : allNicks) {
        allNicksStr += nick + "/#@:*&125";
    }

    out << quint16(0) << allNicksStr.toUtf8();
    out.device()->seek(0);
    out << quint16(unsigned(data.size()) - sizeof(quint16));;

    newSocket->write(data);
}
//ф-ция для отправки сообщения всем, кроме себя
void message_sender::sendMessageNotToSomeone(QVector<QTcpSocket *> allSockets, QByteArray message, QTcpSocket *socketNotTo)
{
    for (auto sendSocket : allSockets) {
        if (sendSocket != socketNotTo)
            sendMessage(sendSocket, message);
    }
}
//ф-ция для прочитывания сообщения
QByteArray message_sender::readMessage(QTcpSocket *socket)
{
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_12);

    if (in.status() == QDataStream::Ok) {
        while (socket->bytesAvailable() > 0) {
            if (nextBlockSize == 0) {
                if (socket->bytesAvailable() < 2) {
                    break;
                } else {
                    in >> nextBlockSize;
                    if (nextBlockSize >= MAX_BYTES) {
                        nextBlockSize = 0;

                        socket->disconnectFromHost();
                        return QString("delete").toUtf8();
                    }
                }
            }

            if (socket->bytesAvailable() < nextBlockSize) {

                break;
            }

            else {
                QByteArray msg;
                in >> msg;
                nextBlockSize = 0;

                return msg;
            }
        }
    }

    return QString("error").toUtf8();
}
