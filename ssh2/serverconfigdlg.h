#ifndef SERVERCONFIGDLG_H
#define SERVERCONFIGDLG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

#include "serverconfigwidget.h"

namespace Ui { class ServerConfigDlg; }

class SshHost;

class ServerConfigDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ServerConfigDlg(QWidget *parent = 0);
    ~ServerConfigDlg();

	ServerConfigDlg(ServerConfigDlg const&) = delete;
	ServerConfigDlg& operator=(ServerConfigDlg const&) = delete;
		
	void setEditHost(SshHost* host);

private:
    Ui::ServerConfigDlg *ui;

	ServerConfigWidget *mConfigWidget;
};

#endif // SERVERCONFIGDLG_H
