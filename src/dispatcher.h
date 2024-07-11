#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "server.h"
#include "database.h"
#include "client.h"

#include <QObject>

class Dispatcher : public QObject
{
	Q_OBJECT

public:
	enum class Action
	{
		None,
		Registration,
		Auth,
		Message
	};

	enum class ErrorCode
	{
		Ok,
		Error,
		LoginExists,
		NoLogin,
		Password
	};

private:
	Server server_;
	Database db_;
	Clients clients_;

public:
	Dispatcher();
	~Dispatcher();

private slots:
	void processMessage(const QString &message, QWebSocket *socket);
	void sendMessage(const QString &message, const Client &client);

public:
	bool start();
	void stop();

private:
	void actionRegistration(const QJsonObject &object, QWebSocket *socket);
	void actionAuth(const QJsonObject &object, QWebSocket *socket);
};

using DispatcherPtr = QSharedPointer<Dispatcher>;

inline DispatcherPtr GetDispatcher()
{
	static DispatcherPtr dispatcher = nullptr;
	if (dispatcher == nullptr)
		dispatcher = QSharedPointer<Dispatcher>::create();
	return dispatcher;
}

#endif // DISPATCHER_H
