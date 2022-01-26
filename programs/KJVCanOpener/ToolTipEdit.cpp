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

#include "ToolTipEdit.h"
#include "myApplication.h"
#include "KJVCanOpener.h"

#ifndef NO_PERSISTENT_SETTINGS
#include "PersistentSettings.h"
#endif

#include <QMainWindow>
#include <QApplication>
#if QT_VERSION >= 0x050B00
#include <QGuiApplication>
#include <QScreen>
#endif
#if QT_VERSION < 0x050E00
#include <QDesktopWidget>
#endif
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
#include <QTimer>

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

QPalette g_tooltipedit_palette(QToolTip::palette());

// ============================================================================

CTipEdit *CTipEdit::instance(const CKJVCanOpener *pCanOpener)
{
	if (pCanOpener == nullptr) return nullptr;
	return pCanOpener->tipEdit();
}

void CTipEdit::setInstance(CTipEdit *pTipEdit)
{
	Q_ASSERT(m_pParentCanOpener != nullptr);
	return m_pParentCanOpener->setTipEdit(pTipEdit);
}

bool CTipEdit::tipEditIsPinned(const CKJVCanOpener *pCanOpener)
{
	// Note: pCanOpener can be NULL during construction of CKJVCanOpener, for example,
	//		and this function may get called by children being constructed
	if (pCanOpener == nullptr) return false;
	return pCanOpener->tipEditIsPinned();
}

void CTipEdit::setTipEditIsPinned(bool bIsPinned)
{
	Q_ASSERT(m_pParentCanOpener != nullptr);
	return m_pParentCanOpener->setTipEditIsPinned(bIsPinned);
}

// ============================================================================

CTipEdit::CTipEdit(CKJVCanOpener *pCanOpener, QWidget *parent)
	:	QTextEdit(parent),
		styleSheetParent(nullptr),
		m_pParentCanOpener(pCanOpener),
		widget(nullptr),
		m_bDoingContextMenu(false),
		m_pPushButton(nullptr),
		m_bFirstActivate(true)
{
//	setWindowFlags(Qt::ToolTip |  /* Qt::SubWindow | */ /* Qt::WindowTitleHint | Qt::WindowSystemMenuHint | */ Qt::BypassGraphicsProxyWidget);
#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::BypassGraphicsProxyWidget | (tipEditIsPinned(m_pParentCanOpener) ? Qt::WindowTitleHint : Qt::WindowType()));
#elif defined(Q_OS_WASM)
	// For some reason, WebAssembly doesn't properly allow the user to move the window
	//	when Qt::CustomizeWindowHint is set even when setting Qt::WindowTitleHint,
	//	unless we set the Qt::WindowSystemMenuHint and Qt::WindowStaysOnTopHint,
	//	Similar to some of the old Emscripten oddities.  However, it must be a Qt::Tool
	//	type and not Qt::Window like old Emscripten.
	setWindowFlags(Qt::Tool | Qt::CustomizeWindowHint | Qt::BypassGraphicsProxyWidget | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint | (tipEditIsPinned(m_pParentCanOpener) ? Qt::WindowTitleHint : Qt::WindowType()));
#else
	setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::BypassGraphicsProxyWidget | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint | (tipEditIsPinned(m_pParentCanOpener) ? Qt::WindowTitleHint : Qt::WindowType()));
