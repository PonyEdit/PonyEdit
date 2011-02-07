#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QWidget>
#include <QAbstractButton>
#include <QDialogButtonBox>

namespace Ui { class StatusWidget; }

class StatusWidget : public QWidget
{
    Q_OBJECT
public:
	explicit StatusWidget(bool dialogChild, QWidget *parent = 0);
	~StatusWidget();

	void setStatus(const QPixmap& pixmap, const QString& message);
	void close(bool operationSuccessful);
	void setInputWidget(QWidget* widget);
	void clearInputWidget();

	inline bool isShowingInput() const { return mCurrentInputWidget != NULL; }

signals:
	void signalUpdateLayouts();
	void buttonClicked(QAbstractButton* button);
	void completed();

protected:
	QDialogButtonBox* getButtonBox();

private slots:
	void updateLayouts();

private:
	Ui::StatusWidget* ui;
	QWidget* mCurrentInputWidget;
	bool mDialogChild;
};

#endif // STATUSWIDGET_H
