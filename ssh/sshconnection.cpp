#include "sshconnection.h"

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
#include "sshhost.h"
#include "SlaveRequest.h"
#include "file/slavefile.h"
#include "connectionstatuswidget.h"
#include "passwordinput.h"
#include "main/globaldispatcher.h"
#include "slavechannel.h"
#include "main/global.h"

#define	KEEPALIVE_TIMEOUT 60000	/* 60 seconds */

#define REMOTE_GETSLAVEMD5 "if [ ! -d ~/.ponyedit ]; then mkdir ~/.ponyedit; fi; " \
	"if [ -e ~/.ponyedit/slave.pl ]; then md5sum ~/.ponyedit/slave.pl; else echo x; fi\n"

class RawSshChannelHandle : public RawChannelHandle
{
public:
	RawSshChannelHandle(RawSshConnection::Channel* channel) : Channel(channel) {}
	RawSshConnection::Channel* Channel;
};

QMutex SshConnection::sSlaveScriptMutex;
bool SshConnection::sSlaveLoaded;
QByteArray SshConnection::sSlaveScript;
QByteArray SshConnection::sSlaveMd5;

/*****************************************************************************/

SshConnection::SshConnection(SshHost* host)
{
	mHost = host;
	mRawConnection = new RawSshConnection();
	startConnectionThread();
}

SshConnection::~SshConnection()
{
}

bool SshConnection::threadConnect()
{
	try
	{
		//
		//	Basic connection
		//

		qDebug() << "Connecting...";

		mRawConnection->connect(mHost->getHostName().toUtf8(), mHost->getPort());
		if (isDeliberatelyDisconnecting()) return false;

		qDebug() << "Checking server fingerprint...";

		//	Check the server's fingerprint
		QByteArray fingerprint = mRawConnection->getServerFingerprint();
		QByteArray knownFingerprint = mRawConnection->getExpectedFingerprint(mHost->getHostName());
		if (knownFingerprint != fingerprint)
			waitForInput(hostkeyWarnDialog, hostkeyWarnCallback, QVariant(knownFingerprint.isEmpty()));
		if (isDeliberatelyDisconnecting()) return false;

		//
		//	Handle authentication
		//

		qDebug() << "Handling authentication...";

		setStatus(Authenticating);
		RawSshConnection::AuthMethods authMethods = mRawConnection->getAuthenticationMethods(mHost->getUserName().toUtf8());
		bool authenticated = false;

		//	Try key authentication first if available
		if (authMethods & RawSshConnection::PublicKey)
		{
			//	Try to see if there is an agent available first...
			authenticated = mRawConnection->authenticateAgent(mHost->getUserName().toUtf8());

			//	If no agent is available, but a keyfile was specified, try that...
			if (!authenticated && !mHost->getKeyFile().isEmpty())
			{
				bool passkeyRejected = true;
				while (passkeyRejected)
				{
					authenticated = mRawConnection->authenticateKeyFile(mHost->getKeyFile().toUtf8(), mHost->getUserName().toUtf8(), mHost->getKeyPassphrase().toUtf8(), &passkeyRejected);

					if (passkeyRejected)
						waitForInput(SshConnection::passwordInputDialog, SshConnection::passwordInputCallback, QVariant(true));
				}
			}
		}

		//	Fall back on password entry
		while (!authenticated)
		{
			waitForInput(SshConnection::passwordInputDialog, SshConnection::passwordInputCallback, QVariant(false));

			if (isDeliberatelyDisconnecting()) return false;
			authenticated = mRawConnection->authenticatePassword(mHost->getUserName().toUtf8(), mHost->getPassword().toUtf8());

			if(!authenticated)
				mHost->setPassword(QString());
		}
		if (isDeliberatelyDisconnecting()) return false;

		return true;
	}
	catch (QString err)
	{
		qDebug() << "Error while connecting: " << err;

		delete mRawConnection;
		mRawConnection = NULL;

		setErrorStatus(QString("Error while ") + getStatusString() + ": " + err);
		return false;
	}
}

RemoteChannel* SshConnection::threadOpenPrimaryChannel()
{
	return threadOpenSlaveChannel();
}

void SshConnection::loadSlaveScript()
{
	sSlaveScriptMutex.lock();
	if (!sSlaveLoaded)
	{
#ifdef Q_OS_MAC
		QFile f(QCoreApplication::applicationDirPath() + QString("/../Resources/slave/slave.pl"));
#else
		QFile f("slave/slave.pl");
#endif

		f.open(QFile::ReadOnly);
		sSlaveScript = f.readAll();

		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(sSlaveScript);
		sSlaveMd5 = hash.result().toHex().toLower();

		sSlaveLoaded = true;
	}
	sSlaveScriptMutex.unlock();
}

QString SshConnection::getName()
{
	return mHost->getName();
}

