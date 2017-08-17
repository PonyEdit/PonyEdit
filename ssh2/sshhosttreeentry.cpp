#include <QStylePainter>
#include "sshhost.h"
#include "sshhosttreeentry.h"

bool SshHostTreeEntry::sCustomDrawKitInitialized = false;
QTextOption SshHostTreeEntry::sLabelTextOption;
QIcon SshHostTreeEntry::sDisconnectedIcon;
QIcon SshHostTreeEntry::sConnectingIcon[4];
QIcon SshHostTreeEntry::sConnectedIcon;
QIcon SshHostTreeEntry::sLogIcon;

#define GUTTER_LOG      1

SshHostTreeEntry::SshHostTreeEntry( SshHost *host ) :
	CustomTreeEntry( QIcon(), QString() ) {
	mHost = host;
	setData< SshHost * >( mHost );
	mAnimationFrame = 0;

	initializeCustomDrawKit();
	addGutterIcon( GUTTER_LOG, true, sLogIcon, "Show log" );

	connect( mHost, SIGNAL( overallStatusChanged() ), this, SLOT( invalidate() ) );
	connect( this, SIGNAL( gutterIconClicked( int ) ), this, SLOT( handleGutterIconClick( int ) ) );
}

void SshHostTreeEntry::initializeCustomDrawKit() {
	if ( sCustomDrawKitInitialized ) {
		return;
	}
	sLabelTextOption.setAlignment( Qt::AlignBottom | Qt::AlignLeft );
	sDisconnectedIcon = QIcon( ":/icons/status-red.png" );
	for ( int i = 0; i < 4; i++ ) {
		sConnectingIcon[i] = QIcon( QString( ":/icons/status-blue-%1.png" ).arg( i + 1 ) );
	}
	sConnectedIcon = QIcon( ":/icons/status-green.png" );
	sLogIcon = QIcon( ":/icons/log.png" );
}

void SshHostTreeEntry::customDraw( QPainter *painter, const QStyleOptionViewItem &option ) {
	initializeCustomDrawKit();
	QRect remainingArea = option.rect;

	drawGutterIcons( painter, &remainingArea );

	SshHost::Status hostStatus = mHost->getOverallStatus();

	// Draw the status
	switch ( hostStatus ) {
	case SshHost::Disconnected:
		drawIcon( painter, &remainingArea, sDisconnectedIcon );
		break;

	case SshHost::Connecting:
		drawIcon( painter, &remainingArea, sConnectingIcon[mAnimationFrame++], true );
		if ( mAnimationFrame > 3 ) {
			mAnimationFrame = 0;
		}
		drawGutterText( painter, &remainingArea, mHost->getConnectionString() );
		break;

	case SshHost::Connected:
		drawIcon( painter, &remainingArea, sConnectedIcon );
		break;
	}

	// Draw the text
	painter->drawText( remainingArea, mHost->getName(), sLabelTextOption );
}

void SshHostTreeEntry::handleGutterIconClick( int iconId ) {
	if ( iconId == GUTTER_LOG ) {
		mHost->showLog();
	}
}
