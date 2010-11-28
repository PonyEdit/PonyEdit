#ifndef OPENFILEMODEL_H
#define OPENFILEMODEL_H

#include <QAbstractItemModel>
#include <QList>
#include "location.h"

class BaseFile;

class OpenFileModel : public QAbstractItemModel
{
    Q_OBJECT
public:
	enum Roles { LocationRole = Qt::UserRole, FileRole = Qt::UserRole + 1 };

	explicit OpenFileModel();
	~OpenFileModel();

	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex& index) const;
	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	int columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex &index, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;

	BaseFile* getFile(const Location& location) const;
	void registerFile(BaseFile* file);
	QModelIndex findFile(BaseFile* file) const;
	void closeButtonClicked(QModelIndex index);
	void closeFiles(const QList<BaseFile*>& files);

private slots:
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

	QList<BaseFile*> mOpenFiles;
	Entry* mTopLevelEntry;
	QMap<BaseFile*, Entry*> mFileLookup;
};

extern OpenFileModel gOpenFileModel;

#endif // OPENFILEMODEL_H