RawChannelHandle* SshConnection::createRawSlaveChannel()
{
	RawSshConnection::Channel* rawChannel = mRawConnection->createShellChannel();

	//	Switch to the remote home directory
	mRawConnection->execute(rawChannel, ntr("cd ~\n"));

	//	Check that Perl is installed and a recent version
	bool validPerl = false;
	QByteArray perlVersion = mRawConnection->execute(rawChannel, "perl -v\n");
	if (perlVersion.length() > 0)
	{
		QRegExp perlVersionRx("This is perl, v(\\d+)\\.(\\d+)");
		if(perlVersionRx.indexIn(perlVersion) > -1)
		{
			QString perlMajorVersionNumber = perlVersionRx.cap(1);
			QString perlMinorVersionNumber = perlVersionRx.cap(2);
			if (perlMajorVersionNumber.toInt() >= 5 && perlMinorVersionNumber.toInt() > 6)
				validPerl = true;
		}
	}
	if (!validPerl) throw(tr("No usable version of Perl found!"));

	//	Make sure the remote slave script is present, and the MD5 hashes match. If not, upload again.
	loadSlaveScript();
	QByteArray remoteMd5 = mRawConnection->execute(rawChannel, REMOTE_GETSLAVEMD5).toLower();
	remoteMd5.truncate(32);
	if (remoteMd5 != sSlaveMd5)
		mRawConnection->writeFile("~/.ponyedit/slave.pl", sSlaveScript.constData(), sSlaveScript.length());

	//	Run the remote slave script
	const char* slaveStarter = "perl .ponyedit/slave.pl\n";
	mRawConnection->writeData(rawChannel, slaveStarter, strlen(slaveStarter));

	//	First line returned from the slave script should be the user's home dir.
	QString homeDirectory = mRawConnection->readLine(rawChannel).trimmed();
	if (homeDirectory.startsWith("~="))
	{
		if (mHomeDirectory.isEmpty())
		{
			homeDirectory = homeDirectory.mid(2);
			if (homeDirectory.endsWith('/'))
				homeDirectory.truncate(homeDirectory.length() - 1);
			mHomeDirectory = homeDirectory.trimmed();
		}
	}
	else
		throw(tr("Failed to start slave script!"));

	return new RawSshChannelHandle(rawChannel);
}

void SshConnection::hostkeyWarnDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target, QVariant param)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);
	bool newKey = param.toBool();

	if (newKey)
		widget->setManualStatus(tr("Verify Host Fingerprint"), QPixmap(":/icons/question.png"));
	else
		widget->setManualStatus(tr("Host Fingerprint Changed!"), QPixmap(":/icons/warning.png"));
	widget->addButton(QDialogButtonBox::YesRole, tr("Connect"));

	QVBoxLayout* layout = new QVBoxLayout(target);
	QString text = newKey ?
				tr("There is no fingerprint on file for this host. If this is the first time you are connecting to this server, it is safe "
				  "to ignore this warning.\n\n Host fingerprints provide extra security by verifying that you are connecting to the server "
				  "that you think you are. The first time you connect to a new server, there will be no fingerprint on file. You will be "
				  "warned whenever you connect to a server if the host fingerprint has changed. ")
				:
				tr("The fingerprint returned by the host, and the one on file do not match! This may be caused by a breech of security, or "
				   "by server reconfiguration. Please check with your host administrator before accepting the changed host key!");

	text += "\n\nFingerprint: " + sshConnection->mRawConnection->getServerFingerprint().toHex();

	QLabel* question = new QLabel(text, target);
	question->setWordWrap(true);
	layout->addWidget(question);
}

bool SshConnection::hostkeyWarnCallback(ConnectionStatusWidget*, RemoteConnection* connection, QDialogButtonBox::ButtonRole, QVariant param)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);
	RawSshConnection::saveFingerprint(sshConnection->mHost->getHostName(), sshConnection->mRawConnection->getServerFingerprint());
	return true;
}

void SshConnection::passwordInputDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target, QVariant param)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);
	bool keyPassphrase = param.toBool();

	widget->addButton(QDialogButtonBox::YesRole, tr("Connect"));
	widget->setManualStatus(keyPassphrase ? tr("Please enter your key passphrase") : tr("Please enter your password"), QPixmap(":/icons/question.png"));

	QVBoxLayout* layout = new QVBoxLayout(target);
	sshConnection->mPasswordInput = new PasswordInput(target);
	layout->addWidget(sshConnection->mPasswordInput);

	sshConnection->mPasswordInput->setSavePassword(keyPassphrase ?
		sshConnection->mHost->getSaveKeyPassphrase() :
		sshConnection->mHost->getSavePassword());
}

bool SshConnection::passwordInputCallback(ConnectionStatusWidget*, RemoteConnection* connection, QDialogButtonBox::ButtonRole, QVariant param)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);

	SshHost* host = sshConnection->mHost;
	if (param.toBool())
	{
		host->setKeyPassphrase(sshConnection->mPasswordInput->getEnteredPassword());
		host->setSaveKeyPassphrase(sshConnection->mPasswordInput->getSavePassword());
	}
	else
	{
		host->setPassword(sshConnection->mPasswordInput->getEnteredPassword());
		host->setSavePassword(sshConnection->mPasswordInput->getSavePassword());
	}

	return true;
}

void SshConnection::sendLine(RawChannelHandle* handle, const QByteArray& data)
{
	mRawConnection->writeData(static_cast<RawSshChannelHandle*>(handle)->Channel, data.constData(), data.length());
}

QByteArray SshConnection::readLine(RawChannelHandle* handle)
{
	return mRawConnection->readLine(static_cast<RawSshChannelHandle*>(handle)->Channel);
}

