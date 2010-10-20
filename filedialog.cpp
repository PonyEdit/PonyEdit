#include "filedialog.h"
#include "ui_filedialog.h"

#include <QDir>

#define LOCATION_ROLE (Qt::UserRole)
#define EXPANDED_ROLE (Qt::UserRole + 1)

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileDialog)
{
	ui->setupUi(this);
	populateFolderTree();

	connect(ui->directoryTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(folderTreeItemExpanded(QTreeWidgetItem*)));
}

FileDialog::~FileDialog()
{
    delete ui;
}

void FileDialog::populateFolderTree()
{
	//
	//	Local computer; contains home dir and root path(s)
	//

	QTreeWidgetItem* localComputer = new QTreeWidgetItem(QStringList("Local Computer"), 0);
	localComputer->setIcon(0, mIconProvider.icon(QFileIconProvider::Computer));
	ui->directoryTree->addTopLevelItem(localComputer);

	Location homeLocation(QDir::homePath());
	addLocationToTree(localComputer, homeLocation);

	QFileInfoList driveList = QDir::drives();
	foreach (QFileInfo driveFileInfo, driveList)
		addLocationToTree(localComputer, Location(driveFileInfo.absoluteFilePath()));

	//
	//	Remote Servers; contains a list of pre-configured known servers
	//

	QTreeWidgetItem* remoteServers = new QTreeWidgetItem(QStringList("Remote Servers"), 0);
	remoteServers->setIcon(0, mIconProvider.icon(QFileIconProvider::Network));
	ui->directoryTree->addTopLevelItem(remoteServers);

	//
	//	Favorite Locations; contains a list of bookmarked locations; local or otherwise
	//

	QTreeWidgetItem* favouriteLocations = new QTreeWidgetItem(QStringList("Favorite Locations"), 0);
	favouriteLocations->setIcon(0, QIcon("icons/favorite.png"));
	ui->directoryTree->addTopLevelItem(favouriteLocations);

	showLocation(homeLocation);
}

QTreeWidgetItem* FileDialog::addLocationToTree(QTreeWidgetItem* parent, const Location& location)
{
	QTreeWidgetItem* newItem = new QTreeWidgetItem(0);
	newItem->setText(0, location.getLabel());
	newItem->setIcon(0, location.getIcon());
	newItem->setChildIndicatorPolicy(location.getType() == Location::Directory ? QTreeWidgetItem::ShowIndicator : QTreeWidgetItem::DontShowIndicator);
	newItem->setData(0, LOCATION_ROLE, QVariant::fromValue<Location>(location));
	newItem->setData(0, EXPANDED_ROLE, QVariant(0));

	if (parent)
		parent->addChild(newItem);
	else
		ui->directoryTree->addTopLevelItem(newItem);
	return newItem;
}

void FileDialog::folderTreeItemExpanded(QTreeWidgetItem* item)
{
	if (!item->data(0, EXPANDED_ROLE).toInt())
	{
		item->setData(0, EXPANDED_ROLE, QVariant(1));
		Location location = item->data(0, LOCATION_ROLE).value<Location>();

		if (!location.isNull())
		{
			QTreeWidgetItem* loadingItem = new QTreeWidgetItem();
			loadingItem->setText(0, "Loading...");
			loadingItem->setIcon(0, QIcon(":/icons/loading.png"));
			item->addChild(loadingItem);

			mLoadingLocations.insert(location.getPath(), item);
			location.asyncGetChildren(this,
				SLOT(folderChildrenLoaded(QList<Location>,QString)),
				SLOT(folderChildrenFailed(QString,QString)));
		}
	}
}

void FileDialog::folderChildrenLoaded(const QList<Location>& children, const QString& locationPath)
{
	QTreeWidgetItem* item = mLoadingLocations.value(locationPath, NULL);
	if (item)
	{
		mLoadingLocations.remove(locationPath);
		while (item->childCount())
			item->removeChild(item->child(0));

		foreach (Location childLocation, children)
			if (childLocation.getType() == Location::Directory && !childLocation.isHidden())
				this->addLocationToTree(item, childLocation);
	}
}

void FileDialog::folderChildrenFailed(const QString& error, const QString& locationPath)
{

}

void FileDialog::showLocation(const Location& location)
{
	ui->currentPath->setText(location.getPath());
	ui->fileList->clear();
}

