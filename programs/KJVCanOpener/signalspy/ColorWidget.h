#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QFrame>
#include <QColor>

class ColorWidget : public QFrame
{
	Q_OBJECT
	
public:
	ColorWidget( QWidget *parent=0 );
	
	const QColor &getColor( void );
	
public slots:
	void setColor( const QColor& );
	void setR( int );
	void setG( int );
	void setB( int );
	void setH( int );
	void setS( int );
	void setV( int );
	
signals:
	void rChanged( int );
	void gChanged( int );
	void bChanged( int );
	void hChanged( int );
	void sChanged( int );
	void vChanged( int );
	
protected:
	void paintEvent( QPaintEvent *ev );

private:
	QColor m_color;
};

#endif // COLORWIDGET_H
