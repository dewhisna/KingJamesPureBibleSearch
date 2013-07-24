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
#ifndef WW_NO_CONFIGWIDGET
#include "qwwconfigwidget.h"
#include "wwglobal_p.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QLayout>
#include <QPainter>
#include <QLabel>
#include <QFrame>
#include <QEvent>

#include "qwwconfigwidget_p.h"

/*!
 *  \class  QwwConfigWidget
 *  \brief  A multipage container widget organizing configuration pages
 *
 */
/*!
 *  \fn QwwConfigWidget::applying(int)
 *  \internal
 */
/*!
 *  \fn QwwConfigWidget::discarding(int)
 *  \internal
 */
/*!
 *  \fn QwwConfigWidget::saving()
 *  \internal
 */
/*!
 *  \fn QwwConfigWidget::currentIndexChanged(int index)
 *  \brief This signal is emitted when current page of the widget is changed to \a index.
 */


/*!
 * Constructs a config widget with a given \a parent.
 * 
 */
QwwConfigWidget::QwwConfigWidget(QWidget *parent)
        : QWidget(parent), QwwPrivatable(new QwwConfigWidgetPrivate(this)) {
    Q_D(QwwConfigWidget);
    d->stack = new QStackedWidget;
    while (d->stack->count()>0) d->stack->removeWidget(d->stack->widget(0));
    d->view = new QListWidget;

    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    d->view->setIconSize(QSize(48, 48));
    d->view->setMaximumWidth(150);
    d->view->viewport()->setObjectName("__qt__passive_listviewviewport");
    d->view->setItemDelegate(new ConfigWidgetDelegate(d->view));
    d->layout = new QHBoxLayout(this);
    d->layout->addWidget(d->view);
    d->layout->setMargin(0);
    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->setSpacing(2);
    d->layout->addLayout(vl);
    d->titleLabel = new QLabel;
    QFont f = d->titleLabel->font();
    f.setBold(true);
    d->titleLabel->setFont(f);
    vl->addWidget(d->titleLabel);
    QFrame *hline = new QFrame;
    hline->setFrameShape(QFrame::HLine);
    vl->addWidget(hline);
    vl->addWidget(d->stack);
    QSizePolicy pol = d->view->sizePolicy();
    pol.setHorizontalStretch(1);
    d->view->setSizePolicy(pol);
    pol = d->stack->sizePolicy();
    pol.setHorizontalStretch(5);
    d->stack->setSizePolicy(pol);
    pol = d->titleLabel->sizePolicy();
    pol.setHorizontalStretch(5);
    d->titleLabel->setSizePolicy(pol);
    pol = hline->sizePolicy();
    pol.setHorizontalStretch(5);
    hline->setSizePolicy(pol);

    connect(d->view, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentIndex(int)));
    connect(d->view, SIGNAL(currentRowChanged(int)), this, SLOT(_q_clicked()));
//     connect(d->stack, SIGNAL(currentChanged(int)), d->view, SLOT(setCurrentRow(int)));
}

/*!
 *  \brief Appends a new \a group with icon \a icon and label \a name into the widget
 *  \sa insertGroup()
 */
void QwwConfigWidget::addGroup(QWidget *group, const QIcon & icon, const QString & name) {
    Q_D(QwwConfigWidget);
    insertGroup(d->stack->count(), group, icon, name);
}

/*!
 * \brief Inserts a new \a group with an \a icon and a \a label to the widget at arbitrary \a index.
 * \sa addGroup()
 */
void QwwConfigWidget::insertGroup(int index, QWidget *group, const QIcon & icon, const QString & label) {
    Q_D(QwwConfigWidget);
    d->stack->insertWidget(index, group);
    QListWidgetItem *item = new QListWidgetItem;

    if (label.isNull()) {
        item->setText(group->windowTitle());
    } else {
        item->setText(label);
        group->setWindowTitle(label);
    }
    if (icon.isNull()) {
        item->setIcon(group->windowIcon());
    } else {
        item->setIcon(icon);
        group->setWindowIcon(icon);
    }
    ((QListWidget*)(d->view))->insertItem(index, item);
    if (d->stack->count()==1) {
        d->titleLabel->setText(d->view->item(index)->text());
        setCurrentIndex(0);
    }
    group->installEventFilter(this);
}

/*!
 * \brief Removes \a group from the widget.
 *
 */
void QwwConfigWidget::removeGroup(QWidget *group) {
    Q_D(QwwConfigWidget);
    int idx = d->stack->indexOf(group);
    if(idx<0)
        return;
    QListWidgetItem *item = d->view->takeItem(idx);
    d->stack->removeWidget(group);
    group->removeEventFilter(this);
    delete item;
    delete group;
}

