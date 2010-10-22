#include "filedialog.h"
#include "ui_filedialog.h"

#include <QDir>
#include <QDebug>
#include <QKeyEvent>

#define LOCATION_ROLE (Qt::UserRole)
#define EXPANDED_ROLE (Qt::UserRole + 1)

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileDialog)
{
	ui->setupUi(this);

	mFileListModel = new QStandardItemModel();

	ui->fileList->setModel(mFileListModel);
	ui->fileList->setShowGrid(false);
	ui->fileList->verticalHeader()->hide();
	ui->fileList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->fileList->setWordWrap(false);
	ui->fileList->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QList<int> sizes;
	sizes.append(1);
	sizes.append(3);
	ui->splitter->setSizes(sizes);

	populateFolderTree();
	connect(ui->directoryTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(folderTreeItemExpanded(QTreeWidgetItem*)));
	connect(ui->directoryTree, SIGNAL(itemSelectionChanged()), this, SLOT(directoryTreeSelected()));
	connect(ui->upLevelButton, SIGNAL(clicked()), this, SLOT(upLevel()));
}

FileDialog::~FileDialog()
{
	delete mFileListModel;
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
	newItem->setChildIndicatorPolicy(location.isDirectory() ? QTreeWidgetItem::ShowIndicator : QTreeWidgetItem::DontShowIndicator);
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
			location.asyncGetChildren(this, SLOT(folderChildrenLoaded(QList<Location>,QString)), SLOT(folderChildrenFailed(QString,QString)));
		}
	}
}

void FileDialog::folderChildrenLoaded(const QList<Location>& children, const QString& locationPath)
{
	//	Update the folder tree if appropriate
	QTreeWidgetItem* item = mLoadingLocations.value(locationPath, NULL);
	if (item)
	{
		mLoadingLocations.remove(locationPath);
		while (item->childCount())
			item->removeChild(item->child(0));

		foreach (Location childLocation, children)
			if (childLocation.isDirectory() && !childLocation.isHidden())
				this->addLocationToTree(item, childLocation);
	}

	//	Update the file list if appropriate
	if (mCurrentLocation.getPath() == locationPath)
	{
		ui->fileListStack->setCurrentWidget(ui->fileListLayer);
		mFileListModel->clear();

		QStringList headerLabels;
		headerLabels.append("Filename");
		headerLabels.append("Size");
		headerLabels.append("Last Modified");
		mFileListModel->setHorizontalHeaderLabels(headerLabels);
		ui->fileList->setColumnWidth(0, 250);

		foreach (Location childLocation, children)
		{
			if (!childLocation.isHidden())
			{
				QList<QStandardItem*> row;

				QStandardItem* item = new QStandardItem();
				item->setIcon(childLocation.getIcon());
				item->setText(childLocation.getLabel());
				row.append(item);

				item = new QStandardItem();
				item->setText(childLocation.isDirectory() ? "" : QString::number(childLocation.getSize()));
				row.append(item);

				item = new QStandardItem();
				item->setText(childLocation.getLastModified().toString());
				row.append(item);

				item = new QStandardItem();
				item->setText(childLocation.isDirectory() ? "D" : "F");
				row.append(item);

				mFileListModel->appendRow(row);
			}
		}

		ui->fileList->resizeColumnsToContents();
		ui->fileList->resizeRowsToContents();
		ui->fileList->setColumnWidth(0, ui->fileList->columnWidth(0) + 30);
		ui->fileList->setColumnHidden(3, true);
		mFileListModel->sort(0, (Qt::SortOrder)(Qt::AscendingOrder | Qt::CaseInsensitive));
		mFileListModel->sort(3, (Qt::SortOrder)(Qt::AscendingOrder | Qt::CaseInsensitive));
	}
}

void FileDialog::folderChildrenFailed(const QString& error, const QString& locationPath)
{

}

void FileDialog::showLocation(const Location& location)
{
	if (location.isNull())
		return;

	ui->currentPath->setText(location.getDisplayPath());
	mCurrentLocation = location;

	mFileListModel->clear();
	ui->loaderIcon->setPixmap(QPixmap(":/icons/loading.png"));
	ui->loaderLabel->setText("Loading...");
	ui->fileListStack->setCurrentWidget(ui->loaderLayer);

	mCurrentLocation.asyncGetChildren(this, SLOT(folderChildrenLoaded(QList<Location>,QString)), SLOT(folderChildrenFailed(QString,QString)));
}

void FileDialog::directoryTreeSelected()
{
	QList<QTreeWidgetItem*> items = ui->directoryTree->selectedItems();
	if (items.length() >= 1)
	{
		Location location = items[0]->data(0, LOCATION_ROLE).value<Location>();
		if (!location.isNull())
			showLocation(location);
	}
}

void FileDialog::keyPressEvent(QKeyEvent *event)
{
	if (focusWidget() == ui->currentPath && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
	{
		if (ui->currentPath->text() != mCurrentLocation.getDisplayPath())
		{
			Location newLocation(ui->currentPath->text());
			showLocation(newLocation);
		}
	}
}

void FileDialog::upLevel()
{
	Location parent = mCurrentLocation.getParent();
	if (!parent.isNull())
		showLocation(parent);
}




