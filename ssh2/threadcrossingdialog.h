#ifndef THREADCROSSINGDIALOG_H
#define THREADCROSSINGDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>
#include <QVariantMap>

UNHIDE_COMPILE_WARNINGS

class ThreadCrossingDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ThreadCrossingDialog(QWidget *parent = 0);

	virtual void setOptions(const QVariantMap& /*options*/) {}
	virtual QVariantMap getResult();
};

#endif // THREADCROSSINGDIALOG_H
