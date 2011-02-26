#include <QtGui/QApplication>
#include <QString>
#include <QDebug>
#include <QMetaType>
#include <QFont>

#include "website/updatemanager.h"
#include "website/sitemanager.h"
#include "ponyedit.h"

int main(int argc, char *argv[])
{
	int result = 1;

	UpdateManager* updateManager = NULL;

	try
	{
		QCoreApplication::setOrganizationName("Pentalon");
		QCoreApplication::setApplicationName("PonyEdit");
		QCoreApplication::setApplicationVersion("0.9-prealphaXI");

		PonyEdit a(argc, argv);

		updateManager = new UpdateManager();

		if(a.isRunning())
		{
			if(argc > 1)
			{
				for(int ii = 1; ii < argc; ii++)
				{
					QString name(argv[ii]);
					if(name.trimmed().isNull())
						continue;

					a.sendMessage(name);
				}
			}

			return 0;
		}

		if(argc > 1)
		{
			for(int ii = 1; ii < argc; ii++)
			{
				QString name(argv[ii]);
				if(name.trimmed().isNull())
					continue;

				Location *loc = new Location(name);
				gMainWindow->openSingleFile(loc);
				delete loc;
			}
		}

		gSiteManager->checkForUpdates();

		result = a.exec();
	}
	catch (QString err)
	{
		qDebug() << "FATAL ERROR: " << err;
	}

	delete updateManager;
	return result;
}
