#ifndef CONNECTIONSTATUSPANE_H
#define CONNECTIONSTATUSPANE_H

#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>

class RemoteConnection;
class ConnectionStatusPane : public QDockWidget
{
    Q_OBJECT

public:
    explicit ConnectionStatusPane(QWidget *parent = 0);

private slots:
	void connectionDropped(RemoteConnection* connection);
	void connectionCompleted();

private:
	QWidget* mInnerWidget;
	QVBoxLayout* mLayout;
};

#endif // CONNECTIONSTATUSPANE_H
