#include "statuswidget.h"
#include "ui_statuswidget.h"
#include <QDialog>

StatusWidget::StatusWidget(bool dialogChild, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StatusWidget)
{
	mDialogChild = dialogChild;
	mCurrentInputWidget = NULL;

	ui->setupUi(this);

	connect(this, SIGNAL(signalUpdateLayouts()), this, SLOT(updateLayouts()), Qt::QueuedConnection);
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SIGNAL(buttonClicked(QAbstractButton*)));
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

QDialogButtonBox* StatusWidget::getButtonBox()
{
	return ui->buttonBox;
}

void StatusWidget::updateLayouts()
{
	setMinimumSize(layout()->minimumSize());
	parentWidget()->adjustSize();
}









