#ifndef REGEXPTESTER_H
#define REGEXPTESTER_H

#include <QWidget>

namespace Ui {
    class RegExpTester;
}

class RegExpTester : public QWidget
{
    Q_OBJECT

public:
    explicit RegExpTester(QWidget *parent = 0);
    ~RegExpTester();

	RegExpTester(RegExpTester const&) = delete;
	RegExpTester& operator=(RegExpTester const&) = delete;

	void takeFocus(QString initialText);

private slots:
	void updateResult();
	void applySettings();

private:
    Ui::RegExpTester *ui;
	bool mUpdating;
};

#endif // REGEXPTESTER_H
