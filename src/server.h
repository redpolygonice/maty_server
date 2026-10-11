#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QString>
#include <QList>

class Server : public QObject
{
	Q_OBJECT

private:
	QWebSocketServer *server_;

public:
	Server(bool secure = false);
	~Server();

signals:
	void messageReceived(const QString &message, QWebSocket *socket);

private slots:
	void newConnection();
	void closed();
	void processTextMessage(const QString &message);
	void processBinaryMessage(const QByteArray &message);
	void socketDisconnected();
	void processSslErrors(const QList<QSslError>& errors);

private:
	bool setSslConfiguration();

public:
	bool start();
	void stop();
	void sendMessage(const QString &message);
};

#endif // SERVER_H