#endif
	setReadOnly(true);
	setLineWrapMode(QTextEdit::NoWrap);
	setWordWrapMode(QTextOption::NoWrap);
	setAcceptRichText(true);
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setContextMenuPolicy(Qt::DefaultContextMenu /* Qt::NoContextMenu */);
	setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard /* Qt::NoTextInteraction */);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setWindowTitle(tr("Details : King James Pure Bible Search", "MainMenu"));

	Q_ASSERT(m_pParentCanOpener != nullptr);

	m_pPushButton = new QPushButton(this);
	m_pPushButton->setFlat(true);
	setPushPinIcon();
	m_pPushButton->setIconSize(QSize(32, 32));
	QTimer::singleShot(1, this, SLOT(setPushPinPosition()));
	connect(m_pPushButton, SIGNAL(clicked()), this, SLOT(en_pushPinPressed()));

	if (instance(m_pParentCanOpener)) instance(m_pParentCanOpener)->deleteLater();
	setInstance(this);
	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	QPalette pal;
	pal.setBrush(QPalette::All, QPalette::Base, CToolTipEdit::palette().toolTipBase());
	pal.setBrush(QPalette::All, QPalette::Text, CToolTipEdit::palette().toolTipText());
	setPalette(pal);
	ensurePolished();
	setFrameStyle(QFrame::Box | QFrame::Plain);
	setAlignment(Qt::AlignLeft);
	qApp->installEventFilter(this);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, nullptr, this) / qreal(255.0));
	setMouseTracking(true);
	fadingOut = false;
	restorePersistentSettings();
}

CTipEdit::~CTipEdit()
{
	savePersistentSettings();
	if (instance(m_pParentCanOpener) == this) setInstance(nullptr);
}

void CTipEdit::savePersistentSettings()
{
#ifndef NO_PERSISTENT_SETTINGS
	if (CPersistentSettings::instance()->settings() == nullptr) return;

	const int nFontSize = fontInfo().pointSize();

	QSettings &settings(*CPersistentSettings::instance()->settings());
	settings.beginGroup(constrToolTipEditGroup);
	settings.setValue(constrFontSizeKey, nFontSize);
	settings.endGroup();
#endif
}

void CTipEdit::restorePersistentSettings()
{
#ifndef NO_PERSISTENT_SETTINGS
	if (CPersistentSettings::instance()->settings() == nullptr) return;

	QSettings &settings(*CPersistentSettings::instance()->settings());

	QFont fnt = font();

	settings.beginGroup(constrToolTipEditGroup);
	const int nFontSize = settings.value(constrFontSizeKey, fnt.pointSize()).toInt();
	settings.endGroup();

	if (nFontSize>0) {
		fnt.setPointSize(nFontSize);
		setFont(fnt);
	}
#endif
}


void CTipEdit::restartExpireTimer()
{
	int time = 10000 + 10 * qMax(0, toPlainText().length()-1000);
	expireTimer.start(time, this);
	hideTimer.stop();
}

void CTipEdit::reuseTip(const QString &strText)
{
	if (styleSheetParent){
		disconnect(styleSheetParent, SIGNAL(destroyed()),
						this, SLOT(styleSheetParentDestroyed()));
		styleSheetParent = nullptr;
	}

//	setWordWrap(Qt::mightBeRichText(strText));
	setText(strText);

	adjustToolTipSize();

	QTimer::singleShot(50, this, SLOT(activate()));
//	activateWindow();
	raise();

	restartExpireTimer();

	m_bFirstActivate = true;
}

