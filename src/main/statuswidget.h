#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QMap>
#include <QWidget>

namespace Ui { class StatusWidget; }

class StatusWidget : public QWidget {
	Q_OBJECT

	public:
		enum Button     // Higher number = comes first, first = default (unless Cancel)
		{
			None      = 0x0000,
			Cancel    = 0x0001,
			ShowLog   = 0x0002,
			Done      = 0x0004,
			Connect   = 0x0008,
			SudoRetry = 0x0010,
			Retry     = 0x0011,
		};
		Q_DECLARE_FLAGS( Buttons, Button )
		enum Result { SuccessResult = 1, FailureResult = 0, SudoRequestedResult = -1 };

		explicit StatusWidget( bool dialogChild, QWidget *parent = nullptr );
		~StatusWidget();

		void setStatus( const QPixmap &pixmap, const QString &message );
		void close( Result result );
		void close( bool result ) {
			close( result ? SuccessResult : FailureResult );
		}

		void setInputWidget( QWidget *widget );
		void clearInputWidget();

		void setButtons( Buttons buttons );
		inline void setCloseOnButton( bool value ) {
			mCloseOnButton = value;
		}

		void setButtonsEnabled( bool enabled );

		inline bool isShowingInput() const {
			return mCurrentInputWidget != nullptr;
		}

		inline Button getResult() const {
			return mResult;
		}

		QLayout *getLogArea();

	signals:
		void signalUpdateLayouts();
		void completed();
		void buttonClicked( StatusWidget::Button button );

	private slots:
		void updateLayouts();
		void buttonClicked( QAbstractButton *button );

	private:
		Ui::StatusWidget *ui;
		QWidget *mCurrentInputWidget;
		bool mDialogChild;
		bool mCloseOnButton;
		QMap< QAbstractButton *, Button > mButtons;
		Button mResult;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( StatusWidget::Buttons );

#endif  // STATUSWIDGET_H
