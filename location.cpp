#include "location.h"
#include <QFileIconProvider>

QFileIconProvider* sIconProvider = NULL;
void Location::initIconProvider() { if (!sIconProvider) sIconProvider = new QFileIconProvider(); }

Location::Location()
{
	initIconProvider();
	mProtocol = Invalid;
	mSpecialType = NotSpecial;
	mType = Unknown;
}

Location::Location(const QString& path)
{
	initIconProvider();
	mType = Unknown;
	setPath(path);
}

Location::Location(const QString& path, Type type)
{
	initIconProvider();
	mType = type;
	setPath(path);
}

void Location::setPath(const QString &path)
{
	mPath = path;
	mSpecialType = NotSpecial;
	if (path.startsWith("special://"))
	{
		mProtocol = Special;
		if (path == LOCATION_LOCALCOMPUTER)
			mSpecialType = LocalComputer;
		else if (path == LOCATION_REMOTESERVERS)
			mSpecialType = RemoteServers;
		else if (mPath == LOCATION_FAVORITELOCATIONS)
			mSpecialType = FavoriteLocations;
		else
			mProtocol = Invalid;
	}
	else if (path.startsWith("file://"))
		mProtocol = Local;
	else if (path.startsWith("ssh://"))
		mProtocol = Ssh;
	else
		mProtocol = Invalid;
}

void Location::cleanup()
{
	if (sIconProvider)
	{
		delete(sIconProvider);
		sIconProvider = NULL;
	}
}

QString Location::getLabel() const
{
	if (mProtocol == Special)
	{
		switch (mSpecialType)
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
	if (mProtocol == Special)
	{
		switch (mSpecialType)
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
	if (mProtocol == Special)
		return HasChildren;
	else
		return MightHaveChildren;
}

QList<Location> Location::getChildren()
{
	if (mSpecialType == LocalComputer)
	{
		QList<Location> tmp;
		tmp.append(Location(LOCATION_REMOTESERVERS));
		return tmp;
	}
}

