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
explicit ServerConfigWidget( QWidget *parent = 0 );
~ServerConfigWidget();

void setEditHost( SshHost* host );
SshHost* getEditHost();

void setFocus();

QString getHostName();
QString getUserName();
QString getPassword();

signals:
void rejected();
void accepted();
void nameUpdated( const QString& oldName );

private slots:
void keyPressEvent( QKeyEvent *event );
void acceptedHandler();
void updateName();
void browseForKeyFile();

private:
QString getAutoName();

Ui::ServerConfigWidget *ui;
SshHost *mEditHost;

QString mLastAutoName;
};

#endif	// SERVERCONFIGWIDGET_H
