#ifndef SERVERCONFIGDLG_H
#define SERVERCONFIGDLG_H

#include <QDialog>

namespace Ui { class ServerConfigDlg; }

class SshHost;

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
	void updateName();
	void browseForKeyFile();

private:
	QString getAutoName();

    Ui::ServerConfigDlg *ui;
	SshHost* mEditHost;

	QString mLastAutoName;
};

#endif // SERVERCONFIGDLG_H
