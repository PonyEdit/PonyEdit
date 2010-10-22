#ifndef SERVERCONFIGDLG_H
#define SERVERCONFIGDLG_H

#include <QDialog>
#include "sshhost.h"

namespace Ui {
    class ServerConfigDlg;
}

class ServerConfigDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ServerConfigDlg(QWidget *parent = 0);
    ~ServerConfigDlg();

	void setEditHost(SshHost* host);

	QString getHostName();
	QString getUserName();
	QString getPassword();

private slots:
	void acceptedHandler();

private:
    Ui::ServerConfigDlg *ui;
	SshHost* mEditHost;
};

#endif // SERVERCONFIGDLG_H
