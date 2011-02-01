#ifndef UPDATENOTIFICATIONDIALOG_H
#define UPDATENOTIFICATIONDIALOG_H

#include <QDialog>
#include <QVariantMap>
#include <QProgressBar>
#include <QLabel>

namespace Ui {
    class UpdateNotificationDialog;
}

class UpdateNotificationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateNotificationDialog(QWidget *parent = 0);
    ~UpdateNotificationDialog();

	void setNewVersion(const QString& version);
	void setChanges(const QVariantMap& changes);
	void setDownloadURL(const QString &downloadURL, const QString& fileURL);

	QProgressBar* getProgressBar();
	QLabel* getProgressLabel();
	QWidget* getButtonWrapper();

signals:
	void downloadAndInstall(QString);

private slots:
	void emitDownloadAndInstall();
	void openDownloadURL();

private:
    Ui::UpdateNotificationDialog *ui;

	QString mDownloadURL;
	QString mFileURL;
};

#endif // UPDATENOTIFICATIONDIALOG_H
