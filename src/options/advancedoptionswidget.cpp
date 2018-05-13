#include "advancedoptionswidget.h"
#include "options.h"
#include "QsLog.h"
#include "ui_advancedoptionswidget.h"

AdvancedOptionsWidget::AdvancedOptionsWidget( QWidget *parent ) :
	OptionsDialogPage( parent ),
	ui( new Ui::AdvancedOptionsWidget ) {
	ui->setupUi( this );

	connect( ui->labelTrace, SIGNAL( clicked() ), this, SLOT( setTrace() ) );
	connect( ui->labelDebug, SIGNAL( clicked() ), this, SLOT( setDebug() ) );
	connect( ui->labelInfo, SIGNAL( clicked() ), this, SLOT( setInfo() ) );
	connect( ui->labelWarn, SIGNAL( clicked() ), this, SLOT( setWarn() ) );
	connect( ui->labelError, SIGNAL( clicked() ), this, SLOT( setError() ) );
	connect( ui->labelFatal, SIGNAL( clicked() ), this, SLOT( setFatal() ) );

	ui->loggingLevel->setValue( QsLogging::Logger::instance().loggingLevel() );
}

AdvancedOptionsWidget::~AdvancedOptionsWidget() {
	delete ui;
}

void AdvancedOptionsWidget::apply() {
	int level = ui->loggingLevel->value();
	QsLogging::Logger::instance().setLoggingLevel( static_cast< QsLogging::Level >( level ) );
	Options::LoggingLevel = level;
}

void AdvancedOptionsWidget::setTrace() {
	ui->loggingLevel->setValue( 0 );
}

void AdvancedOptionsWidget::setDebug() {
	ui->loggingLevel->setValue( 1 );
}

void AdvancedOptionsWidget::setInfo() {
	ui->loggingLevel->setValue( 2 );
}

void AdvancedOptionsWidget::setWarn() {
	ui->loggingLevel->setValue( 3 );
}

void AdvancedOptionsWidget::setError() {
	ui->loggingLevel->setValue( 4 );
}

void AdvancedOptionsWidget::setFatal() {
	ui->loggingLevel->setValue( 5 );
}
