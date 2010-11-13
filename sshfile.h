#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class SshRemoteController;
class SshFile : public BaseFile
{
	Q_OBJECT

public:
	void open();
	void save();
	void fileOpened(int bufferId, const QByteArray& content, const QString& checksum);

	SshFile(const Location& location);	//	Do not call; use File::getFile instead.
	void reconnect();

	void contentPushError(const QString& error);
	void contentPushSuccess();

public slots:
	void connectionStateChanged();

protected:
	virtual ~SshFile();
	virtual void handleDocumentChange(int position, int removeChars, const QByteArray& insert);
	virtual void setLastSavedRevision(int lastSavedRevision);
	void pumpChangeQueue();
	void pushContentToSlave();

	SshHost* mHost;
	SshRemoteController* mController;	//	Not available before opening the file.
	int mBufferId;

	QList<Change*> mChangesSinceLastSave;
	quint64 mChangeBufferSize;
	int mChangePumpCursor;
};

#endif // SSHFILE_H
