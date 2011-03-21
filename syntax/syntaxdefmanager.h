#ifndef SYNTAXDEFMANAGER_H
#define SYNTAXDEFMANAGER_H

#include <QList>
#include "syntaxdefinition.h"

class SyntaxDefManager
{
public:
	struct FilePattern
	{
		FilePattern(const QString& pattern);
		bool matches(const QString& filename);

		bool isSimpleExtension;
		QRegExp regExp;
		QString extension;
		QString rawPattern;
	};

	struct Record
	{
		Record() : valid(false), priority(0) {}
		void pack(const QXmlAttributes& atts);

		QString filename;
		QString syntaxName;
		QString category;
		QList<FilePattern> patterns;
		QDateTime lastUpdated;
		bool valid;
		int priority;
		bool hidden;
	};

    SyntaxDefManager();
	~SyntaxDefManager();

	SyntaxDefinition* getDefinitionForFile(const QString& filename);
	SyntaxDefinition* getDefinitionForSyntax(const QString& syntaxName);
	SyntaxDefinition* getDefinition(const Record* record);

	QStringList getDefinitionCategories() const;
	QStringList getSyntaxesInCategory(const QString& category) const;
	QStringList getFiltersForCategory(const QString& category) const;

private:
	void updateIndex();
	void indexFile(const QFileInfo& fileinfo);
	void addRecord(Record* record);
	Record* getRecordFor(const QString& filename);

	QList<Record*> mRecordList;
	QMap<QString, Record*> mRecordsByName;
	QMap<QString, QString> mSyntaxesByCategory;
	QMap<QString, QString> mFiltersByCategory;

	QList<SyntaxDefinition*> mOpenDefinitionList;
	QMap<QString, SyntaxDefinition*> mOpenDefinitionsByName;
};

extern SyntaxDefManager* gSyntaxDefManager;

#endif // SYNTAXDEFMANAGER_H
