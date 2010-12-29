#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QMap>
#include <QDialog>
#include <QFileIconProvider>
#include <QStandardItemModel>
#include <QTreeWidgetItem>

#include "location.h"

namespace Ui { class FileDialog; }

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
private slots:
	void folderTreeItemExpanded(QTreeWidgetItem* item);
	void folderChildrenLoaded(const QList<Location>& children, const QString& locationPath);
	void folderChildrenFailed(const QString& error, const QString& locationPath);
	void directoryTreeSelected();
	void upLevel();
	void fileDoubleClicked(QModelIndex index);
	void populateRemoteServers();
	void fileListSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	void closing();
	void addToFavorites();

private:
	void keyPressEvent(QKeyEvent *);
	void restoreState();

	void populateFolderTree();
	QTreeWidgetItem* addLocationToTree(QTreeWidgetItem* parent, const Location& location);
	void updateFavorites();

	Ui::FileDialog *ui;
	QFileIconProvider mIconProvider;
	Location mCurrentLocation;
	QStandardItemModel* mFileListModel;
	QTreeWidgetItem* mRemoteServersBranch;
	QTreeWidgetItem* mFavoriteLocationsBranch;

	QMap<QString, QTreeWidgetItem*> mLoadingLocations;

	static Location mLastLocation;

	bool mSaveAs;
};

#endif // FILEDIALOG_H
