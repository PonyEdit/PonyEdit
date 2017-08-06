#ifndef LOCATION_H
#define LOCATION_H

#include <QDateTime>
#include <QString>
#include <QVariant>
#include <tools/callback.h>

class OldSshHost;
class BaseFile;
class LocationShared;
class SshConnection;
class OldSlaveChannel;
class FTPChannel;
class SshHost;

class Location {
	friend class LocationShared;

public:
	enum Type { Unknown,
		File,
		Directory };
	enum Protocol { Local,
		Ssh,
		Sftp,
		Unsaved };

public:
	Location();
	Location( const Location &other );
	Location &operator=( const Location &other );
	Location( const QString &path );
	Location( const Location &parent, const QString &path, Type type, int size, QDateTime lastModified, bool canRead, bool canWrite );
	~Location();

	QString          getDisplayPath() const;
	const QString &  getPath() const;
	const QString &  getLabel() const;
	QIcon            getIcon() const;
	Type             getType() const;
	int              getSize() const;
	const QDateTime &getLastModified() const;
	Protocol         getProtocol() const;
	QString          getHostName() const; //	Regardless of protocol. Returns "Local Computer" for local files.
	QString          getHostlessPath() const; //	Gets the path without the host specifier. Same as getRemotePath for remote files.

	const Location &getParent() const;
	const Location &getDirectory() const; // Returns self if is directory, or parent if is file.
	QString         getParentPath() const;
	QString         getRemotePath() const;
	SshHost *       getRemoteHost() const;

	Location getSudoLocation() const;

	bool isNull() const;
	bool isHidden() const;
	bool isDirectory() const;
	bool canRead() const;
	bool canWrite() const;
	bool isSudo() const;
	bool canSudo() const;

	BaseFile *getFile();

	void asyncGetChildren( bool includeHidden ); //	Results are returned via the global dispatcher (locationListSuccess, locationListFailure)

	bool operator==( const Location &other ) const;

	struct Favorite {
		QString path;
		QString name;
	};
	void                             addToFavorites();
	QString                          getDefaultFavoriteName();
	static void                      deleteFavorite( const QString &path );
	static void                      saveFavorites();
	static void                      loadFavorites();
	static inline QList< Favorite > &getFavorites() {
		return sFavorites;
	}

	void createNewDirectory( const QString &name, const Callback &callback );

	void sshChildLoadResponse( const QList< Location > &children );
	void childLoadError( const QString &error, bool permissionError );

private:
	Location( LocationShared *data );

	void sshFileOpenResponse( SshConnection *controller, quint32 bufferId, const QByteArray &data );
	void fileOpenError( const QString &error );

	LocationShared *mData;

	static void              addSortedFavorite( const Favorite &favorite );
	static QList< Favorite > sFavorites;
};

class LocationShared : public QObject {
	Q_OBJECT
	friend class Location;

public:
	static void cleanupIconProvider();

private slots:
	void sshLsSuccess( QVariantMap results );
	void sshLsFailure( QString error, int flags );
	void sftpLsFailure( QString error, int flags );

private:
	LocationShared();
	static void initIconProvider();

	void setPath( const QString &path );

	void localLoadSelf();
	void localLoadListing( bool includeHidden );
	void sshLoadListing( bool includeHidden );
	void sftpLoadListing( bool includeHidden );

	SshHost *getHost();

	int     mReferences;
	QString mPath;
	QString mLabel;

	Location::Type     mType;
	Location::Protocol mProtocol;
	QDateTime          mLastModified;
	Location           mParent;
	bool               mSelfLoaded;
	int                mSize;
	bool               mCanRead;
	bool               mCanWrite;
	bool               mSudo;

	SshHost *mHost;

	//	Todo: Remote hostname and login should be QByteArrays
	QString          mRemoteHostName;
	QString          mRemoteUserName;
	QString          mRemotePath;
	OldSshHost *     mRemoteHost;
	OldSlaveChannel *mSlaveChannel;
	FTPChannel *     mFtpChannel;
};

Q_DECLARE_METATYPE( Location );

#endif // LOCATION_H
