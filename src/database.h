#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QString>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QVariant>
#include <QVariantList>
#include <QDateTime>
#include <QSharedPointer>
#include <QJsonObject>
#include <QList>

using HistoryRecord = std::tuple<QString, QString, QDateTime>;
using JsonObjectList = QList<QJsonObject>;
using IntList = QList<int>;

// Sqlite database
class Database : public QObject
{
	friend class QSharedPointer<Database>;

	Q_OBJECT

private:
	QSqlDatabase db_;

public:
	Database();
	virtual ~Database();

	Database(const Database&) = delete;
	Database(Database&&) = delete;
	Database& operator= (const Database&) = delete;
	Database& operator= (Database&&) = delete;

public:
	bool appendHistory(const QVariantList &list);
	bool removeHistory(int id);
	bool modifyHistory(int id, const QString &text);
	bool clearHistory(int cid);

	int appendContact(const QJsonObject &object);
	bool modifyContact(const QJsonObject &object);
	bool removeContact(const QJsonObject &object);
	bool contactExists(const QJsonObject &object) const;
	QString getPassword(const QString &login) const;
	bool searchContacts(QJsonObject &object, const QString &name, int cid);
	bool queryContact(QJsonObject &contact, const QString &login);
	bool queryContact(QJsonObject &contact, int cid);
	bool linkContact(const QJsonObject &object);
	bool unlinkContact(const QJsonObject &object);
	IntList queryLinks(int cid);

public:
	bool open();
	void close();
	bool isOpen() const { return db_.isOpen(); }

private:
	bool createTables();
};

#endif // DATABASE_H
