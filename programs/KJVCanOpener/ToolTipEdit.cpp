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

#include "ToolTipEdit.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QHash>
#include <QPointer>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QTextDocument>
#include <QScrollBar>
#include <QDebug>
#include <QToolTip>

// ============================================================================

CTipEdit *CTipEdit::instance = 0;

// ============================================================================

CTipEdit::CTipEdit(const QString &text, QWidget *parent)
	:	QTextEdit(parent),
		styleSheetParent(0),
		widget(0)
{
	setWindowFlags(Qt::ToolTip |  /* Qt::SubWindow | */ /* Qt::WindowTitleHint | Qt::WindowSystemMenuHint | */ Qt::BypassGraphicsProxyWidget);
	setReadOnly(true);
	setLineWrapMode(QTextEdit::NoWrap);
	setAcceptRichText(true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setContextMenuPolicy(Qt::NoContextMenu);
	setTextInteractionFlags(Qt::NoTextInteraction);

	delete instance;
	instance = this;
	QPalette pal;
	pal.setColor(QPalette::Text, CToolTipEdit::palette().toolTipText().color());
	pal.setColor(QPalette::Base, CToolTipEdit::palette().toolTipBase().color());
	setPalette(pal);
	ensurePolished();
	setFrameStyle(QFrame::Box);
	setAlignment(Qt::AlignLeft);
	qApp->installEventFilter(this);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / qreal(255.0));
	setMouseTracking(true);
	fadingOut = false;
	reuseTip(text);
}

CTipEdit::~CTipEdit()
{
	instance = 0;
}

void CTipEdit::restartExpireTimer()
{
	int time = 10000 + 40 * qMax(0, toPlainText().length()-100);
	expireTimer.start(time, this);
	hideTimer.stop();
}

void CTipEdit::reuseTip(const QString &text)
{
	if (styleSheetParent){
		disconnect(styleSheetParent, SIGNAL(destroyed()),
					CTipEdit::instance, SLOT(styleSheetParentDestroyed()));
		styleSheetParent = 0;
	}

//	setWordWrap(Qt::mightBeRichText(text));
	setText(text);
	QFontMetrics fm(font());
	QSize extra(1 + verticalScrollBar()->sizeHint().width() + frameWidth()*2, frameWidth()*2);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.
	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	document()->setTextWidth(document()->idealWidth());
	QSize docSize = document()->size().toSize();
	extern QWidget *g_pMainWindow;
	if (g_pMainWindow) {
		resize(QSize(docSize.width(), qMin(g_pMainWindow->height(), docSize.height())) + extra);
	} else {
		resize(docSize + extra);
	}

//	activateWindow();
	raise();

	restartExpireTimer();
}

void CTipEdit::paintEvent(QPaintEvent *ev)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.init(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();

	QTextEdit::paintEvent(ev);
}

void CTipEdit::resizeEvent(QResizeEvent *e)
{
	QStyleHintReturnMask frameMask;
	QStyleOption option;
	option.init(this);
	if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
		setMask(frameMask.region);

	QTextEdit::resizeEvent(e);
}

void CTipEdit::mouseMoveEvent(QMouseEvent *e)
{
	if (rect.isNull())
		return;
	QPoint pos = e->globalPos();
	if (widget)
		pos = widget->mapFromGlobal(pos);
	if ((!rect.contains(pos)) && (!geometry().contains(pos)))
		hideTip();
	QTextEdit::mouseMoveEvent(e);
}

void CTipEdit::hideTip()
{
	if (!hideTimer.isActive())
		hideTimer.start(300, this);
}

void CTipEdit::hideTipImmediately()
{
	close(); // to trigger QEvent::Close which stops the animation
	deleteLater();
}

void CTipEdit::setTipRect(QWidget *w, const QRect &r)
{
	if (!rect.isNull() && !w)
		qWarning("CToolTipEdit::setTipRect: Cannot pass null widget if rect is set");
	else{
		widget = w;
		rect = r;
	}
}

