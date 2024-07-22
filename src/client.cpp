#include "client.h"
#include "log.h"
#include "database.h"
#include "dispatcher.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <algorithm>

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

ClientPtr ClientService::find(int id) const
{
	ClientList::const_iterator it = std::find_if(clients_.begin(), clients_.end(),
							 [id](const ClientPtr &client) {
		return client->id() == id;
	});

	if (it != clients_.end())
		return *it;
	return nullptr;
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
			checkNewHistory(client);
			checkModifiedHistory(client);
			checkRemovedHistory(client);
		}

		QThread::msleep(100);
	}
}

void ClientService::checkNewHistory(const ClientPtr& client)
{
	QVariantMap options;
	options["all"] = false;
	options["state"] = static_cast<int>(HistoryState::Regular);
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

void ClientService::checkModifiedHistory(const ClientPtr& client)
{
	QVariantMap options;
	options["all"] = false;
	options["state"] = static_cast<int>(HistoryState::Modified);
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
		root["action"] = static_cast<int>(Dispatcher::Action::ModifyHistory);
		emit messageReady(client, QJsonDocument(root).toJson(QJsonDocument::Compact));
		GetDatabase()->setReadHistory(client->id());
		LOG("Modify history, contact: " << client->login().toStdString());
	}
}

void ClientService::checkRemovedHistory(const ClientPtr& client)
{
	QVariantMap options;
	options["all"] = false;
	options["state"] = static_cast<int>(HistoryState::Removed);
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
		root["action"] = static_cast<int>(Dispatcher::Action::RemoveHistory);
		emit messageReady(client, QJsonDocument(root).toJson(QJsonDocument::Compact));
		GetDatabase()->setReadHistory(client->id());
		LOG("Remove history, contact: " << client->login().toStdString());
	}
}
