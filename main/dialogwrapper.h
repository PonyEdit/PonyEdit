#ifndef DIALOGWRAPPER_H
#define DIALOGWRAPPER_H

#include <QDialog>
#include <QVBoxLayout>

template<class T> class DialogWrapper : public QDialog
{
public:
	explicit DialogWrapper(T* content, QWidget* parent = 0);

private:
	T* mContent;
};

template <class T> DialogWrapper<T>::DialogWrapper(T* content, QWidget* parent) :
	QDialog(parent)
{
	mContent = content;

	QVBoxLayout* layout = new QVBoxLayout(this);
	content->setParent(this);
	layout->addWidget(content);
	content->show();
}

#endif // DIALOGWRAPPER_H
