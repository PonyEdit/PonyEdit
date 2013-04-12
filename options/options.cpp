#include "options.h"
#include <QSettings>
#include <QWidget>
#include "main/global.h"
#include <QCheckBox>
#include <QLineEdit>

QFont* Options::EditorFont = NULL;
int Options::EditorFontZoom;
int Options::TabStopWidth;
bool Options::WordWrap;
Options::IndentModes Options::IndentMode;
bool Options::IndentSpaces;
bool Options::StripSpaces;

Options::StartupActions Options::StartupAction;
QStringList Options::StartupFiles;
QList<int> Options::StartupFilesLineNo;
bool Options::ShutdownPrompt;

Options::FileListTypes Options::FileListType;

int Options::LoggingLevel;

Options* Options::sInstance = NULL;

Options* Options::getInstance()
{
	if (!sInstance)
		sInstance = new Options();
	return sInstance;
}

QVariant Options::get(const QString& key, const QVariant& defaultValue)
{
	QSettings settings;
	return settings.value(key, defaultValue);
}

void Options::set(const QString& key, const QVariant& value)
{
	QSettings settings;
	settings.setValue(key, value);
}

void Options::autoPersist(QCheckBox* control, const QString& optionKey, bool defaultValue)
{
	control->setChecked(get(optionKey, defaultValue).toBool());
	getInstance()->autoPersist(control, optionKey, SIGNAL(clicked(bool)), SLOT(persistantCheckBoxChanged(bool)));
}

void Options::autoPersist(QLineEdit* control, const QString& optionKey, const QString& defaultValue)
{
	control->setText(get(optionKey, defaultValue).toString());
	getInstance()->autoPersist(control, optionKey, SIGNAL(textChanged(QString)), SLOT(persistantLineEditChanged(QString)));
}

void Options::autoPersist(QWidget* control, const QString& optionKey, const char* changedSignal, const char* persistSlot)
{
	mPersistantKeys.insert(control, optionKey);
	connect(control, changedSignal, this, persistSlot);
	connect(control, SIGNAL(destroyed(QObject*)), this, SLOT(endAutoPersist(QObject*)));
}

void Options::endAutoPersist(QObject* control)
{
	mPersistantKeys.remove(control);
}

void Options::persistantCheckBoxChanged(bool checked)
{
	persistValue(QObject::sender(), checked);
}

void Options::persistantLineEditChanged(const QString& text)
{
	persistValue(QObject::sender(), text);
}

void Options::persistValue(QObject* control, const QVariant& value)
{
	QString optionKey = mPersistantKeys.value(control);
	if (!optionKey.isEmpty()) {
		set(optionKey, value);
	}
}

void Options::save()
{
	QSettings settings;

	settings.setValue(ntr("editorFont"), QVariant(EditorFont->toString()));
	settings.setValue(ntr("editorFontZoom"), QVariant(EditorFontZoom));
	settings.setValue(ntr("wordWrap"), QVariant(WordWrap));
	settings.setValue(ntr("tabStopWidth"), QVariant(TabStopWidth));
	settings.setValue(ntr("indentMode"), QVariant(static_cast<int>(IndentMode)));
	settings.setValue(ntr("indentSpaces"), QVariant(static_cast<int>(IndentSpaces)));
	settings.setValue(ntr("stripSpaces"), QVariant(StripSpaces));

	settings.setValue(ntr("StartupAction"), QVariant(static_cast<int>(StartupAction)));
	settings.setValue(ntr("ShutdownPrompt"), QVariant(ShutdownPrompt));

	settings.setValue(ntr("FileListType"), QVariant(static_cast<int>(FileListType)));

	settings.setValue(ntr("LoggingLevel"), QVariant(LoggingLevel));

	settings.beginWriteArray("StartupFiles");
	int skipped = 0;
	QString file;
	for(int ii = 0; ii + skipped < StartupFiles.length(); ii++)
	{
		file = StartupFiles[ii].trimmed();
		if(file.isNull())
		{
			skipped++;
			continue;
		}
		settings.setArrayIndex(ii - skipped);
		settings.setValue(ntr("path"), QVariant(file));
		settings.setValue(ntr("line"), QVariant(StartupFilesLineNo[ii]));
	}
	settings.endArray();
}

void Options::load()
{
	QSettings settings;

	QString fontString = settings.value(ntr("editorFont")).toString();
	if (EditorFont != NULL)
		delete EditorFont;
	EditorFont = new QFont();
	EditorFont->fromString(fontString);

	//	Sanity check the font
	if (EditorFont->family().isEmpty())
		EditorFont->setFamily(ntr("inconsolata"));
	if (EditorFont->pointSize() < 5)
		EditorFont->setPointSize(12);

	EditorFontZoom = settings.value(ntr("editorFontZoom"), QVariant(100)).toInt();
	WordWrap = settings.value(ntr("wordWrap"), QVariant(false)).toBool();
	TabStopWidth = settings.value(ntr("tabStopWidth"), QVariant(8)).toInt();
	IndentMode = static_cast<IndentModes>(settings.value(ntr("indentMode"), QVariant(static_cast<int>(KeepIndentOnNextLine))).toInt());
	IndentSpaces = settings.value(ntr("indentSpaces"), QVariant(false)).toBool();
	StripSpaces = settings.value(ntr("stripSpaces"), QVariant(true)).toBool();

	StartupAction = static_cast<StartupActions>(settings.value(ntr("StartupAction"), QVariant(static_cast<int>(NoFiles))).toInt());
	ShutdownPrompt = settings.value(ntr("ShutdownPrompt"), QVariant(true)).toBool();

	FileListType = static_cast<FileListTypes>(settings.value(ntr("FileListType"), QVariant(static_cast<int>(QuickList))).toInt());

	LoggingLevel = settings.value(ntr("LoggingLevel"), QVariant(QsLogging::InfoLevel)).toInt();
	QsLogging::Logger::instance().setLoggingLevel((QsLogging::Level)LoggingLevel);

	int count = settings.beginReadArray("StartupFiles");
	for (int ii = 0; ii < count; ii++)
	{
		settings.setArrayIndex(ii);

		StartupFiles.append(settings.value(ntr("path")).toString());
		StartupFilesLineNo.append(settings.value(ntr("line"), QVariant(1)).toInt());
	}
	settings.endArray();
}
