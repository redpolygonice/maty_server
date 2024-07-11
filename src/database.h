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

using HistoryRecord = std::tuple<QString, QString, QDateTime>;

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
	QString getPassword(int cid) const;

public:
	bool open();
	void close();
	bool isOpen() const { return db_.isOpen(); }

private:
	bool createTables();
};

#endif // DATABASE_H
