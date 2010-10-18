#include "filedialog.h"
#include "ui_filedialog.h"

#include <QDebug>
#include <QDir>

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

void FileDialog::populateFolderTree()
{
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
		addLocalFile(driveFileInfo.filePath(), driveFileInfo, localComputer);

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

}



