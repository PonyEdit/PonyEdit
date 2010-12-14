#ifndef SSHCONNECTINGDIALOG_H
#define SSHCONNECTINGDIALOG_H

#include <QDialog>
#include <QTimer>

class SshHost;
class SshRemoteController;

namespace Ui { class SshConnectingDialog; }
class SshConnectingDialog : public QDialog
{
    Q_OBJECT

public:
	explicit SshConnectingDialog(SshHost* host, SshRemoteController* controller);
    ~SshConnectingDialog();

	void showEvent(QShowEvent *event);

private slots:
	void tick();
	void cancel();

private:
    Ui::SshConnectingDialog *ui;

	int mLastStatusChange;
	SshHost* mHost;
	SshRemoteController* mController;
	QTimer* mTimer;
};

#endif // SSHCONNECTINGDIALOG_H
