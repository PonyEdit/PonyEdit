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
	{
		mSsh->writeFile("/home/thingalon/.remoted/slave.py", sSlaveScript.constData(), sSlaveScript.length());
		/*//	Time to upload a new script
		QByteArray fullPush = QByteArray("cat << \"END_OF_SLAVE_SCRIPT\" > ~/.remoted/slave.py\n");
		fullPush.append(sSlaveScript);
		fullPush.append("\nEND_OF_SLAVE_SCRIPT\n");

		mSsh->execute(fullPush);*/
	}

	qDebug() << mSsh->execute("python ~/.remoted/slave.py\n");
}
