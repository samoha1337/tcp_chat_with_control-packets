#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

#include <QVector>
#include <QTcpSocket>


class socket_manager
{
public:
    socket_manager();

    // Добавление сокета в список
    void addSocket(QTcpSocket *socket);

    // Установка никнейма для указанного сокета
    void setNickname(QTcpSocket *socket, const QString &nickname);

    // Удаление сокета из списка по никнейму или сокету
    void removeSocketByNickname(const QString &nickname);
    void removeSocketBySocket(QTcpSocket *socket);

    // Получение никнейма по сокету и наоборот
    QString getNicknameBySocket(QTcpSocket *socket) const;
    QTcpSocket *getSocketByNickname(const QString &nickname) const;

    // Получение всех никнеймов и всех сокетов
    QStringList getAllNicknames() const;
    QList <QTcpSocket *> getAllSockets() const;

    // Получение количества пользователей
    int getUserCount() const;

    // Удаление всех клиентов
    void clearClients();


    // Проверка, есть ли IP-адрес в списке забаненных
    bool isAddressBanned(const QString &address) const;

    // Добавление IP-адреса в список забаненных
    void addAddressToBanList(const QString &address);

private:

    QMap <QTcpSocket *, QString> _clientMap;

    QVector <QString> _bannedAddresses;
};

#endif // SOCKET_MANAGER_H
