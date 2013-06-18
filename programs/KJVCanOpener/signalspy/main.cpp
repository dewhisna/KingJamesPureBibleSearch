#include <QApplication>
#include <QDialog>
#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <Q3TextEdit>

#include "ColorWidget.h"

#include "Q4puGenericSignalSpy.h"

int main( int argc, char **argv )
{
	QApplication a( argc, argv );
	
	// Create dialog
	QDialog *dlg = new QDialog();
	dlg->setWindowTitle( "Color Conversion Demo" );
	
	// Create widgets
	QLabel *lbl_R = new QLabel( "R", dlg );
	QSlider *sld_R = new QSlider( Qt::Horizontal, dlg );
	sld_R->setRange( 0, 255 );
	QLabel *lbl_G = new QLabel( "G", dlg );
	QSlider *sld_G = new QSlider( Qt::Horizontal, dlg );
	sld_G->setRange( 0, 255 );
	QLabel *lbl_B = new QLabel( "B", dlg );
	QSlider *sld_B = new QSlider( Qt::Horizontal, dlg );
	sld_B->setRange( 0, 255 );

	ColorWidget *cw = new ColorWidget( dlg );
	cw->setMinimumSize( 32, 32 );

	QLabel *lbl_H = new QLabel( "H", dlg );
	QSlider *sld_H = new QSlider( Qt::Horizontal, dlg );
	sld_H->setRange( 0, 255 );
	QLabel *lbl_S = new QLabel( "S", dlg );
	QSlider *sld_S = new QSlider( Qt::Horizontal, dlg );
	sld_S->setRange( 0, 255 );
	QLabel *lbl_V = new QLabel( "V", dlg );
	QSlider *sld_V = new QSlider( Qt::Horizontal, dlg );
	sld_V->setRange( 0, 255 );
	
	Q3TextEdit *te = new Q3TextEdit( dlg );

	// Connect widgets
	QObject::connect( cw, SIGNAL( rChanged( int ) ), sld_R, SLOT( setValue( int ) ) );
	QObject::connect( cw, SIGNAL( gChanged( int ) ), sld_G, SLOT( setValue( int ) ) );
	QObject::connect( cw, SIGNAL( bChanged( int ) ), sld_B, SLOT( setValue( int ) ) );
	QObject::connect( cw, SIGNAL( hChanged( int ) ), sld_H, SLOT( setValue( int ) ) );
	QObject::connect( cw, SIGNAL( sChanged( int ) ), sld_S, SLOT( setValue( int ) ) );
	QObject::connect( cw, SIGNAL( vChanged( int ) ), sld_V, SLOT( setValue( int ) ) );

	QObject::connect( sld_R, SIGNAL( valueChanged( int ) ), cw, SLOT( setR( int ) ) );
	QObject::connect( sld_G, SIGNAL( valueChanged( int ) ), cw, SLOT( setG( int ) ) );
	QObject::connect( sld_B, SIGNAL( valueChanged( int ) ), cw, SLOT( setB( int ) ) );
	QObject::connect( sld_H, SIGNAL( valueChanged( int ) ), cw, SLOT( setH( int ) ) );
	QObject::connect( sld_S, SIGNAL( valueChanged( int ) ), cw, SLOT( setS( int ) ) );
	QObject::connect( sld_V, SIGNAL( valueChanged( int ) ), cw, SLOT( setV( int ) ) );
	
	// Set a color after the connections 
	// have been made to update the slider values
	cw->setColor( Qt::blue );

	// Create and populate layout
	QGridLayout *layout = new QGridLayout();
	
	layout->addWidget( lbl_R, 0, 0 );
	layout->addWidget( sld_R, 0, 1 );
	layout->addWidget( lbl_G, 1, 0 );
	layout->addWidget( sld_G, 1, 1 );
	layout->addWidget( lbl_B, 2, 0 );
	layout->addWidget( sld_B, 2, 1 );
	
	layout->addWidget( cw, 1, 2 );

	layout->addWidget( lbl_H, 0, 4 );
	layout->addWidget( sld_H, 0, 3 );
	layout->addWidget( lbl_S, 1, 4 );
	layout->addWidget( sld_S, 1, 3 );
	layout->addWidget( lbl_V, 2, 4 );
	layout->addWidget( sld_V, 2, 3 );
	
	layout->addWidget( te, 3, 0, 1, 5 );

	// Setup the spy
	Q4puGenericSignalSpy *spy = new Q4puGenericSignalSpy( dlg );
	spy->spyOn( cw );

	QObject::connect( spy, SIGNAL( caughtSignal( const QString& ) ), te, SLOT( append( const QString& ) ) );
	QObject::connect( spy, SIGNAL( caughtSlot( const QString& ) ), te, SLOT( append( const QString& ) ) );
	
	// Apply layout and show dialog
	dlg->setLayout( layout );
	dlg->exec();
	
	return 0;
}
