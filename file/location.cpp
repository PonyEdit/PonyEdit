#include "file/location.h"
#include "file/openfilemanager.h"
#include "ssh/slaverequest.h"
#include "ssh/sshhost.h"
#include "file/slavefile.h"
#include "main/globaldispatcher.h"
#include <QFileIconProvider>
#include <QMetaMethod>
#include <QObject>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include "main/global.h"
#include "file/favoritelocationdialog.h"
#include "ssh/slavechannel.h"
#include "ssh/requeststatuswidget.h"

QRegExp gSshServerRegExp("^(?:([^@:]+)@)?([^:]+\\.[^:]+):([^:]+)?");
QRegExp gLocalPathSeparators("/");
QRegExp gSshPathSeparators("[/:]");
QList<Location::Favorite> Location::sFavorites;


///////////////////////////
//  Icon Provider stuff  //
///////////////////////////

QFileIconProvider* sIconProvider = NULL;

void LocationShared::initIconProvider()
{
	if (!sIconProvider)
		sIconProvider = new QFileIconProvider();
}

void LocationShared::cleanupIconProvider()
{
	if (sIconProvider)
	{
		delete(sIconProvider);
		sIconProvider = NULL;
	}
}


//////////////////////////////////
//  Constructors & Destructors  //
//////////////////////////////////

Location::Location()
{
	mData = NULL;
}

Location::Location(const Location& other)
{
	mData = other.mData;
	mData->mReferences++;
}

Location& Location::operator=(const Location& other)
{
	if (mData != NULL)
	{
		mData->mReferences--;
		if (mData->mReferences <= 0)
			delete(mData);
	}

	mData = other.mData;
	mData->mReferences++;

	return *this;
}

Location::Location(LocationShared* data)
{
	mData = data;
	mData->mReferences++;
}

Location::Location(const QString& path)
{
	mData = new LocationShared();
	mData->setPath(path);
}

Location::Location(const Location& parent, const QString& path, Type type, int size, QDateTime lastModified, bool canRead, bool canWrite)
{
	mData = new LocationShared();
	mData->setPath(path);
	mData->mType = type;
	mData->mSize = size;
	mData->mLastModified = lastModified;
	mData->mSelfLoaded = true;
	mData->mParent = parent;
	mData->mCanRead = canRead;
	mData->mCanWrite = canWrite;
}

LocationShared::LocationShared()
{
	initIconProvider();

	mSlaveChannel = NULL;
	mReferences = 1;
	mType = Location::Unknown;
	mSize = -1;
	mSelfLoaded = false;
	mLoading = false;
	mRemoteHost = NULL;
	mSudo = false;
}

Location::~Location()
{
	if (mData != NULL)
	{
		mData->mReferences--;
		if (mData->mReferences <= 0)
			delete (mData);
	}
}


/////////////////////////////
//  Method Implementation  //
/////////////////////////////

const QString& Location::getPath() const { return mData->mPath; }
const QString& Location::getLabel() const { return mData->mLabel; }
bool Location::isNull() const { return (mData == NULL || getPath() == ""); }
bool Location::isHidden() const { return (mData->mLabel.startsWith('.')); }
int Location::getSize() const { return mData->mSize; }
const QDateTime& Location::getLastModified() const { return mData->mLastModified; }
bool Location::canRead() const { return mData->mCanRead; }
bool Location::canWrite() const { return mData->mCanWrite; }

QString Location::getHostName() const
{
	switch (mData->mProtocol)
	{
	case Local:
		return QObject::tr("Local Computer");

	case Ssh:
		if (mData->mRemoteHost == NULL)
			mData->mRemoteHost = SshHost::getHost(mData->mRemoteHostName, mData->mRemoteUserName);
		return mData->mRemoteHost->getHostName();

	case Unsaved:
		return QObject::tr("New Files");

	default:
		throw(QObject::tr("Unknown protocol"));
	}
}

QString Location::getHostlessPath() const
{
	switch (mData->mProtocol)
	{
	case Local:
		return mData->mPath;

	case Ssh:
		return mData->mRemotePath;

	default:
		throw(QObject::tr("Unknown protocol"));
	}
}

bool Location::operator==(const Location& other) const
{
	return mData->mPath == other.mData->mPath;
}

const Location& Location::getDirectory() const
{
	if (isDirectory())
		return *this;
	else
		return getParent();
}

const Location& Location::getParent() const
{
	if (mData->mParent.isNull())
	{
		QString path = getParentPath();
		// If the path is empty, it's a New File, so just return a copy of this Location
		if(path.isEmpty())
			mData->mParent = Location(*this);
		else
			mData->mParent = Location(path);
	}

	return mData->mParent;
}

QString Location::getParentPath() const
{
	QString parentPath = getPath();
	int lastSlash = parentPath.lastIndexOf('/');
	if (lastSlash < 0)
		return "";
	else
	{
		parentPath.truncate(lastSlash);
		return parentPath;
	}
}

