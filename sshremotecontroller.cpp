#include "sshremotecontroller.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>

bool SshRemoteController::sSlaveLoaded = false;
QByteArray SshRemoteController::sSlaveScript;
QByteArray SshRemoteController::sSlaveMd5;

SshRemoteController::SshRemoteController()
	: mSsh(0)
{
	if (!sSlaveLoaded)
	{
		QFile f(SSH_SLAVE_FILE);
		f.open(QFile::ReadOnly);
		sSlaveScript = f.readAll();

		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(sSlaveScript);
		sSlaveMd5 = hash.result().toHex().toLower();

		sSlaveLoaded = true;
	}
}

void SshRemoteController::attach(SshConnection* connection)
{
	mSsh = connection;
	QByteArray remoteMd5 = mSsh->execute("if [ ! -d ~/.remoted ]; then mkdir ~/.remoted; fi; if [ -e ~/.remoted/slave.py ]; then md5sum ~/.remoted/slave.py; else echo x; fi\n").toLower();
	remoteMd5.truncate(32);
	qDebug() << remoteMd5;
	qDebug() << sSlaveMd5;
	if (remoteMd5 != sSlaveMd5)
		mSsh->writeFile("/home/thingalon/.remoted/slave.py", sSlaveScript.constData(), sSlaveScript.length());

	const char* command = "python ~/.remoted/slave.py\n";
	mSsh->writeData(command, strlen(command));

/*	QByteArray welcomeMsg = mSsh->readLine();
	welcomeMsg = QByteArray::fromBase64(welcomeMsg);

	const char* welcomeData = welcomeMsg.constData();
	int cwdlen = *(int*)welcomeData;
	QString cwd = QString(QByteArray(welcomeData + 4, cwdlen));
	mHomeDirectory = cwd;
	qDebug() << "Home directory: " << mHomeDirectory;*/

	QByteArray testSend;

	int req = 1;
	short msg = 1;
	testSend.append((const char*)&req, 4);
	testSend.append((const char*)&msg, 2);

	testSend = testSend.toBase64();
	testSend.append("\n");
	mSsh->writeData(testSend.constData(), testSend.length());
	QByteArray retval = mSsh->readLine();
	retval = QByteArray::fromBase64(retval);

	const char* data = retval.constData();

	int reqId = *(int*)(data + 0);
	int ok = *(char*)(data + 4);

	qDebug() << "Request ID: " << reqId;
	qDebug() << "OK: " << ok;

	const char* dataEnd = data + retval.length();
	data += 5;
	while (data < dataEnd)
	{
		int strlen = *(int*)data;
		data += 4;

		QString filename = QString(QByteArray(data, strlen));
		data += strlen;

		int filesize = *(int*)data;
		data += 4;

		qDebug() << filename << filesize;
	}
}

QByteArray SshRemoteController::openFile(const char* filename)
{
	return mSsh->readFile(filename);
}














