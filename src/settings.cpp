#include "settings.h"
#include "log.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

Settings::Settings(QObject *parent)
	: QObject{parent}
{
	params_["port"] = 1978;
}

QString Settings::dataPath()
{
	QDir appDir = QCoreApplication::applicationDirPath();

#ifdef QT_DEBUG
	appDir.cdUp();
#endif

	QDir dataDir = appDir.path() + QDir::separator() + "data";
	if (!dataDir.exists())
		dataDir.mkpath(".");

	return dataDir.path();
}

bool Settings::load()
{
	return true;
}

bool Settings::save()
{
	return true;
}
