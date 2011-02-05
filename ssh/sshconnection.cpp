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
#include "slaverequest.h"
#include "file/slavefile.h"
#include "connectionstatuswidget.h"
#include "passwordinput.h"
#include "main/globaldispatcher.h"
#include "slavechannel.h"
#include "main/global.h"
#include "main/tools.h"

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
						waitForInput(SshConnection::passwordInputDialog, SshConnection::passwordInputCallback, QVariant(KeyPassphrase));
				}
			}
		}

		//	Fall back on password entry
		while (!authenticated)
		{
			waitForInput(SshConnection::passwordInputDialog, SshConnection::passwordInputCallback, QVariant(SshPassword));

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

		setErrorStatus(err);
		return false;
	}
}

RemoteChannel* SshConnection::threadOpenPrimaryChannel()
{
	return new SlaveChannel(this, false);
}

RemoteChannel* SshConnection::openChannel(RemoteChannel::Type type)
{
	RemoteChannel::Type baseType = static_cast<RemoteChannel::Type>(type & RemoteChannel::BaseTypeMask);
	bool sudo = type & RemoteChannel::Sudo;

	if (baseType != RemoteChannel::Slave)
		throw(tr("Invalid operation: attempting to open an invalid channel via ssh"));

	RemoteChannel* channel = new SlaveChannel(this, sudo);

	bool ok = channel->waitUntilOpen(mConnectionId);
	if (!ok)
	{
		delete channel;
		return NULL;
	}

	return channel;
}

void SshConnection::loadSlaveScript()
{
	sSlaveScriptMutex.lock();
	if (!sSlaveLoaded)
	{
		QFile f(Tools::getResourcePath("slave/slave.pl"));
		if (!f.open(QFile::ReadOnly))
			throw(tr("Unable to find slave script!"));
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

RawChannelHandle* SshConnection::createRawSlaveChannel(bool sudo)
{
	qDebug() << "Opening raw channel...";
	RawSshConnection::Channel* rawChannel = mRawConnection->createShellChannel();

	//	Switch to the remote home directory
	qDebug() << "Switching to home dir...";
	mRawConnection->execute(rawChannel, ntr("cd ~\n"));

	//	Check that Perl is installed and a recent version
	bool validPerl = false;
	qDebug() << "Checking Perl version...";
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
	qDebug() << "Running remote md5sum...";
	QByteArray remoteMd5 = mRawConnection->execute(rawChannel, REMOTE_GETSLAVEMD5).toLower();
	remoteMd5.truncate(32);
	if (remoteMd5 != sSlaveMd5)
		mRawConnection->writeFile(".ponyedit/slave.pl", sSlaveScript.constData(), sSlaveScript.length());

	//	If this is a sudo connection, sudo at this point.
	QByteArray firstLine;
	if (sudo)
	{
		//	Run the remote slave script inside sudo
		const char* slaveStarter = "sudo -k -p -sudo-prompt%% perl .ponyedit/slave.pl\n";
		qDebug() << "Sending sudo command...";
		mRawConnection->writeData(rawChannel, slaveStarter, strlen(slaveStarter));

		//	Try passwords
		bool firstTry = true;
		while (1)
		{
			//	Check that we haven't run out of tries
			qDebug() << "Looking for sudo prompt...";
			QByteArray reply = mRawConnection->readUntil(rawChannel, "%").trimmed();
			if (reply.startsWith("~="))
			{
				firstLine = reply;
				break;	//	Accepted!
			}
			if (!reply.endsWith("-sudo-prompt"))
				throw(tr("Failed to execute remote sudo command!"));

			if (!firstTry)
				mHost->setSudoPassword("");

			//	Ask the user for a password to use
			if (mHost->getSudoPassword().isEmpty())
				waitForInput(passwordInputDialog, passwordInputCallback, QVariant(SudoPassword));
			if (mHost->getSudoPassword().isEmpty())
				throw(tr("Sudo cancelled"));

			//	Try the password
			QString pass = mHost->getSudoPassword() + "\n";
			qDebug() << "Sending sudo password...";
			mRawConnection->writeData(rawChannel, pass.toUtf8(), pass.length());

			firstTry = false;
		}
	}
	else
	{
		//	Run the remote slave script
		const char* slaveStarter = "perl .ponyedit/slave.pl\n";
		mRawConnection->writeData(rawChannel, slaveStarter, strlen(slaveStarter));
		firstLine = mRawConnection->readUntil(rawChannel, "%").trimmed();
	}

	//	First line returned from the slave script should be the user's home dir.
	QString homeDirectory = firstLine.trimmed();
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

	qDebug() << "Channel opened!";

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

bool SshConnection::hostkeyWarnCallback(ConnectionStatusWidget*, RemoteConnection* connection, QDialogButtonBox::ButtonRole, QVariant /* param */)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);
	RawSshConnection::saveFingerprint(sshConnection->mHost->getHostName(), sshConnection->mRawConnection->getServerFingerprint());
	return true;
}

void SshConnection::passwordInputDialog(ConnectionStatusWidget* widget, RemoteConnection* connection, QWidget* target, QVariant param)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);
	PasswordType passwordType = static_cast<PasswordType>(param.toInt());

	QString query;
	bool savePassword = false;
	switch (passwordType)
	{
	case SshPassword:
		query = tr("Please enter your password");
		savePassword = sshConnection->mHost->getSavePassword();
		break;

	case KeyPassphrase:
		query = tr("Please enter your key passphrase");
		savePassword = sshConnection->mHost->getSaveKeyPassphrase();
		break;

	case SudoPassword:
		query = tr("Please enter your sudo password");
		savePassword = false;
		break;
	}

	widget->addButton(QDialogButtonBox::YesRole, tr("Connect"));
	widget->setManualStatus(query, QPixmap(":/icons/question.png"));

	QVBoxLayout* layout = new QVBoxLayout(target);
	sshConnection->mPasswordInput = new PasswordInput(target);
	layout->addWidget(sshConnection->mPasswordInput);

	sshConnection->mPasswordInput->setSavePassword(savePassword);
}

bool SshConnection::passwordInputCallback(ConnectionStatusWidget*, RemoteConnection* connection, QDialogButtonBox::ButtonRole, QVariant param)
{
	SshConnection* sshConnection = static_cast<SshConnection*>(connection);
	PasswordType passwordType = static_cast<PasswordType>(param.toInt());
	SshHost* host = sshConnection->mHost;

	switch (passwordType)
	{
	case SshPassword:
		host->setPassword(sshConnection->mPasswordInput->getEnteredPassword());
		host->setSavePassword(sshConnection->mPasswordInput->getSavePassword());
		break;

	case KeyPassphrase:
		host->setKeyPassphrase(sshConnection->mPasswordInput->getEnteredPassword());
		host->setSaveKeyPassphrase(sshConnection->mPasswordInput->getSavePassword());
		break;

	case SudoPassword:
		host->setSudoPassword(sshConnection->mPasswordInput->getEnteredPassword());
		break;
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