void CTipEdit::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == hideTimer.timerId()
		|| e->timerId() == expireTimer.timerId()){
		hideTimer.stop();
		expireTimer.stop();
#if defined(Q_WS_MAC) && !defined(QT_NO_EFFECTS)
		if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip)){
			// Fade out tip on mac (makes it invisible).
			// The tip will not be deleted until a new tip is shown.

						// DRSWAT - Cocoa
						macWindowFade(qt_mac_window_for(this));
			CTipEdit::instance->fadingOut = true; // will never be false again.
		}
		else
			hideTipImmediately();
#else
		hideTipImmediately();
#endif
	}
}

bool CTipEdit::eventFilter(QObject *o, QEvent *e)
{
#ifdef Q_WS_MAC
	switch (e->type()) {
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		{
			int key = static_cast<QKeyEvent *>(e)->key();
			Qt::KeyboardModifiers mody = static_cast<QKeyEvent *>(e)->modifiers();
			if (!(mody & Qt::KeyboardModifierMask)
				&& key != Qt::Key_Shift && key != Qt::Key_Control
				&& key != Qt::Key_Alt && key != Qt::Key_Meta) {
				hideTip();
				return false;
			}
			break;
		}
	}
#endif

	switch (e->type()) {
		case QEvent::KeyPress:
		{
			int key = static_cast<QKeyEvent *>(e)->key();
			if ((key == Qt::Key_Escape) ||
				(key == Qt::Key_Enter)) {
				hideTipImmediately();
				return true;
			} else if ((key != Qt::Key_Up) &&
						(key != Qt::Key_Down) &&
						(key != Qt::Key_Left) &&
						(key != Qt::Key_Right) &&
						(key != Qt::Key_PageUp) &&
						(key != Qt::Key_PageDown) &&
						(key != Qt::Key_Home) &&
						(key != Qt::Key_End)) {
				hideTipImmediately();
			}
			break;
		}

		case QEvent::MouseButtonDblClick:
			hideTipImmediately();
			break;

		case QEvent::Leave:				// Leaving us or the parent view hides us
			hideTip();
			break;
		case QEvent::WindowDeactivate:	// Deactivating or focusing us out hides us (but not parent or we'll close prematurely)
		case QEvent::FocusOut:
			if (o == this)
				hideTip();
			break;

		case QEvent::Enter:				// Entering us, activating us, or focusing us halts hiding us
		case QEvent::WindowActivate:
		case QEvent::FocusIn:
			if (o == this)
				hideTimer.stop();
			break;

		case QEvent::MouseMove:
		{
			QMouseEvent *mev = static_cast<QMouseEvent*>(e);

			if (o == widget && !rect.isNull() && !rect.contains(mev->pos()) && (!geometry().contains(mev->pos())))
				hideTip();
			break;
		}
		default:
			break;
	}
	return false;
}

int CTipEdit::getTipScreen(const QPoint &pos, QWidget *w)
{
	if (QApplication::desktop()->isVirtualDesktop())
		return QApplication::desktop()->screenNumber(pos);
	else
		return QApplication::desktop()->screenNumber(w);
}

void CTipEdit::placeTip(const QPoint &pos, QWidget *w)
{
//	if (testAttribute(Qt::WA_StyleSheet) || (w && qobject_cast<QStyleSheetStyle *>(w->style()))) {
	if (testAttribute(Qt::WA_StyleSheet) || (w && w->style())) {
		//the stylesheet need to know the real parent
		CTipEdit::instance->setProperty("_q_stylesheet_parent", QVariant::fromValue(w));
		//we force the style to be the QStyleSheetStyle, and force to clear the cache as well.
		CTipEdit::instance->setStyleSheet(QLatin1String("/* */"));

		// Set up for cleaning up this later...
		CTipEdit::instance->styleSheetParent = w;
		if (w) {
			connect(w, SIGNAL(destroyed()),
				CTipEdit::instance, SLOT(styleSheetParentDestroyed()));
		}
	}

#ifdef Q_WS_MAC
	// When in full screen mode, there is no Dock nor Menu so we can use
	// the whole screen for displaying the tooltip. However when not in
	// full screen mode we need to save space for the dock, so we use
	// availableGeometry instead.
	extern bool qt_mac_app_fullscreen; //qapplication_mac.mm
	QRect screen;
	if(qt_mac_app_fullscreen)
		screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
	else
		screen = QApplication::desktop()->availableGeometry(getTipScreen(pos, w));
#else
	QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
#endif

	QPoint p = pos;
	p += QPoint(2,
#ifdef Q_WS_WIN
				21
#else
				16
#endif
		);

	p.ry() -= this->height()/2;

	if (p.x() + this->width() > screen.x() + screen.width())
		p.rx() -= 4 + this->width();
	if (p.y() + this->height() > screen.y() + screen.height())
		p.ry() -= 24 + this->height()/2;
	if (p.y() < screen.y())
		p.setY(screen.y());
	if (p.x() + this->width() > screen.x() + screen.width())
		p.setX(screen.x() + screen.width() - this->width());
	if (p.x() < screen.x())
		p.setX(screen.x());
	if (p.y() + this->height() > screen.y() + screen.height())
		p.setY(screen.y() + screen.height() - this->height()/2);

	this->move(p);
}

