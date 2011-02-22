#include "file/filedialog.h"
#include "ui_filedialog.h"
#include "main/tools.h"
#include "ssh/sshhost.h"
#include "main/globaldispatcher.h"
#include "newfolderdialog.h"
#include "ssh/serverconfigdlg.h"
#include "file/filelistdelegate.h"

#include <QDir>
#include <QDebug>
#include <QKeyEvent>
#include <QSettings>
#include <QMessageBox>
#include <QMenu>
#include <QPushButton>

#define DATA_ROLE (Qt::UserRole)
#define EXPANDED_ROLE (Qt::UserRole + 1)
#define TYPE_ROLE (Qt::UserRole + 2)
#define HOST_ROLE (Qt::UserRole + 3)

#define NODETYPE_LOCATION 1
#define NODETYPE_FAVORITE 2
#define NODETYPE_ADD_LOCATION 3
#define NODETYPE_ADD_FAVORITE 4
#define NODETYPE_LOCAL_NETWORK 5

Location FileDialog::mLastLocation;

FileDialog::FileDialog(QWidget *parent, bool saveAs) :
	QDialog(parent),
    ui(new Ui::FileDialog)
{
	ui->setupUi(this);

	mFileListModel = new QStandardItemModel();
	ui->fileList->setItemDelegate(new FileListDelegate(this));

	setAcceptDrops(true);

	mSaveAs = saveAs;
	setWindowTitle(saveAs ? tr("Save As") : tr("Open File"));

	ui->fileList->setModel(mFileListModel);
	ui->fileList->setShowGrid(false);
	ui->fileList->verticalHeader()->hide();
	if (mSaveAs)
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

	ui->directoryTree->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui->directoryTree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(directoryTreeContextMenu(QPoint)));
	connect(ui->directoryTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(folderTreeItemExpanded(QTreeWidgetItem*)));
	connect(ui->directoryTree, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(directoryTreeSelected(QTreeWidgetItem*)));
	connect(ui->upLevelButton, SIGNAL(clicked()), this, SLOT(upLevel()));
	connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(refresh()));
	connect(ui->fileList, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(fileDoubleClicked(QModelIndex)));
	connect(gDispatcher, SIGNAL(sshServersUpdated()), this, SLOT(populateRemoteServers()), Qt::QueuedConnection);
	connect(ui->mainButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(ui->mainButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(ui->fileList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(fileListSelectionChanged(const QItemSelection&, const QItemSelection&)));
	connect(gDispatcher, SIGNAL(locationListSuccessful(QList<Location>,QString)), this, SLOT(folderChildrenLoaded(QList<Location>,QString)), Qt::QueuedConnection);
	connect(gDispatcher, SIGNAL(locationListFailed(QString,QString,bool)), this, SLOT(folderChildrenFailed(QString,QString,bool)), Qt::QueuedConnection);
	connect(this, SIGNAL(accepted()), this, SLOT(closing()));
	connect(this, SIGNAL(rejected()), this, SLOT(closing()));
	connect(ui->newFolderButton, SIGNAL(clicked()), this, SLOT(createNewFolder()));
	connect(ui->statusWidget, SIGNAL(buttonClicked(StatusWidget::Button)), this, SLOT(retryButtonClicked(StatusWidget::Button)));
	connect(ui->showHidden, SIGNAL(stateChanged(int)), this, SLOT(refresh()));

	populateFolderTree();

	restoreState();

	showLocation(mLastLocation);

	ui->fileName->setFocus();
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

#ifdef Q_OS_WIN
	mLocalNetworkBranch = new QTreeWidgetItem(QStringList(tr("Local Network")), 0);
	mLocalNetworkBranch->setIcon(0, mIconProvider.icon(QFileIconProvider::Network));

	mLocalNetworkBranch->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

	ui->directoryTree->addTopLevelItem(mLocalNetworkBranch);
#endif

	//
	//	Favorite Locations; contains a list of bookmarked locations; local or otherwise
	//

	mFavoriteLocationsBranch = new QTreeWidgetItem(QStringList("Favorite Locations"), 0);
	mFavoriteLocationsBranch->setIcon(0, QIcon("icons/favorite.png"));
	ui->directoryTree->addTopLevelItem(mFavoriteLocationsBranch);

	QTreeWidgetItem* addFavorite = new QTreeWidgetItem();
	addFavorite->setText(0, tr("Add Favorite..."));
	addFavorite->setIcon(0, QIcon(":/icons/add.png"));
	addFavorite->setData(0, DATA_ROLE, QVariant(1));
	addFavorite->setData(0, TYPE_ROLE, QVariant(NODETYPE_ADD_FAVORITE));
	mFavoriteLocationsBranch->addChild(addFavorite);

	updateFavorites();
	mFavoriteLocationsBranch->setExpanded(true);

	//
	//	Remote Servers; contains a list of pre-configured known servers
	//

	mRemoteServersBranch = new QTreeWidgetItem(QStringList("Remote Servers"), 0);
	mRemoteServersBranch->setIcon(0, mIconProvider.icon(QFileIconProvider::Network));
	ui->directoryTree->addTopLevelItem(mRemoteServersBranch);

	QTreeWidgetItem* addServer = new QTreeWidgetItem();
	addServer->setText(0, tr("Add Server..."));
	addServer->setIcon(0, QIcon(":/icons/add.png"));
	addServer->setData(0, DATA_ROLE, QVariant(1));
	addServer->setData(0, EXPANDED_ROLE, QVariant(1));
	addServer->setData(0, TYPE_ROLE, QVariant(NODETYPE_ADD_LOCATION));
	addServer->setData(0, HOST_ROLE, QVariant(1));
	mRemoteServersBranch->addChild(addServer);

	populateRemoteServers();
	mRemoteServersBranch->setExpanded(true);

	if (mLastLocation.isNull())
		mLastLocation = homeLocation;
}

void FileDialog::populateRemoteServers()
{
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
	for (int i = 1; i < mRemoteServersBranch->childCount(); i++)
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

#ifdef Q_OS_WIN
void FileDialog::populateWindowsShares(QTreeWidgetItem *localNetworkItem, LPNETRESOURCE lpnr)
{
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;
	DWORD cEntries = -1;
	LPNETRESOURCE lpnrLocal;
	DWORD i;

	dwResult = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_DISK, 0, lpnr, &hEnum);

	if (dwResult != NO_ERROR)
		return;

	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
	if (lpnrLocal == NULL)
		return;

	do {
		ZeroMemory(lpnrLocal, cbBuffer);

		dwResultEnum = WNetEnumResource(hEnum, &cEntries, lpnrLocal, &cbBuffer);

		if (dwResultEnum == NO_ERROR) {
			for (i = 0; i < cEntries; i++) {
				QString name((QChar*)lpnrLocal[i].lpRemoteName);
				QTreeWidgetItem* item;
				if(lpnrLocal[i].dwDisplayType == RESOURCEDISPLAYTYPE_SHARE || lpnrLocal[i].dwDisplayType == RESOURCEDISPLAYTYPE_SERVER)
				{
					Location loc = Location(name);
					item = addLocationToTree(localNetworkItem, loc);
				}
				else
				{
					item = new QTreeWidgetItem(0);
					item->setText(0, name);
					item->setIcon(0, mIconProvider.icon(QFileIconProvider::Network));
					item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
					item->setData(0, DATA_ROLE, QVariant(0));
					item->setData(0, EXPANDED_ROLE, QVariant(0));
					item->setData(0, TYPE_ROLE, QVariant(0));
				}
				localNetworkItem->addChild(item);

				if (RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage & RESOURCEUSAGE_CONTAINER))
					populateWindowsShares(item, &lpnrLocal[i]);
			}
		}
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
			break;
	}

	while (dwResultEnum != ERROR_NO_MORE_ITEMS);

	GlobalFree((HGLOBAL) lpnrLocal);

	WNetCloseEnum(hEnum);
}
#endif

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
#ifdef Q_OS_WIN
	if(item == mLocalNetworkBranch)
	{
		LPNETRESOURCE lpnr = NULL;
		populateWindowsShares(item, lpnr);
	}
	else
