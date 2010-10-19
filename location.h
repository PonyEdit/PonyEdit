#ifndef LOCATION_H
#define LOCATION_H

#include <QString>
#include <QVariant>

#define LOCATION_LOCALCOMPUTER "special://localComputer"
#define LOCATION_REMOTESERVERS "special://remoteServers"
#define LOCATION_FAVORITELOCATIONS "special://favoriteLocations"

class LocationShared;
class Location
{
public:
	enum Protocol { Invalid, Special, Local, Ssh };
	enum Type { Unknown, Directory, File };
	enum SpecialType { NotSpecial, LocalComputer, RemoteServers, FavoriteLocations };
	enum Children { NoChildren = 0, MightHaveChildren, HasChildren };

	Location();
	Location(const Location& other);
	Location(const QString& path);
	Location(const QString& path, Type type);
	~Location();
	static void cleanupIconProvider();

	QString getPath() const;
	QString getLabel() const;
	QIcon getIcon() const;
	Children hasChildren() const;

	QList<Location> getChildren();

private:
	LocationShared* mData;
};

class LocationShared : public QObject
{
	Q_OBJECT

public:
	LocationShared() :
		mProtocol(Location::Invalid), mSpecialType(Location::NotSpecial),
		mType(Location::Unknown), mReferences(1)
	{
		initIconProvider();
	}
	~LocationShared() {}

	void initIconProvider();
	void setPath(const QString& path);

	QString mPath;
	Location::Protocol mProtocol;
	Location::SpecialType mSpecialType;
	Location::Type mType;
	int mReferences;
};


Q_DECLARE_METATYPE (Location);

#endif // LOCATION_H
