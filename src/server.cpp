#include "server.h"
#include "log.h"
#include "settings.h"
#include "dispatcher.h"

#include <QFile>
#include <QDir>
#include <QSslKey>

Server::Server(bool secure)
	: server_(new QWebSocketServer("Maty Server",
		  secure == true ? QWebSocketServer::SecureMode : QWebSocketServer::NonSecureMode, this))
{
	if (secure)
		setSslConfiguration();
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
	connect(server_, &QWebSocketServer::sslErrors, this, &Server::processSslErrors);
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
	}
}

void Server::processSslErrors(const QList<QSslError>& errors)
{
	for (const QSslError& error : errors)
		LOGE(error.errorString().toStdString());
}

bool Server::setSslConfiguration()
{
	QSslConfiguration sslConfiguration;

	QFile certFile(GetSettings()->certPath() + QDir::separator() + kCertName);
	if (!certFile.open(QIODevice::ReadOnly))
	{
		LOGE("Can't open cert file!");
		return false;
	}

	QFile keyFile(GetSettings()->certPath() + QDir::separator() + kKeyName);
	if (!keyFile.open(QIODevice::ReadOnly))
	{
		LOGE("Can't open key file!");
		return false;
	}

	QSslCertificate certificate(&certFile, QSsl::Pem);
	sslConfiguration.setLocalCertificate(certificate);

	QSslKey privateKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
	sslConfiguration.setPrivateKey(privateKey);

	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	server_->setSslConfiguration(sslConfiguration);
	return true;
}

void Server::sendMessage(const QString &message)
{
}
