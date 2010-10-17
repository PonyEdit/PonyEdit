#include "filedialog.h"
#include "ui_filedialog.h"
#include <QFileIconProvider>
#include <QDebug>
#include <QDir>

FileDialog::FileDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileDialog)
{
	ui->setupUi(this);

	QFileIconProvider p;

	QTreeWidgetItem* local = new QTreeWidgetItem(QStringList("Local Computer"), 0);
	ui->directoryTree->addTopLevelItem(local);

	QString homePath = QDir::homePath();
	QFileInfo fi(homePath);

	QTreeWidgetItem* child = new QTreeWidgetItem(QStringList(fi.baseName()), 0);
	child->setIcon(0, p.icon(fi));
	local->addChild(child);

	QFileInfoList list = QDir::drives();
	foreach (QFileInfo dfi, list)
	{
		QString driveName = dfi.absolutePath();
		driveName.replace('/', '\\');
		child = new QTreeWidgetItem(QStringList(driveName), 0);

		child->setIcon(0, p.icon(dfi));
		local->addChild(child);
	}

	QTreeWidgetItem* remote = new QTreeWidgetItem(QStringList("Remote Servers"), 0);
	ui->directoryTree->addTopLevelItem(remote);

	QTreeWidgetItem* favourites = new QTreeWidgetItem(QStringList("Favourite Locations"), 0);
	ui->directoryTree->addTopLevelItem(favourites);

	QList<int> sizes;
	sizes.append(30);
	sizes.append(70);
	ui->splitter->setSizes(sizes);
}

FileDialog::~FileDialog()
{
    delete ui;
}
