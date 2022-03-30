/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef TOOL_TIP_EDIT_H
#define TOOL_TIP_EDIT_H

#include <QPoint>
#include <QString>
#include <QFont>
#include <QPalette>
#include <QWidget>
#include <QTextEdit>
#include <QTimer>
#include <QPushButton>
#include <QSizeGrip>

#include "PhraseNavigator.h"		// Needed for TIP_EDIT_TYPE_ENUM

// ============================================================================

// Forward Declarations:
class CKJVCanOpener;

class CTipEdit : public QTextEdit
{
	Q_OBJECT
public:
	CTipEdit(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener, QWidget *parent);
	virtual ~CTipEdit();
	static CTipEdit *instance(TIP_EDIT_TYPE_ENUM nTipType, const CKJVCanOpener *pCanOpener);
	static bool tipEditIsPinned(TIP_EDIT_TYPE_ENUM nTipType, const CKJVCanOpener *pCanOpener);					// Push Pin to convert to modeless dialog

	virtual bool eventFilter(QObject *o, QEvent *e) override;

	QBasicTimer hideTimer, expireTimer;

	bool fadingOut;

	QString text() const { return m_strText; }

	void reuseTip(const QString &strText);
	void hideTip();
	void hideTipImmediately();
	void setTipRect(QWidget *w, const QRect &r);
	void restartExpireTimer();
	bool tipChanged(const QPoint &pos, const QString &strText, QObject *o);
	void placeTip(const QPoint &pos, QWidget *w);

	static int getTipScreen(const QPoint &pos, QWidget *w);
protected:
	virtual void timerEvent(QTimerEvent *e) override;
	virtual void paintEvent(QPaintEvent *e) override;
	virtual void mouseMoveEvent(QMouseEvent *e) override;
	virtual void resizeEvent(QResizeEvent *e) override;
	virtual void contextMenuEvent(QContextMenuEvent *e) override;
	virtual bool event(QEvent *e) override;
	virtual void wheelEvent(QWheelEvent *e) override;

	void adjustToolTipSize();
	void savePersistentSettings();
	void restorePersistentSettings();

public slots:
	void setText(const QString &strText)
	{
		m_strText = strText;
		QTextEdit::setText(strText);
	}

	// Cleanup the _q_stylesheet_parent propery.
	void styleSheetParentDestroyed() {
		setProperty("_q_stylesheet_parent", QVariant());
		styleSheetParent = nullptr;
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
	TIP_EDIT_TYPE_ENUM m_nTipEditType;		// Type of this CTipEdit

private:
	QWidget *widget;
	QSizeGrip *m_pResizer;
	QRect rect;
	bool m_bDoingContextMenu;
	QPushButton *m_pPushButton;
	bool m_bFirstActivate;
	QString m_strText;
};

// ============================================================================

class CToolTipEdit
{
	CToolTipEdit() { }
public:
	static void showText(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener, const QPoint &pos, const QString &strText, QWidget *w = nullptr);
	static void showText(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener, const QPoint &pos, const QString &strText, QWidget *w, const QRect &rect);
	static inline void hideText(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener) { showText(nTipType, pCanOpener, QPoint(), QString()); }

	static bool isVisible(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener);
	static QString text(TIP_EDIT_TYPE_ENUM nTipType, CKJVCanOpener *pCanOpener);

	static QPalette palette();
	static void setPalette(const QPalette &);
	static QFont font();
	static void setFont(const QFont &);
};


#endif // TOOL_TIP_EDIT_H
