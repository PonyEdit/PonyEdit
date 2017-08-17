#ifndef DIALOGRETHREADER_H
#define DIALOGRETHREADER_H

#include <QCoreApplication>
#include <QEvent>
#include <QMutex>
#include <QObject>
#include <QVariantMap>
#include "ssh2/passworddlg.h"
#include "threadcrossingdialog.h"

//
// This is a helper class. It provides an easy way for non-UI threads to summon up a modal dialog.
// Make sure any cross thread dialogs inherit ThreadCrossingDialog instead of QDialog, then summon them thusly:
// QVariant result = DialogRethreader<MyDialog>::rethreadDialog(QVariant options);
//

class DialogRethreader : public QObject {
	Q_OBJECT

	public:
		explicit DialogRethreader();
		template < class T > static QVariantMap rethreadDialog( const QVariantMap& options ) {
			QMutex mutex;

			DialogRethreadRequest rq;
			rq.options = options;
			rq.factoryMethod = ( createDialog< T >);
			rq.lock = &mutex;

			DialogEvent* e = new DialogEvent( sRunDialogEventId );
			e->request = &rq;

			mutex.lock();
			QCoreApplication::postEvent( sInstance, e );

			// Use the mutex to wait for the dialog to finish; ui thread will unlock the mutex when done.
			mutex.lock();

			return rq.result;
		}

		bool event( QEvent* event );

	private:
		typedef ThreadCrossingDialog*(*DialogFactory)();

		struct DialogRethreadRequest {
			DialogFactory factoryMethod;
			QVariantMap options;
			QVariantMap result;
			QMutex* lock;
		};

		class DialogEvent : public QEvent {
			public:
				DialogEvent( int type ) :
					QEvent( ( QEvent::Type ) type ),
					request( NULL ) {}
				DialogRethreadRequest* request;
		};

		template < class T > static ThreadCrossingDialog* createDialog() {
			return new T();
		}

		static DialogRethreader* sInstance;
		static int sRunDialogEventId;
};

#endif  // DIALOGRETHREADER_H
