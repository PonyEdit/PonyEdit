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
		const char* home = "cd ~\n";
		mSsh->writeData(home, strlen(home));

		mSsh->writeFile(".remoted/slave.py", sSlaveScript.constData(), sSlaveScript.length());
	}

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

	short msg = 1;
	int msgLength = 0;
	testSend.append((const char*)&msg, 2);
	testSend.append((const char*)&msgLength, 4);
	testSend = testSend.toBase64();
	testSend.append("\n");
	mSsh->writeData(testSend.constData(), testSend.length());

	QByteArray retval = mSsh->readLine();
	retval = QByteArray::fromBase64(retval);

	const char* data = retval.constData();

	int ok = *(char*)(data);

	qDebug() << "OK: " << ok;

	const char* dataEnd = data + retval.length();
	data += 1;
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
	QByteArray content = mSsh->readFile(filename);

	/*int req = 1;
	short msg = 2;
	int slen = strlen(filename);
	sendShit.append((const char*)&req, 4);
	sendShit.append((const char*)&msg, 2);
	sendShit.append((const char*)&slen, 4);
	sendShit.append(filename);
	mSsh->writeData(sendShit);
	mSsh->readLine();*/

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
			//	Pack all the changes into one
			/*QByteArray sendShit;
			int req = 1;
			short msg;
			int len = mQueue.length();
			for (int i = 0; i < len; i++)
			{
				Push p = mQueue.at(0);
				if (p.save)
				{
					msg = 3;
					int slen = p.add.length();
					sendShit.append((const char*)&req, 4);
					sendShit.append((const char*)&msg, 2);
					sendShit.append((const char*)&p.position, 4);
					sendShit.append((const char*)&p.remove, 2);
					sendShit.append((const char*)&slen, 4);
					sendShit.append(p.add);
				}
				else
				{
					msg = 4;
					sendShit.append((const char*)&req, 4);
					sendShit.append((const char*)&msg, 2);
				}
			}*/
			mQueue.empty();
			mQueueLock.unlock();

			/*mSsh->writeData(sendShit);
			while (len)
			{
				mSsh->readLine();
				len--;
			}*/
		}
		else
			mQueueLock.unlock();

		msleep(500);
	}
}












