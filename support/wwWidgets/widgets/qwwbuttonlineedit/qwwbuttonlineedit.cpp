//
// C++ Implementation: QwwButtonLineEdit
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2009-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qwwbuttonlineedit.h"

#if !defined(WW_NO_BUTTONLINEEDIT)

#include <QToolButton>
#include <QtDebug>
#include <QActionEvent>
#include <QRegExpValidator>

#include "qwwbuttonlineedit_p.h"

/*!
  \class QwwButtonLineEditPrivate
  \internal
 */

/*!
  \internal
 */
QwwButtonLineEditPrivate::QwwButtonLineEditPrivate(QwwButtonLineEdit *pub) : QwwPrivate(pub) {
        button = 0;
        position = QwwButtonLineEdit::RightInside;
        visible = true;
}

/*!
  \internal
 */
void QwwButtonLineEditPrivate::updateButtonPosition(int w) {
        Q_Q(QwwButtonLineEdit);
        QRect ncr = q->rect();
//         int w = button->sizeHint().width();
    if(position==QwwButtonLineEdit::None){
        button->hide();
    } else if(visible)
        button->show();
	if(position == QwwButtonLineEdit::RightInside){
	    button->setGeometry(ncr.right()-w+3, ncr.top()+2, w-4, ncr.height()-4);
	} else if (position == QwwButtonLineEdit::RightOutside)
            button->setGeometry(ncr.right()-w+1, ncr.top(), w, ncr.height());
        else if(position == QwwButtonLineEdit::LeftInside){
	    button->setGeometry(ncr.left()+2, ncr.top()+2, w-4, ncr.height()-4);
	} else
            button->setGeometry(ncr.left(), ncr.top(), w, ncr.height());
}


/*!
 *  \class  QwwButtonLineEdit
 *  \brief  A QLineEdit with an integrated customizable button.
 *
 *          The control is a line edit that contains a tool button
 *          can hold an icon and can be set either on the right or
 *          the left side of the line edit and can occupy space
 *          inside or outside the line edit.
 *
 *          \image qwwbuttonlineedit1.png QwwButtonLineEdit
 *
 *          \image qwwbuttonlineedit2.png QwwButtonLineEdit
 *
 *          It can be used to associate an easy to access action with
 *          the line edit, i.e. that will modify contents of the box
 *          in a definable way.
 *
 *          Using QWidget::addAction() one can associate more than one
 *          action with the button.
 */
/*!
 * \fn 	    void QwwButtonLineEdit::validatorChanged(const QString &regexp)
 * \brief   Signal emitted when the validator of the line edit changes to \a regexp
 */
 /*!
 
   \enum   QwwButtonLineEdit::Position
 
   \value None		   Button is not visible
   \value RightOutside     Button is positioned on the right side of the line edit.
   \value RightInside      Button is positioned inside the line edit on the right.
   \value LeftOutside      Button is positioned on the left side of the line edit.
   \value LeftInside       Button is positioned inside the line edit on the left.
 */
 /*!
    \fn     void QwwButtonLineEdit::buttonClicked()
          This signal is emitted when the button is clicked.

 */



/*!
 * Constructs a button line edit with a given \a parent.
 * 
 */
QwwButtonLineEdit::QwwButtonLineEdit(QWidget * parent) : QLineEdit(parent), QwwPrivatable(new QwwButtonLineEditPrivate(this)) {
    Q_D(QwwButtonLineEdit);
    d->button = new QToolButton(this);
    d->button->setCursor(Qt::ArrowCursor);
    connect(d->button, SIGNAL(clicked()), this, SIGNAL(buttonClicked()));
    setButtonPosition(RightOutside);
}

/*!
 * \brief Constructs a button line edit with a given \a parent and sets its text to \a contents.
 * 
 */
QwwButtonLineEdit::QwwButtonLineEdit(const QString & contents, QWidget * parent): QLineEdit(contents, parent), QwwPrivatable(new QwwButtonLineEditPrivate(this)) {
    Q_D(QwwButtonLineEdit);
    d->button = new QToolButton(this);
    d->button->setCursor(Qt::ArrowCursor);
    connect(d->button, SIGNAL(clicked()), this, SIGNAL(buttonClicked()));
    setButtonPosition(RightOutside);
}

/*!
 \internal
 */
QwwButtonLineEdit::QwwButtonLineEdit(QwwButtonLineEditPrivate & priv, QWidget * parent) : QLineEdit(parent), QwwPrivatable(&priv) {
    Q_D(QwwButtonLineEdit);
    d->button = new QToolButton(this);
    d->button->setCursor(Qt::ArrowCursor);
    connect(d->button, SIGNAL(clicked()), this, SIGNAL(buttonClicked()));
    setButtonPosition(RightOutside);
}

