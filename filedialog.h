#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QMap>
#include <QDialog>
#include <QFileIconProvider>
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

private slots:
	void folderTreeItemExpanded(QTreeWidgetItem* item);
	void folderChildrenLoaded(const QList<Location>& children, const QString& locationPath);
	void folderChildrenFailed(const QString& error, const QString& locationPath);

private:
	void populateFolderTree();
	QTreeWidgetItem* addLocationToTree(QTreeWidgetItem* parent, const Location& location);

	void addLocalFile(const QString& label, const QFileInfo& fileInfo, QTreeWidgetItem* parent);
	void addLocalFile(const QFileInfo& fileInfo, QTreeWidgetItem* parent) { addLocalFile(fileInfo.fileName(), fileInfo, parent); }

    Ui::FileDialog *ui;
	QFileIconProvider mIconProvider;

	QMap<QString, QTreeWidgetItem*> mLoadingLocations;
};

#endif // FILEDIALOG_H
