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

#include "PersistentSettings.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QHash>
#include <QPointer>
#include <QStyle>
#include <QStyleOption>
#include <QStylePainter>
#include <QTextDocument>
#include <QScrollBar>
#include <QDebug>
#include <QToolTip>
#include <QMenu>

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------
	const QString constrToolTipEditGroup("ToolTipEdit");
	const QString constrFontSizeKey("FontSize");
}

// ============================================================================

CTipEdit *CTipEdit::instance = 0;
QPalette g_tooltipedit_palette(QToolTip::palette());

// ============================================================================

CTipEdit::CTipEdit(QWidget *parent)
	:	QTextEdit(parent),
		styleSheetParent(0),
		widget(0),
		m_bDoingContextMenu(false)
{
	setWindowFlags(Qt::ToolTip |  /* Qt::SubWindow | */ /* Qt::WindowTitleHint | Qt::WindowSystemMenuHint | */ Qt::BypassGraphicsProxyWidget);
	setReadOnly(true);
	setLineWrapMode(QTextEdit::NoWrap);
	setAcceptRichText(true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setContextMenuPolicy(Qt::DefaultContextMenu /* Qt::NoContextMenu */);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard /* Qt::NoTextInteraction */);

	delete instance;
	instance = this;
	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	QPalette pal;
	pal.setBrush(QPalette::All, QPalette::Base, CToolTipEdit::palette().toolTipBase());
	pal.setBrush(QPalette::All, QPalette::Text, CToolTipEdit::palette().toolTipText());
	setPalette(pal);
	ensurePolished();
	setFrameStyle(QFrame::Box);
	setAlignment(Qt::AlignLeft);
	qApp->installEventFilter(this);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0, this) / qreal(255.0));
	setMouseTracking(true);
	fadingOut = false;
	restorePersistentSettings();
}

CTipEdit::~CTipEdit()
{
	savePersistentSettings();
	instance = 0;
}

void CTipEdit::savePersistentSettings()
{
	const int nFontSize = fontInfo().pointSize();

	QSettings &settings(CPersistentSettings::instance()->settings());
	settings.beginGroup(constrToolTipEditGroup);
	settings.setValue(constrFontSizeKey, nFontSize);
	settings.endGroup();
}

void CTipEdit::restorePersistentSettings()
{
	QSettings &settings(CPersistentSettings::instance()->settings());

	QFont fnt = font();

	settings.beginGroup(constrToolTipEditGroup);
	const int nFontSize = settings.value(constrFontSizeKey, fnt.pointSize()).toInt();
	settings.endGroup();

	if (nFontSize>0) {
		fnt.setPointSize(nFontSize);
		setFont(fnt);
	}
}


void CTipEdit::restartExpireTimer()
{
	int time = 10000 + 10 * qMax(0, toPlainText().length()-1000);
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

	adjustToolTipSize();

	QTimer::singleShot(50, this, SLOT(activate()));
//	activateWindow();
	raise();

	restartExpireTimer();
}

void CTipEdit::adjustToolTipSize()
{
	QFontMetrics fm(font());
	QSize extra(1 + verticalScrollBar()->sizeHint().width() + frameWidth()*2, frameWidth()*2);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.
	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	document()->setTextWidth(document()->idealWidth());
	QSize docSize = document()->size().toSize();
	extern QWidget *g_pMainWindow;
	if (widget) {
		resize(QSize(docSize.width(), qMin(widget->height(), docSize.height())) + extra);
	} else if (g_pMainWindow) {
		resize(QSize(docSize.width(), qMin(g_pMainWindow->height(), docSize.height())) + extra);
	} else {
		resize(docSize + extra);
	}
}

