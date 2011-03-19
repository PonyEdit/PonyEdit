#include <QEvent>
#include <QFileOpenEvent>
#include <QDebug>
#include <QLocalSocket>
#include <QMessageBox>

#include "ponyedit.h"
#include "file/location.h"
#include "mainwindow.h"
#include "ssh/rawsshconnection.h"
#include "main/globaldispatcher.h"
#include "file/location.h"
#include "main/tools.h"
#include "main/mainwindow.h"
#include "website/sitemanager.h"
#include "syntax/syntaxrule.h"
#include "syntax/syntaxdefmanager.h"
#include "main/stringtrie.h"
#include "options/options.h"
#include "ssh/sshhost.h"

GlobalDispatcher* gDispatcher = NULL;
SiteManager* gSiteManager = NULL;
SyntaxDefManager* gSyntaxDefManager = NULL;
MainWindow* gMainWindow = NULL;

PonyEdit::PonyEdit(int argc, char** argv) : QApplication(argc, argv)
{
#ifndef Q_OS_LINUX
	QStringList substitutions;
	substitutions.append("consolas");
	substitutions.append("courier new");
	substitutions.append("helvetica");
	QFont::insertSubstitutions("inconsolata", substitutions);
#endif

	qRegisterMetaType<Location>("Location");
	qRegisterMetaType< QList<Location> >("QList<Location>");

	RawSshConnection::initializeLib();

	Tools::loadServers();
	Location::loadFavorites();
	Tools::initialize();

	// UUID used to (hopefully) ensure memory name is unique
	mKey = "PonyEdit-lock-138ad7e0-2ecb-11e0-91fa-0800200c9a66";
	mMemoryLock.setKey(mKey);

	if(mMemoryLock.attach())
		mIsRunning = true;
	else
	{
		mIsRunning = false;
		// create shared memory.
		if (!mMemoryLock.create(1))
		{
			qDebug() << "Unable to create single instance.";
			return;
		}

		// create local server and listen to incomming messages from other instances.
		mLocalServer = new QLocalServer(this);
		connect(mLocalServer, SIGNAL(newConnection()), this, SLOT(receiveMessage()));
		mLocalServer->listen(mKey);
	}

	Options::load();

	gSyntaxDefManager = new SyntaxDefManager();
	gSiteManager = new SiteManager();
	gDispatcher = new GlobalDispatcher();

	gMainWindow = new MainWindow();
	gMainWindow->show();

	Tools::loadStartupFiles();
}

PonyEdit::~PonyEdit()
{
	delete gDispatcher;
	delete gSiteManager;
	delete gSyntaxDefManager;
	delete gMainWindow;

	Options::save();
	LocationShared::cleanupIconProvider();
	StringTrie::cleanup();
	SyntaxRule::cleanup();
	RawSshConnection::cleanup();
	SshHost::cleanupHosts();
}

bool PonyEdit::event(QEvent *e)
{
	if(e->type() == QEvent::FileOpen)
	{
		if(!gMainWindow)
			return false;

		QFileOpenEvent *event = static_cast<QFileOpenEvent*>(e);

		QString name = event->file();
		if(name.trimmed().isNull())
			return false;

		e->accept();

		gMainWindow->openSingleFile(Location(name));

		return true;
	}

	return QApplication::event(e);
}
void PonyEdit::receiveMessage()
{
	QLocalSocket *localSocket = mLocalServer->nextPendingConnection();

	if (!localSocket->waitForReadyRead(mTimeout))
	{
		qDebug() << localSocket->errorString().toLatin1();
		return;
	}
	QByteArray byteArray = localSocket->readAll();
	QString message = QString::fromUtf8(byteArray.constData());

	gMainWindow->openSingleFile(Location(message));

	localSocket->disconnectFromServer();
}

bool PonyEdit::isRunning()
{
	return mIsRunning;
}

bool PonyEdit::sendMessage(const QString &message)
{
	if (!mIsRunning)
		return false;

	QLocalSocket localSocket(this);
	localSocket.connectToServer(mKey, QIODevice::WriteOnly);

	//QMessageBox::critical(0, "Sending", message);

	if (!localSocket.waitForConnected(mTimeout))
	{
		qDebug() << localSocket.errorString().toLatin1();
		return false;
	}

	localSocket.write(message.toUtf8());

	if (!localSocket.waitForBytesWritten(mTimeout))
	{
		qDebug() << localSocket.errorString().toLatin1();
		return false;
	}

	localSocket.disconnectFromServer();

	return true;
}
