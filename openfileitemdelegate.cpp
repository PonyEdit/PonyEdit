#include "openfileitemdelegate.h"
#include "openfiletreemodel.h"
#include "basefile.h"
#include "tools.h"
#include <QApplication>
#include <QStylePainter>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QIcon>
#include <QStyle>

OpenFileItemDelegate::OpenFileItemDelegate(QTreeView *parent) : QStyledItemDelegate(parent)
{
	mParent = parent;
}

void OpenFileItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Location location = index.data(OpenFileTreeModel::LocationRole).value<Location>();
	BaseFile* file = (BaseFile*)index.data(OpenFileTreeModel::FileRole).value<void*>();

	//	Always paint the default background
	QStyledItemDelegate::paint(painter, option, index);

	if (index.column() == 1)
	{
		//	Column 1 contains close buttons, but they're only visible on mouse-over.
		QStyledItemDelegate::paint(painter, option, index);
		if (option.state & QStyle::State_MouseOver)
			painter->drawPixmap(option.rect.left(), option.rect.top(), 16, 16, QPixmap(":/icons/cross.png"));
	}
	else
	{
		//	Column 0 contains the label, and if it's a file also the status.
		QRect labelRect = option.rect;

		QPaintDevice* device = painter->device();
		painter->end();

		{
			QStylePainter sp(device, mParent);

			if (file)
			{
				BaseFile::OpenStatus fileStatus = file->getOpenStatus();
				if (fileStatus == BaseFile::Loading)
				{
					//	Show a progress bar while loading the file...
					QStyleOptionProgressBar so;
					so.rect = QRect(labelRect.right() - 32, labelRect.top() + 2, 32, labelRect.height() - 4);
					so.direction = Qt::LeftToRight;
					so.minimum = 0;
					so.maximum = 100;
					so.progress = file->getLoadingPercent();

					sp.drawControl(QStyle::CE_ProgressBar, so);

					labelRect.adjust(0, 0, -34, 0);
				}
				else
				{
					if (file->hasUnsavedChanges())
					{
						sp.drawPixmap(labelRect.right() - 16, labelRect.top(), 16, 16, QPixmap(":/icons/filechanged.png"));
						labelRect.adjust(0, 0, -18, 0);
					}

					if (fileStatus == BaseFile::Disconnected || fileStatus == BaseFile::Reconnecting)
					{
						sp.drawPixmap(labelRect.right() - 16, labelRect.top(), 16, 16, QPixmap(":/icons/disconnected.png"));
						labelRect.adjust(0, 0, -18, 0);
					}
					else if (fileStatus == BaseFile::Repairing)
					{
						sp.drawPixmap(labelRect.right() - 16, labelRect.top(), 16, 16, QPixmap(":/icons/resync.png"));
						labelRect.adjust(0, 0, -18, 0);
					}
				}

				//	Draw the icon
				location.getIcon().paint(&sp, labelRect.left(), labelRect.top(), 16, 16);
				labelRect.adjust(16, 0, 0, 0);
			}

			//	Add some padding around the text...
			labelRect.adjust(2, 0, -2, 0);

			//	Actually draw the text
			sp.setFont(option.font);
			sp.setBrush(option.palette.text());

			QString label;
			if (file)
				label = option.fontMetrics.elidedText(location.getLabel(), Qt::ElideMiddle, labelRect.width());
			else
			{
				label = location.getPath();
				label = Tools::squashLabel(label, option.fontMetrics, labelRect.width());
			}
			sp.drawText(labelRect, label);
		}

		painter->begin(device);
	}
}



















