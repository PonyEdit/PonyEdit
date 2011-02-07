#ifndef CONNECTIONSTATUSWIDGET_H
#define CONNECTIONSTATUSWIDGET_H

#include <QWidget>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include "main/statuswidget.h"

class RemoteConnection;
class ConnectionStatusWidget : public StatusWidget
{
    Q_OBJECT

public:
	explicit ConnectionStatusWidget(RemoteConnection* connection, bool modal, QWidget* parent = 0);
    ~ConnectionStatusWidget();

	void addButton(QDialogButtonBox::ButtonRole role, const QString& label);

private slots:
	void connectionStatusChanged();
	void onButtonClicked(QAbstractButton* button);

protected:
	void showEvent(QShowEvent* e);

private:
	void clearExtraButtons();

	RemoteConnection* mConnection;
	QList<QPushButton*> mExtraButtons;
};

#endif // CONNECTIONSTATUSWIDGET_H
