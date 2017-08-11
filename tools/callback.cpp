#include "callback.h"

Callback::Callback( QObject* target, const char* successSlot, const char* failureSlot, const char* progressSlot )
	: mTarget( target ),
	mSuccessSlot( successSlot ),
	mFailureSlot( failureSlot ),
	mProgressSlot( progressSlot ) {}

void Callback::triggerSuccess( QVariantMap result ) const {
	if ( mTarget == NULL ) {
		return;
	}

	CallbackDummy* tmp = new CallbackDummy();
	QObject::connect( tmp, SIGNAL( triggerSuccess( QVariantMap ) ), mTarget.data(), mSuccessSlot );
	tmp->emitTriggerSuccess( result );
	tmp->deleteLater();
}

void Callback::triggerFailure( QString error, int flags ) const {
	if ( mTarget == NULL ) {
		return;
	}

	CallbackDummy* tmp = new CallbackDummy();
	QObject::connect( tmp, SIGNAL( triggerFailure( QString, int ) ), mTarget.data(), mFailureSlot );
	tmp->emitTriggerFailure( error, flags );
	tmp->deleteLater();
}

void Callback::triggerProgress( int progress ) const {
	if ( mTarget == NULL ) {
		return;
	}

	CallbackDummy* tmp = new CallbackDummy();
	QObject::connect( tmp, SIGNAL( triggerProgress( int ) ), mTarget.data(), mProgressSlot );
	tmp->emitTriggerProgress( progress );
	tmp->deleteLater();
}

CallbackDummy::CallbackDummy() {}
void CallbackDummy::emitTriggerSuccess( QVariantMap result ) {
	emit triggerSuccess( result );
}

void CallbackDummy::emitTriggerFailure( QString error, int flags ) {
	emit triggerFailure( error, flags );
}

void CallbackDummy::emitTriggerProgress( int progress ) {
	emit triggerProgress( progress );
}
