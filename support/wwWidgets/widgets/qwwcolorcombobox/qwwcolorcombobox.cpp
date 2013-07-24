//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2009-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qwwcolorcombobox.h"

#if !defined(WW_NO_COLORCOMBOBOX)
#include <QItemDelegate>
#include <QColorDialog>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QLayout>
#include <QPushButton>
#include <QEvent>
#include <QStandardItemModel>
#include <QStylePainter>
#include <QDragEnterEvent>
#include "wwglobal_p.h"
#include "colormodel.h"


class QwwColorComboBoxPrivate : public QwwPrivate {
public:
    QwwColorComboBoxPrivate(QwwColorComboBox *q) : QwwPrivate(q) {
        m_dlgEnabled = false;
        m_model = 0;
    }

    bool m_dlgEnabled;
    ColorModel *m_model;
    void _q_activated(int i) {
        Q_Q(QwwColorComboBox);
        if (q->isColorDialogEnabled() && i==q->count()-1) {
            _q_popupDialog();
        }

        QVariant v = q->itemData(i, Qt::DecorationRole);
        if (v.isValid()) {
            QColor c = qvariant_cast<QColor>(v);
            if (c.isValid())
                emit q->activated(c);
        }
    }

    void _q_popupDialog() {
        Q_Q(QwwColorComboBox);
#if QT_VERSION >= 0x040500
        QColor newcol = QColorDialog::getColor(q->currentColor(), q, q->tr("Choose color"), QColorDialog::ShowAlphaChannel);
#else
        QColor newcol = QColorDialog::getColor(q->currentColor(), q);
#endif
        if (newcol.isValid()) {
            int ind = q->findData(newcol, Qt::DecorationRole);
            if (ind==-1) {
                q->addColor(newcol, q->tr("Custom color"));
                ind = q->count()-1;
            }
            q->setCurrentIndex(ind);
        }
    }
private:
    WW_DECLARE_PUBLIC(QwwColorComboBox);
};

/*!
 *  \class QwwColorComboBox
 *  \brief The QwwColorComboBox class provides a combobox that allows to pick colors.
 *  \ingroup colorclasses
 */
/*!
 *  \property   QwwColorComboBox::colorDialogEnabled
 *  \brief  	Property holds information whether picking custom colors using a color dialog is enabled.
 */
/*!
 *  \fn QwwColorComboBox::activated(const QColor &color)
 *  \brief Signal emitted when \a color is activated
 */


/*!
 * Constructs a color combo box with a given \a parent.
 * 
 */
QwwColorComboBox::QwwColorComboBox(QWidget *parent)
        : QComboBox(parent), QwwPrivatable(new QwwColorComboBoxPrivate(this)) {
    Q_D(QwwColorComboBox);
    connect(this, SIGNAL(activated(int)), this, SLOT(_q_activated(int)));
    d->m_model = new ColorModel(this);
    setModel(d->m_model);
    view()->installEventFilter(this);
    setAcceptDrops(true);
//     setDuplicatesEnabled(false);
}

/*!
 * \internal
 */
QwwColorComboBox::~QwwColorComboBox() {}

/*!
 * Adds the specified \a color with a \a name to the list of available colors.
 * \sa insertColor()
 */
void QwwColorComboBox::addColor(const QColor & color, const QString & name) {
    insertColor(colorCount(), color, name);
}

/*!
 * \brief Sets \a color as the currently picked color.
 *
 *        Color is added to the list of available colors if it's not there yet.
 *
 * \param color
 */
void QwwColorComboBox::setCurrentColor(const QColor & color) {
    int i = findData(color, Qt::DecorationRole);
    if (i!=-1) {
        setCurrentIndex(i);
    } else {
        addColor(color, tr("Custom color"));
        setCurrentIndex(count()-1);
    }
}


/*!
 *        Inserts the specified \a color with a \a name in the list of available
 *        colors under specified \a index
 *  \sa addColor()
 */
void QwwColorComboBox::insertColor(int index, const QColor & color, const QString & name) {
    Q_D(QwwColorComboBox);
    d->m_model->insertColor(index, color, name);
}

/*!
 * \property QwwColorComboBox::currentColor
 * \brief   Currently chosen color.
 * \sa	colors, colorCount
 */
QColor QwwColorComboBox::currentColor() const {
    return color(currentIndex());
}

/*!
 * \property QwwColorComboBox::colorCount
 * \brief   Number of colors available.
 * \sa	colors, currentColor
 */
int QwwColorComboBox::colorCount() const {
    Q_D(const QwwColorComboBox);
    return d->m_model->rowCount();
}

/*!
 * \brief   Returns the color at position \a index
 * \sa	currentColor, colors
 */
