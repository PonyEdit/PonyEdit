HIDE_COMPILE_WARNINGS

#include <QEvent>
#include <QFileOpenEvent>
#include <QDebug>
#include <QLocalSocket>
#include <QMessageBox>
#include <QCommandLineParser>

UNHIDE_COMPILE_WARNINGS

#include "global.h"
#include "ponyedit.h"
#include "file/location.h"
#include "mainwindow.h"
#include "main/globaldispatcher.h"
#include "file/location.h"
#include "main/tools.h"
#include "main/mainwindow.h"
#include "website/sitemanager.h"
#include "syntax/syntaxrule.h"
#include "syntax/syntaxdefmanager.h"
#include "main/stringtrie.h"
#include "options/options.h"
#include "ssh2/slavechannel.h"
#include "ssh2/xferrequest.h"
#include "ssh2/sshhost.h"
#include "QsLog.h"

GlobalDispatcher* gDispatcher = NULL;
SiteManager* gSiteManager = NULL;
SyntaxDefManager* gSyntaxDefManager = NULL;
MainWindow* gMainWindow = NULL;
bool PonyEdit::sApplicationExiting = false;

PonyEdit::PonyEdit(int argc, char** argv) : QApplication(argc, argv),
    mIsRunning(),
    // UUID used to (hopefully) ensure memory name is unique
#ifdef QT_DEBUG
	mKey("PonyEdit-debug-lock-138ad7e0-2ecb-11e0-91fa-0800200c9a66"),
#else
	mKey("PonyEdit-lock-138ad7e0-2ecb-11e0-91fa-0800200c9a66"),
#endif
    mMemoryLock(mKey),
    mLocalServer(NULL),
    mDialogRethreader(NULL),
    mPositionalArguments(NULL)
{
	//	Parse command line arguments
	QCommandLineParser parser;
	parser.setApplicationDescription( "PonyEdit: The fastest remote text editor under the sun. Or over it. " );
	parser.addHelpOption();
	parser.addVersionOption();
	QCommandLineOption resourceLocationParam = QCommandLineOption(
		QStringList() << "r" << "resource-location",
		"Root directory of application resources. This directory should include syntaxdefs/ and slave/ subdirs.",
		"directory"
	);
	parser.addOption( resourceLocationParam );
	parser.process( *this );

	mPositionalArguments = parser.positionalArguments();
	QString resourceLocation = parser.value( resourceLocationParam );
	if ( ! resourceLocation.isEmpty() )
		Tools::setResourcePath( resourceLocation );

#ifndef Q_OS_LINUX
	QStringList substitutions;
	substitutions.append("consolas");
	substitutions.append("courier new");
	substitutions.append("helvetica");
	QFont::insertSubstitutions("inconsolata", substitutions);
#endif

	qRegisterMetaType<Location>("Location");
	qRegisterMetaType< QList<Location> >("QList<Location>");

	SlaveChannel::initialize();
	qRegisterMetaType<XferRequest*>("XferRequest*");

	Tools::loadServers();
	Location::loadFavorites();
	Tools::initialize();

	// In case PonyEdit crashed last run, attach() and detach(), to force the
	// system to recognise that nothing is connected to this shared memory.
	mMemoryLock.attach();
	mMemoryLock.detach();

	if(mMemoryLock.attach())
		mIsRunning = true;
	else
	{
		mIsRunning = false;
		// create shared memory.
		if (!mMemoryLock.create(1))
		{
			QLOG_ERROR() << "Failed to create shared memory lock";
			return;
		}

		// create local server and listen to incomming messages from other instances.
		mLocalServer = new QLocalServer(this);
		connect(mLocalServer, SIGNAL(newConnection()), this, SLOT(receiveMessage()));
		mLocalServer->listen(mKey);

		Options::load();

		gSyntaxDefManager = new SyntaxDefManager();
		gSiteManager = new SiteManager();
		gDispatcher = new GlobalDispatcher();
		mDialogRethreader = new DialogRethreader();

		gMainWindow = new MainWindow();
		gMainWindow->show();

		QTimer::singleShot( 1, this, SLOT( loadStartupFiles() ) );
	}
}

PonyEdit::~PonyEdit()
{
	sApplicationExiting = true;

	SshHost::cleanup();

	delete gDispatcher;
	delete gSiteManager;
	delete gSyntaxDefManager;
	delete gMainWindow;
	delete mDialogRethreader;

	Options::save();
	LocationShared::cleanupIconProvider();
	StringTrie::cleanup();
	SyntaxRule::cleanup();
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
		QLOG_ERROR() << "Failed to read local socket: " << localSocket->errorString();
		return;
	}
	QByteArray byteArray = localSocket->readAll();
	QString message = QString::fromUtf8(byteArray.constData());

	gMainWindow->openSingleFile(Location(message));

	localSocket->disconnectFromServer();
}

bool PonyEdit::notify(QObject *o, QEvent *e)
{
	bool ret = false;
	try
	{
		ret = QApplication::notify(o, e);
	}
	catch(QString err)
	{
		QLOG_ERROR() << "UNCAUGHT EXCEPTION:" << err;
	}
	catch(...)
	{
		QLOG_ERROR() << "UNKNOWN UNCAUGHT EXCEPTION";
	}

	return ret;
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
		QLOG_ERROR() << "Failed to connect on local socket: " << localSocket.errorString();
		return false;
	}

	localSocket.write(message.toUtf8());

	if (!localSocket.waitForBytesWritten(mTimeout))
	{
		QLOG_ERROR() << "Failed to write to local socket: " << localSocket.errorString();
		return false;
	}

	localSocket.disconnectFromServer();

	return true;
}
