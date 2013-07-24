//
// C++ Implementation: QwwListWidget
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2008-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef WW_NO_LISTWIDGET
#include "qwwlistwidget.h"
#include "wwglobal_p.h"

class QwwListWidgetPrivate : public QwwPrivate {
public:
    QwwListWidgetPrivate(QwwListWidget *pub): QwwPrivate(pub){}
    
    void _q_curCh(QListWidgetItem *i);  
private:
    WW_DECLARE_PUBLIC(QwwListWidget);
};

/*!
 *  \class QwwListWidget
 *  \brief The QwwListWidget class provides an enhanced QListWidget.
 *  \inmodule wwWidgets
 * 
 *  This widget implements enhancements to QListWidget that allow manipulating
 *  the current item using signals and slots.
 */

/*!
 *  \fn     void QwwListWidget::currentAvailable(bool avail)
 *  \brief  Signal emitted when a current item becomes or stops being available.
 *  
 *          \a avail carries information if the item is available or not.
 *          This signal is useful if you want to enable or disable actions
 *          or buttons that operate on a selected item in the list.
 */
/*!
 *   \fn    void QwwListWidget::moveDownAvailable(bool avail)
 *   \brief This signal is emitted when the ability of moving the current item down changes.
 *
 *          \a avail carries information if the item can or cannot be moved down.
 */
/*!
 *   \fn    void QwwListWidget::moveUpAvailable(bool avail)
 *   \brief This signal is emitted when the ability of moving the current item up changes.
 *
 *          \a avail carries information if the item can or cannot be moved up.
 */


/*!
 * Constructs a line widget with a given \a parent.
 * 
 */
QwwListWidget::QwwListWidget(QWidget *parent) : QListWidget(parent), QwwPrivatable(new QwwListWidgetPrivate(this)) {
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), SLOT(_q_curCh(QListWidgetItem*)));
}

/*!
 *  \brief  Moves current item one row down if possible
 */
void QwwListWidget::moveCurrentDown() {
    int pos = currentRow();
    if (pos==count()-1) return;
    QListWidgetItem *item = takeItem(pos);
    insertItem(pos+1, item);
    setCurrentItem(item);
}

/*!
 *  \brief  Moves current item one row up if possible
 */
void QwwListWidget::moveCurrentUp() {
    int pos = currentRow();
    if (pos==0) return;
    QListWidgetItem *item = takeItem(pos);
    insertItem(pos-1, item);
    setCurrentItem(item);
}

/*!
 *  \brief  Removes current item from the list
 */
void QwwListWidget::removeCurrent() {
    Q_D(QwwListWidget);
    QListWidgetItem *item = currentItem();
    delete item;
    d->_q_curCh(currentItem());

}

void QwwListWidgetPrivate::_q_curCh(QListWidgetItem * i) {
    Q_Q(QwwListWidget);
    emit q->currentAvailable(i!=0);
    emit q->moveUpAvailable(q->currentRow()>0);
    emit q->moveDownAvailable(q->currentRow()>=0 && q->currentRow()<q->count()-1);
}

/*!
 * \internal
 */
void QwwListWidget::setCurrentRow(int row)
{
    QListWidget::setCurrentRow(row);
}

#include "moc_qwwlistwidget.cpp"
#endif
