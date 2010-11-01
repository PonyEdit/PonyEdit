#include "sshremotecontroller.h"

#include <QThread>
#include <QMutex>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include "sshhost.h"

const char* SshRemoteController::sStatusStrings[] = { "not connected", "connecting", "password required", "negotiating with remote host", "uploading slave script", "starting slave script", "connected", "error" };


/////////////////////////////////
//  SshControllerThread class  //
/////////////////////////////////

class SshControllerThread : public QThread
{
public:
	SshControllerThread(SshHost* host);
	~SshControllerThread();
	void run();
	void connect();
	void runMainLoop();
	void setStatus(SshRemoteController::Status status) { mStatus = status; mLastStatusChange++; }

	SshRemoteController::Status mStatus;
	int mLastStatusChange;

	QString mErrorString;

	QString mHomeDirectory;
	SshHost* mHost;
	SshConnection* mConnection;
	QList<SshRequest*> mRequestQueue;
	QMutex mRequestQueueLock;
	bool mCloseDown;

	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;
};
bool SshControllerThread::sSlaveLoaded = false;
QByteArray SshControllerThread::sSlaveScript;
QByteArray SshControllerThread::sSlaveMd5;


//////////////////////////////////////
//  SshRemoteController definitions //
//////////////////////////////////////

SshRemoteController::SshRemoteController(SshHost* host)
{
	//	Fire up a thread to manage this connection
	mThread = new SshControllerThread(host);
	mThread->start();
}

SshRemoteController::~SshRemoteController()
{
	abortConnection();
	delete mThread;
}

void SshRemoteController::abortConnection()
{
	mThread->mCloseDown = true;
	if (!mThread->wait(3000))
		mThread->terminate();
}

void SshRemoteController::sendRequest(SshRequest* request)
{
	mThread->mRequestQueueLock.lock();
	mThread->mRequestQueue.append(request);
	mThread->mRequestQueueLock.unlock();
}

const QString& SshRemoteController::getHomeDirectory() const
{
	return mThread->mHomeDirectory;
}

const QString& SshRemoteController::getError() const
{
	return mThread->mErrorString;
}

int SshRemoteController::getLastStatusChange() const
{
	return mThread->mLastStatusChange;
}

SshRemoteController::Status SshRemoteController::getStatus() const
{
	return mThread->mStatus;
}


///////////////////////////////////////
//  SshControllerThread definitions  //
///////////////////////////////////////

SshControllerThread::SshControllerThread(SshHost *host)
{
	//	Make sure the slave script is loaded and ready to be pushed
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

	mCloseDown = false;
	mLastStatusChange = 0;
	mHost = host;
	mConnection = NULL;
	setStatus(SshRemoteController::NotConnected);
}

SshControllerThread::~SshControllerThread()
{
	if (mConnection)
		delete mConnection;
}

void SshControllerThread::run()
{
	connect();
	if (mCloseDown) return;
	if (mStatus == SshRemoteController::Connected)
		runMainLoop();
}

void SshControllerThread::connect()
{
	mConnection = new SshConnection();
	try
	{
		//
		//	Establish a raw SSH connection...
		//

		setStatus(SshRemoteController::Connecting);
		mConnection->connect(mHost->getHostName().toUtf8(), mHost->getPort());
		if (mCloseDown) return;

		//
		//	Handle authentication
		//

		bool authenticated = false;
		while (!authenticated)
		{
			QString password = mHost->getPassword();
			if (password.isEmpty())
			{
				setStatus(SshRemoteController::WaitingForPassword);
				while ((password = mHost->getPassword()).isEmpty())
				{
					if (mCloseDown) return;
					msleep(100);
				}
			}

			setStatus(SshRemoteController::Connecting);
			authenticated = mConnection->authenticatePassword(mHost->getUserName().toUtf8(), password.toUtf8());
		}
		if (mCloseDown) return;

		//
		//	Switch to remote home directory...
		//

		setStatus(SshRemoteController::Negotiating);
		mConnection->execute("cd ~\n");
		if (mCloseDown) return;

		//
		//	Make sure the remote slave script is present, and the md5 hashes match. If not, upload again.
		//

		QByteArray remoteMd5 = mConnection->execute("if [ ! -d ~/.remoted ]; then mkdir ~/.remoted; fi; if [ -e ~/.remoted/slave.py ]; then md5sum ~/.remoted/slave.py; else echo x; fi\n").toLower();
		remoteMd5.truncate(32);
		if (remoteMd5 != sSlaveMd5)
		{
			setStatus(SshRemoteController::UploadingSlave);
			mConnection->writeFile(".remoted/slave.py", sSlaveScript.constData(), sSlaveScript.length());
		}
		if (mCloseDown) return;

		//
		//	Run the remote python script...
		//

		setStatus(SshRemoteController::StartingSlave);
		const char* command = "python ~/.remoted/slave.py\n";
		mConnection->writeData(command, strlen(command));
		if (mCloseDown) return;

		//
		//	The first line returned should be the user's home directory. If not, an error has occurred.
		//

		mHomeDirectory = mConnection->readLine().trimmed();
		if (mHomeDirectory.startsWith("~="))
		{
			mHomeDirectory = mHomeDirectory.mid(2);
			if (mHomeDirectory.endsWith('/'))
				mHomeDirectory.truncate(mHomeDirectory.length() - 1);
			mHomeDirectory = mHomeDirectory.trimmed();
		}
		else
			throw(QString("Failed to start slave script!"));
		if (mCloseDown) return;

		setStatus(SshRemoteController::Connected);
	}
	catch (QString err)
	{
		delete mConnection;
		mConnection = NULL;

		mErrorString = QString("Error while ") + SshRemoteController::sStatusStrings[mStatus] + ": " + err;
		setStatus(SshRemoteController::Error);
		return;
	}
}

void SshControllerThread::runMainLoop()
{
	while (1)
	{
		mRequestQueueLock.lock();
		if (mRequestQueue.length() > 0)
		{
			if (mCloseDown) return;

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
			mConnection->writeData(massSend.constData(), massSend.length());

			//	Wait for a response to each message in turn
			while (sendingMessages.length())
			{
				QByteArray response = QByteArray::fromBase64(mConnection->readLine());
				SshRequest* rq = sendingMessages.takeFirst();
				rq->handleResponse(response);

				if (rq->hasManualComponent())
					rq->doManualWork(mConnection);

				delete rq;
			}
		}
		else
			mRequestQueueLock.unlock();

		if (mCloseDown) return;
		msleep(10);
	}
}





