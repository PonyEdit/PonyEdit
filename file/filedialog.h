#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QMap>
#include <QDialog>
#include <QFileIconProvider>
#include <QStandardItemModel>
#include <QTreeWidgetItem>
#include <QMouseEvent>
#include <QAbstractButton>
#include <QTableView>

#include "location.h"
#include "main/statuswidget.h"

#ifdef Q_OS_WIN
	#include <windows.h>
	#include <winnetwk.h>
#endif

namespace Ui { class FileDialog; }

//	Specialty tableview; used in the file list to force selection cursor to leftmost column
class SelectionlessTable : public QTableView
{
public:
	SelectionlessTable(QWidget* p) : QTableView(p) {}
	virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous)
	{
		if (current.column() > 0)
			this->setCurrentIndex(current.sibling(current.row(), 0));
	}
};

class FileDialog : public QDialog
{
    Q_OBJECT
public:
	explicit FileDialog(QWidget *parent = 0, bool saveAs = false);
    ~FileDialog();

	void showLocation(const Location& location);
	QList<Location> getSelectedLocations() const;
	Location getNewLocation() const;

public slots:
	void accept();

protected:
	void dragEnterEvent(QDragEnterEvent *);
	void dropEvent(QDropEvent *);

private slots:
	void folderTreeItemExpanded(QTreeWidgetItem* item);
	void folderChildrenLoaded(const QList<Location>& children, const QString& locationPath);
	void folderChildrenFailed(const QString& error, const QString& locationPath, bool permissionError);
	void directoryTreeSelected(QTreeWidgetItem*);
	void upLevel();
	void fileDoubleClicked(QModelIndex index);
	void populateRemoteServers();
	void fileListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void closing();
	void addToFavorites();
	void directoryTreeContextMenu(QPoint point);
	void createNewFolder();
	void retryButtonClicked(StatusWidget::Button button);	//	Called when "try again" or "sudo" is clicked on an error
	void refresh();
	void fileNameIndexChanged();
	void columnHeaderClicked(int column);
#ifdef Q_OS_WIN
	void populateWindowsShares(QTreeWidgetItem* localNetworkItem, LPNETRESOURCE lpnr);
#endif

private:
	void keyPressEvent(QKeyEvent *);
	void restoreState();
	void showStatus(const QPixmap& icon, const QString& text);
	void applySort();

	void populateFolderTree();
	QTreeWidgetItem* addLocationToTree(QTreeWidgetItem* parent, const Location& location);
	void updateFavorites();
        void populateFilterList();

	Ui::FileDialog *ui;
	QFileIconProvider mIconProvider;
	Location mCurrentLocation;
	QStandardItemModel* mFileListModel;
	QTreeWidgetItem* mRemoteServersBranch;
	QTreeWidgetItem* mFavoriteLocationsBranch;
#ifdef Q_OS_WIN
	QTreeWidgetItem* mLocalNetworkBranch;
#endif

	QMap<QString, QTreeWidgetItem*> mLoadingLocations;

	static Location mLastLocation;

	bool mSaveAs;
	bool mInEditHandler;
	int mSortingColumn;
	bool mReverseSorting;

	QString mSelectFile;
};

#endif // FILEDIALOG_H
