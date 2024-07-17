#include "client.h"
#include "log.h"
#include "database.h"
#include "dispatcher.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Client::Client(int id, const QString &login, QWebSocket *socket)
	: id_(id)
	, login_(login)
	, socket_(socket)
{
}

ClientService::ClientService()
	: active_(false)
	, thread_(nullptr)
{
}

ClientService::~ClientService()
{
	stop();
}

void ClientService::sendMessage(const ClientPtr& client, const QString& text)
{
	client->socket()->sendTextMessage(text);
}

void ClientService::start()
{
	active_ = true;
	connect(this, &ClientService::messageReady, this, &ClientService::sendMessage);
	thread_ = QThread::create([this]() { run(); });
	thread_->start();
}

void ClientService::stop()
{
	active_ = false;
	thread_->wait();
}

void ClientService::remove(int id)
{
	clients_.removeIf([id](const ClientPtr &client) {
		return client->id() == id;
	});
}

void ClientService::remove(const WebSocketPtr& socket)
{
	clients_.removeIf([socket](const ClientPtr &client) {
		return client->socket() == socket;
	});
}

void ClientService::remove(const QWebSocket* socket)
{
	clients_.removeIf([socket](const ClientPtr &client) {
		return client->socket().get() == socket;
	});
}

void ClientService::run()
{
	while (active_)
	{
		for (const ClientPtr &client : clients_)
		{
			checkHistory(client);
		}

		QThread::msleep(100);
	}
}

void ClientService::checkHistory(const ClientPtr& client)
{
	QVariantMap options;
	options["all"] = false;
	options["cid"] = client->id();

	VariantMapList historyList;
	if (GetDatabase()->queryHistory(historyList, options))
	{
		QJsonObject root;
		QJsonArray historyArray;
		for (const QVariantMap &data : historyList)
		{
			QJsonObject history = QJsonObject::fromVariantMap(data);
			historyArray.push_back(history);
		}

		root["history"] = historyArray;
		root["action"] = static_cast<int>(Dispatcher::Action::NewHistory);
		emit messageReady(client, QJsonDocument(root).toJson(QJsonDocument::Compact));
		GetDatabase()->setReadHistory(client->id());
		LOG("Update history, contact: " << client->login().toStdString());
	}
}
