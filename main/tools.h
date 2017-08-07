#ifndef TOOLS_H
#define TOOLS_H

#include <QFontMetrics>
#include <QMap>
#include <QString>
#include <QtXml>

#include "file/location.h"

class QThread;

class Tools
{
public:
static QString humanReadableBytes( quint64 bytes );
static void saveServers();
static void saveHostFingerprints();
static void loadServers();
static bool isMainThread();

static QList< Location > loadRecentFiles();
static void saveRecentFiles( QList< Location > recentFiles );

static void loadStartupFiles();
static void saveCurrentFiles();

static void initialize();

static QString squashLabel( const QString& label, const QFontMetrics& metrics, int width );

static QString getStringXmlAttribute( const QXmlAttributes& attribs, const QString& key );
static QChar getCharXmlAttribute( const QXmlAttributes& attribs, const QString& key );
static int getIntXmlAttribute( const QXmlAttributes& attribs, const QString& key, int defaulVal );

static bool compareSubstring( const QString& superstring,
                              const QString& substring,
                              int superstringIndex,
                              Qt::CaseSensitivity caseSensitivity );
static QStringList splitQuotedList( const QString& list, QChar separator );

static QString getResourcePath( const QString& subpath );
static void setResourcePath( const QString& path );

static QString stringifyIpAddress( unsigned long ipAddress );

static int unbin( QByteArray& target, const char* source, int maxTarget, int maxSource, bool* leftoverEscape = NULL );
static QByteArray bin( const QByteArray& source );
static unsigned char unbinEscape( unsigned char c );

private:
static QString sResourcePath;
};

#endif	// TOOLS_H
