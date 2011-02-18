#ifndef REQUESTSTATUSWIDGET_H
#define REQUESTSTATUSWIDGET_H

#include "main/statuswidget.h"
#include "ssh/remoterequest.h"

class RemoteChannel;
class RequestStatusWidget : public StatusWidget
{
	Q_OBJECT

public:
	enum Status { Working, Success, Error };

	RequestStatusWidget(RemoteChannel* channel, RemoteRequest* request, QString description, bool allowSudo);
	~RequestStatusWidget();

	void error(RemoteRequest::Error error);
	void success();
	void showEvent(QShowEvent*);
	void sendRequest();

private slots:
	void update();
	void onButtonClicked(StatusWidget::Button button);

signals:
	void updateRethreadSignal();

private:
	QString mDescription;
	RemoteChannel* mChannel;
	RemoteRequest* mRequest;
	Status mStatus;
	QString mErrorMessage;
	bool mPermissionError;
	bool mAllowSudo;
};

#endif // REQUESTSTATUSWIDGET_H

