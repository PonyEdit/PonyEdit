HIDE_COMPILE_WARNINGS

#include <QDebug>
#include <QDialog>
#include <QPushButton>
#include <QTimer>

#include "ui_statuswidget.h"

UNHIDE_COMPILE_WARNINGS

#include "filestatuswidget.h"
#include "main/statuswidget.h"

FileStatusWidget::FileStatusWidget(BaseFile* file, QWidget* parent) : StatusWidget(true, parent),
    mFile(file)
{
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

