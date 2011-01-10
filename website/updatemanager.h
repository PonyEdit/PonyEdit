#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QVariantMap>

class UpdateManager : public QObject
{
    Q_OBJECT
public:
    explicit UpdateManager(QObject *parent = 0);

signals:

public slots:
	void updateFound(const QString& version, const QVariantMap& changes);

};

#endif // UPDATEMANAGER_H
