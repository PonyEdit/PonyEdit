#include "filedialog.h"
#include "ui_filedialog.h"

#include <QDebug>
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

void FileDialog::addLocalFile(const QString& label, const QFileInfo& fileInfo, QTreeWidgetItem* parent)
{
	QString realLabel = label;
	#ifdef Q_WS_WIN
		realLabel.replace('/', '\\');
	#endif

	QTreeWidgetItem* newItem = new QTreeWidgetItem(QStringList(label), 0);
	newItem->setIcon(0, mIconProvider.icon(fileInfo));
	if (fileInfo.isDir())
		newItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	parent->addChild(newItem);
}

QTreeWidgetItem* FileDialog::addLocationToTree(const Location& location, QTreeWidgetItem* parent)
{
	QTreeWidgetItem* newItem = new QTreeWidgetItem(0);
	newItem->setText(0, location.getLabel());
	newItem->setIcon(0, location.getIcon());
	newItem->setChildIndicatorPolicy(location.hasChildren() ? QTreeWidgetItem::ShowIndicator : QTreeWidgetItem::DontShowIndicator);
	newItem->setData(0, LOCATION_ROLE, QVariant::fromValue<Location>(location));
	newItem->setData(0, EXPANDED_ROLE, QVariant(0));

	if (parent)
		parent->addChild(newItem);
	else
		ui->directoryTree->addTopLevelItem(newItem);
	return newItem;
}

void FileDialog::populateFolderTree()
{
	QTreeWidgetItem* localComputer = addLocationToTree(Location(LOCATION_LOCALCOMPUTER, Location::Directory), NULL);
	folderTreeItemExpanded(localComputer);
	localComputer->setExpanded(true);


	/*
	//
	//	Local computer; contains home dir and root path(s)
	//

	QTreeWidgetItem* localComputer = new QTreeWidgetItem(QStringList("Local Computer"), 0);
	localComputer->setIcon(0, mIconProvider.icon(QFileIconProvider::Computer));
	ui->directoryTree->addTopLevelItem(localComputer);

	QFileInfo homeFolder(QDir::homePath());
	addLocalFile(homeFolder, localComputer);

	QFileInfoList driveList = QDir::drives();
	foreach (QFileInfo driveFileInfo, driveList)
		addLocalFile(driveFileInfo.filePath(), driveFileInfo, localComputer);*/

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
}

void FileDialog::folderTreeItemExpanded(QTreeWidgetItem* item)
{
	if (!item->data(0, EXPANDED_ROLE).toInt())
	{
		item->setData(0, EXPANDED_ROLE, QVariant(1));
		Location location = item->data(0, LOCATION_ROLE).value<Location>();

		QList<Location> children = location.getChildren();
		foreach(Location child, children)
			addLocationToTree(child, item);
	}
}



