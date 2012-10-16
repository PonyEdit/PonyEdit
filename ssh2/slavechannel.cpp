#include "slavechannel.h"
#include "sshhost.h"
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>
#include <libssh2.h>
#include "tools/json.h"
#include "slaverequest.h"
#include "main/tools.h"
#include "file/slavefile.h"
#include "dialogrethreader.h"

#define SLAVE_INIT	" cd ~;"\
					"if type perl >/dev/null 2>&1;then "\
						"perl -e '"\
							"use Digest::MD5;"\
							"my $f=\".ponyedit/slave.pl\";"\
							"die \"Old Perl $^V\\n\" if($^V lt v5.8.1);"\
							"die \"No Slave\\n\" if(!-e $f);"\
							"$d=Digest::MD5->new;"\
							"open F,$f;"\
							"$d->addfile(F);"\
							"die \"Slave checksum fail\\n\" if($d->hexdigest ne \"[[CHECKSUM]]\");"\
							"exec \"[[SLAVE_RUN]]\";"\
						"';"\
					"else "\
						"echo 'No Perl';"\
					"fi\n"


#define SLAVE_START_UPLOAD	" perl -e '"\
								"mkdir \".ponyedit\" unless(-d \".ponyedit\");"\
								"open F,\">.ponyedit/slave.pl\" or die \"Slave write error\\n\";"\
								"print \"uploader ready\";"\
								"while(1)"\
								"{"\
									"my $line=<STDIN>;"\
									"last if($line=~/^END_OF_SLAVE/);"\
									"print F $line;"\
								"}"\
								"close F;"\
							"'&&"\
							"[[SLAVE_RUN]]\n"

/*
#define SLAVE_START_UPLOAD	" perl -e '"\
								"use List::Util qw(min);"\
								"my $f=\".ponyedit/slave.pl\",$s=[[SLAVE_SIZE]],$d;"\
								"mkdir \".ponyedit\" unless(-d \".ponyedit\");"\
								"open F,\">$f\" or die \"Slave write error\\n\";"\
								"print \"uploader ready\";"\
								"while($s > 0)"\
								"{"\
									"my $w=min $s,2048;"\
									"my $r=read STDIN,$d,$w;"\
									"die \"Slave upload error\\n\" if($r<$w);"\
									"$s-=$r;"\
									"print F $d;"\
									"}"\
								"close F;"\
							"'&&"\
							"[[SLAVE_RUN]]\n"
*/

QByteArray SlaveChannel::sSlaveChannelInit(SLAVE_INIT);
QByteArray SlaveChannel::sSlaveScript;
QByteArray SlaveChannel::sSlaveUpload;

SlaveChannel::SlaveChannel(SshHost* host, bool sudo) : ShellChannel(host), mInternalStatus(_WaitingForShell), mCurrentRequest(0), mNextMessageId(1)
{
	mSudo = sudo;
	SSHLOG_TRACE(host) << "Creating a new slave channel";
}

SlaveChannel::~SlaveChannel()
{
	SSHLOG_TRACE(mHost) << "Slave channel deleted";

	//	Inform the files that depend on this slave channel before passing the critical error
	emit channelShutdown();
}

#include <QTime>

void SlaveChannel::initialize()
{
	//	TODO: Use Tools::getResourcePath
	QFile f(Tools::getResourcePath("slave/slave.pl"));
    QLOG_INFO() << f.fileName();
	if (!f.open(QFile::ReadOnly))
		throw(tr("Unable to find slave script!"));
	sSlaveScript = f.readAll();

	QCryptographicHash hash(QCryptographicHash::Md5);
	hash.addData(sSlaveScript);
	QByteArray checksum = hash.result().toHex().toLower();

	sSlaveChannelInit.replace("[[CHECKSUM]]", checksum);
	sSlaveUpload = SLAVE_START_UPLOAD;// + sSlaveScript;
}

bool SlaveChannel::update()
{
	switch (mStatus)
	{
		case Opening:
			return handleOpening();

		case Open:
			return mainUpdate();

		case Disconnected:
			return false;

		default:;
	}

	return false;
}

void SlaveChannel::shellReady()
{
	setInternalStatus(_CheckingSlave);
}

