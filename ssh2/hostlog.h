#ifndef HOSTLOG_H
#define HOSTLOG_H

HIDE_COMPILE_WARNINGS

#include <QWidget>

UNHIDE_COMPILE_WARNINGS

namespace Ui {
class HostLog;
}

class SshHost;
class HostLog : public QWidget
{
	Q_OBJECT
	
public:
	explicit HostLog(SshHost* host);
	~HostLog();

	HostLog(HostLog const&) = delete;
	HostLog& operator=(HostLog const&) = delete;

	void closeEvent(QCloseEvent*);

public slots:
	void newLogLine(QString line);
	
private:
	Ui::HostLog *ui;
};

#endif // HOSTLOG_H
