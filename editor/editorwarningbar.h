#ifndef EDITORWARNINGBAR_H
#define EDITORWARNINGBAR_H

HIDE_COMPILE_WARNINGS

#include <QWidget>
#include <QHBoxLayout>

UNHIDE_COMPILE_WARNINGS

class EditorWarningBar : public QWidget
{
	Q_OBJECT

public:
	EditorWarningBar(QWidget* parent, const QPixmap& icon, const QString& message);

	EditorWarningBar(EditorWarningBar const&) = delete;
	EditorWarningBar& operator=(EditorWarningBar const&) = delete;

	void addButton(const QString& label, QObject* callbackTarget, const char* callbackMethod);
	void addCloseButton();

public slots:
	void closeAndDestroy();

private:
	QHBoxLayout* mLayout;
};

#endif // EDITORWARNINGBAR_H
