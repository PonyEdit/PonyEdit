#ifndef OPENFILEMODEL_H
#define OPENFILEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "location.h"

class BaseFile;

class OpenFileTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
	enum Roles { LocationRole = Qt::UserRole, FileRole = Qt::UserRole + 1 };

	OpenFileTreeModel(QObject* parent, int flags, const QList<BaseFile*>* explicitFiles = NULL);    // Displays explicitFiles if specified; if left NULL, gets a list of all currently open files
	~OpenFileTreeModel();

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	QModelIndex findFile(BaseFile* file) const;
	BaseFile* getFileAtIndex(const QModelIndex& index);
	QList<BaseFile*> getIndexAndChildFiles(const QModelIndex& index);

	void removeFile(BaseFile* file);            //  Only useful when explicitly specified files.

private slots:
	void fileOpened(BaseFile* file);
	void fileClosed(BaseFile* file);
	void fileChanged();

private:
	struct Entry
	{
		Entry() : parent(0), file(0) {}

		Entry* parent;
		BaseFile* file;
		Location location;
		QList<Entry*> children;
	};

	QModelIndex registerDirectory(const Location& location);
	QModelIndex addToTree(QModelIndex parent, Entry* entry);
	void removeEntry(Entry* entry);

	QList<BaseFile*> mFiles;   // Used if a list of files explicitly supplied
	Entry* mTopLevelEntry;
	QMap<BaseFile*, Entry*> mFileLookup;

	int mOptionFlags;
	bool mExplicitFiles;
};

#endif // OPENFILEMODEL_H