/*!
 *  \brief Returns a configuration group widget with a specified \a index.
 *  \sa currentGroup()
 */
QWidget * QwwConfigWidget::group(int index) const {
    Q_D(const QwwConfigWidget);
    return d->stack->widget(index);
}

/*!
 * \brief Returns a configuration group widget that is currently selected.
 * \sa group()
 */
QWidget * QwwConfigWidget::currentGroup() const {
    Q_D(const QwwConfigWidget);
    return d->stack->currentWidget();
}

/*!
 * \property QwwConfigWidget::currentIndex
 * \brief index of the currently selected group.
 */
int QwwConfigWidget::currentIndex() const {
    Q_D(const QwwConfigWidget);
    return d->stack->currentIndex();
}

/*!
 * \brief Sets the currently selected group to the group specified by \a index.
 */
void QwwConfigWidget::setCurrentIndex(int index) {
    Q_D(QwwConfigWidget);
    if (index==currentIndex()) return;
    if (index<0 || index>=d->view->count()) return;
    d->stack->setCurrentIndex(index);
    d->view->setCurrentRow(index);
    d->titleLabel->setText(d->view->item(index)->text());
    emit currentIndexChanged(index);
}

/*!
 *
 * \overload
 */
void QwwConfigWidget::removeGroup(int index) {
    Q_D(QwwConfigWidget);
    removeGroup(d->stack->widget(index));
}

/*!
 * \internal
 * FIXME
 */
void QwwConfigWidget::setCurrentGroup(const QString &) {}

/*!
 * \brief Sets the currently selected group to the specified \a widget.
 */
void QwwConfigWidget::setCurrentGroup(QWidget * widget) {
    Q_D(const QwwConfigWidget);
    setCurrentIndex(d->stack->indexOf(widget));
}

/*!
 * \internal
 */
void QwwConfigWidget::save() {
    Q_D(QwwConfigWidget);
    if (d->saving) return; // safe check
    d->saving = true;
    emit saving();
    d->saving = false;
}

/*!
 * \internal
 */
void QwwConfigWidget::apply() {
    Q_D(QwwConfigWidget);
    if (d->applying) return; // safe check
    d->applying = true;
    emit applying(currentIndex());
    QMetaObject::invokeMethod(currentGroup(), "apply");
    d->applying = false;
}

/*!
 * \internal
 */
void QwwConfigWidget::discard() {
    Q_D(QwwConfigWidget);
    if (d->discarding) return; // safe check
    d->discarding = true;
    emit discarding(currentIndex());
    QMetaObject::invokeMethod(currentGroup(), "discard");
    d->discarding = false;
}

/*!
 * \property 	QwwConfigWidget::count
 * \brief 	This property holds the number of pages in the widget.
 */
int QwwConfigWidget::count() const {
    Q_D(const QwwConfigWidget);
    return d->stack->count();
}

/*!
 * \brief Sets \a icon as the icon of group specified by \a index.
 */
void QwwConfigWidget::setGroupIcon(int index, const QIcon &icon) {
    Q_D(QwwConfigWidget);
    QWidget *w = group(index);
    w->setWindowIcon(icon);
    d->view->item(index)->setIcon(icon);
}

/*!
 * \brief Sets \a title as the label of a group specified by \a index.
 */
void QwwConfigWidget::setGroupLabel(int index, const QString &title) {
    Q_D(QwwConfigWidget);
    QWidget *w = group(index);
    w->setWindowTitle(title);
    d->view->item(index)->setText(title);
    if (index==currentIndex()) {
        d->titleLabel->setText(title);
    }
}

/*!
 * \property QwwConfigWidget::iconSize
 * \brief This property holds the icon size used for this button.
 *
 *        The default size is defined by the GUI style. This is a maximum size for the icons. Smaller icons will not be scaled up.
 */
QSize QwwConfigWidget::iconSize() const {
    Q_D(const QwwConfigWidget);
    return d->view->iconSize();
}

void QwwConfigWidget::setIconSize(const QSize &s) {
    Q_D(QwwConfigWidget);
    d->view->setIconSize(s);
}

/*!
 * \internal
 * \reimp
 */
bool QwwConfigWidget::eventFilter(QObject *object, QEvent *event){
  if(event->type()==QEvent::WindowTitleChange){
    QWidget *w = qobject_cast<QWidget*>(object);
    for(int ind=0;ind<count();ind++){
      if(group(ind)==w){
        setGroupLabel(ind, w->windowTitle());
      }
    }
    return false;
  } 
  
  return QWidget::eventFilter(object, event);
}

#include "moc_qwwconfigwidget.cpp"

#endif
