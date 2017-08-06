#ifndef HTMLPREVIEW_H
#define HTMLPREVIEW_H

#include <QWidget>

#include "main/mainwindow.h"

namespace Ui {
	class HTMLPreview;
}

class HTMLPreview : public QWidget {
	Q_OBJECT

public:
	explicit HTMLPreview( MainWindow *parent = 0 );
	~HTMLPreview();

	void displayHTML( QString html );
	void displayURL();

public slots:
	void fileSaved();
	void fileSelected( BaseFile *file );
	void fileChanged();
	void manualRefresh();

private:
	Ui::HTMLPreview *ui;
	MainWindow *     mParent;
};

#endif // HTMLPREVIEW_H
