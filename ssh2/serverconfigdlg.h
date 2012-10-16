#ifndef SERVERCONFIGDLG_H
#define SERVERCONFIGDLG_H

#include <QDialog>
#include "serverconfigwidget.h"

namespace Ui { class ServerConfigDlg; }

class SshHost;

class ServerConfigDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ServerConfigDlg(QWidget *parent = 0);
    ~ServerConfigDlg();

	void setEditHost(SshHost* host);

private:
    Ui::ServerConfigDlg *ui;

	ServerConfigWidget *mConfigWidget;
};

#endif // SERVERCONFIGDLG_H
