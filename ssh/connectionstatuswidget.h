#ifndef CONNECTIONSTATUSWIDGET_H
#define CONNECTIONSTATUSWIDGET_H

#include <QWidget>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include "main/statuswidget.h"
#include <QPlainTextEdit>

class RemoteConnection;
class ConnectionStatusWidget : public StatusWidget
{
    Q_OBJECT

public:
	explicit ConnectionStatusWidget(RemoteConnection* connection, bool modal, QWidget* parent = 0);
    ~ConnectionStatusWidget();

private slots:
	void connectionStatusChanged();
	void logUpdated(QString newLine);
	void onButtonClicked(StatusWidget::Button button);

protected:
	void showEvent(QShowEvent* e);

	RemoteConnection* mConnection;
	QPlainTextEdit* mLog;
};

#endif // CONNECTIONSTATUSWIDGET_H
