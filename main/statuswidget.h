#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QMap>
#include <QWidget>
#include <QAbstractButton>
#include <QDialogButtonBox>

namespace Ui { class StatusWidget; }

class StatusWidget : public QWidget
{
    Q_OBJECT
public:
	enum Button	//	Higher number = comes first, first = default (unless Cancel)
	{
		None      = 0x0000,
		Cancel    = 0x0001,
		Done      = 0x0002,
		Connect   = 0x0004,
		SudoRetry = 0x0008,
		Retry     = 0x0010,
	};
	Q_DECLARE_FLAGS(Buttons, Button);
	enum Result { SuccessResult = 1, FailureResult = 0, SudoRequestedResult = -1 };

	explicit StatusWidget(bool dialogChild, QWidget *parent = 0);
	~StatusWidget();

	void setStatus(const QPixmap& pixmap, const QString& message);
	void close(Result result);
	void close(bool result) { close(result ? SuccessResult : FailureResult); }
	void setInputWidget(QWidget* widget);
	void clearInputWidget();

	void setButtons(Buttons buttons);
	inline void setCloseOnButton(bool value) { mCloseOnButton = value; }
	void setButtonsEnabled(bool enabled);

	inline bool isShowingInput() const { return mCurrentInputWidget != NULL; }
	inline Button getResult() const { return mResult; }

signals:
	void signalUpdateLayouts();
	void completed();
	void buttonClicked(StatusWidget::Button button);

private slots:
	void updateLayouts();
	void buttonClicked(QAbstractButton* button);

private:
	Ui::StatusWidget* ui;
	QWidget* mCurrentInputWidget;
	bool mDialogChild;
	bool mCloseOnButton;
	QMap<QAbstractButton*, Button> mButtons;
	Button mResult;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(StatusWidget::Buttons);

#endif // STATUSWIDGET_H
