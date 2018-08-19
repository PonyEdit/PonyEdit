#include <QDebug>
#include <QJsonDocument>
#include <utility>

#include "main/tools.h"
#include "serverrequest.h"

ServerRequest::ServerRequest( ServerFile *file,
                              QByteArray request,
                              QVariant parameters,
                              const Callback &callback ) :
	mFile( file ),
	mOpeningFile( nullptr ),
	mRequest( std::move( request ) ),
	mParameters( std::move( parameters ) ),
	mMessageId( 0 ),
	mPackedRequest() {
	if ( callback.getFailureSlot() ) {
		connect( this,
		         SIGNAL( requestFailure( QString, int ) ),
		         callback.getTarget(),
		         callback.getFailureSlot() );
	}
	if ( callback.getSuccessSlot() ) {
		connect( this,
		         SIGNAL( requestSuccess( QVariantMap ) ),
		         callback.getTarget(),
		         callback.getSuccessSlot() );
	}
}

const QByteArray &ServerRequest::prepare( int bufferId ) {
	QVariantMap requestRoot;
	requestRoot.insert( "i", mMessageId );
	requestRoot.insert( "c", mRequest );
	requestRoot.insert( "p", mParameters );

	if ( bufferId > -1 ) {
		requestRoot.insert( "b", bufferId );
	}

	mPackedRequest = QJsonDocument::fromVariant( QVariant( requestRoot ) ).toJson();
	mPackedRequest = Tools::bin( mPackedRequest );
	mPackedRequest += "\n";

	// "bin" the request; clear out characters that are trouble for ssh comms

	return mPackedRequest;
}

void ServerRequest::failRequest( const QString &error, int errorFlags ) {
	emit requestFailure( error, errorFlags );
}

void ServerRequest::handleReply( const QVariantMap &reply ) {
	if ( ! reply.contains( "error" ) ) {
		emit requestSuccess( reply );
	} else {
		QByteArray code = reply.value( "code" ).toByteArray();
		int errorFlags = 0;

		if ( code == "perm" ) {
			errorFlags |= PermissionError;
		}

		failRequest( reply.value( "error" ).toString(), errorFlags );
	}
}
