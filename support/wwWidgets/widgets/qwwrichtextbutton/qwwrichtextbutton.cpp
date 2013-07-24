//
// C++ Implementation: QwwRichTextButton
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2008-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WW_NO_RICHTEXTBUTTON

#include "qwwrichtextbutton.h"
#include <QPaintEvent>
#include <QTextDocument>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QStyleOptionButton>
#include <QApplication>
#include "wwglobal_p.h"

class QwwRichTextButtonPrivate : public QwwPrivate {
public:
    QwwRichTextButtonPrivate(QwwRichTextButton *pub) : QwwPrivate(pub){}
    
    QTextDocument *doc;
    void initStyleOption(QStyleOptionButton *opt) const;
    bool internalDoc;    
    
    void _q_documentSizeChanged(const QSizeF &);
    void _q_documentDestroyed();
private:
    WW_DECLARE_PUBLIC(QwwRichTextButton);
};


/*!
 *  \class QwwRichTextButton
 *  \inmodule wwWidgets
 *  \brief The QwwRichTextButton class provides a button that can display rich text content.
 *
 *  This widget is similar in its behavior to QPushButton, but it can display not only
 *  plain text, but also rich text. The text to be displayed can be set either using
 *  the \a html property or by assigning a QTextDocument to the widget using setDocument().
 *
 */
/*!
 * \property QwwRichTextButton::html
 *
 * This property holds the text displayed on the button.
 */
/*!
 * \fn      void QwwRichTextButton::setText(const QString &t)
 * \internal
 * \brief   Equivalent of setHtml(\a t)
 * \sa      setHtml()
 */
/*!
 * \fn      QString QwwRichTextButton::text() const
 * \internal
 * \brief   Equivalent of html()
 * \sa      html()
 */
/*!
 * \property QwwRichTextButton::text
 * \internal
 */


/*!
 * Constructs a rich text button with a given \a parent.
 */
QwwRichTextButton::QwwRichTextButton(QWidget * parent) : QAbstractButton(parent), QwwPrivatable(new QwwRichTextButtonPrivate(this)) {
    Q_D(QwwRichTextButton);
    d->doc = 0;
    setText("");
    d->internalDoc = false;
    QTextDocument *doc = new QTextDocument(this);
    doc->setHtml("Rich text button");
    setDocument(doc);
    d->internalDoc = true;
    QSizePolicy sp(QSizePolicy::Minimum, QSizePolicy::Fixed);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setAttribute(Qt::WA_Hover);
}

/*!
 * Constructs a rich text button with a given \a parent and \a doc set as the document
 */
QwwRichTextButton::QwwRichTextButton(QTextDocument * doc, QWidget * parent) : QAbstractButton(parent), QwwPrivatable(new QwwRichTextButtonPrivate(this)) {
    Q_D(QwwRichTextButton);
    d->internalDoc = false;
    setDocument(doc);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setAttribute(Qt::WA_Hover);
}

/*!
 * \internal
 */
QwwRichTextButton::~QwwRichTextButton() {
    Q_D(QwwRichTextButton);
    if (d->internalDoc) {
        delete d->doc;
    }
}

/*!
 * \internal
 */
void QwwRichTextButton::paintEvent(QPaintEvent *) {
    Q_D(QwwRichTextButton);
    QPainter p(this);
    QStyleOptionButton opt;
    d->initStyleOption(&opt);
    style()->drawControl(QStyle::CE_PushButtonBevel, &opt, &p, this);
    if (!document()) return;
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = palette();
    QRect r = style()->subElementRect ( QStyle::SE_PushButtonContents, &opt, this);
    p.translate(r.left()-rect().left(), r.top()-rect().top());
    int hei = qRound(document()->documentLayout()->documentSize().height());
    if(r.height()>hei){
        p.translate(0, (r.height()-hei)/2);
    }
    if (isDown() || isChecked()) {
        QSize shift(style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this), style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &opt, this));
        p.translate(shift.width(), shift.height());
    }
    qreal oldwidth = d->doc->textWidth();
    d->doc->setTextWidth(r.width());
    p.save();
    p.setClipRect(0,0,r.width(), r.height());
    p.setClipping(true);
    document()->documentLayout()->draw(&p, ctx);
    d->doc->setTextWidth(oldwidth);
    p.restore();
}

/*!
 *  \brief  Makes \a doc the new document for the button.
 *
 *          The content of the document is displayed on the button.
 *          Widget does not take ownership of the document.
 */