/*!
 * \internal
 * \reimp
 * 
 */
void QwwButtonLineEdit::resizeEvent(QResizeEvent * ev) {
    Q_D(QwwButtonLineEdit);
    int w = qMin(d->button->sizeHint().width(), height());
    d->updateButtonPosition(w);
    QLineEdit::resizeEvent(ev);
}

/*!
 * \property QwwButtonLineEdit::icon
 * This property holds the icon shown on the button
 *
 */

QIcon QwwButtonLineEdit::icon() const {
    Q_D(const QwwButtonLineEdit);
    return d->button ? d->button->icon() : QIcon();
}


void QwwButtonLineEdit::setIcon(const QIcon &ic) {
    Q_D(QwwButtonLineEdit);
    d->button->setIcon(ic);
}

void QwwButtonLineEdit::setAutoRaise(bool v) {
    Q_D(QwwButtonLineEdit);
    d->button->setAutoRaise(v);
}

/*!
 * \property QwwButtonLineEdit::autoRaise
 * \brief    This property holds whether auto-raising is enabled or not.
 *
 *          The default is disabled (i.e. false).
 *
 *          This property is currently ignored on Mac OS X when using QMacStyle.
 */

bool QwwButtonLineEdit::autoRaise() const {
    Q_D(const QwwButtonLineEdit);
    return d->button->autoRaise();
}

/*!
 *  \property QwwButtonLineEdit::buttonPosition
 *  This property holds the position of the button relative to the line edit.
 *
 */

QwwButtonLineEdit::Position QwwButtonLineEdit::buttonPosition() const {
    Q_D(const QwwButtonLineEdit);
    return d->position;
}

/*!
 *
 */
void QwwButtonLineEdit::setButtonPosition(Position pos) {
    Q_D(QwwButtonLineEdit);
    if (d->position==pos)
        return;
    d->position = pos;
    int w = qMin(d->button->sizeHint().width(), testAttribute(Qt::WA_Resized) ? height() : sizeHint().height());
    switch (pos) {
    case None:
#if QT_VERSION < 0x040500
        setStyleSheet("");
#else
        setTextMargins(0,0,0,0);
#endif
        setContentsMargins(0,0,0,0);
        break;
    case RightOutside:
#if QT_VERSION < 0x040500
        setStyleSheet("");
#else
        setTextMargins(0,0,0,0);
#endif
        setContentsMargins(0, 0, w+2, 0);
        break;
    case RightInside:
#if QT_VERSION < 0x040500
        setStyleSheet("");
#endif
        setContentsMargins(0,0,0,0);
#if QT_VERSION >= 0x040500
        setTextMargins(0, 0, w, 0);
#else
        setStyleSheet(QString("QwwButtonLineEdit { padding-right: %1; }").arg(w));
        setMinimumHeight(w);
        updateGeometry();
#endif

        break;
    case LeftOutside:
#if QT_VERSION < 0x040500
        setStyleSheet("");
#else
        setTextMargins(0,0,0,0);
#endif
        setContentsMargins(w+2, 0, 0, 0);
        break;
    case LeftInside:
#if QT_VERSION < 0x040500
        setStyleSheet("");
#endif
        setContentsMargins(0,0,0,0);
#if QT_VERSION >= 0x040500
        setTextMargins(w, 0, 0, 0);
#else
        setStyleSheet(QString("QwwButtonLineEdit { padding-left: %1; }").arg(w));
        setMinimumHeight(w);
        updateGeometry();
#endif
        break;
    }
    d->updateButtonPosition(w);
    update();
}

/*!
 * \internal
 * 
 */
void QwwButtonLineEdit::actionEvent(QActionEvent * ev) {
    Q_D(QwwButtonLineEdit);
    switch (ev->type()) {
    case QEvent::ActionAdded:
        d->button->insertAction(ev->before(), ev->action());
        break;
    case QEvent::ActionRemoved:
        d->button->removeAction(ev->action());
        break;
    case QEvent::ActionChanged:
    default:
        QLineEdit::actionEvent(ev);
    }
}

/**
 *
 */
void QwwButtonLineEdit::setButtonVisible(bool v) {
    Q_D(QwwButtonLineEdit);
    d->visible = v;
    d->button->setVisible(v && d->position!=None);
}

/*!
 *  \property QwwButtonLineEdit::buttonVisible
 *  This property holds information whether the button is visible.
 *  \sa button(), buttonPosition
 */

bool QwwButtonLineEdit::buttonIsVisible() const {
    Q_D(const QwwButtonLineEdit);
    return d->visible;
}

/*!
 *  Returns a pointer to the button.
 */
