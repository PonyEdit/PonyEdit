#include "main/tools.h"
#include "options/options.h"
#include "mainwindow.h"
#include "windowmanager.h"
#include "editor/editor.h"
#include "file/openfilemanager.h"
#include "ssh2/sshhost.h"

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
		if (host->getSaveHost())
		{
			settings.setArrayIndex(index++);
			settings.setValue("hostname", host->getHostname());
			settings.setValue("port", host->getPort());
			settings.setValue("username", host->getUsername());
			settings.setValue("savePassword", host->getSavePassword());
			settings.setValue("password", host->getSavePassword() ? host->getPassword().toBase64() : "");
			settings.setValue("keyFile", host->getKeyFile());
			settings.setValue("saveKeyPassphrase", host->getSaveKeyPassphrase());
			settings.setValue("keyPassphrase", host->getSaveKeyPassphrase() ? host->getKeyPassphrase().toBase64() : "");
			settings.setValue("name", host->getName());
			settings.setValue("defaultDirectory", host->getDefaultDirectory());
			settings.setValue("connectionType", host->getConnectionType());
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

		host->setHostname(settings.value("hostname").toByteArray());
		host->setPort(settings.value("port", 22).toInt());
		host->setUsername(settings.value("username").toByteArray());
		host->setKeyFile(settings.value("keyFile").toByteArray());

		QByteArray password = QByteArray::fromBase64(settings.value("password").toByteArray());
		bool savePassword = settings.value("savePassword", password.length() > 0).toBool();
		if (savePassword)
		{
			host->setPassword(password);
			host->setSavePassword(savePassword);
		}

		QByteArray keyPassphrase = QByteArray::fromBase64(settings.value("keyPassphrase").toByteArray());
		bool saveKeyPassphrase = settings.value("saveKeyPassphrase", keyPassphrase.length() > 0).toBool();
		if (saveKeyPassphrase)
		{
			host->setKeyPassphrase(keyPassphrase);
			host->setSaveKeyPassphrase(saveKeyPassphrase);
		}

		host->setName(settings.value("name").toString());
		host->setDefaultDirectory(settings.value("defaultDirectory", QVariant("~")).toByteArray());
		host->setConnectionType((SshHost::ConnectionType)settings.value("connectionType", QVariant(SshHost::SSH)).toInt());

//		host->setConnectionType(static_cast<OldSshHost::ConnectionType>(settings.value("connectionType", QVariant(OldSshHost::SSH)).toInt()));

		SshHost::recordKnownHost(host);
	}
	settings.endArray();


	count = settings.beginReadArray("hostkeys");
	for (int i = 0; i < count; i++)
	{
		settings.setArrayIndex(i);
		QString hostname = settings.value("hostname").toString();
		QByteArray key = settings.value("hostkey").toByteArray();
		SshHost::registerKnownFingerprint(hostname, key);
	}
}

void Tools::saveHostFingerprints()
{
	QSettings settings;

	const QMap<QString, QByteArray>& fingerprints = SshHost::getKnownFingerprints();

	int index = 0;
	settings.beginWriteArray("hostkeys");
	for (QMap<QString, QByteArray>::const_iterator i = fingerprints.begin(); i != fingerprints.end(); ++i)
	{
		settings.setArrayIndex(index++);
		settings.setValue("hostname", i.key());
		settings.setValue("hostkey", i.value());
	}
	settings.endArray();
}

QList<Location> Tools::loadRecentFiles()
{
	QSettings settings;

	QList<Location> recentFiles;

	int count = settings.beginReadArray("recentFiles");
	for (int i = 0; i < count; i++)
	{
		settings.setArrayIndex(i);
		recentFiles.append(Location(settings.value("path").toString()));
	}
	settings.endArray();

	return recentFiles;
}

