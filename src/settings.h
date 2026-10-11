#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QVariantMap>

constexpr char kCertName[] = "server.crt";
constexpr char kKeyName[] = "server.key";

class Settings : public QObject
{
	friend class QSharedPointer<Settings>;

	Q_OBJECT

private:
	const QString settingsFile_ = "settings.json";
	QVariantMap params_;

private:
	explicit Settings(QObject *parent = nullptr);

public:
	QString logPath() const;
	bool load();
	bool save();

	QVariantMap &params() { return params_; }
	void setParams(const QVariantMap &map) { params_ = map; }

	static QString dataPath();
	static QString certPath();
};

using SettingsPtr = QSharedPointer<Settings>;

inline SettingsPtr GetSettings()
{
	static SettingsPtr settings = nullptr;
	if (settings == nullptr)
		settings = QSharedPointer<Settings>::create();
	return settings;
}

#endif // SETTINGS_H