void CTipEdit::adjustToolTipSize()
{
	QFontMetrics fm(font());
	QSize extra(1 + verticalScrollBar()->sizeHint().width() + frameWidth()*2, 1 + horizontalScrollBar()->sizeHint().height() + frameWidth()*2);

	// Make it look good with the default ToolTip font on Mac, which has a small descent.
	if (fm.descent() == 2 && fm.ascent() >= 11)
		++extra.rheight();

	document()->setTextWidth(document()->idealWidth());
	QSize docSize = document()->size().toSize();

	Q_ASSERT(!g_pMyApplication.isNull());
	CKJVCanOpener *pCanOpener = g_pMyApplication->activeCanOpener();
	if (widget) {
		resize(QSize(qMin(widget->width(),docSize.width()), qMin(widget->height(), docSize.height())) + extra);
	} else if (pCanOpener) {
		resize (QSize(qMin(pCanOpener->width()/2, docSize.width()), qMin(pCanOpener->height(), docSize.height())) + extra);
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
// Note: This code causes weird painting bug with scrollbar sliding.  I think we
//			are getting a double paint that doesn't quite agree with each other:
//
//	QStylePainter p(this->viewport());
//	QStyleOptionFrame opt;
//	opt.init(this->viewport());
//	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
//	p.end();

	QTextEdit::paintEvent(ev);
}

void CTipEdit::resizeEvent(QResizeEvent *e)
{
// Note: This masking causes the Windows platform to limit the window to just the
//			editor, which causes the title and resizing frame to not work correctly.
//			It came from the Qt ToolTip code and we shouldn't need it anyway...
//
//	QStyleHintReturnMask frameMask;
//	QStyleOption option;
//	option.init(this);
//	if (style()->styleHint(QStyle::SH_ToolTip_Mask, &option, this, &frameMask))
//		setMask(frameMask.region);

	QTextEdit::resizeEvent(e);
	setPushPinPosition();
}

void CTipEdit::setPushPinPosition()
{
	QSize szViewPort = viewport()->size();
	m_pPushButton->move(szViewPort.width() - m_pPushButton->size().width(), 0);
}

void CTipEdit::setPushPinIcon()
{
	m_pPushButton->setIcon(QIcon(tipEditIsPinned(m_pParentCanOpener) ? ":/res/Map-Marker-Push-Pin-2-Left-Chartreuse-icon-128.png" : ":/res/Map-Marker-Push-Pin-1-Chartreuse-icon-r-128.png"));
}

void CTipEdit::en_pushPinPressed()
{
	setTipEditIsPinned(!tipEditIsPinned(m_pParentCanOpener));
	setPushPinIcon();
	if (tipEditIsPinned(m_pParentCanOpener)) {
		QPoint pntTip = pos();
		pntTip.ry() += this->height()/2;
		setWindowFlags(windowFlags() | Qt::WindowTitleHint);
		CToolTipEdit::showText(m_pParentCanOpener, pntTip, text(), widget);
	} else {
		setWindowFlags(windowFlags() & ~Qt::WindowTitleHint);
#if (defined(EMSCRIPTEN) || defined(VNCSERVER)) && !defined(Q_OS_WASM)
		CToolTipEdit::hideText(m_pParentCanOpener);
#endif
	}
}

void CTipEdit::contextMenuEvent(QContextMenuEvent *e)
{
	hideTimer.stop();
	expireTimer.stop();
	QPointer<QMenu> pMenu = createStandardContextMenu();
#ifndef USE_ASYNC_DIALOGS
	m_bDoingContextMenu = true;
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
#else
	pMenu->setAttribute(Qt::WA_DeleteOnClose);
	pMenu->popup(e->globalPos());
	pMenu->connect(pMenu, SIGNAL(destroyed()), this, SLOT(activate()));
#endif
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
	if (!tipEditIsPinned(m_pParentCanOpener)) {
		close(); // to trigger QEvent::Close which stops the animation
		if (instance(m_pParentCanOpener) == this) setInstance(nullptr);
		deleteLater();
	}
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
//		if (widget) widget->activateWindow();
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
				if (widget) widget->activateWindow();
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
			if (!m_bFirstActivate) {
				if ((!m_bDoingContextMenu) && (!tipEditIsPinned(m_pParentCanOpener)))
					hideTip();
			} else {
				m_bFirstActivate = false;
				restartExpireTimer();
			}
			break;

		case QEvent::Enter:				// Entering us, activating us, or focusing us halts hiding us
		case QEvent::FocusIn:
			expireTimer.stop();
			// Fall through...
		case QEvent::WindowActivate:
			hideTimer.stop();
			break;

		default:
			break;
	}

	return QTextEdit::event(e);
}

