#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QFileIconProvider>
#include <QMap>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QTableView>
#include <QTreeWidgetItem>

#include "location.h"
#include "main/statuswidget.h"

#ifdef Q_OS_WIN
	#include <windows.h>
	#include <winnetwk.h>
#endif

namespace Ui { class FileDialog; }

// Specialty tableview; used in the file list to force selection cursor to leftmost column
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
class SelectionlessTable : public QTableView {
	public:
		SelectionlessTable( QWidget *p ) :
			QTableView( p ) {}
		virtual void currentChanged( const QModelIndex &current, const QModelIndex & /*previous*/ ) {
			if ( current.column() > 0 ) {
				this->setCurrentIndex( current.sibling( current.row(), 0 ) );
			}
		}
};
#pragma clang diagnostic pop


class CustomTreeEntry;
class FileDialog : public QDialog {
	Q_OBJECT

	public:
		explicit FileDialog( QWidget *parent = nullptr, bool saveAs = false );
		~FileDialog();

		void showLocation( const Location &location );
		QList< Location > getSelectedLocations() const;
		Location getNewLocation() const;

	public slots:
		void accept();

	protected:
		void dragEnterEvent( QDragEnterEvent * );
		void dropEvent( QDropEvent * );
		bool eventFilter( QObject *target, QEvent *event );

	private slots:
		void folderChildrenLoaded( const QList< Location > &children, const QString &locationPath );
		void folderChildrenFailed( const QString &error, const QString &locationPath, bool permissionError );
		void upLevel();
		void fileDoubleClicked( QModelIndex index );
		void populateRemoteServers();
		void fileListSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
		void closing();
		void addToFavorites();
		void createNewFolder();
		void statusButtonClicked( StatusWidget::Button button );// Called when "try again" or "sudo" is clicked
		                                                        // on an
		                                                        // error
		void refresh();
		void fileNameIndexChanged();
		void columnHeaderClicked( int column );
		void addNewServer();
		void serverClicked( CustomTreeEntry *entry );
		void locationClicked( CustomTreeEntry *entry );
		void locationExpanded( CustomTreeEntry *entry );
		void favoriteClicked( CustomTreeEntry *entry );
		void favoriteMenu( CustomTreeEntry *entry, QPoint pos );
#ifdef Q_OS_WIN
		void populateWindowsShares( CustomTreeEntry *entry );
#endif

	private:
		void keyPressEvent( QKeyEvent * );
		void restoreState();
		void showStatus( const QPixmap &icon, const QString &text );
		void applySort();

		void populateFolderTree();
		CustomTreeEntry *addLocationToTree( CustomTreeEntry *parent, const Location &location );
		void updateFavorites();
		void populateFilterList();

		Ui::FileDialog *ui;
		QFileIconProvider mIconProvider;
		Location mCurrentLocation;
		QStandardItemModel *mFileListModel;
		CustomTreeEntry *mRemoteServersBranch{};
		CustomTreeEntry *mFavoriteLocationsBranch{};
#ifdef Q_OS_WIN
		CustomTreeEntry *mLocalNetworkBranch;
#endif

		QMap< QString, CustomTreeEntry * > mLoadingLocations;

		static Location mLastLocation;

		bool mSaveAs;
		bool mInEditHandler;
		int mSortingColumn{};
		bool mReverseSorting{};

		QString mSelectFile;
};

#endif  // FILEDIALOG_H
