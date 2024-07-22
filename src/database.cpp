#include "database.h"
#include "dbnames.h"
#include "settings.h"
#include "log.h"

#include <QVariant>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>

Database::Database()
{
}

Database::~Database()
{
	close();
}

bool Database::open()
{
	// Create database path
	QDir dbDir = Settings::dataPath() + QDir::separator() + "db";
	if (!dbDir.exists())
		dbDir.mkpath(".");

	// Open or create database
	QString dbFile = dbDir.absolutePath() + QDir::separator() + kDbName;
	db_ = QSqlDatabase::addDatabase("QSQLITE");
	db_.setHostName(kDbHostName);
	db_.setDatabaseName(dbFile);

	if (db_.open())
	{
		if (db_.tables().empty())
			return createTables();
	}
	else
		return false;

	return true;
}

bool Database::createTables()
{
	QSqlQuery query(db_);
	if (!query.exec("CREATE TABLE " + QString(kHistoryName) + " ("
					"id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"hid INTEGER KEY NOT NULL, " // Id from cliemt history
					"cid INTEGER KEY NOT NULL, " // Sender id
					"rid INTEGER KEY NOT NULL, " // Receyver id
					"text TEXT NOT NULL,"
					"read BOOLEAN,"
					"state INTEGER,"
					"ts TIMESTAMP NOT NULL)"))
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	if (!query.exec("CREATE TABLE " + QString(kContactsName) + " ("
					"id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"name VARCHAR(50) NOT NULL,"
					"login VARCHAR(50) NOT NULL,"
					"password VARCHAR(50) NOT NULL,"
					"image TEXT,"
					"phone VARCHAR(20),"
					"about VARCHAR(250),"
					"ts TIMESTAMP NOT NULL)"))
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	if (!query.exec("CREATE TABLE " + QString(kLinkContactsName) + " ("
					"id INTEGER PRIMARY KEY AUTOINCREMENT, "
					"cid INTEGER KEY NOT NULL, "
					"rid INTEGER KEY NOT NULL, "
					"approved BOOLEAN,"
					"ts TIMESTAMP NOT NULL)"))
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

void Database::close()
{
	db_.close();
}

bool Database::appendHistory(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("INSERT INTO " + QString(kHistoryName) + " (hid, cid, rid, text, read, state, ts)"
														   " VALUES (:hid, :cid, :rid, :text, :read, :state, :ts)");

	query.bindValue(":hid", object["hid"].toInt());
	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid",  object["rid"].toInt());
	query.bindValue(":text", object["text"].toString());
	query.bindValue(":read", false);
	query.bindValue(":state", static_cast<int>(HistoryState::Regular));
	query.bindValue(":ts", QVariant(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")));

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::modifyHistory(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("UPDATE " + QString(kHistoryName) + " SET text = :text, state = :state, read = :read"
													  " WHERE hid = :hid AND cid = :cid AND rid = :rid");
	query.bindValue(":hid", object["hid"].toInt());
	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid", object["rid"].toInt());
	query.bindValue(":read", false);
	query.bindValue(":text", object["text"].toString());
	query.bindValue(":state", static_cast<int>(HistoryState::Modified));

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::modifyRemoveHistory(const QJsonObject& object)
{
	QSqlQuery query(db_);
	query.prepare("UPDATE " + QString(kHistoryName) + " SET state = :state, read = :read"
													  " WHERE hid = :hid AND cid = :cid AND rid = :rid");
	query.bindValue(":hid", object["hid"].toInt());
	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid", object["rid"].toInt());
	query.bindValue(":read", false);
	query.bindValue(":state", static_cast<int>(HistoryState::Removed));

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::removeHistory(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("DELETE FROM " + QString(kHistoryName) + " WHERE hid = :hid AND cid = :cid AND rid = :rid");
	query.bindValue(":hid", object["hid"].toInt());
	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid", object["rid"].toInt());

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::clearHistory(int cid)
{
	QSqlQuery query(db_);
	query.prepare("DELETE FROM " + QString(kHistoryName) + " WHERE cid = :cid OR rid = :cid");
	query.bindValue(":cid", cid);

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::queryHistory(VariantMapList& mapList, const QVariantMap &options)
{
	mapList.clear();
	QSqlQuery query(db_);

	if (options["all"].toBool())
		query.prepare("SELECT * FROM " + QString(kHistoryName) +
					  " WHERE (rid = " + QString::number(options["cid"].toInt()) +
					  " OR cid = " + QString::number(options["cid"].toInt()) +
					  ") AND state != " + QString::number(static_cast<int>(HistoryState::Removed)));

	else if (options["state"].toInt() == static_cast<int>(HistoryState::Modified))
		query.prepare("SELECT * FROM " + QString(kHistoryName) +
					  " WHERE read IS FALSE"
					  " AND state = " + QString::number(static_cast<int>(HistoryState::Modified)) +
					  " AND rid = " + QString::number(options["cid"].toInt()));

	else if (options["state"].toInt() == static_cast<int>(HistoryState::Removed))
		query.prepare("SELECT * FROM " + QString(kHistoryName) +
					  " WHERE read IS FALSE"
					  " AND state = " + QString::number(static_cast<int>(HistoryState::Removed)) +
					  " AND rid = " + QString::number(options["cid"].toInt()));

	else
		query.prepare("SELECT * FROM " + QString(kHistoryName) +
					  " WHERE read IS FALSE"
					  " AND state = " + QString::number(static_cast<int>(HistoryState::Regular)) +
					  " AND rid = " + QString::number(options["cid"].toInt()));

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	while (query.next())
	{
		QVariantMap history;
		history["hid"] = query.value("id").toInt();
		history["cid"] = query.value("cid").toInt();
		history["rid"] = query.value("rid").toInt();
		history["text"] = query.value("text").toString();
		history["read"] = query.value("read").toBool();
		history["state"] = query.value("state").toInt();
		history["ts"] = query.value("ts").toDateTime();
		mapList.push_back(history);
	}

	return mapList.size() > 0;
}

bool Database::setReadHistory(int cid)
{
	QSqlQuery query(db_);
	query.prepare("UPDATE " + QString(kHistoryName) + " SET read = 1 WHERE rid = :rid OR cid = :rid");
	query.bindValue(":rid", cid);

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

int Database::appendContact(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("INSERT INTO " + QString(kContactsName) + " (name, login, password, image, phone, ts)"
															" VALUES (:name, :login, :password, :image, :phone, :ts)");

	query.bindValue(":name", object["name"].toString());
	query.bindValue(":login", object["login"].toString());
	query.bindValue(":password", object["password"].toString());
	query.bindValue(":image", object["image"].toString());
	query.bindValue(":phone", object["phone"].toString());
	query.bindValue(":ts", QVariant(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")));

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return 0;
	}

	// Query contact id
	query.prepare("SELECT id FROM " + QString(kContactsName));
	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return 0;
	}

	query.last();
	return query.value(0).toInt();
}

bool Database::modifyContact(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("UPDATE " + QString(kContactsName) + " SET name = :name, login = :login,"
													   " password = :password. image = :image, "
													   "phone = :phone WHERE id = :id");

	query.bindValue(":id", object["id"].toInt());
	query.bindValue(":name", object["name"].toString());
	query.bindValue(":login", object["login"].toString());
	query.bindValue(":password", object["password"].toString());
	query.bindValue(":image", object["image"].toString());
	query.bindValue(":phone", object["phone"].toString());

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::removeContact(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("DELETE FROM " + QString(kContactsName) + " WHERE id = :id");
	query.bindValue(":id", object["id"].toInt());

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::contactExists(const QJsonObject &object) const
{
	QSqlQuery query(db_);
	query.prepare("SELECT login FROM " + QString(kContactsName) + " WHERE login = :login");
	query.bindValue(":login", object["login"].toString());

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return true;
	}

	return query.next();
}

QString Database::queryPassword(const QString &login) const
{
	QSqlQuery query(db_);
	query.prepare("SELECT password FROM " + QString(kContactsName) + " WHERE login = :login");
	query.bindValue(":login", login);

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return "";
	}

	if (!query.next())
		return "";

	query.first();
	return query.value(0).toString();
}

bool Database::searchContacts(QJsonObject &object, const QString &name, int cid)
{
	QSqlQuery query(db_);
	query.prepare("SELECT * FROM " + QString(kContactsName) + " WHERE login LIKE '%" + name + "%'");

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	QJsonArray array;
	while (query.next())
	{
		// My contact
		if (query.value("id").toInt() == cid)
			continue;

		QJsonObject contact;
		contact["id"] = query.value("id").toInt();
		contact["name"] = query.value("name").toString();
		contact["login"] = query.value("login").toString();
		contact["image"] = query.value("image").toString();
		contact["phone"] = query.value("phone").toString();
		array.push_back(contact);
	}

	object["contacts"] = array;
	return array.size() > 0;
}

bool Database::queryContact(QJsonObject &contact, const QString &login)
{
	QSqlQuery query(db_);
	query.prepare("SELECT * FROM " + QString(kContactsName) + " WHERE login = :login");
	query.bindValue(":login", login);

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return true;
	}

	if (!query.next())
		return false;

	contact["id"] = query.value("id").toInt();
	contact["name"] = query.value("name").toString();
	contact["login"] = query.value("login").toString();
	contact["image"] = query.value("image").toString();
	contact["phone"] = query.value("phone").toString();

	// Links
	query.prepare("SELECT * FROM " + QString(kLinkContactsName) + " WHERE cid = :cid");
	query.bindValue(":cid", contact["id"].toInt());
	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return true;
	}

	if (!query.next())
		return true;

	QJsonArray links;
	while (query.next())
	{
		QJsonObject link;
		link["cid"] = query.value("cid").toInt();
		link["rid"] = query.value("rid").toInt();
		links.push_back(link);
	}

	contact["links"] = links;
	return true;
}

bool Database::queryContact(QJsonObject& contact, int id)
{
	QSqlQuery query(db_);
	query.prepare("SELECT * FROM " + QString(kContactsName) + " WHERE id = :id");
	query.bindValue(":id", id);

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return true;
	}

	if (!query.next())
		return false;

	contact["id"] = query.value("id").toInt();
	contact["name"] = query.value("name").toString();
	contact["login"] = query.value("login").toString();
	contact["image"] = query.value("image").toString();
	contact["phone"] = query.value("phone").toString();
	return true;
}

bool Database::linkExists(const QJsonObject& object)
{
	QSqlQuery query(db_);
	query.prepare("SELECT rid FROM " + QString(kLinkContactsName) + " WHERE cid = :cid AND rid = :rid");
	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid", object["rid"].toInt());

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return query.next();
}

bool Database::linkContact(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("INSERT INTO " + QString(kLinkContactsName) + " (cid, rid, approved, ts)"
																" VALUES (:cid, :rid, :approved, :ts)");

	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid", object["rid"].toInt());
	query.bindValue(":approved", object["rapprovedid"].toBool());
	query.bindValue(":ts", QVariant(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss")));

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

bool Database::unlinkContact(const QJsonObject &object)
{
	QSqlQuery query(db_);
	query.prepare("DELETE FROM " + QString(kLinkContactsName) + " WHERE cid = :cid AND rid = :rid");
	query.bindValue(":cid", object["cid"].toInt());
	query.bindValue(":rid", object["rid"].toInt());

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return false;
	}

	return true;
}

IntList Database::queryLinks(int cid)
{
	IntList rids;
	QSqlQuery query(db_);
	query.prepare("SELECT rid FROM " + QString(kLinkContactsName) + " WHERE cid = :cid");
	query.bindValue(":cid", cid);

	if (!query.exec())
	{
		LOGE(query.lastError().text().toStdString());
		return rids;
	}

	while (query.next())
		rids.push_back(query.value("rid").toInt());

	return rids;
}