bool SlaveChannel::handleOpening()
{
	//	Pass control to the shell channel until it's ready.
	if (mInternalStatus == _WaitingForShell)
		return ShellChannel::handleOpening();

	if (mInternalStatus == _CheckingSlave)
	{
		if (mHost->waitBeforeCheckingSlave(this))
			return true;

		QByteArray slaveInit = sSlaveChannelInit;
		slaveInit.replace("[[SLAVE_RUN]]", getSlaveRun(mSudo));

		SendResponse r = sendData(slaveInit);
		if (r == SendAgain) return true;
		if (r != SendSucceed) criticalError("Failed to send slave script initializer");

		setInternalStatus(_CheckingSlaveResponse);
	}

	if (mInternalStatus == _CheckingSlaveResponse)
	{
		ReadReply reply = readUntilPrompt();
		if (reply.readAgain)
			return true;

		if (reply.data.isNull())
			return false;

		QByteArray response = reply.data.trimmed();
		if (response.length() == 0)
			return true;	//	Try again

		if (response.startsWith("No Perl"))
		{
			criticalError("No Perl found on the remote server!");
			return false;
		}

		if (response.startsWith("Old Perl"))
		{
			criticalError("PonyEdit requires at least Perl 5.8. Found: " + response.mid(9));
			return false;
		}

		if (response.startsWith("Slave OK") || response.contains("Sudo-prompt"))
		{
			finalizeSlaveInit(response);
			return true;
		}

		if (response.startsWith("No Slave") || response.startsWith("Slave checksum fail"))
			setInternalStatus(_StartingSlaveUploader);
		else
		{
			criticalError("Unexpected response to slave initialization");
			return false;
		}
	}

	if (mInternalStatus == _StartingSlaveUploader)
	{
		QByteArray slaveUpload = sSlaveUpload;
		slaveUpload.replace("[[SLAVE_RUN]]", getSlaveRun(false));

		SendResponse r = sendData(slaveUpload);
		if (r == SendAgain) return true;
		if (r != SendSucceed) criticalError("Failed to start slave uploader");

		setInternalStatus(_WaitingForSlaveUploader);
	}

	if (mInternalStatus == _WaitingForSlaveUploader)
	{
		ReadReply rr = readUntil("uploader ready");
		if (rr.readAgain)
			return true;

		setInternalStatus(_UploadingSlaveScript);
	}

	if (mInternalStatus == _UploadingSlaveScript)
	{
		SendResponse r = sendData(sSlaveScript + QByteArray("END_OF_SLAVE\n"));
		if (r == SendAgain) return true;
		if (r != SendSucceed) criticalError("Failed to upload slave script");

		setInternalStatus(_WaitingForSlaveUploadResponse);
	}

	if (mInternalStatus == _WaitingForSlaveUploadResponse)
	{
		ReadReply reply = readUntilPrompt();
		if (reply.readAgain)
			return true;

		if (reply.data.isNull())
			return false;

		QByteArray response = reply.data.trimmed();
		if (response.length() == 0)
			return true;	//	Try again

		if (response.contains("Sudo fail"))
		{
			criticalError("Failed to sudo");
			return false;
		}

		if (response.startsWith("Slave OK") || response.contains("Sudo-prompt"))
		{
			finalizeSlaveInit(response);
			return true;
		}
		else
		{
			criticalError("Unexpected response to slave initialization: " + response);
			return false;
		}
	}

	if (mInternalStatus == _SendingSudoPassword)
	{
		if (mTriedSudoPassword)
		{
			//	Need to ask the user for a new sudo password
			QVariantMap options;
			options.insert("title", QObject::tr("%1 Sudo Password").arg(mHost->getName()));
			options.insert("blurb", QObject::tr("Please enter your sudo password for %1 below.").arg(mHost->getName()));
			options.insert("memorable", false);

			QVariantMap result = DialogRethreader::rethreadDialog<PasswordDlg>(options);
			if (!result.value("accepted").toBool())
			{
				criticalError("Failed to sudo");
				return false;
			}

			mSudoPasswordAttempt = result.value("password").toByteArray();
			mTriedSudoPassword = false;
		}

		//	Send the password
		SendResponse r = sendData(mSudoPasswordAttempt + "\n");
		if (r == SendAgain) return true;
		if (r != SendSucceed) criticalError("Failed to send sudo password");

		mTriedSudoPassword = true;
		mHost->setSudoPassword(mSudoPasswordAttempt);
		setInternalStatus(_WaitingForSlaveUploadResponse);
	}

	return true;
}

void SlaveChannel::criticalError(const QString& error)
{
	//	Fail the current job (if there is one)
	if (mCurrentRequest)
	{
		mCurrentRequest->failRequest(error, SlaveRequest::ConnectionError);
		delete mCurrentRequest;
		mCurrentRequest = NULL;
	}

	SshChannel::criticalError(error);
}

