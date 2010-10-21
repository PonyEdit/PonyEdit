#include "location.h"
#include <QFileIconProvider>
#include <QMetaMethod>
#include <QObject>
#include <QDir>


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
	mData = new LocationShared();
}

Location::Location(const Location& other)
{
	mData = other.mData;
	mData->mReferences++;
}

Location::Location(const QString& path)
{
	mData = new LocationShared();
	mData->setPath(path);
}

Location::Location(const QString& path, Type type, int size, QDateTime lastModified)
{
	mData = new LocationShared();
	mData->setPath(path);
	mData->mType = type;
	mData->mSize = size;
	mData->mLastModified = lastModified;
	mData->mSelfLoaded = true;
}

LocationShared::LocationShared()
{
	initIconProvider();

	mReferences = 1;
	mType = Location::Unknown;
	mSize = -1;
	mSelfLoaded = false;
	mListLoaded = false;
}

Location::~Location()
{
	mData->mReferences--;
	if (mData->mReferences <= 0)
		delete (mData);
}


/////////////////////////////
//  Method Implementation  //
/////////////////////////////

const QString& Location::getPath() const { return mData->mPath; }
const QString& Location::getLabel() const { return mData->mLabel; }
bool Location::isNull() const { return (mData->mPath.isEmpty()); }
bool Location::isHidden() const { return (mData->mLabel.startsWith('.')); }
int Location::getSize() const { return mData->mSize; }
const QDateTime& Location::getLastModified() const { return mData->mLastModified; }

QIcon Location::getIcon() const
{
	switch (mData->mProtocol)
	{
	case Location::Local:
		return sIconProvider->icon(QFileInfo(mData->mPath));

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

void LocationShared::setPath(const QString &path)
{
	mPath = path;
	mPath.replace('\\', '/');

	//	Clean off any trailing slashes
	while (mPath.endsWith('/'))
		mPath.truncate(mPath.length() - 1);

	//	Work out what to label this path...
	int lastSlashIndex = mPath.lastIndexOf('/');
	mLabel = mPath.mid(lastSlashIndex + 1);

	//	Work out what type of path this is... Later. For now just assume they're all local
	mProtocol = Location::Local;
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
		mChildren.append(Location(fileInfo.absoluteFilePath(), fileInfo.isDir() ? Location::Directory : Location::File, fileInfo.size(), fileInfo.lastModified()));
	}

	mListLoaded = true;
	emit loadListSuccessful(mChildren, mPath);
}

void Location::asyncGetChildren(QObject* callbackTarget, const char* succeedSlot, const char* failSlot)
{
	if (mData->mListLoaded)
	{
		//	If already loaded, just call the success callback immediately.
		const QMetaObject* metaObject = callbackTarget->metaObject();
		int callbackIndex = metaObject->indexOfMethod(succeedSlot);
		QMetaMethod metaMethod = metaObject->method(callbackIndex);
		metaMethod.invoke(callbackTarget, Qt::QueuedConnection, Q_ARG(QList<Location>, mData->mChildren), Q_ARG(QString, getPath()));
	}
	else
	{
		QObject::connect(mData, SIGNAL(loadListSuccessful(QList<Location>,QString)), callbackTarget, succeedSlot);
		QObject::connect(mData, SIGNAL(loadListFailed(QString,QString)), callbackTarget, failSlot);

		if (!mData->mLoading)
		{
			if (mData->mProtocol == Local)
				mData->localLoadListing();
			else
				throw("Remote loading not implemented yet!");
		}
	}
}












