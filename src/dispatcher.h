#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "server.h"
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
		Message,
		Search,
		QueryContact,
		LinkContact,
		UnlinkContact,
		AddHistory,
		ModifyHistory,
		RemoveHistory,
		ClearHistory,
		NewHistory
	};

	enum class ErrorCode
	{
		Ok,
		Error,
		LoginExists,
		NoLogin,
		Password
	};

	enum class SearchResult
	{
		Found,
		NotFound
	};

private:
	Server server_;
	ClientService clientService_;

public:
	Dispatcher();
	~Dispatcher();

private slots:
	void processMessage(const QString &message, QWebSocket *socket);
	void sendMessage(const QString &message, const Client &client);

public:
	bool start();
	void stop();
	ClientService& clientService() { return clientService_; }

private:
	void actionRegistration(QJsonObject &object, QWebSocket *socket);
	void actionAuth(const QJsonObject &object, QWebSocket *socket);
	void actionSearch(const QJsonObject &object, QWebSocket *socket);
	void actionMessage(const QJsonObject &object, QWebSocket *socket);
	void actionLinkContact(const QJsonObject &object, QWebSocket *socket);
	void actionUnlinkContact(const QJsonObject &object, QWebSocket *socket);
	void actionQueryContact(const QJsonObject &object, QWebSocket *socket);

	void actionAddHistory(const QJsonObject &object, QWebSocket *socket);
	void actionModifyHistory(const QJsonObject &object, QWebSocket *socket);
	void actionRemoveHistory(const QJsonObject &object, QWebSocket *socket);
	void actionClearHistory(const QJsonObject &object, QWebSocket *socket);
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
