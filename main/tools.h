#ifndef TOOLS_H
#define TOOLS_H

#include <QFontMetrics>
#include <QString>
#include <QMap>

class QThread;
class QDomElement;

class Tools
{
public:
	static QString humanReadableBytes(quint64 bytes);
	static void saveServers();
	static void loadServers();
	static bool isMainThread();

	static void initialize();

	static QString squashLabel(const QString& label, const QFontMetrics& metrics, int width);

	static QString getStringXmlAttribute(const QDomElement* node, const QString& attribute);
	static QChar getCharXmlAttribute(const QDomElement* node, const QString& attribute);
	static int getIntXmlAttribute(const QDomElement* node, const QString& attribute, int defaultValue);
	static bool getBoolXmlAttribute(const QDomElement* node, const QString& attribute, bool defaultValue);
};

#endif // TOOLS_H
