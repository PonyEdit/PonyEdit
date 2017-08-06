#ifndef FAVORITELOCATIONDIALOG_H
#define FAVORITELOCATIONDIALOG_H

HIDE_COMPILE_WARNINGS

#include <QDialog>

UNHIDE_COMPILE_WARNINGS

#include "location.h"

namespace Ui { class FavoriteLocationDialog; }

class FavoriteLocationDialog : public QDialog
{
    Q_OBJECT

public:
	explicit FavoriteLocationDialog(QWidget* parent, Location::Favorite* favorite);
    ~FavoriteLocationDialog();

	FavoriteLocationDialog(FavoriteLocationDialog const&) = delete;
	FavoriteLocationDialog& operator=(FavoriteLocationDialog const&) = delete;
		
private slots:
	void acceptHandler();

private:
	Ui::FavoriteLocationDialog* ui;
	Location::Favorite* mFavorite;
};

#endif // FAVORITELOCATIONDIALOG_H
