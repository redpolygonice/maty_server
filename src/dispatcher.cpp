#include "dispatcher.h"
#include "log.h"

#include <QJsonDocument>
#include <QJsonObject>

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

void Dispatcher::processMessage(const QString &message)
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
	Action action = static_cast<Action>(rootObject["action"].toInt());
	if (action == Action::Registration)
	{
	}
}

void Dispatcher::sendMessage(const QString &message, const Client &client)
{
	client.socket()->sendTextMessage(message);
}
