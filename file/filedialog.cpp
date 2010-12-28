#include "file/filedialog.h"
#include "ui_filedialog.h"
#include "main/tools.h"
#include "ssh/sshhost.h"
#include "main/globaldispatcher.h"

#include <QDir>
#include <QDebug>
#include <QKeyEvent>
#include <QSettings>
#include <QMessageBox>

#define DATA_ROLE (Qt::UserRole)
#define EXPANDED_ROLE (Qt::UserRole + 1)
#define TYPE_ROLE (Qt::UserRole + 2)
#define HOST_ROLE (Qt::UserRole + 3)

#define NODETYPE_LOCATION 1

Location FileDialog::mLastLocation;

FileDialog::FileDialog(QWidget *parent, bool saveAs) :
	QDialog(parent),
    ui(new Ui::FileDialog)
{
	ui->setupUi(this);

	mFileListModel = new QStandardItemModel();

	mSaveAs = saveAs;

	ui->fileList->setModel(mFileListModel);
	ui->fileList->setShowGrid(false);
	ui->fileList->verticalHeader()->hide();
	if(mSaveAs)
		ui->fileList->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->fileList->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->fileList->setWordWrap(false);
	ui->fileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->fileList->setFocus();
	ui->fileList->horizontalHeader()->setHighlightSections(false);

	QList<int> sizes = ui->splitter->sizes();
	sizes[0] = 1;
	sizes[1] = 300;
	ui->splitter->setSizes(sizes);

	connect(ui->directoryTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(folderTreeItemExpanded(QTreeWidgetItem*)));
	connect(ui->directoryTree, SIGNAL(itemSelectionChanged()), this, SLOT(directoryTreeSelected()));
	connect(ui->upLevelButton, SIGNAL(clicked()), this, SLOT(upLevel()));
	connect(ui->fileList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(fileDoubleClicked(QModelIndex)));
	connect(gDispatcher, SIGNAL(sshServersUpdated()), this, SLOT(populateRemoteServers()), Qt::QueuedConnection);
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(ui->fileList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(fileListSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(gDispatcher, SIGNAL(locationListSuccessful(QList<Location>,QString)), this, SLOT(folderChildrenLoaded(QList<Location>,QString)), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(locationListFailed(QString,QString)), this, SLOT(folderChildrenFailed(QString,QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(accepted()), this, SLOT(closing()));
	connect(this, SIGNAL(rejected()), this, SLOT(closing()));

	populateFolderTree();

	restoreState();

	showLocation(mLastLocation);
}

FileDialog::~FileDialog()
{
	delete mFileListModel;
    delete ui;
}

void FileDialog::restoreState()
{
	QSettings settings;
	restoreGeometry(settings.value("filedialog/geometry").toByteArray());
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

#ifdef Q_OS_MAC
	QDir directory("/Volumes");
	QStringList entries = directory.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	foreach (QString entry, entries)
	{
		QFileInfo driveFileInfo("/Volumes/" + entry);
		addLocationToTree(localComputer, Location(driveFileInfo.absoluteFilePath()));
	}
#elif defined Q_OS_WIN
	QFileInfoList driveList = QDir::drives();
	foreach (QFileInfo driveFileInfo, driveList)
		addLocationToTree(localComputer, Location(driveFileInfo.absoluteFilePath()));
#else
	addLocationToTree(localComputer, Location("/"));
#endif
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

	if (mLastLocation.isNull())
		mLastLocation = homeLocation;
}

void FileDialog::populateRemoteServers()
{
	//mRemoteServersBranch

	//	Take a quick inventory of the servers in the list now...
	QMap<SshHost*, bool> currentList;
	for (int i = 0; i < mRemoteServersBranch->childCount(); i++)
	{
		QTreeWidgetItem* child = mRemoteServersBranch->child(i);
		SshHost* host = (SshHost*)child->data(0, HOST_ROLE).value<void*>();
		currentList.insert(host, false);
	}

	//	Go through the list of servers that should be there. Add new entries, mark existing ones as "ok to keep"
	QList<SshHost*> knownHosts = SshHost::getKnownHosts();
	foreach (SshHost* host, knownHosts)
	{
		if (currentList.contains(host))
			currentList.insert(host, true);
		else
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(0, host->getName());
			item->setIcon(0, QIcon(":/icons/server.png"));
			item->setData(0, DATA_ROLE, QVariant::fromValue<Location>(host->getDefaultLocation()));
			item->setData(0, EXPANDED_ROLE, QVariant(1));
			item->setData(0, TYPE_ROLE, QVariant(NODETYPE_LOCATION));
			item->setData(0, HOST_ROLE, QVariant::fromValue<void*>(host));
			mRemoteServersBranch->addChild(item);
		}
	}

	//	Remove the list entries that have not been marked as "ok to keep"
	for (int i = 0; i < mRemoteServersBranch->childCount(); i++)
	{
		QTreeWidgetItem* child = mRemoteServersBranch->child(i);
		SshHost* host = (SshHost*)child->data(0, HOST_ROLE).value<void*>();
		if (!currentList.value(host, true))
		{
			i--;
			delete child;
		}
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
			location.asyncGetChildren();
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

void FileDialog::folderChildrenFailed(const QString& error, const QString& /*locationPath*/)
{
	mFileListModel->clear();
	ui->loaderIcon->setPixmap(QPixmap(":/icons/error.png"));
	ui->loaderLabel->setText(QString("Error: " + error));
	ui->fileListStack->setCurrentWidget(ui->loaderLayer);
}

void FileDialog::showLocation(const Location& location)
{
	if (location.isNull())
		return;

	mLastLocation = location;

	ui->currentPath->setText(location.getDisplayPath());
	mCurrentLocation = location;

	mFileListModel->clear();
	ui->loaderIcon->setPixmap(QPixmap(":/icons/loading.png"));
	ui->loaderLabel->setText("Loading...");
	ui->fileListStack->setCurrentWidget(ui->loaderLayer);

	mCurrentLocation.asyncGetChildren();
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
	}
}

void FileDialog::keyPressEvent(QKeyEvent *event)
{
	if(event->key() == Qt::Key_Escape)
		reject();

	if (focusWidget() == ui->currentPath && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
	{
		if (ui->currentPath->text() != mCurrentLocation.getDisplayPath())
		{
			Location newLocation(ui->currentPath->text());
			showLocation(newLocation);
		}
	}
	else if ((focusWidget() == ui->fileList || focusWidget() == ui->fileName) && (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return))
	{
		QList<Location> selections = getSelectedLocations();
		if(selections.length() > 1 || (selections.length() > 0 && !selections[0].isDirectory()))
			accept();
		else
			showLocation(selections[0]);
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
		accept();
}

void FileDialog::fileListSelectionChanged(const QItemSelection&, const QItemSelection&)
{
	QList<Location> selections = getSelectedLocations();
	QStringList selectionLabels;

	foreach (Location l, selections)
		if (!l.isDirectory())
			selectionLabels.append(QString('"') + l.getLabel() + '"');

	ui->fileName->setText(selectionLabels.join(", "));
}

QList<Location> FileDialog::getSelectedLocations() const
{
	QList<Location> selections;
	QModelIndexList allSelected = ui->fileList->selectionModel()->selectedIndexes();
	foreach (QModelIndex index, allSelected)
		if (index.column() == 0)
			selections.append(mFileListModel->itemFromIndex(index)->data(DATA_ROLE).value<Location>());

	return selections;
}

Location FileDialog::getNewLocation() const
{
	QList<Location> selections = getSelectedLocations();
	if(selections.length() > 0)
		return selections[0];

	Location newFile(mCurrentLocation.getPath() + "/" + ui->fileName->text());

	return newFile;
}

void FileDialog::accept()
{
	if(mSaveAs && ui->fileList->selectionModel()->selectedIndexes().length() > 0)
	{
		QMessageBox msgBox;
		msgBox.setText(tr("This file already exists."));
		msgBox.setInformativeText(tr("Are you sure you want to overwrite it?"));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::Yes);

		if(msgBox.exec() == QMessageBox::No)
			return;
	}

	QDialog::accept();
}

void FileDialog::closing()
{
	//	Save the geometry of this window on the way out
	QSettings settings;
	settings.setValue("filedialog/geometry", saveGeometry());
}


