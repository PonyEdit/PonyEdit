#include "sshfileengine.h"
#include "serverconfigdlg.h"

#include <QDebug>

QAbstractFileEngine* SshFileEngineHandler::create(const QString& filename) const
{
	qDebug() << filename;
	if (filename.startsWith("scp://"))
		return new SshFileEngine();
	return 0;
}

SshFileEngine::SshFileEngine()
	: QAbstractFileEngine()
{
	ServerConfigDlg dlg;
	dlg.exec();
}
