#ifndef SSHHOST_H
#define SSHHOST_H

#include "sshchannel.h"
#include "sshsession.h"
#include "file/location.h"
#include <QString>
#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QVariant>
#include <QStringList>
#include "QsLog.h"
#include "hostlog.h"
#include <QPointer>

//	Configuration
#define CHANNEL_LIMIT_GUESS 10		/* Guess at the max number of channels per connection */
#define SLAVE_CHANNEL_QUEUE_MULTIPLIER 3	/* When slave-channel:slave-queue-length ratio is off by at least this multiplier, think about opening additional slave channels */
#define MAX_SLAVE_CHANNELS 3		/* Maximum number of slave channels per host */
#define XFER_CHANNEL_QUEUE_MULTIPLIER 2
#define MAX_XFER_CHANNELS 7
#define MAX_SFTP_CHANNELS 5
#define SFTP_CHANNEL_QUEUE_MULTIPLIER 2

//	Host-specific tracing macros
#define SSHLOG_TRACE(h) _SSHLOG_IF(h, QsLogging::TraceLevel)
#define SSHLOG_DEBUG(h) _SSHLOG_IF(h, QsLogging::DebugLevel)
#define SSHLOG_INFO(h)  _SSHLOG_IF(h, QsLogging::InfoLevel)
#define SSHLOG_WARN(h)  _SSHLOG_IF(h, QsLogging::WarnLevel)
#define SSHLOG_ERROR(h) _SSHLOG_IF(h, QsLogging::ErrorLevel)
#define SSHLOG_FATAL(h) _SSHLOG_IF(h, QsLogging::FatalLevel)
#define _SSHLOG_IF(h,l) if(QsLogging::Logger::instance().loggingLevel() > l) {} else h->log(l).stream()

class SlaveFile;
class SlaveRequest;
class SFTPRequest;
class SlaveChannel;
class XferRequest;
class XferChannel;
class SshHost : public QObject
{
	Q_OBJECT

public:
	enum Status { Connected, Disconnected, Connecting };
	enum ConnectionType { SSH, SFTP };
	struct StatusReport
	{
		Status overallStatus;
		QString connectingStage;
	};

	SshHost();
	~SshHost();
	static void cleanup();

	static SshHost* getHost(const QByteArray& hostname, const QByteArray& username);

	void connect();
	void disconnect();

	inline Status getOverallStatus() const { return mOverallStatus; }
	inline QString getConnectionString() const { return mConnectionString; }

	void sendSlaveRequest(bool sudo, SlaveFile* file, const QByteArray& request, const QVariant& parameters = QVariant(), const Callback& callback = Callback());
	void sendSftpRequest(SFTPRequest* request);

	void getFileContent(bool sudo, const QByteArray& filename, const Callback& callback = Callback());
	void setFileContent(bool sudo, const QByteArray& filename, const QByteArray& content, const Callback& callback = Callback());

	//
	//	Stuff for saving hosts
	//

	static QList<SshHost*>& getKnownHosts() { return sKnownHosts; }
	static SshHost* getBlankHost(bool save);
	static void recordKnownHost(SshHost* host) { sKnownHosts.append(host); }

	QByteArray getHostFingerprint();
	void setNewFingerprint(const QByteArray& fingerprint);
	static void registerKnownFingerprint(const QString& hostname, const QByteArray& fingerprint);
	static const QMap<QString, QByteArray>& getKnownFingerprints() { return sKnownHostFingerprints; }

	//
	//	Per host logging
	//

	class LogHelper
	{
	public:
		LogHelper(SshHost* host, QsLogging::Level level) : mHost(host), mLevel(level), mDebug(&mBuffer) {}
		~LogHelper();
		inline QDebug& stream() { return mDebug; }

	private:
		SshHost* mHost;
		QsLogging::Level mLevel;
		QString mBuffer;
		QDebug mDebug;
	};
	friend class LogHelper;
	LogHelper log(QsLogging::Level level) { return LogHelper(this, level); }
	inline const QStringList& getLog() { return mHostLog; }
	void showLog();

