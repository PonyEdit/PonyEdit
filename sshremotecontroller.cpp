#include "sshremotecontroller.h"

#include <QThread>
#include <QMutex>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QTimer>
#include "sshhost.h"
#include "sshrequest.h"
#include "sshconnection.h"

const char* SshRemoteController::sStatusStrings[] = { "not connected", "connecting", "password required", "negotiating with remote host", "uploading slave script", "starting slave script", "connected", "error" };


/////////////////////////////////
//  SshControllerThread class  //
/////////////////////////////////

class SshControllerThread : public QThread
{
public:
	SshControllerThread(SshRemoteController* controller, SshHost* host);
	~SshControllerThread();
	void run();
	void connect();
	void runMainLoop();
	void setStatus(SshRemoteController::Status status) { mStatus = status; mLastStatusChange++; }
	void loadScript(SshRemoteController::ScriptType type);

	SshRemoteController::Status mStatus;
	int mLastStatusChange;

	QString mErrorString;

	QString mHomeDirectory;
	SshHost* mHost;
	SshConnection* mConnection;
	QList<SshRequest*> mRequestQueue;
	QMutex mRequestQueueLock;
	bool mCloseDown;

	SshRemoteController* mController;

	static bool sSlaveLoaded[SshRemoteController::NumScriptTypes];
	static QByteArray sSlaveScript[SshRemoteController::NumScriptTypes];
	static QByteArray sSlaveMd5[SshRemoteController::NumScriptTypes];
	static const char* sSlaveScriptNames[];
	static const char* sSlaveStartCommands[];
};
bool SshControllerThread::sSlaveLoaded[SshRemoteController::NumScriptTypes] = { false };
QByteArray SshControllerThread::sSlaveScript[SshRemoteController::NumScriptTypes];
QByteArray SshControllerThread::sSlaveMd5[SshRemoteController::NumScriptTypes];
const char* SshControllerThread::sSlaveScriptNames[] = { "", "slave.py", "slave.pl" };
const char* SshControllerThread::sSlaveStartCommands[] = { "", "python ~/.remoted/slave.py\n", "perl ~/.remoted/slave.pl\n" };
const char* SshRemoteController::sScriptTypeLabels[] = { "Auto-detect", "Python", "Perl" };


//////////////////////////////////////
//  SshRemoteController definitions //
//////////////////////////////////////

SshRemoteController::SshRemoteController(SshHost* host)
{
	//	Fire up a thread to manage this connection
	mThread = new SshControllerThread(this, host);
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

SshControllerThread::SshControllerThread(SshRemoteController* controller, SshHost *host)
{
	mController = controller;
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

void SshControllerThread::loadScript(SshRemoteController::ScriptType type)
{
	if (type == SshRemoteController::AutoDetect || type >= SshRemoteController::NumScriptTypes)
		throw(QString("Invalid slave script type!"));

	if (!sSlaveLoaded[type])
	{
		QFile f(sSlaveScriptNames[type]);
		f.open(QFile::ReadOnly);
		sSlaveScript[type] = f.readAll();

		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(sSlaveScript[type]);
		sSlaveMd5[type] = hash.result().toHex().toLower();

		sSlaveLoaded[type] = true;
	}
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
		//	Check which script to use
		//

		SshRemoteController::ScriptType scriptType = mHost->getScriptType();
		if (scriptType == SshRemoteController::AutoDetect)
		{
			QByteArray pythonVersion = mConnection->execute("python -V\n");
			if(pythonVersion.length() > 0)
			{
				QRegExp pythonVersionRx("^\\d+\\.\\d+");

				pythonVersion.replace("Python ", "");

				if(pythonVersionRx.indexIn(pythonVersion) > -1 && pythonVersion >= "2.4")
					scriptType = SshRemoteController::Python;
			}

			if (scriptType == SshRemoteController::AutoDetect)
			{
				QByteArray perlVersion = mConnection->execute("perl -v\n");
				if(perlVersion.length() > 0)
				{
					QRegExp perlVersionRx("This is perl, v(\\w+)");
					if(perlVersionRx.indexIn(perlVersion) > -1)
					{
						QString perlVersionNumber = perlVersionRx.cap(1);
						if(perlVersionNumber >= "5.6")
							scriptType = SshRemoteController::Perl;
					}
				}
			}

			if (scriptType == SshRemoteController::AutoDetect)
				throw(QString("No usable version of Python or Perl found!"));
		}

		//
		//	Make sure the remote slave script is present, and the md5 hashes match. If not, upload again.
		//

		loadScript(scriptType);
		const char* scriptName = sSlaveScriptNames[scriptType];

		QByteArray remoteMd5 = mConnection->execute((QString("if [ ! -d ~/.remoted ]; then mkdir ~/.remoted; fi; if [ -e ~/.remoted/") +
			scriptName + " ]; then md5sum ~/.remoted/" + scriptName + "; else echo x; fi\n").toAscii()).toLower();
		remoteMd5.truncate(32);
		if (remoteMd5 != sSlaveMd5)
		{
			setStatus(SshRemoteController::UploadingSlave);
			mConnection->writeFile((QString(".remoted/") + scriptName).toAscii(), sSlaveScript[scriptType].constData(), sSlaveScript[scriptType].length());
		}
		if (mCloseDown) return;

		//
		//	Run the remote script...
		//

		setStatus(SshRemoteController::StartingSlave);
		const char* startCommand = sSlaveStartCommands[scriptType];
		mConnection->writeData(startCommand, strlen(startCommand));
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
	QTime lastMessageTime;
	lastMessageTime.start();
	SshRequest_keepalive keepAliveMessage;

	while (1)
	{
		mRequestQueueLock.lock();
		if (mRequestQueue.length() > 0 || lastMessageTime.elapsed() > KEEPALIVE_TIMEOUT)
		{
			if (mCloseDown) return;

			//	If no messages are being sent, send a keepalive signal
			if (mRequestQueue.length() == 0)
				mRequestQueue.append(&keepAliveMessage);

			//	Pack all of the requests into one bytearray, and unlock the main queue as quickly as possible
			QByteArray massSend;
			QList<SshRequest*> sendingMessages;
			foreach (SshRequest* rq, mRequestQueue)
			{
				rq->setConnection(mConnection);
				rq->setController(mController);

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

				try
				{
					rq->handleResponse(response);

					if (rq->hasManualComponent())
						rq->doManualWork(mConnection);

					rq->success();
				}
				catch (QString error)
				{
					rq->error(error);
				}

				//	Don't delete the keepalive message; just reuse the same object over and over.
				if (rq != &keepAliveMessage)
					delete rq;
			}

			lastMessageTime.start();
		}
		else
			mRequestQueueLock.unlock();

		if (mCloseDown) return;
		msleep(10);
	}
}





