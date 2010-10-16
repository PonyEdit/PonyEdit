#include "sshremotecontroller.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include "remotecontrolmessage.h"

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

	const char* home = "cd ~\n";
	mSsh->execute(home);

	QByteArray remoteMd5 = mSsh->execute("if [ ! -d ~/.remoted ]; then mkdir ~/.remoted; fi; if [ -e ~/.remoted/slave.py ]; then md5sum ~/.remoted/slave.py; else echo x; fi\n").toLower();
	remoteMd5.truncate(32);
	if (remoteMd5 != sSlaveMd5)
		mSsh->writeFile(".remoted/slave.py", sSlaveScript.constData(), sSlaveScript.length());

	const char* command = "python ~/.remoted/slave.py\n";
	mSsh->writeData(command, strlen(command));
}

QByteArray SshRemoteController::openFile(const char* filename)
{
	QByteArray content = mSsh->readFile(filename);

	RemoteControlMessage msg(mtOpen);
	msg.addData('f', filename);

	QByteArray data = msg.finalize();
	data = data.toBase64();
	data.append("\n", 1);

	mSsh->writeData(data.constData(), data.length());
	mSsh->readLine();

	return content;
}

void SshRemoteController::splitThread()
{
	mThread.mSsh = mSsh;
	mThread.start();
}

void SshRemoteController::push(Push p)
{
	mThread.mQueueLock.lock();
	mThread.mQueue.push_back(p);
	mThread.mQueueLock.unlock();
}

void SshRemoteController::ControllerThread::run()
{
	while (1)
	{
		mQueueLock.lock();
		if (mQueue.length())
		{
			int saving = 0;

			//	Pack all the changes into one
			QByteArray sendShit;
			int len = mQueue.length();
			for (int i = 0; i < len; i++)
			{
				Push p = mQueue.at(i);
				if (!p.save)
				{
					RemoteControlMessage msg(mtChange);
					msg.addData('p', p.position);
					msg.addData('r', p.remove);
					msg.addData('a', p.add.toUtf8().constData());
					sendShit.append(msg.finalize());
				}
				else
				{
					saving = 1;

					RemoteControlMessage msg(mtSave);
					sendShit.append(msg.finalize());
				}
			}
			mQueue.clear();
			mQueueLock.unlock();

			sendShit = sendShit.toBase64();
			sendShit.append("\n");
			mSsh->writeData(sendShit.constData(), sendShit.length());
			while (len)
			{
				mSsh->readLine();
				len--;
			}

			if (saving)
				qDebug() << "FILE SAVED!";
		}
		else
			mQueueLock.unlock();

		msleep(10);
	}
}












