#ifndef TOOLS_H
#define TOOLS_H

#include <QFontMetrics>
#include <QString>
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
};

#endif // TOOLS_H