void CTipEdit::wheelEvent(QWheelEvent *e)
{
	if (e->modifiers() & Qt::ControlModifier) {
#if QT_VERSION >= 0x050000
		const int delta = e->angleDelta().y();
#else
		const int delta = e->delta();
#endif
		if (delta < 0) {
			zoomOut();
		} else if (delta > 0) {
			zoomIn();
		}
		adjustToolTipSize();
		return;
	}

	QAbstractScrollArea::wheelEvent(e);
	updateMicroFocus();
}

int CTipEdit::getTipScreen(const QPoint &pos, QWidget *w)
{
#if QT_VERSION >= 0x050B00
	if (QGuiApplication::primaryScreen()->virtualSiblings().size() > 1) {
		QScreen *screen = QGuiApplication::screenAt(pos);
		return screen ? QGuiApplication::screens().indexOf(screen) : 0;
	} else {
#if QT_VERSION >= 0x050E00
		QScreen *screen = w ? w->screen() : nullptr;
		return screen ? QGuiApplication::screens().indexOf(screen) : 0;
#else
		return QApplication::desktop()->screenNumber(w);
#endif
	}
#else
	if (QApplication::desktop()->isVirtualDesktop())
		return QApplication::desktop()->screenNumber(pos);
	else
		return QApplication::desktop()->screenNumber(w);
#endif
}

void CTipEdit::placeTip(const QPoint &pos, QWidget *w)
{
//	if (testAttribute(Qt::WA_StyleSheet) || (w && qobject_cast<QStyleSheetStyle *>(w->style()))) {
	if (testAttribute(Qt::WA_StyleSheet) || (w && w->style() && w->style()->inherits("QStyleSheetStyle"))) {
		//the stylesheet need to know the real parent
		instance(m_pParentCanOpener)->setProperty("_q_stylesheet_parent", QVariant::fromValue(w));
		//we force the style to be the QStyleSheetStyle, and force to clear the cache as well.
//		instance(m_pParentCanOpener)->setStyleSheet(QLatin1String(" "));

		// Set up for cleaning up this later...
		instance(m_pParentCanOpener)->styleSheetParent = w;
		if (w) {
			connect(w, SIGNAL(destroyed()),
					instance(m_pParentCanOpener), SLOT(styleSheetParentDestroyed()));
		}
	}

#if QT_VERSION >= 0x050B00

	const QScreen *pScreen = QGuiApplication::screens().value(getTipScreen(pos, w),
                                                             QGuiApplication::primaryScreen());

	QRect screen;

	// a QScreen's handle *should* never be null, so this is a bit paranoid
	if (pScreen) {
#ifdef Q_OS_MAC
		// When in full screen mode, there is no Dock nor Menu so we can use
		// the whole screen for displaying the tooltip. However when not in
		// full screen mode we need to save space for the dock, so we use
		// availableGeometry instead.

		Q_ASSERT(!g_pMyApplication.isNull());
		CKJVCanOpener *pCanOpener = g_pMyApplication->activeCanOpener();

		if ((pCanOpener != nullptr) && (pCanOpener->isFullScreen())) {
			screen = pScreen->geometry();
		} else {
			screen = pScreen->availableGeometry();
		}
#else
		screen = pScreen->geometry();
#endif
	}

#else

#ifdef Q_OS_MAC
	// When in full screen mode, there is no Dock nor Menu so we can use
	// the whole screen for displaying the tooltip. However when not in
	// full screen mode we need to save space for the dock, so we use
	// availableGeometry instead.

	Q_ASSERT(!g_pMyApplication.isNull());
	CKJVCanOpener *pCanOpener = g_pMyApplication->activeCanOpener();

	QRect screen;
	if ((pCanOpener != nullptr) && (pCanOpener->isFullScreen()))
		screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
	else
		screen = QApplication::desktop()->availableGeometry(getTipScreen(pos, w));
#else
	QRect screen = QApplication::desktop()->screenGeometry(getTipScreen(pos, w));
#endif

#endif	// QT_VERSION

	QPoint p = pos;
	p += QPoint(2,
#ifdef Q_OS_WIN32
				21
#else
				16
#endif
		);

	p.ry() -= this->height()/2;

	if (!screen.isNull()) {
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
	}

	this->move(p);
}

