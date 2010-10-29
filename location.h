#ifndef LOCATION_H
#define LOCATION_H

#include <QString>
#include <QVariant>
#include <QDateTime>

class SshHost;
class LocationShared;
class Location
{
	friend class LocationShared;
	friend class SshRequest_ls;

public:
	enum Type { Unknown = 0, File = 1, Directory = 2 };
	enum Protocol { Local = 0, Ssh = 1 };

public:
	Location();
	Location(const Location& other);
	Location& operator=(const Location& other);
	Location(const QString& path);
	~Location();

	QString getDisplayPath() const;
	const QString& getPath() const;
	const QString& getLabel() const;
	QIcon getIcon() const;
	Type getType() const;
	int getSize() const;
	const QDateTime& getLastModified() const;

	const Location& getParent();
	QString getParentPath() const;
	QString getRemotePath() const;

	bool isNull() const;
	bool isHidden() const;
	bool isDirectory() const;

	void asyncGetChildren(QObject* callbackTarget, const char* succeedSlot, const char* failSlot);

private:
	Location(const Location& parent, const QString& path, Type type, int size, QDateTime lastModified);
	Location(LocationShared* data);

	void sshChildLoadResponse(const QList<Location>& children);

	LocationShared* mData;
};

class LocationShared : public QObject
{
	Q_OBJECT
	friend class Location;

public:
	static void cleanupIconProvider();

signals:
	void loadListSuccessful(const QList<Location>& children, QString locationPath);
	void loadListFailed(const QString& error, QString locationPath);

private:
	LocationShared();
	static void initIconProvider();

	void setPath(const QString& path);
	bool ensureConnected();

	void emitListLoadedSignal();
	void localLoadSelf();
	void localLoadListing();
	void sshLoadListing();

	int mReferences;
	QString mPath;
	QString mLabel;

	Location::Type mType;
	Location::Protocol mProtocol;
	QDateTime mLastModified;
	QList<Location> mChildren;
	Location mParent;
	bool mSelfLoaded;
	bool mListLoaded;
	bool mLoading;
	int mSize;

	QString mRemoteHostName;
	QString mRemoteUserName;
	QString mRemotePath;
	SshHost* mRemoteHost;
};

Q_DECLARE_METATYPE (Location);

#endif // LOCATION_H
