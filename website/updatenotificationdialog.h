#ifndef UPDATENOTIFICATIONDIALOG_H
#define UPDATENOTIFICATIONDIALOG_H

#include <QDialog>
#include <QVariantMap>

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
	void setDownloadURL(const QString& href, const QString& display);

private:
    Ui::UpdateNotificationDialog *ui;
};

#endif // UPDATENOTIFICATIONDIALOG_H
