#ifndef SSHFILE_H
#define SSHFILE_H

#include "basefile.h"

class SlaveChannel;
class RemoteConnection;
class SlaveFile : public BaseFile
{
	Q_OBJECT

public:
	BaseFile* newFile(const QString& content);
	void open();
	void save();
	void close();
	void fileOpened(int bufferId, const QString& content, const QString& checksum, bool readOnly);

	SlaveFile(const Location& location);	//	Do not call; use File::getFile instead.
	void reconnect();

	void resyncError(const QString& error);

public slots:
	void connectionStateChanged();
	void resyncSuccess(int revision);

signals:
	void resyncSuccessRethreadSignal(int);

protected:
	virtual ~SlaveFile();
	virtual void handleDocumentChange(int position, int removeChars, const QString& insert);
	virtual void setLastSavedRevision(int lastSavedRevision);
	void pumpChangeQueue();
	void resync();
	void movePumpCursor(int revision);
	void getChannel();

	SshHost* mHost;
	RemoteConnection* mConnection;
	SlaveChannel* mChannel;		//	Not available before opening the file.
	int mBufferId;

	QList<Change*> mChangesSinceLastSave;
	quint64 mChangeBufferSize;
	int mChangePumpCursor;

private:
	bool mCreatingNewFile;
};

#endif // SSHFILE_H
