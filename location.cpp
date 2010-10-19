#include "location.h"
#include <QFileIconProvider>
#include <QObject>

QFileIconProvider* sIconProvider = NULL;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Icon Provider stuff; initialize and cleanup
//

void LocationShared::initIconProvider()
{
	if (!sIconProvider)
		sIconProvider = new QFileIconProvider();
}

void Location::cleanupIconProvider()
{
	if (sIconProvider)
	{
		delete(sIconProvider);
		sIconProvider = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Constructors & Destructors
//

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
	mData->mType = Unknown;
	mData->setPath(path);
}

Location::Location(const QString& path, Type type)
{
	mData = new LocationShared();
	mData->mType = type;
	mData->setPath(path);
}

Location::~Location()
{
	mData->mReferences--;
	if (mData->mReferences <= 0)
		delete (mData);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Method implementation
//

void LocationShared::setPath(const QString &path)
{
	mPath = path;
	mSpecialType = Location::NotSpecial;
	if (path.startsWith("special://"))
	{
		mProtocol = Location::Special;
		if (path == LOCATION_LOCALCOMPUTER)
			mSpecialType = Location::LocalComputer;
		else if (path == LOCATION_REMOTESERVERS)
			mSpecialType = Location::RemoteServers;
		else if (mPath == LOCATION_FAVORITELOCATIONS)
			mSpecialType = Location::FavoriteLocations;
		else
			mProtocol = Location::Invalid;
	}
	else if (path.startsWith("file://"))
		mProtocol = Location::Local;
	else if (path.startsWith("ssh://"))
		mProtocol = Location::Ssh;
	else
		mProtocol = Location::Invalid;
}

QString Location::getPath() const
{
	return mData->mPath;
}

QString Location::getLabel() const
{
	if (mData->mProtocol == Special)
	{
		switch (mData->mSpecialType)
		{
		case LocalComputer:
			return "Local Computer";

		case RemoteServers:
			return "Remote Servers";

		case FavoriteLocations:
			return "FavoriteLocations";

		case NotSpecial:
			break;
		}
	}
	else
	{
	}

	return "(error)";
}

QIcon Location::getIcon() const
{
	if (mData->mProtocol == Special)
	{
		switch (mData->mSpecialType)
		{
		case LocalComputer:
			return sIconProvider->icon(QFileIconProvider::Computer);

		case RemoteServers:
			return sIconProvider->icon(QFileIconProvider::Network);

		case FavoriteLocations:
			return QIcon("icons/favorite.png");

		case NotSpecial:
			break;
		}
	}
	else
	{
	}

	return QIcon();
}

Location::Children Location::hasChildren() const
{
	if (mData->mProtocol == Special)
		return HasChildren;
	else
		return MightHaveChildren;
}

QList<Location> Location::getChildren()
{
	//if (mData->mSpecialType == LocalComputer)
	{
		QList<Location> tmp;
		tmp.append(Location(LOCATION_REMOTESERVERS));
		return tmp;
	}
}