bool CTipEdit::tipChanged(const QPoint &pos, const QString &strText, QObject *o)
{
	if (instance(m_pParentCanOpener)->text() != strText)
		return true;

	if (o != widget)
		return true;

	if (!rect.isNull())
		return !rect.contains(pos);
	else
	   return false;
}

// ============================================================================

void CToolTipEdit::showText(CKJVCanOpener *pCanOpener, const QPoint &pos, const QString &strText, QWidget *w, const QRect &rect)
{
	if (CTipEdit::instance(pCanOpener) && CTipEdit::instance(pCanOpener)->isVisible()){ // a tip does already exist
		if (strText.isEmpty()){ // empty text means hide current tip
			if (!CTipEdit::tipEditIsPinned(pCanOpener)) {
				CTipEdit::instance(pCanOpener)->hideTip();
			} else {
				CTipEdit::instance(pCanOpener)->setText(QString());
			}
			return;
		}
		else if (!CTipEdit::instance(pCanOpener)->fadingOut){
			if (!CTipEdit::tipEditIsPinned(pCanOpener)) {
				// If the tip has changed, reuse the one
				// that is showing (removes flickering)
				QPoint localPos = pos;
				if (w)
					localPos = w->mapFromGlobal(pos);
				if (CTipEdit::instance(pCanOpener)->tipChanged(localPos, strText, w)){
					CTipEdit::instance(pCanOpener)->setTipRect(w, rect);
					CTipEdit::instance(pCanOpener)->reuseTip(strText);
					CTipEdit::instance(pCanOpener)->placeTip(pos, w);
				}
			} else {
				CTipEdit::instance(pCanOpener)->setText(strText);
			}
			return;
		}
	}

	if ((!strText.isEmpty()) || CTipEdit::tipEditIsPinned(pCanOpener)) { // no tip can be reused, create new tip:
#ifndef Q_OS_WIN32
		new CTipEdit(pCanOpener, w); // sets CTipEdit::instance() to itself
#else
//		// On windows, we can't use the widget as parent otherwise the window will be
//		// raised when the tooltip will be shown
//		new CTipEdit(pCanOpener, QApplication::desktop()->screen(CTipEdit::getTipScreen(pos, w)));

		// For normal tooltips, the above applies, but since we are a special popup
		//	scroll widget, we need to or user can't activate us in a modal dialog!
		new CTipEdit(pCanOpener, w);
#endif
		CTipEdit::instance(pCanOpener)->setTipRect(w, rect);
		CTipEdit::instance(pCanOpener)->reuseTip(strText);
		CTipEdit::instance(pCanOpener)->placeTip(pos, w);
		CTipEdit::instance(pCanOpener)->setObjectName(QLatin1String("ctooltip_edit"));
		CTipEdit::instance(pCanOpener)->show();
	}
}

void CToolTipEdit::showText(CKJVCanOpener *pCanOpener, const QPoint &pos, const QString &strText, QWidget *w)
{
	CToolTipEdit::showText(pCanOpener, pos, strText, w, QRect());
}

bool CToolTipEdit::isVisible(CKJVCanOpener *pCanOpener)
{
	return ((CTipEdit::instance(pCanOpener) != nullptr) && CTipEdit::instance(pCanOpener)->isVisible());
}

QString CToolTipEdit::text(CKJVCanOpener *pCanOpener)
{
	if (CTipEdit::instance(pCanOpener))
		return CTipEdit::instance(pCanOpener)->text();
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
}

void CToolTipEdit::setFont(const QFont &font)
{
	QApplication::setFont(font, "CTipEdit");
}

