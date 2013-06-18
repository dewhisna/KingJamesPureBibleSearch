#include <QPainter>

#include "ColorWidget.h"

ColorWidget::ColorWidget( QWidget *parent ) : QFrame( parent )
{
	setFrameStyle( QFrame::Panel | QFrame::Sunken );
	setLineWidth( 2 );
	
	m_color = Qt::black;
}

void ColorWidget::paintEvent( QPaintEvent *ev )
{
	QPainter p( this );
	QRect contents = contentsRect();
	contents.adjust( 0, 0 ,-lineWidth(), -lineWidth() );

	drawFrame( &p );
	p.setPen( m_color );
	p.setBrush( m_color );
	p.drawRect( contents );
}

void ColorWidget::setColor( const QColor &color )
{
	int r,g,b,h,s,v;
	int m_r, m_g, m_b, m_h, m_s, m_v;
	
	color.getRgb( &r, &g, &b );
	color.getHsv( &h, &s, &v );
	
	m_color.getRgb( &m_r, &m_g, &m_b );
	m_color.getHsv( &m_h, &m_s, &m_v );
	
	m_color = color;
	
	bool changed = false;
	
	if( r != m_r )
	{
		changed = true; 
		emit( rChanged( r ) );
	}
	
	if( g != m_g )
	{
		changed = true;
		emit( gChanged( g ) );
	}
	
	if( b != m_b )
	{
		changed = true;
		emit( bChanged( b ) );
	}

	if( changed )
	{	
		if( h != m_h )
			emit( hChanged( h ) );

		if( s != m_s )
			emit( sChanged( s ) );

		if( v != m_v )
			emit( vChanged( v ) );
	
		repaint();
	}
}

void ColorWidget::setR( int v )
{
	int r, g, b;
	
	m_color.getRgb( &r, &g, &b );
	r = v;
	
	setColor( QColor::fromRgb( r, g, b ) );
}

void ColorWidget::setG( int v )
{
	int r, g, b;
	
	m_color.getRgb( &r, &g, &b );
	g = v;
	
	setColor( QColor::fromRgb( r, g, b ) );
}

void ColorWidget::setB( int v )
{
	int r, g, b;
	
	m_color.getRgb( &r, &g, &b );
	b = v;
	
	setColor( QColor::fromRgb( r, g, b ) );
}

void ColorWidget::setS( int vv )
{
 	int h, s, v;
	
	m_color.getHsv( &h, &s, &v );
	s = vv;		
	
	setColor( QColor::fromHsv( h, s, v ) );
}

void ColorWidget::setH( int vv )
{
 	int h, s, v;
	
	m_color.getHsv( &h, &s, &v );
	h = vv;
	
	setColor( QColor::fromHsv( h, s, v ) );
}

void ColorWidget::setV( int vv )
{
 	int h, s, v;
	
	m_color.getHsv( &h, &s, &v );
	v = vv;
	
	setColor( QColor::fromHsv( h, s, v ) );
}
