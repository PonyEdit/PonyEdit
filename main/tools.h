#ifndef TOOLS_H
#define TOOLS_H

#include <QFontMetrics>
#include <QString>
#include <QtXml>
#include <QMap>

#include "file/location.h"

class QThread;

class Tools
{
public:
	static QString humanReadableBytes(quint64 bytes);
	static void saveServers();
	static void loadServers();
	static bool isMainThread();

	static QList<Location> loadRecentFiles();
	static void saveRecentFiles(QList<Location> recentFiles);

	static void loadStartupFiles();
	static void saveCurrentFiles();

	static void initialize();

	static QString squashLabel(const QString& label, const QFontMetrics& metrics, int width);

	static QString getStringXmlAttribute(const QXmlAttributes& attribs, const QString& key);
	static QChar getCharXmlAttribute(const QXmlAttributes& attribs, const QString& key);
	static int getIntXmlAttribute(const QXmlAttributes& attribs, const QString& key, int defaulVal);

	static bool compareSubstring(const QString& superstring, const QString& substring, int superstringIndex, Qt::CaseSensitivity caseSensitivity);

	static QString getResourcePath(const QString& subpath);
};

#endif // TOOLS_H
