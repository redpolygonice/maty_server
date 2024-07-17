#include "dispatcher.h"
#include "log.h"
#include "database.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCryptographicHash>

Dispatcher::Dispatcher()
{
}

Dispatcher::~Dispatcher()
{
	stop();
}

bool Dispatcher::start()
{
	if (!GetDatabase()->open())
	{
		LOGE("Can't open database!");
		return false;
	}

	if (!server_.start())
		return false;

	connect(&server_, &Server::messageReceived, this, &Dispatcher::processMessage);
	clientService_.start();
	return true;
}

void Dispatcher::stop()
{
	server_.stop();
	GetDatabase()->close();
	clientService_.stop();
}

void Dispatcher::processMessage(const QString &message, QWebSocket *socket)
{
	LOG("Message received: " << message.toStdString());

	QJsonParseError error;
	QJsonDocument document = QJsonDocument::fromJson(message.toUtf8(), &error);
	if (error.error != QJsonParseError::NoError)
	{
		LOGE(error.errorString().toStdString());
		return;
	}

	QJsonObject rootObject = document.object();
	if (rootObject.empty())
		return;

	Action action = static_cast<Action>(rootObject["action"].toInt());
	if (action == Action::Registration)
		actionRegistration(rootObject, socket);
	else if (action == Action::Auth)
		actionAuth(rootObject, socket);
	else if (action == Action::Search)
		actionSearch(rootObject, socket);
	else if (action == Action::Message)
		actionMessage(rootObject, socket);
	else if (action == Action::QueryContact)
		actionQueryContact(rootObject, socket);
	else if (action == Action::LinkContact)
		actionLinkContact(rootObject, socket);
	else if (action == Action::UnlinkContact)
		actionUnlinkContact(rootObject, socket);
	else if (action == Action::AddHistory)
		actionAddHistory(rootObject, socket);
	else if (action == Action::ModifyHistory)
		actionModifyHistory(rootObject, socket);
	else if (action == Action::RemoveHistory)
		actionRemoveHistory(rootObject, socket);
	else if (action == Action::ClearHistory)
		actionClearHistory(rootObject, socket);
}

void Dispatcher::sendMessage(const QString &message, const Client &client)
{
	client.socket()->sendTextMessage(message);
}

void Dispatcher::actionRegistration(QJsonObject &object, QWebSocket *socket)
{
	QJsonObject contact;
	contact["action"] = static_cast<int>(Action::Registration);

	if (GetDatabase()->contactExists(object))
		contact["code"] = static_cast<int>(ErrorCode::LoginExists);
	else
	{
		object["password"] = QString(QCryptographicHash::hash(object["password"].toString().toLocal8Bit(),
									 QCryptographicHash::Sha256).toHex());
		contact["code"] = static_cast<int>(ErrorCode::Ok);
		contact["id"] = GetDatabase()->appendContact(object);
	}

	socket->sendTextMessage(QJsonDocument(contact).toJson(QJsonDocument::Compact));
}

void Dispatcher::actionAuth(const QJsonObject &object, QWebSocket *socket)
{
	QJsonObject contact;
	contact["action"] = static_cast<int>(Action::Auth);

	if (!GetDatabase()->contactExists(object))
		contact["code"] = static_cast<int>(ErrorCode::NoLogin);
	else if (QString(QCryptographicHash::hash(object["password"].toString().toLocal8Bit(),
											  QCryptographicHash::Sha256).toHex()) != GetDatabase()->queryPassword(object["login"].toString()))
		contact["code"] = static_cast<int>(ErrorCode::Password);
	else
	{
		contact["code"] = static_cast<int>(ErrorCode::Ok);

		if (object["querydata"].toBool())
		{
			contact["update"] = true;
			GetDatabase()->queryContact(contact, object["login"].toString());

			// Query contacts
			QJsonArray links;
			IntList rids = GetDatabase()->queryLinks(contact["id"].toInt());
			for (int rid : rids)
			{
				QJsonObject linkContact;
				if (GetDatabase()->queryContact(linkContact, rid))
					links.push_back(linkContact);
			}

			contact["links"] = links;

			// Query history
			QVariantMap options;
			options["all"] = true;
			options["cid"] = contact["id"].toInt();

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

				contact["history"] = historyArray;
				GetDatabase()->setReadHistory(contact["id"].toInt());
				LOG("Query history, contact: " << contact["id"].toInt());
			}
		}

		if (object["id"].toInt() == 0)
		{
			GetDatabase()->queryContact(contact, object["login"].toString());
			clientService_.add(contact["id"].toInt(), contact["login"].toString(), socket);
		}
		else
			clientService_.add(object["id"].toInt(), object["login"].toString(), socket);
	}

	socket->sendTextMessage(QJsonDocument(contact).toJson(QJsonDocument::Compact));
}

void Dispatcher::actionSearch(const QJsonObject &object, QWebSocket *socket)
{
	QJsonObject root;
	if (!GetDatabase()->searchContacts(root, object["text"].toString(), object["cid"].toInt()))
		root["searchResult"] = static_cast<int>(SearchResult::NotFound);
	else
		root["searchResult"] = static_cast<int>(SearchResult::Found);

	root["action"] = static_cast<int>(Action::Search);
	socket->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

void Dispatcher::actionMessage(const QJsonObject &object, QWebSocket *socket)
{
}

void Dispatcher::actionLinkContact(const QJsonObject& object, QWebSocket* socket)
{
	if (!GetDatabase()->linkContact(object))
		LOGW("Can't link contact!");
}

void Dispatcher::actionUnlinkContact(const QJsonObject& object, QWebSocket* socket)
{
	if (!GetDatabase()->unlinkContact(object))
		LOGW("Can't link contact!");
}

void Dispatcher::actionQueryContact(const QJsonObject& object, QWebSocket* socket)
{
	QJsonObject contact;
	if (!GetDatabase()->queryContact(contact, object["id"].toInt()))
	{
		LOGW("Can't query contact!");
		return;
	}

	QJsonObject root;
	root["contact"] = contact;
	root["action"] = static_cast<int>(Action::QueryContact);
	socket->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

void Dispatcher::actionAddHistory(const QJsonObject& object, QWebSocket* socket)
{
	if (!GetDatabase()->appendHistory(object))
		LOGW("Can't append history!");
}

void Dispatcher::actionModifyHistory(const QJsonObject& object, QWebSocket* socket)
{
	if (!GetDatabase()->modifyHistory(object))
		LOGW("Can't modify history!");
}

void Dispatcher::actionRemoveHistory(const QJsonObject& object, QWebSocket* socket)
{
	if (!GetDatabase()->removeHistory(object["id"].toInt()))
		LOGW("Can't remove history!");
}

void Dispatcher::actionClearHistory(const QJsonObject& object, QWebSocket* socket)
{
	if (!GetDatabase()->clearHistory(object["cid"].toInt()))
		LOGW("Can't clear history!");
}
