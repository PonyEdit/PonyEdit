#include "sshremotecontroller.h"

#include <QThread>
#include <QMutex>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QTimer>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QCoreApplication>
#include "ssh/sshhost.h"
#include "ssh/sshrequest.h"
#include "ssh/sshconnection.h"
#include "file/sshfile.h"
#include "connectionstatuswidget.h"
#include "passwordinput.h"
#include "main/globaldispatcher.h"

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
	void loadScript();

	void setStatus(RemoteConnection::Status status);

	QString mErrorString;

	QString mHomeDirectory;
	SshHost* mHost;
	SshConnection* mConnection;
	QList<SshRequest*> mRequestQueue;
	QMutex mRequestQueueLock;

	SshRemoteController* mController;

	static bool sSlaveLoaded;
	static QByteArray sSlaveScript;
	static QByteArray sSlaveMd5;
	static const char* sSlaveScriptName;
	static const char* sSlaveStartCommand;
};
bool SshControllerThread::sSlaveLoaded = false;
QByteArray SshControllerThread::sSlaveScript;
QByteArray SshControllerThread::sSlaveMd5;
const char* SshControllerThread::sSlaveScriptName = "slave.pl";
const char* SshControllerThread::sSlaveStartCommand = "perl ~/.ponyedit/slave.pl\n";


//////////////////////////////////////
//  SshRemoteController definitions //
//////////////////////////////////////

SshRemoteController::SshRemoteController(SshHost* host)
{
	//	Fire up a thread to manage this connection
	mThread = new SshControllerThread(this, host);
	mThread->start();
	mPasswordInput = NULL;
}

SshRemoteController::~SshRemoteController()
{
	delete mThread;
}

void SshRemoteController::sendRequest(SshRequest* request)
{
	request->setController(this);
	bool connected;

	//	Lock the thread before checking if the status is ok; avoids race conditions during disconnects
	mThread->mRequestQueueLock.lock();
	connected = isConnected();
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

QString SshRemoteController::getName()
{
	return mThread->mHost->getName();
}

void SshRemoteController::hostkeyWarnDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target)
{
	SshRemoteController* controller = static_cast<SshRemoteController*>(connection);

	if (controller->mNewHostKey)
		widget->setManualStatus(tr("Verify Host Fingerprint"), QPixmap(":/icons/question.png"));
	else
		widget->setManualStatus(tr("Host Fingerprint Changed!"), QPixmap(":/icons/warning.png"));
	widget->addButton(QDialogButtonBox::YesRole, tr("Connect"));

	QVBoxLayout* layout = new QVBoxLayout(target);
	QString text = controller->mNewHostKey ?
				tr("There is no fingerprint on file for this host. If this is the first time you are connecting to this server, it is safe "
				  "to ignore this warning.\n\n Host fingerprints provide extra security by verifying that you are connecting to the server "
				  "that you think you are. The first time you connect to a new server, there will be no fingerprint on file. You will be "
				  "warned whenever you connect to a server if the host fingerprint has changed. ")
				:
				tr("The fingerprint returned by the host, and the one on file do not match! This may be caused by a breech of security, or "
				   "by server reconfiguration. Please check with your host administrator before accepting the changed host key!");

	text += "\n\nFingerprint: " + controller->mThread->mConnection->getServerFingerprint().toHex();

	QLabel* question = new QLabel(text, target);
	question->setWordWrap(true);
	layout->addWidget(question);
}

bool SshRemoteController::hostkeyWarnCallback(ConnectionStatusWidget* /* widget */, RemoteConnection* connection, QDialogButtonBox::ButtonRole /* buttonRole */)
{
	SshRemoteController* controller = static_cast<SshRemoteController*>(connection);
	SshConnection::saveFingerprint(controller->mThread->mHost->getHostName(), controller->mThread->mConnection->getServerFingerprint());
	return true;
}

void SshRemoteController::passwordInputDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target)
{
	SshRemoteController* controller = static_cast<SshRemoteController*>(connection);

	widget->addButton(QDialogButtonBox::YesRole, tr("Connect"));
	widget->setManualStatus(tr("Please enter your password"), QPixmap(":/icons/question.png"));

	QVBoxLayout* layout = new QVBoxLayout(target);
	controller->mPasswordInput = new PasswordInput(target);
	layout->addWidget(controller->mPasswordInput);
}

bool SshRemoteController::passwordInputCallback(ConnectionStatusWidget* /* widget */, RemoteConnection* connection, QDialogButtonBox::ButtonRole /* buttonRole */)
{
	SshRemoteController* controller = static_cast<SshRemoteController*>(connection);

	controller->mThread->mHost->setPassword(controller->mPasswordInput->getEnteredPassword());
	controller->mThread->mHost->setSavePassword(controller->mPasswordInput->getSavePassword());

	return true;
}

///////////////////////////////////////
//  SshControllerThread definitions  //
///////////////////////////////////////

SshControllerThread::SshControllerThread(SshRemoteController* controller, SshHost *host)
{
	mController = controller;
	mHost = host;
	mConnection = NULL;
	setStatus(SshRemoteController::Uninitialized);
}

SshControllerThread::~SshControllerThread()
{
	disconnect();
}

void SshControllerThread::setStatus(RemoteConnection::Status status)
{
	mRequestQueueLock.lock();
	mController->setStatus(status);
	mRequestQueueLock.unlock();
}

void SshControllerThread::run()
{
	runMainLoop();
	disconnect();
}