void QwwRichTextButton::setDocument(QTextDocument * doc) {
    Q_D(QwwRichTextButton);
    if (document()) {
//         disconnect(document()->documentLayout(), SIGNAL(documentSizeChanged( const QSizeF& )), this, SLOT(_q_documentSizeChanged(const QSizeF&)));
        disconnect(document(), SIGNAL(contentsChanged()), this, SLOT(update()));
        disconnect(document(), SIGNAL(destroyed()), this, SLOT(_q_documentDestroyed()));
    }
    if (d->internalDoc) {
        delete d->doc;
    }
    d->doc = doc;
    if (!doc) return;
#if QT_VERSION >= 0x040200
    d->doc->adjustSize();
#endif
//     connect(d->doc->documentLayout(), SIGNAL(documentSizeChanged( const QSizeF& )), this, SLOT(_q_documentSizeChanged(const QSizeF&)));
    updateGeometry();
    connect(d->doc, SIGNAL(contentsChanged()), this, SLOT(update()));
    connect(document(), SIGNAL(destroyed()), this, SLOT(_q_documentDestroyed()));
    d->internalDoc = false;
}

/*!
 *  \brief  Returns the current document for the button.
 */
QTextDocument * QwwRichTextButton::document() const {
    Q_D(const QwwRichTextButton);
    return d->doc;
}

/*!
 * \internal
 */
QSize QwwRichTextButton::sizeHint() const {
    Q_D(const QwwRichTextButton);
    if (!document())
        return QSize(80,25);
    QStyleOptionButton opt;
    d->initStyleOption(&opt);
    int m = style()->pixelMetric(QStyle::PM_ButtonMargin, &opt, this);
    m+=style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
    m*=2;
    return QSize(80,25).expandedTo(QApplication::globalStrut()).expandedTo(document()->size().toSize()+QSize(m, m));
}

/*!
 * \internal
 */
void QwwRichTextButtonPrivate::initStyleOption(QStyleOptionButton * opt) const {
    Q_Q(const QwwRichTextButton);
    if (!opt) return;
    opt->initFrom(q);
//    opt->features = StyleOptionButton::None;
    if (q->isDown())
        opt->state |= QStyle::State_Sunken;
    else
        opt->state |= QStyle::State_Raised;
    if (q->isChecked())
        opt->state |= QStyle::State_On;
}

void QwwRichTextButtonPrivate::_q_documentSizeChanged(const QSizeF &) {
    Q_Q(QwwRichTextButton);
    q->updateGeometry();
}

/*!
 *  Returns text of the current document.
 */
QString QwwRichTextButton::html() const {
    return (document() ? document()->toHtml() : "");
}

/*!
 *  Sets \a txt as new rich text document of the button.
 */
void QwwRichTextButton::setHtml(const QString &txt) {
    Q_D(QwwRichTextButton);
    if (!document()) {
        QTextDocument *doc = new QTextDocument(this);
        doc->setHtml(txt);
        d->internalDoc = false;
        setDocument(doc);
        d->internalDoc = true;
    } else {
        if (d->internalDoc) {
            d->doc->setHtml(txt);

        } else {
            QTextDocument *doc = new QTextDocument(this);
            doc->setHtml(txt);
            setDocument(doc);
            d->internalDoc = true;
        }
    }
#if QT_VERSION >=0x040200
    document()->adjustSize();
#endif
    updateGeometry();
    update();
}

void QwwRichTextButtonPrivate::_q_documentDestroyed() {
    Q_Q(QwwRichTextButton);
    if (!internalDoc) {
        q->setDocument(new QTextDocument(q));
        internalDoc = true;
    }
}

/*!
 * \internal
 */
QSize QwwRichTextButton::minimumSizeHint() const {
    return QSize(80, 25).expandedTo(QApplication::globalStrut());
}

/*!
 * \internal
 */
int QwwRichTextButton::heightForWidth(int w) const {
    Q_D(const QwwRichTextButton);
    if (!d->doc) return -1;
    QStyleOptionButton opt;
    d->initStyleOption(&opt);
    int m = style()->pixelMetric(QStyle::PM_ButtonMargin, &opt, this);
    m+=style()->pixelMetric(QStyle::PM_DefaultFrameWidth, 0, this);
    d->doc->setTextWidth(w-m);
    return d->doc->size().toSize().height()+m;
}

void QwwRichTextButton::setText(const QString &t){
        setHtml(t);
}

#include "moc_qwwrichtextbutton.cpp"
#endif
