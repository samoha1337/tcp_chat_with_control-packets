#ifndef MESSAGE_SENDER_H
#define MESSAGE_SENDER_H

#include <QTcpSocket>
#include <QDataStream>

const qint16 MAX_BYTES = 16000;

class message_sender
{
public:
    message_sender();

    // Отправить сообщение по указанному сокету
    void sendMessage(QTcpSocket *socket, const QString &message);

    // Отправить сообщение всем клиентам в списке
    void sendMessageToClients(const QVector<QTcpSocket *> &clients, const QString &message);

    void sendAllNicksToNewConnectedSocket(const QStringList allNicks, QTcpSocket *newSocket);

    void sendMessageNotToSomeone(QVector<QTcpSocket *> allSockets, QByteArray message, QTcpSocket *socketNotTo);//отправка сообщения всем кроме...


    QByteArray readMessage(QTcpSocket *socket);

private:
    qint16 nextBlockSize;

};

#endif // MESSAGE_SENDER_H
