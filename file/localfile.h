#ifndef LOCALFILE_H
#define LOCALFILE_H

#include <QFile>
#include <QTextStream>

#include "basefile.h"

class LocalFile : public BaseFile {
	Q_OBJECT

	public:
		LocalFile( const Location& location );

		BaseFile* newFile( const QString& content );
		void open();
		void save();
		void close();
		void refresh();

	signals:
		void localFileOpened( const QString& content, const QByteArray& checksum, bool readOnly );
};

#endif  // LOCALFILE_H
