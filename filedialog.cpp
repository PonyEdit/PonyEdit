#include "filedialog.h"
#include "ui_filedialog.h"
#include "tools.h"
#include "sshhost.h"
#include "globaldispatcher.h"

#include <QDir>
#include <QDebug>
#include <QKeyEvent>

#define DATA_ROLE (Qt::UserRole)
#define EXPANDED_ROLE (Qt::UserRole + 1)
#define TYPE_ROLE (Qt::UserRole + 2)

#define NODETYPE_LOCATION 1
#define NODETYPE_SSHHOST 2

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
	ui->fileList->setFocus();
	ui->fileList->horizontalHeader()->setHighlightSections(false);

	QList<int> sizes;
	sizes.append(1);
	sizes.append(3);
	ui->splitter->setSizes(sizes);

	populateFolderTree();
	connect(ui->directoryTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(folderTreeItemExpanded(QTreeWidgetItem*)));
	connect(ui->directoryTree, SIGNAL(itemSelectionChanged()), this, SLOT(directoryTreeSelected()));
	connect(ui->upLevelButton, SIGNAL(clicked()), this, SLOT(upLevel()));
	connect(ui->fileList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(fileDoubleClicked(QModelIndex)));
	connect(gDispatcher, SIGNAL(sshServersUpdated()), this, SLOT(populateRemoteServers()), Qt::QueuedConnection);
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
	localComputer->setExpanded(true);

	//
	//	Remote Servers; contains a list of pre-configured known servers
	//

	mRemoteServersBranch = new QTreeWidgetItem(QStringList("Remote Servers"), 0);
	mRemoteServersBranch->setIcon(0, mIconProvider.icon(QFileIconProvider::Network));
	ui->directoryTree->addTopLevelItem(mRemoteServersBranch);
	populateRemoteServers();
	mRemoteServersBranch->setExpanded(true);

	//
	//	Favorite Locations; contains a list of bookmarked locations; local or otherwise
	//

	QTreeWidgetItem* favouriteLocations = new QTreeWidgetItem(QStringList("Favorite Locations"), 0);
	favouriteLocations->setIcon(0, QIcon("icons/favorite.png"));
	ui->directoryTree->addTopLevelItem(favouriteLocations);

	showLocation(homeLocation);
}

void FileDialog::populateRemoteServers()
{
	QList<SshHost*> knownHosts = SshHost::getKnownHosts();
	foreach (SshHost* host, knownHosts)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, host->getName());
		item->setIcon(0, QIcon(":/icons/server.png"));
		item->setData(0, DATA_ROLE, qVariantFromValue((void*)host));
		item->setData(0, EXPANDED_ROLE, QVariant(1));
		item->setData(0, TYPE_ROLE, QVariant(NODETYPE_SSHHOST));
		mRemoteServersBranch->addChild(item);
	}
}

QTreeWidgetItem* FileDialog::addLocationToTree(QTreeWidgetItem* parent, const Location& location)
{
	QTreeWidgetItem* newItem = new QTreeWidgetItem(0);
	newItem->setText(0, location.getLabel());
	newItem->setIcon(0, location.getIcon());
	newItem->setChildIndicatorPolicy(location.isDirectory() ? QTreeWidgetItem::ShowIndicator : QTreeWidgetItem::DontShowIndicator);
	newItem->setData(0, DATA_ROLE, QVariant::fromValue<Location>(location));
	newItem->setData(0, EXPANDED_ROLE, QVariant(0));
	newItem->setData(0, TYPE_ROLE, QVariant(NODETYPE_LOCATION));

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
		Location location = item->data(0, DATA_ROLE).value<Location>();

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
				item->setData(QVariant::fromValue<Location>(childLocation), DATA_ROLE);
				row.append(item);

				item = new QStandardItem();
				item->setText(childLocation.isDirectory() ? "" : Tools::humanReadableBytes(childLocation.getSize()));
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
		ui->fileList->setColumnWidth(1, ui->fileList->columnWidth(1) + 30);
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
		int nodeType = items[0]->data(0, TYPE_ROLE).toInt();

		if (nodeType == NODETYPE_LOCATION)
		{
			Location location = items[0]->data(0, DATA_ROLE).value<Location>();
			if (!location.isNull())
				showLocation(location);
		}
		else if (nodeType == NODETYPE_SSHHOST)
		{
			SshHost* host = (SshHost*)items[0]->data(0, DATA_ROLE).value<void*>();
			if (host)
			{
				Location location(host->getFullPath());
				if (!location.isNull())
					showLocation(location);
			}
		}
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

void FileDialog::fileDoubleClicked(QModelIndex index)
{
	QStandardItem* item = mFileListModel->itemFromIndex(index);
	int row = item->row();
	QStandardItem* primaryItem = mFileListModel->item(row, 0);
	Location location = primaryItem->data(DATA_ROLE).value<Location>();

	if (location.isDirectory())
		showLocation(location);
	else
		qDebug() << "WOULD LOAD A FILE AT THIS POINT!!";
}




