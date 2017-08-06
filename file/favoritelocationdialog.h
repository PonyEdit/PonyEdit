#ifndef FAVORITELOCATIONDIALOG_H
#define FAVORITELOCATIONDIALOG_H

#include "location.h"
#include <QDialog>

namespace Ui {
	class FavoriteLocationDialog;
}

class FavoriteLocationDialog : public QDialog {
	Q_OBJECT

public:
	explicit FavoriteLocationDialog( QWidget *parent, Location::Favorite *favorite );
	~FavoriteLocationDialog();

private slots:
	void acceptHandler();

private:
	Ui::FavoriteLocationDialog *ui;
	Location::Favorite *        mFavorite;
};

#endif // FAVORITELOCATIONDIALOG_H
