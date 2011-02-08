#include "statuswidget.h"
#include "ui_statuswidget.h"
#include <QDialog>
#include <QPushButton>

StatusWidget::StatusWidget(bool dialogChild, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StatusWidget)
{
	mDialogChild = dialogChild;
	mCurrentInputWidget = NULL;
	mCloseOnButton = false;

	ui->setupUi(this);

	connect(this, SIGNAL(signalUpdateLayouts()), this, SLOT(updateLayouts()), Qt::QueuedConnection);
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(buttonClicked(QAbstractButton*)));
}

StatusWidget::~StatusWidget()
{
	clearInputWidget();
	delete ui;
}

void StatusWidget::close(bool operationSuccessful)
{
	if (mDialogChild)
	{
		QDialog* parentDialog = static_cast<QDialog*>(parentWidget());
		if (operationSuccessful)
			parentDialog->accept();
		else
			parentDialog->reject();
	}
	emit completed();
	delete this;
}

void StatusWidget::setStatus(const QPixmap &pixmap, const QString &message)
{
	ui->statusIcon->setPixmap(pixmap);
	ui->statusLabel->setText(message);
}

void StatusWidget::setInputWidget(QWidget *widget)
{
	clearInputWidget();
	mCurrentInputWidget = widget;
	ui->childArea->addWidget(mCurrentInputWidget);

	if (hasFocus())
		mCurrentInputWidget->setFocus();

	emit signalUpdateLayouts();
}

void StatusWidget::clearInputWidget()
{
	if (mCurrentInputWidget != NULL)
	{
		delete mCurrentInputWidget;
		mCurrentInputWidget = NULL;

		emit signalUpdateLayouts();
	}
}

void StatusWidget::updateLayouts()
{
	setMinimumSize(layout()->minimumSize());
	parentWidget()->adjustSize();
}

void StatusWidget::buttonClicked(QAbstractButton *button)
{
	Button buttonType = mButtons.value(button);
	if (mCloseOnButton)
		this->close(true);
	else
		emit buttonClicked(buttonType);
}

void StatusWidget::setButtons(Buttons buttons)
{
	ui->buttonBox->clear();
	mButtons.clear();

	if (buttons & Connect)   mButtons.insert(ui->buttonBox->addButton(tr("Connect"), QDialogButtonBox::ActionRole), Connect);
	if (buttons & Done)      mButtons.insert(ui->buttonBox->addButton(tr("Done"),    QDialogButtonBox::ActionRole), Done);
	if (buttons & Retry)     mButtons.insert(ui->buttonBox->addButton(tr("Retry"),   QDialogButtonBox::ActionRole), Retry);
	if (buttons & SudoRetry) mButtons.insert(ui->buttonBox->addButton(tr("Sudo"),    QDialogButtonBox::ActionRole), SudoRetry);
	if (buttons & Cancel)    mButtons.insert(ui->buttonBox->addButton(tr("Cancel"),  QDialogButtonBox::ActionRole), Cancel);

	//	Pick a default button
	QAbstractButton* firstButton = NULL;
	int highestValue = 1;
	QMapIterator<QAbstractButton*, Button> i(mButtons);
	while (i.hasNext())
	{
		i.next();
		if (i.value() > highestValue)
		{
			highestValue = i.value();
			firstButton = i.key();
		}
	}

	if (firstButton)
		static_cast<QPushButton*>(firstButton)->setDefault(true);
}

void StatusWidget::setButtonsEnabled(bool enabled)
{
	ui->buttonBox->setEnabled(enabled);
}




