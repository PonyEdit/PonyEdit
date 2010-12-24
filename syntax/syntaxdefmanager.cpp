#include "syntaxdefmanager.h"
#include "syntaxdefxmlhandler.h"
#include "main/tools.h"
#include <QDir>

SyntaxDefManager gSyntaxDefManager;

void SyntaxDefManager::Record::pack(const QXmlAttributes& atts)
{
	syntaxName = Tools::getStringXmlAttribute(atts, "name");
	category = Tools::getStringXmlAttribute(atts, "section");
	priority = Tools::getIntXmlAttribute(atts, "priority", 0);
	hidden = Tools::getIntXmlAttribute(atts, "hidden", 0);

	QStringList patternStrings = Tools::getStringXmlAttribute(atts, "extensions").split(';');
	foreach (const QString& pattern, patternStrings)
	{
		QString trimmed = pattern.trimmed();
		if (trimmed.length())
			patterns.append(FilePattern(pattern));
	}

	if (!syntaxName.isEmpty() && !category.isEmpty())
		valid = true;
}

SyntaxDefManager::FilePattern::FilePattern(const QString& pattern)
{
	if ((isSimpleExtension = (pattern.startsWith("*.") && pattern.indexOf('*', 2) == -1)))
		extension = pattern.mid(1);
	else
		regExp = QRegExp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
}

bool SyntaxDefManager::FilePattern::matches(const QString& filename)
{
	if (isSimpleExtension)
		return filename.endsWith(extension);
	else
		return filename.contains(regExp);
}

SyntaxDefManager::SyntaxDefManager()
{
	updateIndex();
}

SyntaxDefManager::~SyntaxDefManager()
{
	foreach (Record* r, mRecordList)
		delete r;
	foreach (SyntaxDefinition* d, mOpenDefinitionList)
		delete d;
}

void SyntaxDefManager::updateIndex()
{
	QDir defDir("syntaxdefs/");
	QFileInfoList fileInfos = defDir.entryInfoList();
	foreach (QFileInfo info, fileInfos)
		indexFile(info);
}

void SyntaxDefManager::indexFile(const QFileInfo& fileinfo)
{
	Record* record = new Record();
	record->lastUpdated = QDateTime::currentDateTime();
	record->filename = fileinfo.filePath();

	//	Open the file and read just enough to pull out the <language> block
	if (fileinfo.isFile())
	{
		QFile file(fileinfo.filePath());
		if (file.open(QFile::ReadOnly))
		{
			SyntaxDefXmlHandler handler(record);
			QXmlSimpleReader reader;
			reader.setContentHandler(&handler);
			reader.setErrorHandler(&handler);
			reader.parse(&file);
		}
	}

	if (record->valid)
		addRecord(record);
}

void SyntaxDefManager::addRecord(Record *record)
{
	//	Keep the record list in priority order
	int i;
	for (i = 0; i < mRecordList.length(); i++)
	{
		if (mRecordList[i]->priority < record->priority)
			break;
	}
	mRecordList.insert(i, record);
	mRecordsByName.insert(record->syntaxName, record);
	if (!record->hidden) mSyntaxesByCategory.insertMulti(record->category, record->syntaxName);
}

QStringList SyntaxDefManager::getDefinitionCategories() const
{
	return mSyntaxesByCategory.uniqueKeys();
}

QStringList SyntaxDefManager::getSyntaxesInCategory(const QString& category) const
{
	return mSyntaxesByCategory.values(category);
}

SyntaxDefinition* SyntaxDefManager::getDefinitionForFile(const QString& filename)
{
	Record* record = getRecordFor(filename);
	if (record == NULL)
		return NULL;

	return getDefinition(record);
}

SyntaxDefinition* SyntaxDefManager::getDefinitionForSyntax(const QString& syntax)
{
	Record* record = (mRecordsByName.contains(syntax) ? mRecordsByName.value(syntax) : NULL);
	if (record == NULL)
		return NULL;

	return getDefinition(record);
}


SyntaxDefinition* SyntaxDefManager::getDefinition(const Record* record)
{
	if (mOpenDefinitionsByName.contains(record->syntaxName))
		return mOpenDefinitionsByName.value(record->syntaxName);

	SyntaxDefinition* newDefinition = new SyntaxDefinition(record->filename);
	if (!newDefinition->isValid())
	{
		delete newDefinition;
		qDebug() << "WARNING: Attempted to use an invalid syntax definition: " << record->filename;
		return NULL;
	}

	mOpenDefinitionList.append(newDefinition);
	mOpenDefinitionsByName.insert(record->syntaxName, newDefinition);
	return newDefinition;
}

SyntaxDefManager::Record* SyntaxDefManager::getRecordFor(const QString& filename)
{
	foreach (Record* record, mRecordList)
		foreach (FilePattern pattern, record->patterns)
			if (pattern.matches(filename))
				return record;
	return NULL;
}
















