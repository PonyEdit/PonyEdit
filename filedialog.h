#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QMap>
#include <QDialog>
#include <QFileIconProvider>
#include <QStandardItemModel>
#include <QTreeWidgetItem>
#include "location.h"

namespace Ui {
    class FileDialog;
}

class FileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FileDialog(QWidget *parent = 0);
    ~FileDialog();

	void showLocation(const Location& location);

private slots:
	void folderTreeItemExpanded(QTreeWidgetItem* item);
	void folderChildrenLoaded(const QList<Location>& children, const QString& locationPath);
	void folderChildrenFailed(const QString& error, const QString& locationPath);
	void directoryTreeSelected();

private:
	void populateFolderTree();
	QTreeWidgetItem* addLocationToTree(QTreeWidgetItem* parent, const Location& location);

	Ui::FileDialog *ui;
	QFileIconProvider mIconProvider;
	Location mCurrentLocation;
	QStandardItemModel* mFileListModel;

	QMap<QString, QTreeWidgetItem*> mLoadingLocations;
};

#endif // FILEDIALOG_H
