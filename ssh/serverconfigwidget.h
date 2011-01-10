#ifndef SERVERCONFIGWIDGET_H
#define SERVERCONFIGWIDGET_H

#include <QWidget>

namespace Ui {
    class ServerConfigWidget;
}

class SshHost;

class ServerConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ServerConfigWidget(QWidget *parent = 0);
    ~ServerConfigWidget();

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

	Ui::ServerConfigWidget *ui;
	SshHost* mEditHost;

	QString mLastAutoName;
};

#endif // SERVERCONFIGWIDGET_H
