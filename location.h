#ifndef LOCATION_H
#define LOCATION_H

#include <QString>
#include <QVariant>

#define LOCATION_LOCALCOMPUTER "special://localComputer"
#define LOCATION_REMOTESERVERS "special://remoteServers"
#define LOCATION_FAVORITELOCATIONS "special://favoriteLocations"

class Location
{
public:
	enum Protocol { Invalid, Special, Local, Ssh };
	enum Type { Unknown, Directory, File };
	enum SpecialType { NotSpecial, LocalComputer, RemoteServers, FavoriteLocations };
	enum Children { NoChildren = 0, MightHaveChildren, HasChildren };

	Location();
	Location(const QString& path);
	Location(const QString& path, Type type);
	static void cleanup();

	QString getPath() const { return mPath; }
	QString getLabel() const;
	QIcon getIcon() const;
	Children hasChildren() const;

	QList<Location> getChildren();

private:
	void setPath(const QString& path);
	void initIconProvider();

	QString mPath;
	Protocol mProtocol;
	SpecialType mSpecialType;
	Type mType;
};

Q_DECLARE_METATYPE (Location);

#endif // LOCATION_H
