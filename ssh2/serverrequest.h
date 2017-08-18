#ifndef SERVEREQUEST_H
#define SERVEREQUEST_H

#include <QByteArray>
#include <QObject>
#include <QVariant>
#include "file/serverfile.h"
#include "tools/callback.h"

class ServerRequest : QObject {
	Q_OBJECT

	public:
		enum ErrorFlags { PermissionError = 0x01, ConnectionError = 0x02 };

		explicit ServerRequest( ServerFile *file,
		                        const QByteArray &request,
		                        const QVariant &parameters,
		                        const Callback &callback );

		inline void setMessageId( int messageId ) {
			mMessageId = messageId;
		}

		inline int getMessageId() const {
			return mMessageId;
		}

		inline ServerFile *getFile() const {
			return mFile;
		}

		inline const QByteArray &getRequest() const {
			return mRequest;
		}

		// Requests to open files don't pass a file ptr in to the constructor; the attach this separate opening
		// file pointer. This is because server channels use the mFile pointer to lock file-bound requests to
		// the channels with corresponding bufferIds.
		inline ServerFile *getOpeningFile() const {
			return mOpeningFile;
		}

		inline void setOpeningFile( ServerFile *file ) {
			mOpeningFile = file;
		}

		inline const QByteArray &getPackedRequest( int bufferId ) {
			return mPackedRequest.isNull() ? prepare( bufferId ) : mPackedRequest;
		}

		void handleReply( const QVariantMap &reply );
		void failRequest( const QString &error, int errorFlags );

	signals:
		void requestSuccess( QVariantMap results );
		void requestFailure( QString error, int errorFlags );

	private:
		const QByteArray &prepare( int bufferId );

		QPointer< ServerFile > mFile;
		QPointer< ServerFile > mOpeningFile;

		QByteArray mRequest;
		QVariant mParameters;

		int mMessageId;

		QByteArray mPackedRequest;
};

#endif  // SERVEREQUEST_H
