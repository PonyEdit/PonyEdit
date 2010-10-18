#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QDialog>
#include <QFileIconProvider>
#include <QTreeWidgetItem>

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

private:
	void populateFolderTree();
	void addLocalFile(const QString& label, const QFileInfo& fileInfo, QTreeWidgetItem* parent);
	void addLocalFile(const QFileInfo& fileInfo, QTreeWidgetItem* parent) { addLocalFile(fileInfo.fileName(), fileInfo, parent); }

    Ui::FileDialog *ui;
	QFileIconProvider mIconProvider;
};

#endif // FILEDIALOG_H