void SshControllerThread::loadScript()
{
	if (!sSlaveLoaded)
	{
#ifdef Q_OS_MAC
		QFile f(QCoreApplication::applicationDirPath() + QString("/../Resources/slave/") + sSlaveScriptName);
#else
		QFile f(QString("slave/") + sSlaveScriptName);
#endif
		f.open(QFile::ReadOnly);
		sSlaveScript = f.readAll();

		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(sSlaveScript);
		sSlaveMd5 = hash.result().toHex().toLower();

		sSlaveLoaded = true;
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

		setStatus(RemoteConnection::Connecting);
		mConnection->connect(mHost->getHostName().toUtf8(), mHost->getPort());
		if (mController->isDisconnecting()) return;

		//
		//	Check the server's fingerprint
		//

		QByteArray fingerprint = mConnection->getServerFingerprint();
		QByteArray knownFingerprint = mConnection->getExpectedFingerprint(mHost->getHostName());
		if (knownFingerprint != fingerprint)
			mController->waitForInput(SshRemoteController::hostkeyWarnDialog, SshRemoteController::hostkeyWarnCallback);
		if (mController->isDisconnecting()) return;

		//
		//	Handle authentication
		//

		SshConnection::AuthMethods authMethods = mConnection->getAuthenticationMethods(mHost->getUserName().toUtf8());
		bool authenticated = false;

		//	If key authentication is available
		if (authMethods & SshConnection::PublicKey)
		{
			//	Try with an agent first
			//authenticated = mConnection->authenticateAgent(mHost->getUserName().toUtf8());

			//	If no agent available, try with a straight-up key
			if (!authenticated && !mHost->getKeyFile().isEmpty())
				authenticated = mConnection->authenticateKeyFile(mHost->getKeyFile().toUtf8(), "brains!!");
		}

		//	Fall back on password entry
		while (!authenticated)
		{
			if (mHost->getPassword().isEmpty())
				mController->waitForInput(SshRemoteController::passwordInputDialog, SshRemoteController::passwordInputCallback);

			if (mController->isDisconnecting()) return;
			authenticated = mConnection->authenticatePassword(mHost->getUserName().toUtf8(), mHost->getPassword().toUtf8());

			if(!authenticated)
				mHost->setPassword(QString());
		}
		if (mController->isDisconnecting()) return;

		//
		//	Switch to remote home directory...
		//

		qDebug() << "Switching to home dir...";

		setStatus(RemoteConnection::Configuring);
		mConnection->execute("cd ~\n");
		if (mController->isDisconnecting()) return;

		//
		//	Check that Perl is installed and a recent version
		//

		bool validPerl = false;
		QByteArray perlVersion = mConnection->execute("perl -v\n");
		if(perlVersion.length() > 0)
		{
			QRegExp perlVersionRx("This is perl, v(\\d+)\\.(\\d+)");
			if(perlVersionRx.indexIn(perlVersion) > -1)
			{
				QString perlMajorVersionNumber = perlVersionRx.cap(1);
				QString perlMinorVersionNumber = perlVersionRx.cap(2);
				if(perlMajorVersionNumber.toInt() >= 5 && perlMinorVersionNumber.toInt() > 6)
					validPerl = true;
			}
		}

		if(!validPerl)
			throw(tr("No usable version of Perl found!"));

		//
		//	Make sure the remote slave script is present, and the md5 hashes match. If not, upload again.
		//

		loadScript();

		QByteArray remoteMd5 = mConnection->execute((QString("if [ ! -d ~/.ponyedit ]; then mkdir ~/.ponyedit; fi; if [ -e ~/.ponyedit/") +
			sSlaveScriptName + " ]; then md5sum ~/.ponyedit/" + sSlaveScriptName + "; else echo x; fi\n").toAscii()).toLower();
		remoteMd5.truncate(32);
		if (remoteMd5 != sSlaveMd5)
			mConnection->writeFile((QString(".ponyedit/") + sSlaveScriptName).toAscii(), sSlaveScript.constData(), sSlaveScript.length());
		if (mController->isDisconnecting()) return;

		//
		//	Run the remote script...
		//

		qDebug() << "Running remote script...";

		mConnection->writeData(sSlaveStartCommand, strlen(sSlaveStartCommand));
		if (mController->isDisconnecting()) return;

		//
		//	The first line returned should be the user's home directory. If not, an error has occurred.
		//

		mHomeDirectory = mConnection->readLine().trimmed();
		qDebug() << "Received: " << mHomeDirectory;

		if (mHomeDirectory.startsWith("~="))
		{
			mHomeDirectory = mHomeDirectory.mid(2);
			if (mHomeDirectory.endsWith('/'))
				mHomeDirectory.truncate(mHomeDirectory.length() - 1);
			mHomeDirectory = mHomeDirectory.trimmed();
		}
		else
			throw(QString("Failed to start slave script!"));
		if (mController->isDisconnecting()) return;

		//
		//	Connected!!
		//

		setStatus(SshRemoteController::Connected);
	}
	catch (QString err)
	{
		delete mConnection;
		mConnection = NULL;

		mController->setErrorStatus(QString("Error while ") + mController->getStatusString() + ": " + err);
		return;
	}
}

void SshControllerThread::runMainLoop()
{
	//
	//	Outer loop: Connects and tries to pump the queue
	//

	while (!mController->isDeliberatelyDisconnecting())
	{
		connect();
		if (mController->isDisconnecting()) break;

		//	Handle connection failure; try again if files are open, fail if not.
		if (!mController->isConnected())
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
			while (!mController->isDisconnecting())
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

				if (mController->isDisconnecting()) return;
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
			mRequestQueueLock.unlock();

			disconnect();
			gDispatcher->emitConnectionDropped(mController);
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
	setStatus(SshRemoteController::Disconnected);
}



