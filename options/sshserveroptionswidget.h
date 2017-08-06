#ifndef SSHSERVEROPTIONSWIDGET_H
#define SSHSERVEROPTIONSWIDGET_H

HIDE_COMPILE_WARNINGS

#include <QWidget>
#include <QListWidgetItem>

UNHIDE_COMPILE_WARNINGS

#include "ssh2/serverconfigwidget.h"
#include "options/optionsdialog.h"
#include "options/optionsdialogpage.h"

namespace Ui {
    class SshServerOptionsWidget;
}

class SshServerOptionsWidget : public OptionsDialogPage
{
    Q_OBJECT

public:
    explicit SshServerOptionsWidget(QWidget *parent = 0);
    ~SshServerOptionsWidget();

	SshServerOptionsWidget(SshServerOptionsWidget const&) = delete;
	SshServerOptionsWidget& operator=(SshServerOptionsWidget const&) = delete;
	
	QListWidgetItem* populateServers();

signals:
	void accepted();
	void rejected();

public slots:
	void accept();
	void reject();
	void serverClicked(QListWidgetItem *current, QListWidgetItem *previous = NULL);
	void serverNameUpdated(const QString& newName);
	void newServer();
	void deleteServer();

private:
    Ui::SshServerOptionsWidget *ui;

	OptionsDialog* mParent;
	QList<ServerConfigWidget*> mConfigWidgets;
};

#endif // SSHSERVEROPTIONSWIDGET_H
