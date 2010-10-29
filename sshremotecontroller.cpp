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

	const char* home = "cd ~\n";
	mSsh->execute(home);

	QByteArray remoteMd5 = mSsh->execute("if [ ! -d ~/.remoted ]; then mkdir ~/.remoted; fi; if [ -e ~/.remoted/slave.py ]; then md5sum ~/.remoted/slave.py; else echo x; fi\n").toLower();
	remoteMd5.truncate(32);
	if (remoteMd5 != sSlaveMd5)
		mSsh->writeFile(".remoted/slave.py", sSlaveScript.constData(), sSlaveScript.length());

	const char* command = "python ~/.remoted/slave.py\n";
	mSsh->writeData(command, strlen(command));

	//	First line returned should be the user's home dir. If not, an error has occurred.
	mHomeDirectory = mSsh->readLine().trimmed();
	if (mHomeDirectory.startsWith("~="))
	{
		mHomeDirectory = mHomeDirectory.mid(2);
		if (mHomeDirectory.endsWith('/'))
			mHomeDirectory.truncate(mHomeDirectory.length() - 1);
		mHomeDirectory = mHomeDirectory.trimmed();
	}
	else
		throw("Failed to start slave script!");
}

void SshRemoteController::splitThread()
{
	mThread.mSsh = mSsh;
	mThread.start();
}

void SshRemoteController::sendRequest(SshRequest* request)
{
	mThread.mRequestQueueLock.lock();
	mThread.mRequestQueue.append(request);
	mThread.mRequestQueueLock.unlock();
}

void SshRemoteController::ControllerThread::run()
{
	while (1)
	{
		mRequestQueueLock.lock();
		if (mRequestQueue.length() > 0)
		{
			//	Pack all of the requests into one bytearray, and unlock the main queue as quickly as possible
			QByteArray massSend;
			QList<SshRequest*> sendingMessages;
			foreach (SshRequest* rq, mRequestQueue)
			{
				rq->packMessage(&massSend);
				sendingMessages.append(rq);
			}
			mRequestQueue.clear();
			mRequestQueueLock.unlock();

			//	Encode and send the bytearray
			massSend = massSend.toBase64();
			massSend.append('\n');
			mSsh->writeData(massSend.constData(), massSend.length());

			//	Wait for a response to each message in turn
			while (sendingMessages.length())
			{
				QByteArray response = QByteArray::fromBase64(mSsh->readLine());
				SshRequest* rq = sendingMessages.takeFirst();
				rq->handleResponse(response);
				delete rq;
			}
		}
		else
			mRequestQueueLock.unlock();

		msleep(10);
	}
}












