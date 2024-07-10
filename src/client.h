#ifndef CLIENT_H
#define CLIENT_H

#include <QWebSocket>
#include <QString>
#include <QList>
#include <QSharedPointer>

using WebSocketPtr = QSharedPointer<QWebSocket>;

class Client
{
private:
	int id_;
	WebSocketPtr socket_;

public:
	Client(int id, QWebSocket *socket);

public:
	int id() const { return id_; }
	WebSocketPtr socket() const { return socket_; }
};

using Clients = QList<Client>;

#endif // CLIENT_H
