#include "callback.h"

#include <utility>

Callback::Callback( QObject *target, const char *successSlot, const char *failureSlot, const char *progressSlot )
	: mTarget( target ),
	mSuccessSlot( successSlot ),
	mFailureSlot( failureSlot ),
	mProgressSlot( progressSlot ) {}

void Callback::triggerSuccess( QVariantMap result ) const {
	if ( mTarget == nullptr ) {
		return;
	}

	auto *tmp = new CallbackDummy();
	QObject::connect( tmp, SIGNAL( triggerSuccess( QVariantMap ) ), mTarget.data(), mSuccessSlot );
	tmp->emitTriggerSuccess( std::move( result ) );
	tmp->deleteLater();
}

void Callback::triggerFailure( QString error, int flags ) const {
	if ( mTarget == nullptr ) {
		return;
	}

	auto *tmp = new CallbackDummy();
	QObject::connect( tmp, SIGNAL( triggerFailure( QString, int ) ), mTarget.data(), mFailureSlot );
	tmp->emitTriggerFailure( std::move( error ), flags );
	tmp->deleteLater();
}

void Callback::triggerProgress( int progress ) const {
	if ( mTarget == nullptr ) {
		return;
	}

	auto *tmp = new CallbackDummy();
	QObject::connect( tmp, SIGNAL( triggerProgress( int ) ), mTarget.data(), mProgressSlot );
	tmp->emitTriggerProgress( progress );
	tmp->deleteLater();
}

CallbackDummy::CallbackDummy() = default;
void CallbackDummy::emitTriggerSuccess( QVariantMap result ) {
	emit triggerSuccess( std::move( result ) );
}

void CallbackDummy::emitTriggerFailure( QString error, int flags ) {
	emit triggerFailure( std::move( error ), flags );
}

void CallbackDummy::emitTriggerProgress( int progress ) {
	emit triggerProgress( progress );
}