#endif
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
		bool showHidden = ui->showHidden->isChecked();

		foreach (Location childLocation, children)
		{
			if (showHidden || !childLocation.isHidden())
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

		if(!mSelectFile.isNull())
		{
			QList<QStandardItem *> items = mFileListModel->findItems(mSelectFile);

			QItemSelectionModel *mdl = ui->fileList->selectionModel();
			mdl->clearSelection();

			foreach(QStandardItem *item, items)
				mdl->select(item->index(), QItemSelectionModel::Select);

			mSelectFile = "";
		}
	}
}

void FileDialog::folderChildrenFailed(const QString& error, const QString& /*locationPath*/, bool permissionError)
{
	mFileListModel->clear();

	showStatus(QPixmap(":/icons/error.png"), QString("Error: " + error));
	ui->statusWidget->setButtons(StatusWidget::Retry |
		(permissionError && !mCurrentLocation.isSudo() && mCurrentLocation.canSudo() ? StatusWidget::SudoRetry : StatusWidget::None));
}

void FileDialog::retryButtonClicked(StatusWidget::Button button)
{
	switch (button)
	{
	case StatusWidget::SudoRetry:
		showLocation(mCurrentLocation.getSudoLocation());
		break;

	case StatusWidget::Retry:
		showLocation(mCurrentLocation);
		break;

	default: break;
	}
}