QToolButton * QwwButtonLineEdit::button() const
{
   Q_D(const QwwButtonLineEdit);
   return d->button;
}

/*!
 * \property	QwwButtonLineEdit::regExp
 *		This property holds the regular expression used to validate contents of the widget
 */

QString QwwButtonLineEdit::regExp() const {
    const QRegExpValidator *rxvalid = qobject_cast<const QRegExpValidator*>(validator());
    if (!rxvalid) return ".*";
    return rxvalid->regExp().pattern();
}

/*!
 * \brief   Sets \a v as a regular expression validator
 * \param v regular expression to set
 */

void QwwButtonLineEdit::setRegExp(const QString &v) {
    QValidator *valid = const_cast<QValidator*>(validator());
    QRegExpValidator *rxvalid = qobject_cast<QRegExpValidator*>(valid);
    if (rxvalid && rxvalid->regExp().pattern()==v) {
        return;
    }
    QRegExp rx(v);
    if (v==".*" || v.isEmpty()) {
        setValidator(0);
        emit validatorChanged(".*");
        return;
    }
    if (rxvalid) {
        rxvalid->setRegExp(rx);
    } else {
        setValidator(new QRegExpValidator(rx, this));
    }
    emit validatorChanged(v);
}

/*!
 *
 * \param rx
 * \overload
 */
void QwwButtonLineEdit::setRegExp(const QRegExp &rx) {
    QValidator *valid = const_cast<QValidator*>(validator());
    QRegExpValidator *rxvalid = qobject_cast<QRegExpValidator*>(valid);
    if (rxvalid && rxvalid->regExp()==rx) {
        return;
    }
    if (rx.pattern()==".*") {
        setValidator(0);
        emit validatorChanged(".*");
        return;
    }
    if (rxvalid) {
        rxvalid->setRegExp(rx);
    } else {
        setValidator(new QRegExpValidator(rx, this));
    }
    emit validatorChanged(rx.pattern());
}

/*!
 * \property QwwButtonLineEdit::buttonFocusPolicy()
 * \brief This property allows to specify the focus policy for the button that accompanies
 *        the line edit.
 */

Qt::FocusPolicy QwwButtonLineEdit::buttonFocusPolicy() const {
    return button()->focusPolicy();
}

void QwwButtonLineEdit::setButtonFocusPolicy(Qt::FocusPolicy p) {
    button()->setFocusPolicy(p);
}

#if QT_VERSION < 0x040700
QString QwwButtonLineEdit::placeholderText() const {
    Q_D(const QwwButtonLineEdit);
    return d->placeholderText;
}

void QwwButtonLineEdit::setPlaceholderText(const QString &txt) {
    Q_D(QwwButtonLineEdit);
    if(d->placeholderText == txt)
        return;
    d->placeholderText = txt;
    if(!hasFocus() && text().isEmpty())
        update();
}
void QwwButtonLineEdit::paintEvent(QPaintEvent *pe) {
    Q_D(QwwButtonLineEdit);
    QLineEdit::paintEvent(pe);
    if(!hasFocus() && text().isEmpty() && !d->placeholderText.isEmpty()) {

        QPainter p(this);

        QStyleOptionFrameV2 panel;
        initStyleOption(&panel);
        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);
#if QT_VERSION >= 0x040500
        int lM, rM, tM, bM;
        getTextMargins(&lM, &tM, &rM, &bM);
        r.setX(r.x() + lM);
        r.setY(r.y() + tM);
        r.setRight(r.right() - rM);
        r.setBottom(r.bottom() - bM);
#endif
        p.setClipRect(r);

        QFontMetrics fm = fontMetrics();
        Qt::Alignment va = QStyle::visualAlignment(layoutDirection(), QFlag(alignment()));
        int vscroll = r.y() + (r.height() - fm.height() + 1) / 2;
//        switch (va & Qt::AlignVertical_Mask) {
//         case Qt::AlignBottom:
//             vscroll = r.y() + r.height() - fm.height() - d->verticalMargin;
//             break;
//         case Qt::AlignTop:
//             d->vscroll = r.y() + d->verticalMargin;
//             break;
//         default:
//             //center
//             d->vscroll = r.y() + (r.height() - fm.height() + 1) / 2;
//             break;
//        }
        QRect lineRect(r.x()+1, vscroll, r.width() - 2, fm.height());


        QColor col = palette().text().color();



        col.setAlpha(128);
        QPen oldpen = p.pen();
        QFont oldfont = p.font();
        QFont newfont = oldfont;
        newfont.setItalic(true);
        p.setFont(newfont);
        p.setPen(col);
        p.drawText(lineRect, va, d->placeholderText);
        p.setPen(oldpen);

    }
}
#endif

#endif
