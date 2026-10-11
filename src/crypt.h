#ifndef CRYPT_H
#define CRYPT_H

#include <QByteArray>
#include <QString>

class Crypt
{
public:
	Crypt();

public:
	void encrypt(const QString &in, QByteArray &out);
	void decrypt(const QByteArray &in, QString &out);
	QByteArray encrypt(const QString &text);
	QString decrypt(const QByteArray &data);

private:
	QByteArray generateKey();
	QByteArray generateIv();
};

#endif // CRYPT_H