void CTipEdit::activate()
{
	activateWindow();
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

void CTipEdit::contextMenuEvent(QContextMenuEvent *e)
{
	hideTimer.stop();
	expireTimer.stop();
	m_bDoingContextMenu = true;
	QMenu *pMenu = createStandardContextMenu();
	pMenu->exec(e->globalPos());
	delete pMenu;
	m_bDoingContextMenu = false;

	// If user is running totally by keyboard to activate the menu,
	//		the mouse isn't over us and our parent window (or other
	//		window) may active, particularly if X-Mouse is in use.
	//		So, trigger our reactivation to make sure we stay active
	//		and not prematurely dismiss.  This must be larger than
	//		the X-Mouse activation time and less than the hideTip
	//		time:
	QTimer::singleShot(250, this, SLOT(activate()));
}

void CTipEdit::mouseMoveEvent(QMouseEvent *e)
{
//	if (rect.isNull())
//		return;
//	QPoint pos = e->globalPos();
//	if (widget)
//		pos = widget->mapFromGlobal(pos);
//	if ((!rect.contains(pos)) && (!geometry().contains(pos)))
//		hideTip();

//	if (geometry().contains(e->pos())) restartExpireTimer();

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
	if (m_bDoingContextMenu) {
		restartExpireTimer();
		return;
	}
	if (e->timerId() == hideTimer.timerId()
		|| e->timerId() == expireTimer.timerId()){
		hideTimer.stop();
		expireTimer.stop();
		hideTipImmediately();
	}
}

bool CTipEdit::eventFilter(QObject *o, QEvent *e)
{
	switch (e->type()) {
		case QEvent::KeyPress:
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
			int key = keyEvent->key();
			if ((!m_bDoingContextMenu) &&
				((key == Qt::Key_Escape) ||
				 (key == Qt::Key_Enter))) {
				hideTipImmediately();
				return true;
			}

			if (keyEvent->modifiers() & Qt::ControlModifier) {
				if ((key == Qt::Key_Plus) ||	// This one handles the on the keypad
					((keyEvent->modifiers() & Qt::ShiftModifier) &&
					 (key == Qt::Key_Equal))) {	// On the main keyboard, Ctrl-+ is on the Equal Key with a Shift (Ctrl-Shift-+)
					zoomIn();
					adjustToolTipSize();
					e->accept();
					return true;
				} else if (key == Qt::Key_Minus) {
					zoomOut();
					adjustToolTipSize();
					e->accept();
					return true;
				}
			}
			break;
		}

		case QEvent::MouseButtonDblClick:
		case QEvent::Leave:
		case QEvent::WindowDeactivate:
		case QEvent::FocusOut:
		case QEvent::Enter:
		case QEvent::WindowActivate:
		case QEvent::FocusIn:
		case QEvent::MouseMove:
			return false;

		default:
			break;
	}

	return QTextEdit::eventFilter(o, e);
}

bool CTipEdit::event(QEvent *e)
{
	switch (e->type()) {
		case QEvent::Leave:				// Leaving us, deactivating, or focusing us out hides us
		case QEvent::WindowDeactivate:
		case QEvent::FocusOut:
			if (!m_bDoingContextMenu)
				hideTip();
			break;

		case QEvent::Enter:				// Entering us, activating us, or focusing us halts hiding us
		case QEvent::WindowActivate:
		case QEvent::FocusIn:
			hideTimer.stop();
			expireTimer.stop();
			break;

		default:
			break;
	}

	return QTextEdit::event(e);
}

void CTipEdit::wheelEvent(QWheelEvent *e)
{
	QTextEdit::wheelEvent(e);
	adjustToolTipSize();
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
	if (testAttribute(Qt::WA_StyleSheet) || (w && w->style() && w->style()->inherits("QStyleSheetStyle"))) {
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
	extern QWidget *g_pMainWindow;
	QRect screen;
	if ((g_pMainWindow != NULL) && (g_pMainWindow->isFullScreen()))
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
				CTipEdit::instance->setTipRect(w, rect);
				CTipEdit::instance->reuseTip(text);
				CTipEdit::instance->placeTip(pos, w);
			}
			return;
		}
	}

	if (!text.isEmpty()){ // no tip can be reused, create new tip:
#ifndef Q_WS_WIN
		new CTipEdit(w); // sets CTipEdit::instance to itself
#else
//		// On windows, we can't use the widget as parent otherwise the window will be
//		// raised when the tooltip will be shown
//		new CTipEdit(QApplication::desktop()->screen(CTipEdit::getTipScreen(pos, w)));

		// For normal tooptips, the above applies, but since we are a special popup
		//	scroll widget, we need to or user can't activate us in a modal dialog!
		new CTipEdit(w);
#endif
		CTipEdit::instance->setTipRect(w, rect);
		CTipEdit::instance->reuseTip(text);
		CTipEdit::instance->placeTip(pos, w);
		CTipEdit::instance->setObjectName(QLatin1String("qtooltip_label"));		//"ctooltip_edit"));
		CTipEdit::instance->show();
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

