#include "sshremotecontroller.h"

#include <QThread>
#include <QMutex>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QTimer>
#include "ssh/sshhost.h"
#include "ssh/sshrequest.h"
#include "ssh/sshconnection.h"
#include "file/sshfile.h"

const char* SshRemoteController::sStatusStrings[] = { "not connected", "connecting", "password required", "negotiating with remote host", "uploading slave script", "starting slave script", "pushing buffers", "connected", "error" };


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
	void disconnect();
	void runMainLoop();
	void setStatus(SshRemoteController::Status status) { mStatus = status; mLastStatusChange++; mController->emitStateChanged(); }
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
	request->setController(this);
	bool connected;

	//	Lock the thread before checking if the status is ok; avoids race conditions during disconnects
	mThread->mRequestQueueLock.lock();
	connected = (mThread->mStatus == Connected);
	if (connected)
		mThread->mRequestQueue.append(request);
	mThread->mRequestQueueLock.unlock();

	//	If not connected, auto-fail & delete the request :(
	if (!connected)
	{
		request->error("Not connected!");
		delete request;
	}
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
	disconnect();
}

void SshControllerThread::run()
{
	runMainLoop();
	disconnect();
}

void SshControllerThread::loadScript(SshRemoteController::ScriptType type)
{
	if (type == SshRemoteController::AutoDetect || type >= SshRemoteController::NumScriptTypes)
		throw(QString("Invalid slave script type!"));

	if (!sSlaveLoaded[type])
	{
		QFile f(QString("slaves/") + sSlaveScriptNames[type]);
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

		SshConnection::AuthMethods authMethods = mConnection->getAuthenticationMethods(mHost->getUserName().toUtf8());
		bool authenticated = false;

		//	Always try ssh-agent/putty first if key auth is available
		if (authMethods & SshConnection::PublicKey)
			authenticated = mConnection->authenticateAgent(mHost->getUserName().toUtf8());

		//	Fall back on password entry
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

			if(!authenticated)
				mHost->setPassword("");
		}
		if (mCloseDown) return;

		//
		//	Switch to remote home directory...
		//

		qDebug() << "Switching to home dir...";

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
		if (remoteMd5 != sSlaveMd5[scriptType])
		{
			setStatus(SshRemoteController::UploadingSlave);
			mConnection->writeFile((QString(".remoted/") + scriptName).toAscii(), sSlaveScript[scriptType].constData(), sSlaveScript[scriptType].length());
		}
		if (mCloseDown) return;

		//
		//	Run the remote script...
		//

		qDebug() << "Running remote script...";

		setStatus(SshRemoteController::StartingSlave);
		const char* startCommand = sSlaveStartCommands[scriptType];
		mConnection->writeData(startCommand, strlen(startCommand));
		if (mCloseDown) return;

		//
		//	The first line returned should be the user's home directory. If not, an error has occurred.
		//

		mHomeDirectory = mConnection->readLine().trimmed();
		qDebug() << "Recevied: " << mHomeDirectory;

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

		//
		//	Connected!!
		//

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
	//
	//	Outer loop: Connects and tries to pump the queue
	//

	while (!mCloseDown)
	{
		connect();
		if (mCloseDown) break;

		//	Handle connection failure; try again if files are open, fail if not.
		if (mStatus != SshRemoteController::Connected)
		{
			qDebug() << "Failed to connect";
			if (mHost->numOpenFiles() > 0)
				continue;
			else
				return;
		}

		QTime lastMessageTime;
		lastMessageTime.restart();
		SshRequest_keepalive keepAliveMessage;
		QList<SshRequest*> sendingMessages;

		//
		//	Inner loop: Pumps messages until the connection dies or closedown is signalled
		//

		try
		{
			while (!mCloseDown)
			{
				//
				//	Check the send queue for messages. Drop them in a temp array if they're there
				//	so the mutex doesn't need to be locked for too long.
				//

				mRequestQueueLock.lock();
				sendingMessages.append(mRequestQueue);
				mRequestQueue.clear();
				mRequestQueueLock.unlock();

				if (lastMessageTime.elapsed() > KEEPALIVE_TIMEOUT)
					sendingMessages.append(&keepAliveMessage);

				//
				//	If there are messages to send, pack them all into one message and send it
				//

				if (sendingMessages.length() > 0)
				{
					QByteArray sendBlob;
					foreach (SshRequest* rq, sendingMessages)
						rq->packMessage(&sendBlob);

					sendBlob = sendBlob.toBase64();
					sendBlob.append('\n');
					mConnection->writeData(sendBlob.constData(), sendBlob.length());

					//
					//	Wait for a response for each message in turn
					//

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

						//	Delete each request if it's not the keepalive message
						if (rq != &keepAliveMessage)
							delete rq;
					}

					lastMessageTime.restart();
				}

				if (mCloseDown) return;
				msleep(10);
			}
		}
		catch (QString error)
		{
			qDebug() << "Connection error caught: " << error;

			//
			//	Connection lost. Fail all the jobs in the queue.
			//

			mRequestQueueLock.lock();
			sendingMessages.append(mRequestQueue);
			mRequestQueue.clear();

			foreach (SshRequest* rq, sendingMessages)
			{
				rq->error("Connection lost: " + error);
				if (rq != &keepAliveMessage)
					delete rq;
			}

			sendingMessages.clear();

			disconnect();	//	This is inside the lock so status is "not connected" by the time the mutex comes undone.

			mRequestQueueLock.unlock();
		}
	}
}

void SshControllerThread::disconnect()
{
	if (mConnection)
	{
		delete mConnection;
		mConnection = NULL;
	}
	setStatus(SshRemoteController::NotConnected);
}

