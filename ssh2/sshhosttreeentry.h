#ifndef SSHHOSTTREEENTRY_H
#define SSHHOSTTREEENTRY_H

HIDE_COMPILE_WARNINGS

#include <QTextOption>

UNHIDE_COMPILE_WARNINGS

#include "main/customtreeentry.h"

class SshHost;
class SshHostTreeEntry : public CustomTreeEntry
{
	Q_OBJECT

public:
	SshHostTreeEntry(SshHost* host);

	SshHostTreeEntry(SshHostTreeEntry const&) = delete;
	SshHostTreeEntry& operator=(SshHostTreeEntry const&) = delete;

	//	Custom drawing stuff
	virtual bool isCustomDrawn() const { return true; }
	virtual void customDraw(QPainter* painter, const QStyleOptionViewItem& option);

private slots:
	void handleGutterIconClick(int iconId);

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

#endif // SSHHOSTTREEENTRY_H
