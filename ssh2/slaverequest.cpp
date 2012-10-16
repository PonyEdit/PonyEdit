#include "slaverequest.h"
#include "tools/json.h"
#include "main/tools.h"
#include <QDebug>

SlaveRequest::SlaveRequest(SlaveFile* file, const QByteArray& request, const QVariant& parameters, const Callback& callback)
	: mFile(file), mOpeningFile(NULL), mRequest(request), mParameters(parameters)
{
	if (callback.getFailureSlot()) connect(this, SIGNAL(requestFailure(QString, int)), callback.getTarget(), callback.getFailureSlot());
	if (callback.getSuccessSlot()) connect(this, SIGNAL(requestSuccess(QVariantMap)), callback.getTarget(), callback.getSuccessSlot());
}

const QByteArray& SlaveRequest::prepare(int bufferId)
{
	QVariantMap requestRoot;
	requestRoot.insert("i", mMessageId);
	requestRoot.insert("c", mRequest);
	requestRoot.insert("p", mParameters);

	if (bufferId > -1)
		requestRoot.insert("b", bufferId);

	mPackedRequest = Json::serialize(QVariant(requestRoot));
	mPackedRequest = Tools::bin(mPackedRequest);
	mPackedRequest += "\n";

	//	"bin" the request; clear out characters that are trouble for ssh comms


	return mPackedRequest;
}

void SlaveRequest::failRequest(const QString& error, int errorFlags)
{
	emit requestFailure(error, errorFlags);
}

void SlaveRequest::handleReply(const QVariantMap& reply)
{
	if (!reply.contains("error"))
		emit requestSuccess(reply);
	else
	{
		QByteArray code = reply.value("code").toByteArray();
		int errorFlags = 0;

		if (code == "perm")
			errorFlags |= PermissionError;

		failRequest(reply.value("error").toString(), errorFlags);
	}
}

