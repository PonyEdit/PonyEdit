#ifndef TABBEDFILELIST_H
#define TABBEDFILELIST_H

#include "basefile.h"

#include <QDockWidget>
#include <QTabBar>

class TabbedFileList : public QDockWidget {
	Q_OBJECT

	public:
		explicit TabbedFileList( QWidget *parent = nullptr );

		int findTab( BaseFile *file );

	signals:
	public slots:
	private slots:
		void fileOpened( BaseFile *file );
		void fileClosed( BaseFile *file );
		void fileSelected( BaseFile *file );
		void fileChanged();

		void currentChanged( int index );
		void tabCloseRequested( int index );

	private:
		QTabBar *mTabs;
};

#endif  // TABBEDFILELIST_H
