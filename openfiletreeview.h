#ifndef OPENFILETREEVIEW_H
#define OPENFILETREEVIEW_H

#include <QTreeView>

class OpenFileTreeModel;
class OpenFileItemDelegate;
class BaseFile;

class OpenFileTreeView : public QTreeView
{
    Q_OBJECT

public:
	enum OptionFlags { NoFlags = 0x00, CloseButtons = 0x01, MultiSelect = 0x02, UnsavedOnly = 0x04 };

	explicit OpenFileTreeView(QWidget* parent, int optionFlags, const QList<BaseFile*>* files = NULL);
	virtual void dataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight);

	BaseFile* getSelectedFile() const;      //  Fetches one selected file. If multiple selected, returns only the first. Mostly useful when in single selection mode.
	void selectFile(BaseFile* file);

private slots:
	void itemClicked(QModelIndex index);

private:
	OpenFileTreeModel* mModel;
	OpenFileItemDelegate* mDelegate;
};

#endif // OPENFILETREEVIEW_H
