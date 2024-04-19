#include "socket_manager.h"

socket_manager::socket_manager()
{

}

void socket_manager::addSocket(QTcpSocket *socket)
{
    if (socket && !_clientMap.contains(socket)) {
        _clientMap.insert(socket, "");
    }
}

void socket_manager::setNickname(QTcpSocket *socket, const QString &nickname)
{
    if (socket && _clientMap.contains(socket)) {
        _clientMap[socket] = nickname;
    }
}

void socket_manager::removeSocketByNickname(const QString &nickname)
{
    QMapIterator<QTcpSocket *, QString> iter(_clientMap);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value() == nickname) {
            _clientMap.remove(iter.key());
            break;
        }
    }
}

void socket_manager::removeSocketBySocket(QTcpSocket *socket)
{
    _clientMap.remove(socket);
}

QString socket_manager::getNicknameBySocket(QTcpSocket *socket) const
{
    return _clientMap.value(socket);
}

QTcpSocket *socket_manager::getSocketByNickname(const QString &nickname) const
{
    QMapIterator<QTcpSocket *, QString> iter(_clientMap);
    while (iter.hasNext()) {
        iter.next();
        if (iter.value() == nickname) {
            return iter.key();
        }
    }
    return nullptr;
}

QStringList socket_manager::getAllNicknames() const
{
    return _clientMap.values();
}

QList<QTcpSocket *> socket_manager::getAllSockets() const
{
    return _clientMap.keys();
}

int socket_manager::getUserCount() const
{
    return _clientMap.size();
}

void socket_manager::clearClients()
{
    _clientMap.clear();
}

bool socket_manager::isAddressBanned(const QString &address) const
{
    return _bannedAddresses.contains(address);
}

void socket_manager::addAddressToBanList(const QString &address)
{
    if (!_bannedAddresses.contains(address)) {
        _bannedAddresses.append(address);
    }
}