bool SlaveChannel::mainUpdate()
{
	int rc;

	//	Read as much as there is to be read
	ReadReply rr;
	do
	{
		rr = readUntil("\n");
		if (!rr.readAgain)
		{
			if (rr.data.length())
			{
				SSHLOG_TRACE(mHost) << "Received: " << rr.data;

				//	We have a message! Decode it.
				bool ok;
				QVariantMap response = Json::parse(rr.data, ok).toMap();
				if (int responseId = response.value("i", 0).toInt())
				{
					//	Look up the request that this response relates to
					SlaveRequest* request = mRequestsAwaitingReplies.value(responseId, NULL);
					if (request != NULL)
					{
						//	If the request was opening a file, and a bufferId is returned, record the relationship
						if (request->getOpeningFile() && response.contains("bufferId"))
						{
							mBufferIds.insert(request->getOpeningFile(), response.value("bufferId").toInt());
							connect(this, SIGNAL(channelShutdown()), request->getOpeningFile(), SLOT(slaveChannelFailure()), Qt::QueuedConnection);
						}

						//	Handle the response.
						SlaveRequest* request = mRequestsAwaitingReplies.value(responseId, NULL);
						if (request != NULL)
							request->handleReply(response);
					}
					else
						//	This response has an id, but no related request. This should not happen. TODO: Log this.
						SSHLOG_ERROR(mHost) << "Received slave response, found no corresponding request";
				}
				else
				{
					//	Got an unsolicted message. Tell the host.
					mHost->handleUnsolicitedSlaveMessage(response);
				}
			}
		}
	} while (!rr.readAgain && mStatus == Open);

	//	If attempting to read resulted in a catastrophic failure, abandon ship!
	if (mStatus != Open)
		return false;

	//	Check if there's requests to be made
	if (mInternalStatus == _WaitingForRequests)
	{
		mCurrentRequest = mHost->getNextSlaveRequest(mSudo, mBufferIds);
		if (mCurrentRequest)
		{
			mCurrentRequest->setMessageId(mNextMessageId++);
			mInternalStatus = _SendingRequest;

			//	If this new request is closing a file, remove it from my record of bufferIds.
			if (mCurrentRequest->getRequest() == "close")
			{
				disconnect(mCurrentRequest->getFile());
				mBufferIds.remove(mCurrentRequest->getFile());
			}
		}
		else
			return false;
	}

	//	Make requests if there are any to make.
	if (mInternalStatus == _SendingRequest)
	{
		const QByteArray& packedRequest = mCurrentRequest->getPackedRequest(mBufferIds.value(mCurrentRequest->getFile(), -1));
		rc = libssh2_channel_write(mHandle, packedRequest, packedRequest.length());
		if (rc < 0)
		{
			if (rc == -1) rc = libssh2_session_last_errno(mSession->sessionHandle());
			if (rc == LIBSSH2_ERROR_EAGAIN)
				return true;
			criticalError(tr("Failed to initialize send a slave request: %1").arg(rc));
			return false;
		}

		SSHLOG_TRACE(mHost) << "Sent: " << packedRequest;
		mRequestsAwaitingReplies.insert(mCurrentRequest->getMessageId(), mCurrentRequest);

		mCurrentRequest = NULL;
		setInternalStatus(_WaitingForRequests);
	}

	return true;
}

void SlaveChannel::finalizeSlaveInit(const QByteArray& initString)
{
	if (initString.contains("Sudo-prompt"))
	{
		mSudoPasswordAttempt = mHost->getSudoPassword();
		mTriedSudoPassword = mSudoPasswordAttempt.isNull();

		mInternalStatus = _SendingSudoPassword;
		return;
	}

	QList<QByteArray> lines = initString.split('\n');

	if (lines.length() < 2) criticalError("Invalid slave script init");

	bool ok;
	QVariantMap initBlob = Json::parse(QString(lines[1]), ok).toMap();
	if (!ok) criticalError("JSON initialization blob invalid");

	QByteArray homeDir = initBlob.value("~").toByteArray();
	SSHLOG_INFO(mHost) << "Home directory: " << homeDir;
	mHost->setHomeDirectory(homeDir);

	setInternalStatus(_WaitingForRequests);

	mHost->firstSlaveCheckComplete();
	setStatus(Open);
}

int SlaveChannel::getConnectionScore()
{
	if (mStatus == Opening)
		return mInternalStatus;
	else
		return ShellChannel::getConnectionScore();
}

QString SlaveChannel::getConnectionDescription()
{
	if (mStatus == Opening)
	{
		switch (mInternalStatus)
		{
		case _CheckingSlave:
		case _CheckingSlaveResponse:
			return tr("Checking slave");

		case _StartingSlaveUploader:
		case _WaitingForSlaveUploader:
		case _UploadingSlaveScript:
		case _WaitingForSlaveUploadResponse:
			return tr("Updating slave");

		case _SendingSudoPassword:
			return tr("Requesting sudo");

		default:;
		}
	}

	return ShellChannel::getConnectionDescription();
}

void SlaveChannel::setInternalStatus(InternalStatus newStatus)
{
	if (newStatus != mInternalStatus)
	{
		mInternalStatus = newStatus;
		mHost->invalidateOverallStatus();
	}
}

QByteArray SlaveChannel::getSlaveRun(bool sudo)
{
	if (sudo) mSudoPasswordAttempt = mHost->getSudoPassword();
	return sudo ? "sudo -p Sudo-prompt%-ponyedit-% perl .ponyedit/slave.pl" : "perl .ponyedit/slave.pl";
}

