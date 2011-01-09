#ifndef CONNECTIONSTATUSWIDGET_H
#define CONNECTIONSTATUSWIDGET_H

#include <QWidget>
#include <QDialogButtonBox>
#include <QAbstractButton>

namespace Ui { class ConnectionStatusWidget; }
class RemoteConnection;

class ConnectionStatusWidget : public QWidget
{
    Q_OBJECT

public:
	explicit ConnectionStatusWidget(RemoteConnection* connection, bool modal, QWidget* parent = 0);
    ~ConnectionStatusWidget();

	void setManualStatus(QString text, QPixmap icon);
	void addButton(QDialogButtonBox::ButtonRole role, const QString& label);

private slots:
	void connectionStatusChanged();
	void updateLayouts();
	void buttonClicked(QAbstractButton* button);

signals:
	void signalUpdateLayouts();
	void completed();

private:
	void showInput();
	void hideInput();
	void clearExtraButtons();

	Ui::ConnectionStatusWidget* ui;
	RemoteConnection* mConnection;
	QWidget* mCurrentInputWidget;
	bool mModal;
	QList<QPushButton*> mExtraButtons;
};

#endif // CONNECTIONSTATUSWIDGET_H
