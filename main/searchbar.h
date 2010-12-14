#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>

namespace Ui { class SearchBar; }

class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = 0);
	~SearchBar();

	void takeFocus();

signals:
	void closeRequested();
	void find(const QString& text, bool backwards);

private slots:
	void findNext();
	void findPrev();

protected:
	void keyPressEvent(QKeyEvent *event);

private:
	Ui::SearchBar *ui;
};

#endif // SEARCHBAR_H
