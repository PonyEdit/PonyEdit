#ifndef CALLBACK_H
#define CALLBACK_H

HIDE_COMPILE_WARNINGS

#include <QVariantMap>
#include <QObject>
#include <QPointer>

UNHIDE_COMPILE_WARNINGS

class QObject;
class Callback
{
public:
	Callback(QObject* target = 0, const char* successSlot = 0, const char* failureSlot = 0, const char* progressSlot = 0);

	Callback(Callback const& obj);
	Callback& operator=(Callback const& obj);

	inline QObject* getTarget() const { return mTarget.data(); }
	inline const char* getSuccessSlot() const { return mSuccessSlot; }
	inline const char* getFailureSlot() const { return mFailureSlot; }
	inline const char* getProgressSlot() const { return mProgressSlot; }

	void triggerSuccess(QVariantMap result = QVariantMap()) const;
	void triggerFailure(QString error, int flags = 0) const;
	void triggerProgress(int progress) const;

private:
    QPointer<QObject> mTarget;
	const char* mSuccessSlot;
	const char* mFailureSlot;
	const char* mProgressSlot;
};

class CallbackDummy : QObject
{
	Q_OBJECT
	friend class Callback;

protected:
	CallbackDummy();
	void emitTriggerSuccess(QVariantMap result);
	void emitTriggerFailure(QString error, int flags);
	void emitTriggerProgress(int progress);

signals:
	void triggerSuccess(QVariantMap result);
	void triggerFailure(QString error, int flags);
	void triggerProgress(int progress);
};

#endif // CALLBACK_H
