#ifndef EDITORWARNINGBAR_H
#define EDITORWARNINGBAR_H

#include <QHBoxLayout>
#include <QWidget>

class EditorWarningBar : public QWidget {
	Q_OBJECT

	public:
		EditorWarningBar( QWidget* parent, const QPixmap& icon, const QString& message );
		void addButton( const QString& label, QObject* callbackTarget, const char* callbackMethod );
		void addCloseButton();

	public slots:
		void closeAndDestroy();

	private:
		QHBoxLayout* mLayout;
};

#endif  // EDITORWARNINGBAR_H
