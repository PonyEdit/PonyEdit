#ifndef LOCATION_H
#define LOCATION_H

#include <QString>
#include <QVariant>
#include <QDateTime>

class SshHost;
class BaseFile;
class LocationShared;
class SshConnection;
class SlaveChannel;
class FTPChannel;

class Location
{
	friend class LocationShared;

public:
	enum Type { Unknown, File, Directory };
	enum Protocol { Local, Ssh, Sftp, Unsaved };

public:
	Location();
	Location(const Location& other);
	Location& operator=(const Location& other);
	Location(const QString& path);
	Location(const Location& parent, const QString& path, Type type, int size, QDateTime lastModified, bool canRead, bool canWrite);
	~Location();

	QString getDisplayPath() const;
	const QString& getPath() const;
	const QString& getLabel() const;
	QIcon getIcon() const;
	Type getType() const;
	int getSize() const;
	const QDateTime& getLastModified() const;
	Protocol getProtocol() const;
	QString getHostName() const;		//	Regardless of protocol. Returns "Local Computer" for local files.
	QString getHostlessPath() const;	//	Gets the path without the host specifier. Same as getRemotePath for remote files.

	const Location& getParent() const;
	const Location& getDirectory() const;		// Returns self if is directory, or parent if is file.
	QString getParentPath() const;
	QString getRemotePath() const;
	SshHost* getRemoteHost() const;

	Location getSudoLocation() const;

	bool isNull() const;
	bool isHidden() const;
	bool isDirectory() const;
	bool canRead() const;
	bool canWrite() const;
	bool isSudo() const;
	bool canSudo() const;

	BaseFile* getFile();

	void asyncGetChildren(bool includeHidden);

	bool operator==(const Location& other) const;

	struct Favorite { QString path; QString name; };
	void addToFavorites();
	QString getDefaultFavoriteName();
	static void deleteFavorite(const QString& path);
	static void saveFavorites();
	static void loadFavorites();
	static inline QList<Favorite>& getFavorites() { return sFavorites; }

	bool createNewDirectory(QString name);

	void sshChildLoadResponse(const QList<Location>& children);
	void childLoadError(const QString& error, bool permissionError);

private:
	Location(LocationShared* data);

	void sshFileOpenResponse(SshConnection* controller, quint32 bufferId, const QByteArray& data);
	void fileOpenError(const QString& error);

	bool sshCreateDirectory(QString name);

	LocationShared* mData;

	static void addSortedFavorite(const Favorite& favorite);
	static QList<Favorite> sFavorites;
};

class LocationShared
{
	friend class Location;

public:
	static void cleanupIconProvider();

private:
	LocationShared();
	static void initIconProvider();

	void setPath(const QString& path);
	bool ensureConnected();

	void childLoadError(const QString& error, bool permissionError);

	void emitListLoadedSignal();
	void emitListLoadError(const QString& error, bool permissionError);
	void localLoadSelf();
	void localLoadListing(bool includeHidden);
	void sshLoadListing(bool includeHidden);
	void sftpLoadListing(bool includeHidden);

	int mReferences;
	QString mPath;
	QString mLabel;

	Location::Type mType;
	Location::Protocol mProtocol;
	QDateTime mLastModified;
	QList<Location> mChildren;
	Location mParent;
	bool mSelfLoaded;
	bool mLoading;
	int mSize;
	bool mCanRead;
	bool mCanWrite;
	bool mSudo;

	QString mRemoteHostName;
	QString mRemoteUserName;
	QString mRemotePath;
	SshHost* mRemoteHost;
	SlaveChannel* mSlaveChannel;
	FTPChannel* mFtpChannel;
};

Q_DECLARE_METATYPE (Location);

#endif // LOCATION_H
