#include "filestatuswidget.h"
#include "ui_statuswidget.h"
#include "main/statuswidget.h"
#include <QDebug>
#include <QDialog>
#include <QPushButton>
#include <QTimer>

FileStatusWidget::FileStatusWidget(BaseFile* file, QWidget* parent) :
	StatusWidget(true, parent)
{
	mFile = file;
	setButtons(Cancel);
	connect(file, SIGNAL(openStatusChanged(int)), this, SLOT(openStatusChanged()));
}

FileStatusWidget::~FileStatusWidget()
{
}

void FileStatusWidget::showEvent(QShowEvent*)
{
	openStatusChanged();
}

void FileStatusWidget::openStatusChanged()
{
	BaseFile::OpenStatus status = mFile->getOpenStatus();

	this->setStatus(QPixmap(":/icons/loading.png"), BaseFile::sStatusLabels[status]);

	if (status == BaseFile::Ready)
		close(true);
	else if (status == BaseFile::SyncError || status == BaseFile::Closed)
		close(false);
}

