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

	enum class Code
	{
		Ok,
		Error
	};

private:
	Server server_;
	Database db_;
	Clients clients_;

public:
	Dispatcher();
	~Dispatcher();

private slots:
	void processMessage(const QString &message);
	void sendMessage(const QString &message, const Client &client);

public:
	bool start();
	void stop();
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