	//
	//	Cooperation with partner classes
	//

	void showStatus();

	inline void lockNewSessions() { mNewSessionMutex.lock(); }
	inline void unlockNewSessions() { mNewSessionMutex.unlock(); }

	bool waitBeforeCheckingSlave(SshChannel* channel);	//	Returns true if the specified channel should wait before checking slave (first check only)
	void firstSlaveCheckComplete();

	SlaveRequest* getNextSlaveRequest(bool sudo, const QMap<SlaveFile*,int>& registeredBuffers);
	XferRequest* getNextXferRequest(bool sudo);
	SFTPRequest* getNextSftpRequest();

	SshChannel* takeNextHomelessChannel();

	void handleUnsolicitedSlaveMessage(const QVariantMap& message);

	//
	//	Getters & Setters for cached stuff.
	//

	inline unsigned long getCachedIpAddress() const { return mCachedIpAddress; }
	inline SshSession::AuthMethod getCachedAuthMethod() const { return mCachedAuthMethod; }
	inline int getChannelLimitGuess() const { return mChannelLimitGuess; }
	const QByteArray& getHomeDirectory();		//	Will make a guess if no home directory specified.

	inline void setCachedIpAddress(unsigned long ipAddress) { mCachedIpAddress = ipAddress; }
	inline void setCachedAuthMethod(SshSession::AuthMethod authMethod) { mCachedAuthMethod = authMethod; }
	inline void setChannelLimitGuess(int guess) { mChannelLimitGuess = guess; }
	inline void setHomeDirectory(const QByteArray& homeDir) { mHomeDirectory = homeDir; }

	//
	//	Getters & Setters for saved state.
	//

	inline const QString& getName() const { return mName; }
	inline const QByteArray& getHostname() const { return mHostname; }
	inline int getPort() const { return mPort; }
	inline const QByteArray& getUsername() const { return mUsername; }
	inline const QByteArray& getPassword() const { return mPassword; }
	inline const QByteArray& getKeyFile() const { return mKeyFile; }
	inline const QByteArray& getKeyPassphrase() const { return mKeyPassphrase; }
	inline const QByteArray& getSudoPassword() const { return mSudoPassword; }
	inline const QByteArray& getDefaultDirectory() const { return mDefaultDirectory; }
	inline ConnectionType getConnectionType() const { return mConnectionType; }

	QString getDefaultPath();
	Location getDefaultLocation();

	inline void setName(const QString& name) { mName = name; }
	inline void setHostname(const QByteArray& hostname) { mHostname = hostname; }
	inline void setPort(int port) { mPort = port; }
	inline void setUsername(const QByteArray& username) { mUsername = username; }
	inline void setPassword(const QByteArray& password) { mPassword = password; }
	inline void setKeyFile(const QByteArray& keyFile) { mKeyFile = keyFile; }
	inline void setKeyPassphrase(const QByteArray& keyPassphrase) { mKeyPassphrase = keyPassphrase; }
	inline void setSudoPassword(const QByteArray& sudoPassword) { mSudoPassword = sudoPassword; }
	inline void setDefaultDirectory(const QByteArray& defaultDirectory) { mDefaultDirectory = defaultDirectory; }
	inline void setConnectionType(ConnectionType type) { mConnectionType = type; }

	void setSaveHost(bool saveHost);
	void setSavePassword(bool savePassword);
	void setSaveKeyPassphrase(bool saveKeyPassphrase);

	inline bool getSaveHost() const { return mSaveHost; }
	inline bool getSavePassword() const { return mSavePassword; }
	inline bool getSaveKeyPassphrase() const { return mSaveKeyPassphrase; }

