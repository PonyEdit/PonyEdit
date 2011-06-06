#ifndef LICENCE_H
#define LICENCE_H

#include <QByteArray>
#include <QDateTime>
#include <QDebug>

class Licence
{
public:
	Licence();
	Licence(const QByteArray& key);

	void setKey(const QByteArray& key);

	inline bool isValid() const { return mValid; }
	inline const QDate& getExpiry() const { return mExpiry; }
	inline bool hasExpired() const { return (mExpiry.isValid() && mExpiry < QDate::currentDate()); }
	inline const QString& getLogin() const { return mLogin; }
	inline const QByteArray& getKey() const { return mKey; }
	inline int getMaximumVersion() const { return mVersion; }
	inline int getIssueId() const { return mIssueId; }
	inline int getDaysLeft() const { return QDate::currentDate().daysTo(mExpiry); }
	inline bool isTrial() const { return mExpiry.isValid(); }

	void save();

private:
	bool verifySignature(const QByteArray& data, const QByteArray& signature);

	QByteArray mKey;
	QString mLogin;
	int mVersion;
	QDate mExpiry;
	int mIssueId;

	bool mValid;
};

#endif // LICENCE_H
