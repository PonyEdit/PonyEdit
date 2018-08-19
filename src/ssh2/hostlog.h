#ifndef HOSTLOG_H
#define HOSTLOG_H

#include <QWidget>

namespace Ui {
class HostLog;
}

class SshHost;
class HostLog : public QWidget {
	Q_OBJECT

	public:
		explicit HostLog( SshHost *host );
		~HostLog();
		void closeEvent( QCloseEvent * );

	public slots:
		void newLogLine( const QString &line );

	private:
		Ui::HostLog *ui;
};

#endif  // HOSTLOG_H
