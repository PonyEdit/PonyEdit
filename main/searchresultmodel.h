#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include "file/location.h"

class BaseFile;

class SearchResultModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	struct Result
	{
		Result() : matchedLine(), location(), lineNumber(-1), start(-1), length(-1) {}
		Result(QString ml, const Location& loc, int ln, int s, int l)
			: matchedLine(ml), location(loc), lineNumber(ln), start(s), length(l) {}

		QString matchedLine;
		Location location;
		int lineNumber;
		int start;
		int length;
	};

    explicit SearchResultModel(QObject *parent = 0);
	~SearchResultModel();

	SearchResultModel(SearchResultModel const&) = delete;
	SearchResultModel& operator=(SearchResultModel const&) = delete;

	void addResult(const Result& result);
	void addResults(const QList<Result>& results);
	void clear();
	void setShowCheckboxes(bool checkboxes);

	void replaceSelectedResults(const QString& replacement);

	Result* getResultForIndex(const QModelIndex& index);

	virtual QModelIndex index(int row, int column, const QModelIndex& parent) const;
	virtual QModelIndex parent(const QModelIndex& child) const;
	virtual int rowCount(const QModelIndex& parent) const;
	virtual int columnCount(const QModelIndex& parent) const;
	virtual QVariant data(const QModelIndex& index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;

private:
	struct InternalTreeNode
	{
		InternalTreeNode() : result(), parent(NULL), children(), checked(Qt::Checked) {}
		~InternalTreeNode() { foreach (InternalTreeNode* n, children) delete n; }

		InternalTreeNode(InternalTreeNode const&) = delete;
		InternalTreeNode& operator=(InternalTreeNode const&) = delete;

		Result result;
		InternalTreeNode* parent;
		QList<InternalTreeNode*> children;
		Qt::CheckState checked;
	};

	InternalTreeNode* getNodeForIndex(const QModelIndex& index) const;
	InternalTreeNode* createFileNode(const Location& location);

	InternalTreeNode* mRootNode;
	QMap<QString, InternalTreeNode*> mFileNodeMap;

	bool mCheckboxes;
};

#endif // SEARCHRESULTMODEL_H