QString Location::getDisplayPath() const
{
	QString p = getPath();

#ifdef Q_OS_WIN
	if (mData->mProtocol == Local)
		p.replace('/', '\\');
#endif

	return p;
}

QIcon Location::getIcon() const
{
	//	TODO: So far only Linux can provide icons for remote files without pooping its pants.
	//	do something nicer for Windows.
	switch (mData->mProtocol)
	{
	case Location::Local:
		return sIconProvider->icon(QFileInfo(mData->mPath));

	case Location::Ssh:
		if (isDirectory())
			return sIconProvider->icon(QFileIconProvider::Folder);
		else
			return sIconProvider->icon(QFileIconProvider::File);

	default:
		return QIcon();
	}

}

Location::Type Location::getType() const
{
	if (!mData->mSelfLoaded && mData->mProtocol == Local)
		mData->localLoadSelf();

	return mData->mType;
}

bool Location::isSudo() const
{
	return mData->mSudo;
}

bool Location::isDirectory() const
{
	if (!mData->mSelfLoaded && mData->mProtocol == Local)
		mData->localLoadSelf();

	return mData->mType == Directory;
}

void LocationShared::setPath(const QString &path)
{
	mPath = path.trimmed();
	mPath.replace('\\', '/');

	//	Clean off any trailing slashes
	while (mPath.endsWith('/') && mPath.length() > 1)
		mPath.truncate(mPath.length() - 1);

	//	Work out what kind of path this is. Default if no pattern matches, is local.
	if (gSshServerRegExp.indexIn(mPath) > -1)
	{
		mProtocol = Location::Ssh;
		QStringList parts = gSshServerRegExp.capturedTexts();
		mRemoteUserName = parts[1];
		mRemoteHostName = parts[2];
		mRemotePath = parts[3];

		if (mRemoteUserName.endsWith('*'))
		{
			mSudo = true;
			mRemoteUserName.truncate(mRemoteUserName.length() - 1);
		}
	}
	else if(mPath.length() == 0)
		mProtocol = Location::Unsaved;
	else
		mProtocol = Location::Local;

	//	Work out what to label this path...
	int lastSeparatorIndex = mPath.lastIndexOf(mProtocol == Location::Ssh ? gSshPathSeparators : gLocalPathSeparators);
	mLabel = mPath.mid(lastSeparatorIndex + 1);
	if (mPath == "/")
		mLabel = "Root (/)";
	else if(mPath.isEmpty())
		mLabel = QString("New File %1").arg(gOpenFileManager.newFileNumber());
}

void LocationShared::localLoadSelf()
{
	QFileInfo fileInfo = QFileInfo(mPath);
	mType = fileInfo.isDir() ? Location::Directory : Location::File;
	mSelfLoaded = true;
}

void LocationShared::localLoadListing()
{
	if (!mSelfLoaded)
		localLoadSelf();

	mChildren.clear();

	QDir directory(mPath);
	QStringList entries = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
	foreach (QString entry, entries)
	{
		QFileInfo fileInfo(mPath + "/" + entry);
		mChildren.append(Location(Location(this), fileInfo.absoluteFilePath(),
			fileInfo.isDir() ? Location::Directory : Location::File, fileInfo.size(),
			fileInfo.lastModified(), fileInfo.isReadable(), fileInfo.isWritable()));
	}

	mLoading = false;
	emitListLoadedSignal();
}

void LocationShared::emitListLoadedSignal()
{
	gDispatcher->emitLocationListSuccessful(mChildren, mPath);
}

void LocationShared::emitListLoadError(const QString& error, bool permissionError)
{
	gDispatcher->emitLocationListFailed(error, mPath, permissionError);
}

void Location::asyncGetChildren()
{
	if (!mData->mLoading)
	{
		mData->mLoading = true;
		switch (mData->mProtocol)
		{
		case Local:
			mData->localLoadListing();
			break;

		case Ssh:
			mData->sshLoadListing();
			break;

		default:
			throw(QString("Invalid file protocol!"));
		}
	}
}

BaseFile* Location::getFile()
{
	if (!mData->ensureConnected())
		throw(QString("Failed to connect to remote host!"));

	return BaseFile::getFile(*this);
}

QString Location::getRemotePath() const
{
	return mData->mRemotePath;
}

Location::Protocol Location::getProtocol() const
{
	return mData->mProtocol;
}

SshHost* Location::getRemoteHost() const
{
	if (!mData->ensureConnected())
		throw(QString("Failed to connect to remote host!"));

	return mData->mRemoteHost;
}

void LocationShared::sshLoadListing()
{
	if (!ensureConnected())
		childLoadError(QObject::tr("Failed to connect to remote host!"), false);
	else
		mSlaveChannel->sendRequest(new SlaveRequest_ls(Location(this)));
}

