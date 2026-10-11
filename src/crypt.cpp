#include "crypt.h"

#include <aes.h>

Crypt::Crypt() {}

void Crypt::encrypt(const QString &in, QByteArray &out)
{
	aes_context context;
	memset(&context, 0, sizeof(aes_context));

	QByteArray key = generateKey();
	QByteArray iv = generateIv();
	out.resize(in.size() + iv.size());
	memcpy(&out[in.size()], reinterpret_cast<void*>(iv.data()), iv.size());

	int iv_off = 0;
	aes_setkey_enc(&context, reinterpret_cast<unsigned char*>(key.data()), 256);
	aes_crypt_cfb128(&context, AES_ENCRYPT, in.size(), &iv_off,
					 reinterpret_cast<unsigned char*>(iv.data()),
					 reinterpret_cast<unsigned char*>(in.toLocal8Bit().data()),
					 reinterpret_cast<unsigned char*>(out.data()));
}

void Crypt::decrypt(const QByteArray &in, QString &out)
{
	aes_context context;
	memset(&context, 0, sizeof(aes_context));

	QByteArray key = generateKey();
	QByteArray iv(16, 0);
	int size = in.size() - iv.size();

	QByteArray encrypted(in);
	memcpy(reinterpret_cast<void*>(iv.data()), &encrypted[size], iv.size());
	encrypted.truncate(size);
	QByteArray decrypted(size, 0);

	int iv_off = 0;
	aes_setkey_enc(&context, reinterpret_cast<unsigned char*>(key.data()), 256);
	aes_crypt_cfb128(&context, AES_DECRYPT, size, &iv_off,
					 reinterpret_cast<unsigned char*>(iv.data()),
					 reinterpret_cast<unsigned char*>(encrypted.data()),
					 reinterpret_cast<unsigned char*>(decrypted.data()));

	out = decrypted;
}

QByteArray Crypt::encrypt(const QString &text)
{
	QByteArray data;
	encrypt(text, data);
	return data;
}

QString Crypt::decrypt(const QByteArray &data)
{
	QString text;
	decrypt(data, text);
	return text;
}

QByteArray Crypt::generateKey()
{
	QByteArray byteKey(32, 0);
	byteKey[0] = 28; byteKey[1] = 52; byteKey[2] = 17; byteKey[3] = 252;
	byteKey[4] = 35; byteKey[5] = 162; byteKey[6] = 19; byteKey[7] = 89;
	byteKey[8] = 129; byteKey[9] = 117; byteKey[10] = 82; byteKey[11] = 158;
	byteKey[12] = 11; byteKey[13] = 84; byteKey[14] = 168; byteKey[15] = 57;
	byteKey[16] = 21; byteKey[17] = 27; byteKey[18] = 24; byteKey[19] = 81;
	byteKey[20] = 176; byteKey[21] = 223; byteKey[22] = 35; byteKey[23] = 113;
	byteKey[24] = 94; byteKey[25] = 68; byteKey[26] = 165; byteKey[27] = 72;
	byteKey[28] = 118; byteKey[29] = 51; byteKey[30] = 83; byteKey[31] = 62;
	return byteKey;
}

QByteArray Crypt::generateIv()
{
	QByteArray byteIv(16, 0);
	srand(time(nullptr));
	for (int i = 0; i < byteIv.size(); ++i)
		byteIv[i] = rand() & 0xff;
	return byteIv;
}