QColor QwwColorComboBox::color(int index) const {
    return qvariant_cast<QColor>(itemData(index, Qt::DecorationRole));
}

/*!
 * \brief   Fills the list of available colors with standard colors.
 * \sa      colors
 */
void QwwColorComboBox::setStandardColors() {
    Q_D(QwwColorComboBox);
    d->m_model->clear();
    QStringList clist = QColor::colorNames();
    foreach(QString col, clist) {
        QColor c;
        c.setNamedColor(col);
        addColor(c, col);
    }
}


/*!
 * \brief   Returns whether the color dialog is enabled.
 */
bool QwwColorComboBox::isColorDialogEnabled() const {
    Q_D(const QwwColorComboBox);
    return d->m_dlgEnabled;
}

/*!
 * \brief   Enables or disables the color dialog.
 */
void QwwColorComboBox::setColorDialogEnabled(bool enabled) {
    Q_D(QwwColorComboBox);
    d->m_dlgEnabled = enabled;
}



/*!
 *  \internal
 *  Reimplemented from QComboBox.
 */
bool QwwColorComboBox::eventFilter(QObject * o, QEvent * e) {
    if (o==view()) {
        if (e->type()==QEvent::Show) {
            if (isColorDialogEnabled()) {
                addItem(tr("Other"));
                int ind = count()-1;
                setItemData(ind, Qt::AlignCenter, Qt::TextAlignmentRole);
                setItemData(ind, palette().color(QPalette::Button), Qt::BackgroundRole);
                setItemData(ind, tr("Choose a custom color"), Qt::ToolTipRole);
            }
            return false;
        }
        if (e->type()==QEvent::Hide) {
            if (isColorDialogEnabled())
                removeItem(count()-1);
            return false;
        }

    }
    return QComboBox::eventFilter(o, e);
}

/*!
 * \internal
 *
 * Taken from QComboBox
 */
void QwwColorComboBox::paintEvent(QPaintEvent * ) {
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);
    if (opt.currentIcon.isNull()) {
        QColor c = qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
        if (c.isValid()) {
            int siz = style()->pixelMetric(QStyle::PM_ButtonIconSize, &opt, this);
            QPixmap px(siz, siz);
            px.fill(c);
            opt.currentIcon = px;
        }
    }
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);

}

/*!
 *  \internal
 *  \reimp
 */
void QwwColorComboBox::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasColor())
        event->acceptProposedAction();
    else if (event->mimeData()->hasText()) {
        QColor col;
        col.setNamedColor(event->mimeData()->text());
        if (col.isValid())
            event->acceptProposedAction();
    }
}

/*!
 *  \internal
 *  \reimp
 */
void QwwColorComboBox::dropEvent(QDropEvent *event) {
    QColor c;
    if (event->mimeData()->hasColor()) {
        c = qvariant_cast<QColor>(event->mimeData()->colorData());
    } else if (event->mimeData()->hasText()) {
        c.setNamedColor(event->mimeData()->text());
    }
    setCurrentColor(c);
}

/*!
 * \property QwwColorComboBox::colors
 * \brief list of colors known to the button.
 * \sa colorCount, currentColor
 */
QStringList QwwColorComboBox::colors() const {
    Q_D(const QwwColorComboBox);
    QStringList slist;
    for (int i=0;i<d->m_model->rowCount();i++) {
        QModelIndex ind = d->m_model->index(i,0);
        slist << QString("%1,%2").arg(qvariant_cast<QColor>(ind.data(Qt::DecorationRole)).name()).arg(ind.data(Qt::DisplayRole).toString());
    }
    return slist;
}

/*!
 *  \brief Sets a list of colors encoded into strings as available colors
 *
 */
void QwwColorComboBox::setColors(const QStringList &map) {
    clear();
    Q_D(QwwColorComboBox);
    d->m_model->clear();
    foreach(QString nam, map) {
        QStringList slist = nam.split(",");
        QColor c;
        c.setNamedColor(slist[0]);
        addColor(c, slist[1]);
    }
    d->_q_activated(0);
    update();
}

/*!
 * \internal
 * Work around faulty Designer behaviour
 */
void QwwColorComboBox::showPopup() {
#if QT_VERSION < 0x040600
    // remove all items that don't have colors associated with them
    while (count()>0) {
        int it = count()-1;
        QVariant dat = itemData(it, Qt::DecorationRole);
        QColor c = qvariant_cast<QColor>(dat);
        if (!c.isValid()) {
            removeItem(it);
        } else
            break;
    }
#endif
    QComboBox::showPopup();
}

#include "moc_qwwcolorcombobox.cpp"
#endif