void Tools::saveRecentFiles(QList<Location> recentFiles)
{
	QSettings settings;

	int index = 0;
	settings.beginWriteArray("recentFiles");
	foreach (Location loc, recentFiles)
	{
		settings.setArrayIndex(index++);
		settings.setValue("path", loc.getDisplayPath());
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
	#ifdef Q_OS_MAC
		availableWidth -= 2;
	#endif

	int fullWidth = metrics.boundingRect(label).width();
	int shortFall = fullWidth - availableWidth;

	int cursor = 0;
	QString result = label;
	while (shortFall > 0)
	{
		int nextSeparator = result.indexOf(separators, cursor);
		if (nextSeparator == -1)
			return metrics.elidedText(result, Qt::ElideMiddle, availableWidth);

		QString shorten = result.mid(cursor, nextSeparator - cursor);
		int cullLength = metrics.boundingRect(shorten.mid(1)).width();

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

QString Tools::getResourcePath(const QString& subpath)
{
#ifdef Q_OS_MAC && !defined QT_DEBUG
    return QCoreApplication::applicationDirPath() + QString("/../Resources/") + subpath;
#elif defined Q_OS_WIN && !defined QT_DEBUG
	return QCoreApplication::applicationDirPath() + QString("/") + subpath;
#elif defined Q_OS_LINUX && !defined QT_DEBUG
	return QCoreApplication::applicationDirPath() + QString("/") + subpath;
#else
	return subpath;
#endif
}

void Tools::loadStartupFiles()
{
	switch(Options::StartupAction)
	{
		case Options::BlankFile:
			gMainWindow->openSingleFile(Location(""));
			break;
		case Options::SetFiles:
		case Options::ReopenFiles:
			for(int ii = 0; ii < Options::StartupFiles.length(); ii++)
			{
				QString name = Options::StartupFiles[ii].trimmed();

				if(name.isNull())
					continue;

				gMainWindow->openSingleFile(Location(name));

				Editor* current = gMainWindow->getCurrentEditor();
				if (current)
					current->gotoLine(Options::StartupFilesLineNo[ii]);
			}
			break;
		case Options::NoFiles:
		default:
			return;
	}
}

void Tools::saveCurrentFiles()
{
	if(Options::StartupAction != Options::ReopenFiles)
		return;

	Options::StartupFiles.clear();
	Options::StartupFilesLineNo.clear();

	QList<BaseFile*> files = gOpenFileManager.getOpenFiles();

	foreach(BaseFile* file, files)
	{
		Location loc = file->getLocation();
		if(loc.getProtocol() == Location::Unsaved)
			continue;

		Options::StartupFiles.append(loc.getDisplayPath());
		if (file->getAttachedEditors().size() > 0)
			Options::StartupFilesLineNo.append(file->getAttachedEditors().at(0)->currentLine());
	}

	Options::save();
}

QStringList Tools::splitQuotedList(const QString& quotedList, QChar separator)
{
	bool inQuotes = false;
	QStringList result;
	QString current;

	for (int i = 0; i < quotedList.length(); i++)
	{
		const QChar& c = quotedList.at(i);
		if (!inQuotes && c == separator)	//	Separator; push current to list
		{
			if (current.length() > 0)
				result.append(current);
			current.clear();
		}
		else if (c == '\\')		//	Escape char; skip next char
			i++;
		else if (c == '"')		//	Handle quotes
			inQuotes = !inQuotes;
		else if (current.length() || !c.isSpace())
			current.append(c);
	}

	if (current.length() > 0)
		result.append(current);

	return result;
}

QString Tools::stringifyIpAddress(unsigned long ipAddress)
{
	return QString("%1.%2.%3.%4").arg(ipAddress & 0xFF)
			.arg((ipAddress >> 8) & 0xFF)
			.arg((ipAddress >> 16) & 0xFF)
			.arg((ipAddress >> 24) & 0xFF);
}

QByteArray Tools::bin(const QByteArray& source)
{
	const unsigned char* c = (const unsigned char*)source.constData();
	const unsigned char* end = c + source.length();
	QByteArray result;

	while (c < end)
	{
		if (*c == 0x3 || *c == 0x4 || *c == 0x8 || *c == 0x11 || *c == 0x13 || *c == 0x1D || *c == 0x1E || *c == 0x18 || *c == 0x1A || *c == 0x1C || *c == 0x7F)
			result.append(255).append(*c + 128);
		else if (*c == 10)
			result.append(253);
		else if (*c == 13)
			result.append(254);
		else if (*c == 253)
			result.append(255).append('A');
		else if (*c == 254)
			result.append(255).append('B');
		else if (*c == 255)
			result.append(255).append('C');
		else
			result.append(*c);

		c++;
	}

	return result;
}

unsigned char Tools::unbinEscape(unsigned char c)
{
	return (c < 128 ? c + 188 : c - 128);
}

int Tools::unbin(QByteArray& target, const char* source, int maxTarget, int maxSource, bool* leftoverEscape)
{
	const unsigned char* unsignedSource = (const unsigned char*)source;
	const unsigned char* sourceEnd = unsignedSource + maxSource;
	const unsigned char* c = unsignedSource;

	if (leftoverEscape && *leftoverEscape && c < sourceEnd)
	{
		target.append(unbinEscape(*c));
		*leftoverEscape = false;
		c++;
	}

	for (; c < sourceEnd; c++)
	{
		if (target.length() >= maxTarget) return (c - unsignedSource);

		if (*c < 253)
			target.append(*c);
		else if (*c == 253)
			target.append((unsigned char)10);
		else if (*c == 254)
			target.append((unsigned char)13);
		else
		{
			c++;
			if (c >= sourceEnd)
			{
				if (leftoverEscape) *leftoverEscape = true;
				break;
			}

			target.append(unbinEscape(*c));
		}
	}

	return c - unsignedSource;
}