bool CTipEdit::tipChanged(const QPoint &pos, const QString &text, QObject *o)
{
	if (CTipEdit::instance->toHtml() != text)
		return true;

	if (o != widget)
		return true;

	if (!rect.isNull())
		return !rect.contains(pos);
	else
	   return false;
}

void CToolTipEdit::showText(const QPoint &pos, const QString &text, QWidget *w, const QRect &rect)
{
	if (CTipEdit::instance && CTipEdit::instance->isVisible()){ // a tip does already exist
		if (text.isEmpty()){ // empty text means hide current tip
			CTipEdit::instance->hideTip();
			return;
		}
		else if (!CTipEdit::instance->fadingOut){
			// If the tip has changed, reuse the one
			// that is showing (removes flickering)
			QPoint localPos = pos;
			if (w)
				localPos = w->mapFromGlobal(pos);
			if (CTipEdit::instance->tipChanged(localPos, text, w)){
				CTipEdit::instance->reuseTip(text);
				CTipEdit::instance->setTipRect(w, rect);
				CTipEdit::instance->placeTip(pos, w);
			}
			return;
		}
	}

	if (!text.isEmpty()){ // no tip can be reused, create new tip:
#ifndef Q_WS_WIN
		new CTipEdit(text, w); // sets CTipEdit::instance to itself
#else
		// On windows, we can't use the widget as parent otherwise the window will be
		// raised when the tooltip will be shown
		new CTipEdit(text, QApplication::desktop()->screen(CTipEdit::getTipScreen(pos, w)));
#endif
		CTipEdit::instance->setTipRect(w, rect);
		CTipEdit::instance->placeTip(pos, w);
		CTipEdit::instance->setObjectName(QLatin1String("qtooltip_label"));


//#if !defined(QT_NO_EFFECTS) && !defined(Q_WS_MAC)
//		if (QApplication::isEffectEnabled(Qt::UI_FadeTooltip))
//			qFadeEffect(CTipEdit::instance);
//		else if (QApplication::isEffectEnabled(Qt::UI_AnimateTooltip))
//			qScrollEffect(CTipEdit::instance);
//		else
//			CTipEdit::instance->show();
//#else
		CTipEdit::instance->show();
//#endif
	}
}

void CToolTipEdit::showText(const QPoint &pos, const QString &text, QWidget *w)
{
	CToolTipEdit::showText(pos, text, w, QRect());
}

bool CToolTipEdit::isVisible()
{
	return (CTipEdit::instance != 0 && CTipEdit::instance->isVisible());
}

QString CToolTipEdit::text()
{
	if (CTipEdit::instance)
		return CTipEdit::instance->toHtml();
	return QString();
}

QPalette g_tooltipedit_palette(QToolTip::palette());

QPalette CToolTipEdit::palette()
{
	return g_tooltipedit_palette;
}

QFont CToolTipEdit::font()
{
	return QApplication::font("CTipEdit");
}

void CToolTipEdit::setPalette(const QPalette &palette)
{
	g_tooltipedit_palette = palette;
	if (CTipEdit::instance)
		CTipEdit::instance->setPalette(palette);
}

void CToolTipEdit::setFont(const QFont &font)
{
	QApplication::setFont(font, "CTipEdit");
}