	void invalidateOverallStatus();		//	Mark the overall status info as out-of-date, trigger a signal to lazily update.

public slots:
	void channelRejected(SshChannel* channel);	//	Called when an SshSession couldn't handle a new SshChannel.
	void channelNeatlyClosed(SshChannel* channel);	//	Called when an SshChannel closes voluntarily.
	void updateOverallStatus();
	void sessionEnded(SshSession* session);

signals:
	void wakeAllSessions();	//	Used to nudge all sessions out of their slumber. Useful when enqueuing jobs for them.
	void overallStatusInvalidated();	//	see: invalidateOverallStatus
	void overallStatusChanged();
	void newLogLine(QString line);

protected:
	void checkHeadroom();
	void checkChannelCount();
	SshSession* openSession();
	void enqueueXferRequest(XferRequest* request);
	void setOverallStatus(Status newStatus, const QString& connectionString);

	void registerChannel(SshChannel* channel);
	void removeChannel(SshChannel* channel);
	void assignSession(SshChannel* channel);
	int countChannels(SshChannel::Type type);

	void failSlaveRequests(const QString& error, int flags, QMutex& listLock, QList<SlaveRequest*>& requestList, SlaveChannel* channel);
	void failXferRequests(const QString& error, int flags, QMutex& listLock, QList<XferRequest*>& requestList);
	void failAllRequests(const QString& error, int flags);
	void failAllHomelessChannels();

	void appendToHostLog(const QString& line);

private:
	//	Stored servers
	static QList<SshHost*> sKnownHosts;
	static QMap<QString, QByteArray> sKnownHostFingerprints;

	//	Server state
	Status mDesiredStatus;
	Status mOverallStatus;	//	This is updated as sessions connect and disconnect to reflect the current overall status of this ssh host.
	QString mConnectionString;	//	During connection, this string is updated to reflect the current phase of connection.
	bool mOverallStatusDirty;	//	Used to allow SshHost overall status to lazy-update. So, multiple rapid status changes will only trigger 1 update.
	QMutex mNewSessionMutex;		//	In the interests of UI sanity, only allow 1 new SSH session to connect at any given time.

	QStringList mHostLog;
    QPointer<HostLog> mLogWindow;
	QMutex mModifyingLogMutex;

	QList<SshSession*> mSessions;
	QList<SshChannel*> mChannels;

	QMutex mHomelessChannelsMutex;
	QList<SshChannel*> mHomelessChannels;

	QMutex mSlaveRequestQueueMutex;
	QMutex mSudoSlaveRequestQueueMutex;
	QMutex mXferRequestQueueMutex;
	QMutex mSudoXferRequestQueueMutex;
	QMutex mSftpRequestQueueMutex;

	QList<SlaveRequest*> mSlaveRequestQueue;
	QList<SlaveRequest*> mSudoSlaveRequestQueue;
	QList<XferRequest*> mXferRequestQueue;
	QList<XferRequest*> mSudoXferRequestQueue;
	QList<SFTPRequest*> mSftpRequestQueue;

	//	Stuff for ensuring no two channels check the slave script simultaneously on the first run
	QMutex mFirstSlaveScriptCheckerLock;
	SshChannel* mFirstSlaveScriptChecker;
	bool mSlaveScriptChecked;

	//	Cached stuff for fast reconnection
	unsigned long mCachedIpAddress;	//	TODO: Extend to support IPv6
	SshSession::AuthMethod mCachedAuthMethod;
	int mChannelLimitGuess;
	QByteArray mHomeDirectory;

	//	Saved server parameters
	QString mName;
	bool mSaveHost;
	bool mSavePassword;
	bool mSaveKeyPassphrase;
	QByteArray mHostname;
	int mPort;
	QByteArray mUsername;
	QByteArray mPassword;
	QByteArray mKeyFile;
	QByteArray mKeyPassphrase;
	QByteArray mSudoPassword;
	QByteArray mDefaultDirectory;
	ConnectionType mConnectionType;
};

#endif // SSHHOST_H
