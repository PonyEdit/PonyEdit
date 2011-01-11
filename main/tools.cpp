#include "main/tools.h"
#include "ssh/sshhost.h"
#include <QSettings>
#include <QDebug>
#include <QThread>
#include <QRegExp>
#include <QtXml>

#define TERABYTE_MULTIPLIER	1099511627776ll
#define GIGABYTE_MULTIPLIER 1073741824
#define MEGABYTE_MULTIPLIER 1048576
#define KILOBYTE_MULTIPLIER 1024

QThread* sMainThread = NULL;

QString Tools::humanReadableBytes(quint64 bytes)
{
	if (bytes >= TERABYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)TERABYTE_MULTIPLIER, 'f', 1) + " TiB";
	else if (bytes >= GIGABYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)GIGABYTE_MULTIPLIER, 'f', 1) + " GiB";
	else if (bytes >= MEGABYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)MEGABYTE_MULTIPLIER, 'f', 1) + " MiB";
	else if (bytes >= KILOBYTE_MULTIPLIER)
		return QString::number((double)bytes / (double)KILOBYTE_MULTIPLIER, 'f', 1) + " KiB";
	else
		return QString::number(bytes) + " bytes";
}

void Tools::saveServers()
{
	QSettings settings;
	QList<SshHost*> knownHosts = SshHost::getKnownHosts();

	int index = 0;
	settings.beginWriteArray("servers");
	foreach (SshHost* host, knownHosts)
	{
		if (host->getSave())
		{
			settings.setArrayIndex(index++);
			settings.setValue("hostname", host->getHostName());
			settings.setValue("port", host->getPort());
			settings.setValue("username", host->getUserName());
			settings.setValue("password", host->getSavePassword() ? host->getPassword() : "");
			settings.setValue("keyFile", host->getKeyFile());
			settings.setValue("name", host->getName());
			settings.setValue("defaultDirectory", host->getDefaultDirectory());
		}
	}
	settings.endArray();
}

void Tools::loadServers()
{
	QSettings settings;

	int count = settings.beginReadArray("servers");
	for (int i = 0; i < count; i++)
	{
		settings.setArrayIndex(i);
		SshHost* host = new SshHost();

		host->setHostName(settings.value("hostname").toString());
		host->setPort(settings.value("port", 22).toInt());
		host->setUserName(settings.value("username").toString());
		host->setKeyFile(settings.value("keyFile").toString());
		host->setName(settings.value("name").toString());
		host->setDefaultDirectory(settings.value("defaultDirectory", QVariant("~")).toString());

		QString password = settings.value("password").toString();
		host->setPassword(password);

		SshHost::recordKnownHost(host);
	}
}

QList<Location*> Tools::loadRecentFiles()
{
	QSettings settings;

	QList<Location*> recentFiles;

	int count = settings.beginReadArray("recentFiles");
	for (int i = 0; i < count; i++)
	{
		settings.setArrayIndex(i);
		Location *loc = new Location(settings.value("path").toString());

		recentFiles.append(loc);
	}

	return recentFiles;
}

void Tools::saveRecentFiles(QList<Location*> recentFiles)
{
	QSettings settings;

	int index = 0;
	settings.beginWriteArray("recentFiles");
	foreach (Location* loc, recentFiles)
	{
		settings.setArrayIndex(index++);
		settings.setValue("path", loc->getDisplayPath());
	}
	settings.endArray();
}

bool Tools::isMainThread()
{
	return (QThread::currentThread() == sMainThread);
}

void Tools::initialize()
{
	sMainThread = QThread::currentThread();
}

QString Tools::squashLabel(const QString& label, const QFontMetrics& metrics, int availableWidth)
{
	QRegExp separators("[\\/\\\\@\\:\\.]");

	int fullWidth = metrics.size(Qt::TextSingleLine, label).width();
	int shortFall = fullWidth - availableWidth;

	int cursor = 0;
	QString result = label;
	while (shortFall > 0)
	{
		int nextSeparator = result.indexOf(separators, cursor);
		if (nextSeparator == -1)
			return metrics.elidedText(result, Qt::ElideMiddle, availableWidth);

		QString shorten = result.mid(cursor, nextSeparator - cursor);
		int cullLength = metrics.size(Qt::TextSingleLine, shorten.mid(1)).width();

		result.replace(cursor, shorten.length(), shorten[0]);
		cursor = cursor + 2;

		shortFall -= cullLength;
	}

	return result;
}

QString Tools::getStringXmlAttribute(const QXmlAttributes& attribs, const QString& key)
{
	for (int i = 0; i < attribs.length(); i++)
		if (attribs.localName(i).compare(key, Qt::CaseInsensitive) == 0)
			return attribs.value(i);
	return QString();
}

QChar Tools::getCharXmlAttribute(const QXmlAttributes& attribs, const QString& key)
{
	QString value = getStringXmlAttribute(attribs, key);
	if (value.isEmpty()) return QChar();
	return value.at(0);
}

int Tools::getIntXmlAttribute(const QXmlAttributes& attribs, const QString& key, int defaulVal)
{
	bool ok;
	QString stringValue = getStringXmlAttribute(attribs, key);
	int value = stringValue.toInt(&ok);
	if (ok) return value;

	if (stringValue.compare("true", Qt::CaseInsensitive) == 0)
		return 1;

	return defaulVal;
}

bool Tools::compareSubstring(const QString& superstring, const QString& substring, int superstringIndex, Qt::CaseSensitivity caseSensitivity)
{
	int l = substring.length();
	if (superstring.length() - superstringIndex < l)
		return false;

	const QChar* a = superstring.constData() + superstringIndex;
	const QChar* b = substring.constData();

	if (caseSensitivity == Qt::CaseInsensitive)
	{
		while (l-- && a->toLower() == b->toLower())
			a++,b++;
	}
	else
	{
		while (l-- && *a == *b)
			a++,b++;
	}

	return (l == -1);
}




