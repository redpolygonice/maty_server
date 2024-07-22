#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QString>
#include <QList>
#include <QSharedPointer>
#include <QThread>

class Client;
using WebSocketPtr = QSharedPointer<QWebSocket>;
using ClientPtr = QSharedPointer<Client>;
using ClientList = QList<ClientPtr>;

class Client
{
private:
	int id_;
	QString login_;
	WebSocketPtr socket_;

public:
	Client(int id, const QString &login, QWebSocket *socket);

public:
	int id() const { return id_; }
	QString login() const { return login_; }
	WebSocketPtr socket() const { return socket_; }
};

class ClientService : public QObject
{
	Q_OBJECT

private:
	std::atomic_bool active_;
	ClientList clients_;
	QThread *thread_;

public:
	ClientService();
	~ClientService();

signals:
	void messageReady(const ClientPtr &client, const QString &text);

private slots:
	void sendMessage(const ClientPtr &client, const QString &text);

public:
	void start();
	void stop();
	void add(int id, const QString &login, QWebSocket *socket) {
		clients_.push_back(QSharedPointer<Client>::create(id, login, socket)); }
	void add(const ClientPtr &client) { clients_.push_back(client); }
	ClientPtr find(int id) const;
	void remove(int id);
	void remove(const WebSocketPtr &socket);
	void remove(const QWebSocket *socket);

private:
	void run();
	void checkNewHistory(const ClientPtr &client);
	void checkModifiedHistory(const ClientPtr &client);
	void checkRemovedHistory(const ClientPtr &client);
};

#endif // CLIENT_H
