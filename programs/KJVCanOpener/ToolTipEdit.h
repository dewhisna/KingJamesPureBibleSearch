/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#ifndef TOOLTIPEDIT_H
#define TOOLTIPEDIT_H

#include <QPoint>
#include <QString>
#include <QFont>
#include <QPalette>
#include <QWidget>
#include <QTextEdit>
#include <QTimer>
#include <QPushButton>

// ============================================================================

// Forward Declarations:
class CKJVCanOpener;

class CTipEdit : public QTextEdit
{
	Q_OBJECT
public:
	CTipEdit(CKJVCanOpener *pCanOpener, QWidget *parent);
	virtual ~CTipEdit();
	static CTipEdit *instance(const CKJVCanOpener *pCanOpener);
	static bool tipEditIsPinned(const CKJVCanOpener *pCanOpener);					// Push Pin to convert to modeless dialog

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
	void contextMenuEvent(QContextMenuEvent *e);
	bool event(QEvent *e);
	void wheelEvent(QWheelEvent *e);

	void adjustToolTipSize();
	void savePersistentSettings();
	void restorePersistentSettings();

public slots:
	// Cleanup the _q_stylesheet_parent propery.
	void styleSheetParentDestroyed() {
		setProperty("_q_stylesheet_parent", QVariant());
		styleSheetParent = 0;
	}

protected slots:
	void activate();
	void setPushPinPosition();
	void setPushPinIcon();
	void en_pushPinPressed();

private:
	void setInstance(CTipEdit *pTipEdit);
	void setTipEditIsPinned(bool bIsPinned);

private:
	QWidget *styleSheetParent;
	CKJVCanOpener *m_pParentCanOpener;		// Active CanOpener whent his tip was created (i.e. the CanOpener that it belongs to)

private:
	QWidget *widget;
	QRect rect;
	bool m_bDoingContextMenu;
	QPushButton *m_pPushButton;
};

// ============================================================================

class CToolTipEdit
{
	CToolTipEdit() { }
public:
	static void showText(CKJVCanOpener *pCanOpener, const QPoint &pos, const QString &text, QWidget *w = 0);
	static void showText(CKJVCanOpener *pCanOpener, const QPoint &pos, const QString &text, QWidget *w, const QRect &rect);
	static inline void hideText(CKJVCanOpener *pCanOpener) { showText(pCanOpener, QPoint(), QString()); }

	static bool isVisible(CKJVCanOpener *pCanOpener);
	static QString text(CKJVCanOpener *pCanOpener);

	static QPalette palette();
	static void setPalette(const QPalette &);
	static QFont font();
	static void setFont(const QFont &);
};


#endif // TOOLTIPEDIT_H
