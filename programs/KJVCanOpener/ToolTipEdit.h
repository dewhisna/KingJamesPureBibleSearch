#ifndef TOOLTIPEDIT_H
#define TOOLTIPEDIT_H

#include <QPoint>
#include <QString>
#include <QFont>
#include <QPalette>
#include <QWidget>
#include <QTextEdit>
#include <QTimer>

// ============================================================================

class CTipEdit : public QTextEdit
{
	Q_OBJECT
public:
	CTipEdit(const QString &text, QWidget *parent);
	virtual ~CTipEdit();
	static CTipEdit *instance;

	bool eventFilter(QObject *o, QEvent *e);

	QBasicTimer hideTimer, expireTimer;

	bool fadingOut;

	void reuseTip(const QString &text);
	void hideTip();
	void hideTipImmediately();
	void setTipRect(QWidget *w, const QRect &r);
	void restartExpireTimer();
	bool tipChanged(const QPoint &pos, const QString &text, QObject *o);
	void placeTip(const QPoint &pos, QWidget *w);

	static int getTipScreen(const QPoint &pos, QWidget *w);
protected:
	void timerEvent(QTimerEvent *e);
	void paintEvent(QPaintEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void resizeEvent(QResizeEvent *e);

public slots:
	// Cleanup the _q_stylesheet_parent propery.
	void styleSheetParentDestroyed() {
		setProperty("_q_stylesheet_parent", QVariant());
		styleSheetParent = 0;
	}

private:
	QWidget *styleSheetParent;

private:
	QWidget *widget;
	QRect rect;
};

// ============================================================================

class CToolTipEdit
{
	CToolTipEdit() { }
public:
	static void showText(const QPoint &pos, const QString &text, QWidget *w = 0);
	static void showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect);
	static inline void hideText() { showText(QPoint(), QString()); }

	static bool isVisible();
	static QString text();

	static QPalette palette();
	static void setPalette(const QPalette &);
	static QFont font();
	static void setFont(const QFont &);
};


#endif // TOOLTIPEDIT_H