void Location::sshChildLoadResponse(const QList<Location>& children)
{
	mData->mChildren = children;
	mData->mLoading = false;
	mData->emitListLoadedSignal();
}

void Location::childLoadError(const QString& error, bool permissionError)
{
	mData->childLoadError(error, permissionError);
}

void LocationShared::childLoadError(const QString& error, bool permissionError)
{
	mLoading = false;
	emitListLoadError(error, permissionError);
}

bool LocationShared::ensureConnected()
{
	//	Local locations are always connected
	if (mProtocol == Location::Local) return true;

	// Unsaved files are always connected
	if (mProtocol == Location::Unsaved) return true;

	if (mProtocol == Location::Ssh)
	{
		if (mRemoteHost == NULL)
			mRemoteHost = SshHost::getHost(mRemoteHostName, mRemoteUserName);

		if (mRemoteHost)
		{
			RemoteConnection* connection = mRemoteHost->getConnection();
			if (connection)
			{
				mSlaveChannel = (mSudo ? connection->getSudoChannel() : connection->getSlaveChannel());

				if (mSlaveChannel)
				{
					mPath.replace("~", connection->getHomeDirectory());
					mRemotePath.replace("~", connection->getHomeDirectory());
					mPath = mPath.trimmed();
					mRemotePath = mRemotePath.trimmed();

					if (mRemotePath.length() == 0)
						mRemotePath = "/";

					return true;
				}
			}
		}
	}

	return false;
}

void Location::addToFavorites()
{
	if (!isNull())
	{
		foreach (Favorite f, sFavorites)
			if (f.path == getPath())
				return;

		Favorite f;
		f.name = getDefaultFavoriteName();
		f.path = getPath();

		FavoriteLocationDialog dialog(NULL, &f);
		if (dialog.exec() == QDialog::Accepted)
		{
			addSortedFavorite(f);
			saveFavorites();
		}
	}
}

void Location::deleteFavorite(const QString& path)
{
	for (int i = 0; i < sFavorites.length(); i++)
	{
		if (sFavorites[i].path == path)
		{
			sFavorites.removeAt(i);
			saveFavorites();
			return;
		}
	}
}

void Location::saveFavorites()
{
	QSettings settings;

	int index = 0;
	settings.beginWriteArray(ntr("favorites"));
	foreach (Favorite f, sFavorites)
	{
		settings.setArrayIndex(index++);
		settings.setValue(ntr("name"), f.name);
		settings.setValue(ntr("path"), f.path);
	}
	settings.endArray();
}

void Location::loadFavorites()
{
	QSettings settings;

	int count = settings.beginReadArray(ntr("favorites"));
	for (int i = 0; i < count; i++)
	{
		settings.setArrayIndex(i);

		Favorite f;
		f.name = settings.value(ntr("name")).toString();
		f.path = settings.value(ntr("path")).toString();
		addSortedFavorite(f);
	}
}

void Location::addSortedFavorite(const Favorite& favorite)
{
	int i;
	for (i = 0; i < sFavorites.length(); i++)
		if (favorite.name.compare(sFavorites[i].name, Qt::CaseInsensitive) > 0)
			break;
	sFavorites.insert(i, favorite);
}

QString Location::getDefaultFavoriteName()
{
	switch (mData->mProtocol)
	{
	case Ssh:
		return QObject::tr("%1 on %2", "eg: ~ on Server X").arg(getLabel()).arg(mData->mRemoteHostName);

	case Unsaved:
		return QObject::tr("Unsaved");

	case Local:
	default:
		return QObject::tr("%1 (local)").arg(getLabel());
	}
}

bool Location::createNewDirectory(QString name)
{
	QDir currentDir;
	Location currentLocDir = this->getDirectory();
	switch(mData->mProtocol)
	{
	case Ssh:
		return sshCreateDirectory(name);
		break;

	case Local:
		currentDir.setPath(currentLocDir.getPath());
		return currentDir.mkdir(name);
		break;

	default: break;
	}

	return false;
}

bool Location::sshCreateDirectory(QString name)
{
	if (!mData->ensureConnected())
		return false;

	RequestStatusWidget::Result result = mData->mSlaveChannel->waitForRequest(new SlaveRequest_createDirectory(Location(*this), name),
																			  QObject::tr("Creating New Folder"), !isSudo() && canSudo());
	if (result == RequestStatusWidget::SudoRequestedResult)
	{
		Location sudoLocation = getSudoLocation();
		return sudoLocation.createNewDirectory(name);
	}
	else
		return result == RequestStatusWidget::SuccessResult;
}

bool Location::canSudo() const
{
	return mData->mProtocol == Ssh;
}

Location Location::getSudoLocation() const
{
	if (isSudo()) return *this;
	return Location(mData->mRemoteUserName + "*@" + mData->mRemoteHostName + ":" + mData->mRemotePath);
}


