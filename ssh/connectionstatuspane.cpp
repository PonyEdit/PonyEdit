#include "connectionstatuspane.h"
#include "main/globaldispatcher.h"
#include "connectionstatuswidget.h"

ConnectionStatusPane::ConnectionStatusPane(QWidget *parent) : QDockWidget(parent)
{
	hide();

	setFeatures(QDockWidget::NoDockWidgetFeatures);
	setWindowTitle("Dropped Connections");

	mInnerWidget = new QWidget(this);
	mLayout = new QVBoxLayout(mInnerWidget);
	setWidget(mInnerWidget);
	mInnerWidget->show();

	connect(gDispatcher, SIGNAL(connectionDropped(RemoteConnection*)), this, SLOT(connectionDropped(RemoteConnection*)), Qt::QueuedConnection);
}

void ConnectionStatusPane::connectionDropped(RemoteConnection *connection)
{
	ConnectionStatusWidget* child = new ConnectionStatusWidget(connection, false, this);
	connect(child, SIGNAL(completed()), this, SLOT(connectionCompleted()));
	mLayout->addWidget(child);
	show();
}

void ConnectionStatusPane::connectionCompleted()
{
	if (mLayout->children().length() <= 1)
		hide();
}
