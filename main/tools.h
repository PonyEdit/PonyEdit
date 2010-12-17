#ifndef TOOLS_H
#define TOOLS_H

#include <QFontMetrics>
#include <QString>
#include <QtXml>
#include <QMap>

class QThread;

class Tools
{
public:
	static QString humanReadableBytes(quint64 bytes);
	static void saveServers();
	static void loadServers();
	static bool isMainThread();

	static void initialize();

	static QString squashLabel(const QString& label, const QFontMetrics& metrics, int width);

	static QString getStringXmlAttribute(const QXmlAttributes& attribs, const QString& key);
	static QChar getCharXmlAttribute(const QXmlAttributes& attribs, const QString& key);
	static int getIntXmlAttribute(const QXmlAttributes& attribs, const QString& key, int defaulVal);
};

#endif // TOOLS_H
