#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFont>
#include <QList>
#include <QStringList>
#include <QVariant>

class QLineEdit;
class QCheckBox;
class Options : QObject {
	Q_OBJECT

	public:
		enum IndentModes {
			NoAutoIndent = 0,
			KeepIndentOnNextLine = 1,
			SmartIndent = 2 // Not used yet; reserved.
		};

		enum StartupActions {
			NoFiles = 0,
			BlankFile = 1,
			ReopenFiles = 2,
			SetFiles = 3
		};

		enum FileListTypes {
			QuickList = 0,
			TabbedList = 1
		};

		static QVariant get( const QString &key, const QVariant &defaultValue = QVariant() );
		static void set( const QString &key, const QVariant &value );

// Auto-persist: a set-and-forget way to make controls retain their settings across PonyEdit sessions.
		static void autoPersist( QCheckBox *control, const QString &optionKey, bool defaultValue );
		static void autoPersist( QLineEdit *control, const QString &optionKey, const QString &defaultValue );

		static void save();
		static void load();

		static QFont *EditorFont;
		static int EditorFontZoom;
		static bool WordWrap;
		static int TabStopWidth;
		static IndentModes IndentMode;
		static bool IndentSpaces;       // Indent with spaces instead of tabs?
		static bool StripSpaces;

		static StartupActions StartupAction;
		static QStringList StartupFiles;
		static QList< int > StartupFilesLineNo;

		static bool ShutdownPrompt;

		static FileListTypes FileListType;

		static int LoggingLevel;

	private slots:
		void persistantCheckBoxChanged( bool checked );
		void persistantLineEditChanged( const QString &text );
		void endAutoPersist( QObject *control );

	private:
		static Options *getInstance();
		void autoPersist( QWidget *control, const QString &optionKey, const char *changedSignal, const char *persistSlot );
		void persistValue( QObject *control, const QVariant &value );

		static Options *sInstance;
		QMap< QObject *, QString > mPersistantKeys;
};

#endif  // OPTIONS_H