void FileDialog::showLocation(const Location& location)
{
	if (location.isNull())
		return;

	mLastLocation = location;

	ui->currentPath->setText(location.getDisplayPath());
	mCurrentLocation = location;

	mFileListModel->clear();
	showStatus(QPixmap(":/icons/loading.png"), tr("Loading ..."));

	mCurrentLocation.asyncGetChildren();
}

void FileDialog::showStatus(const QPixmap& icon, const QString& text)
{
	ui->statusWidget->setStatus(icon, text);
	ui->fileListStack->setCurrentWidget(ui->loaderLayer);
	ui->statusWidget->setButtons(StatusWidget::None);
}

void FileDialog::directoryTreeSelected(QTreeWidgetItem* item)
{
	int nodeType = item->data(0, TYPE_ROLE).toInt();
	switch (nodeType)
	{
		case NODETYPE_LOCATION:
		{
			Location location = item->data(0, DATA_ROLE).value<Location>();
			if (!location.isNull())
				showLocation(location);
			break;
		}

		case NODETYPE_FAVORITE:
		{
			showLocation(Location(item->data(0, DATA_ROLE).toString()));
			break;
		}

		case NODETYPE_ADD_LOCATION:
		{
			SshHost::getHost("", "", true);
			break;
		}

		case NODETYPE_ADD_FAVORITE:
		{
			addToFavorites();
			break;
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
		if(selections.length() > 1 || (selections.length() > 0 && !selections[0].isDirectory()) || (selections.length() == 0 && ui->fileName->text().length() > 0))
			accept();
		else if(selections.length() > 0 && selections[0].isDirectory())
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

void FileDialog::addToFavorites()
{
	mCurrentLocation.addToFavorites();
	updateFavorites();
}

void FileDialog::updateFavorites()
{
	//	take inventory of the favorites in the list now...
	QMap<QString, bool> currentList;
	for (int i = 0; i < mFavoriteLocationsBranch->childCount(); i++)
	{
		QTreeWidgetItem* child = mFavoriteLocationsBranch->child(i);
		currentList.insert(child->data(0, DATA_ROLE).toString(), false);
	}

	//	Go through the list of favorites. Add new entries, mark existing ones as keepers
	QList<Location::Favorite> favorites = Location::getFavorites();
	foreach (Location::Favorite f, favorites)
	{
		if (currentList.contains(f.path))
			currentList.insert(f.path, true);
		else
		{
			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(0, f.name);
			item->setData(0, DATA_ROLE, QVariant::fromValue<QString>(f.path));
			item->setData(0, TYPE_ROLE, QVariant(NODETYPE_FAVORITE));
			mFavoriteLocationsBranch->addChild(item);
		}
	}

	//	Remove list entries that don't belong
	for (int i = 1; i < mFavoriteLocationsBranch->childCount(); i++)
	{
		QTreeWidgetItem* child = mFavoriteLocationsBranch->child(i);
		QString path = child->data(0, DATA_ROLE).toString();
		if (!currentList.value(path, true))
		{
			i--;
			delete child;
		}
	}
}

void FileDialog::directoryTreeContextMenu(QPoint point)
{
	QTreeWidgetItem* item = ui->directoryTree->itemAt(point);
	if (!item) return;
	int nodeType = item->data(0, TYPE_ROLE).toInt();

	point = ui->directoryTree->mapToGlobal(point);

	switch (nodeType)
	{
		case NODETYPE_FAVORITE:
		{
			//	Get the favorite item's path
			QString path = item->data(0, DATA_ROLE).toString();

			//	Show a context menu appropriate to favorites
			QMenu* contextMenu = new QMenu(this);
			QAction* deleteAction = contextMenu->addAction(tr("Delete Favorite"));
			QAction* selectedAction = contextMenu->exec(point);

			if (selectedAction == deleteAction)
			{
				Location::deleteFavorite(path);
				updateFavorites();
			}

			break;
		}
	}
}

void FileDialog::createNewFolder()
{
	NewFolderDialog dlg;
	if(dlg.exec())
	{
		mCurrentLocation.createNewDirectory(dlg.folderName());
		showLocation(mCurrentLocation);
	}
}

void FileDialog::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("text/uri-list"))
		event->acceptProposedAction();
}

void FileDialog::dropEvent(QDropEvent *event)
{
	QList<QUrl> urls = event->mimeData()->urls();
	if (urls.isEmpty())
		return;

	QString fileName = urls.first().toLocalFile();
	if (fileName.isEmpty())
		return;

	Location loc(fileName);

	if(loc.isDirectory())
		showLocation(loc);
	else
	{
		showLocation(loc.getDirectory());
		mSelectFile = loc.getLabel();
	}
}

void FileDialog::refresh()
{
	showLocation(mCurrentLocation);
}
