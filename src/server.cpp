#include "server.h"
#include "log.h"
#include "settings.h"
#include "dispatcher.h"

Server::Server()
	: server_(new QWebSocketServer("Maty Server", QWebSocketServer::NonSecureMode, this))
{
}

Server::~Server()
{
	stop();
}

bool Server::start()
{
	int port = GetSettings()->params()["port"].toInt();
	if (!server_->listen(QHostAddress::Any, port))
	{
		LOGE("Can't start WebSocket Server!");
		return false;
	}

	connect(server_, &QWebSocketServer::newConnection, this, &Server::newConnection);
	connect(server_, &QWebSocketServer::closed, this, &Server::closed);
	return true;
}

void Server::stop()
{
	server_->close();
}

void Server::newConnection()
{
	QWebSocket *socket = server_->nextPendingConnection();
	connect(socket, &QWebSocket::textMessageReceived, this, &Server::processTextMessage);
	connect(socket, &QWebSocket::binaryMessageReceived, this, &Server::processBinaryMessage);
	connect(socket, &QWebSocket::disconnected, this, &Server::socketDisconnected);
}

void Server::closed()
{
}

void Server::processTextMessage(const QString &message)
{
	QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
	emit messageReceived(message, socket);
}

void Server::processBinaryMessage(const QByteArray &message)
{
}

void Server::socketDisconnected()
{
	QWebSocket *socket = qobject_cast<QWebSocket *>(sender());
	if (socket)
	{
		GetDispatcher()->clientService().remove(socket);
		//socket->deleteLater();
	}
}

void Server::sendMessage(const QString &message)
{
}
