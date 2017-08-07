#ifndef UPDATENOTIFICATIONDIALOG_H
#define UPDATENOTIFICATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QProgressBar>
#include <QVariantMap>

namespace Ui {
class UpdateNotificationDialog;
}

class UpdateNotificationDialog : public QDialog
{
Q_OBJECT

public:
explicit UpdateNotificationDialog( QWidget *parent = 0 );
~UpdateNotificationDialog();

void setNewVersion( const QString& version );
void setChanges( const QStringList &alerts, const QStringList &changes );
void setDownloadURL( const QString& fileURL );

QProgressBar* getProgressBar();
QLabel* getProgressLabel();
QWidget* getButtonWrapper();

signals:
void downloadAndInstall( QString );

private slots:
void emitDownloadAndInstall();
void openDownloadURL();

private:
Ui::UpdateNotificationDialog *ui;

QString mFileURL;
};

#endif	// UPDATENOTIFICATIONDIALOG_H
