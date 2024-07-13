#include "dispatcher.h"
#include "log.h"

#include <QJsonDocument>
#include <QJsonObject>
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
	if (!db_.open())
	{
		LOGE("Can't open database!");
		return false;
	}

	if (!server_.start())
		return false;

	connect(&server_, &Server::messageReceived, this, &Dispatcher::processMessage);
	return true;
}

void Dispatcher::stop()
{
	server_.stop();
	db_.close();
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
}

void Dispatcher::sendMessage(const QString &message, const Client &client)
{
	client.socket()->sendTextMessage(message);
}

void Dispatcher::actionRegistration(QJsonObject &object, QWebSocket *socket)
{
	QJsonObject contact;
	contact["action"] = static_cast<int>(Action::Registration);

	if (db_.contactExists(object))
		contact["code"] = static_cast<int>(ErrorCode::LoginExists);
	else
	{
		object["password"] = QString(QCryptographicHash::hash(object["password"].toString().toLocal8Bit(),
									 QCryptographicHash::Sha256).toHex());
		contact["code"] = static_cast<int>(ErrorCode::Ok);
		contact["cid"] = db_.appendContact(object);
	}

	socket->sendTextMessage(QJsonDocument(contact).toJson(QJsonDocument::Compact));
}

void Dispatcher::actionAuth(const QJsonObject &object, QWebSocket *socket)
{
	QJsonObject contact;
	contact["action"] = static_cast<int>(Action::Auth);

	if (!db_.contactExists(object))
		contact["code"] = static_cast<int>(ErrorCode::NoLogin);
	else if (QString(QCryptographicHash::hash(object["password"].toString().toLocal8Bit(),
					QCryptographicHash::Sha256).toHex()) != db_.getPassword(object["cid"].toInt()))
		contact["code"] = static_cast<int>(ErrorCode::Password);
	else
		contact["code"] = static_cast<int>(ErrorCode::Ok);

	socket->sendTextMessage(QJsonDocument(contact).toJson(QJsonDocument::Compact));
}

void Dispatcher::actionSearch(const QJsonObject &object, QWebSocket *socket)
{
	QJsonObject root;
	if (!db_.searchContacts(root, object["text"].toString()))
		root["searchResult"] = static_cast<int>(SearchResult::NotFound);
	else
		root["searchResult"] = static_cast<int>(SearchResult::Found);

	root["action"] = static_cast<int>(Action::Search);
	socket->sendTextMessage(QJsonDocument(root).toJson(QJsonDocument::Compact));
}
