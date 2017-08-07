#ifndef SSHHOSTTREEENTRY_H
#define SSHHOSTTREEENTRY_H

#include <QTextOption>
#include "main/customtreeentry.h"


class SshHost;
class SshHostTreeEntry : public CustomTreeEntry
{
Q_OBJECT

public:
SshHostTreeEntry( SshHost* host );

// Custom drawing stuff
virtual bool isCustomDrawn() const {
	return true;
}

virtual void customDraw( QPainter* painter, const QStyleOptionViewItem& option );

private slots:
void handleGutterIconClick( int iconId );

private:
static void initializeCustomDrawKit();
static bool sCustomDrawKitInitialized;
static QTextOption sLabelTextOption;
static QIcon sDisconnectedIcon;
static QIcon sConnectingIcon[4];
static QIcon sConnectedIcon;
static QIcon sLogIcon;

SshHost* mHost;
int mAnimationFrame;
};

#endif	// SSHHOSTTREEENTRY_H
