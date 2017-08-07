#ifndef THREADCROSSINGDIALOG_H
#define THREADCROSSINGDIALOG_H

#include <QDialog>
#include <QVariantMap>

class ThreadCrossingDialog : public QDialog
{
Q_OBJECT
public:
explicit ThreadCrossingDialog( QWidget *parent = 0 );

virtual void setOptions( const QVariantMap& /*options*/ ) {}
virtual QVariantMap getResult();
};

#endif	// THREADCROSSINGDIALOG_H
