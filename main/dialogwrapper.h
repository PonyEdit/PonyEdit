#ifndef DIALOGWRAPPER_H
#define DIALOGWRAPPER_H

#include <QDialog>
#include <QVBoxLayout>

template<class T> class DialogWrapper : public QDialog
{
public:
	explicit DialogWrapper(const QString& title, T* content, bool closeButton, QWidget* parent = 0)
		: QDialog(parent), mContent(content)
	{
		if (!closeButton)
			setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
		setWindowTitle(title);

		QVBoxLayout* layout = new QVBoxLayout(this);
		content->setParent(this);
		layout->addWidget(content);
		content->show();
	}

	DialogWrapper(DialogWrapper const&) = delete;
	DialogWrapper& operator=(DialogWrapper const&) = delete;
		
private:
	T* mContent;
};

#endif // DIALOGWRAPPER_H
