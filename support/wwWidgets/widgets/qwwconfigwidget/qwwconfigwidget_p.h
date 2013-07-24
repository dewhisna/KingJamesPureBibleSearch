//
// C++ Interface: qwwconfigwidget_p
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCONFIGWIDGET_P_H
#define QWWCONFIGWIDGET_P_H
#include <QAbstractItemDelegate>
#include <QPainter>

class ConfigWidgetDelegate : public QAbstractItemDelegate {
public:
    ConfigWidgetDelegate(QObject *parent=0) : QAbstractItemDelegate(parent) {}

    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
        QStyleOptionViewItem opt = option;
        if (opt.state & QStyle::State_Selected) {
            painter->fillRect(opt.rect, opt.palette.highlight());
        }

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QSize ds = opt.decorationSize;
        QRect decorationRect(opt.rect.x()+4, opt.rect.top()+4, opt.rect.width()-8, ds.height());

        icon.paint(painter, decorationRect, Qt::AlignHCenter|Qt::AlignTop, opt.state & QStyle::State_Enabled ? ((opt.state & QStyle::State_Selected) && opt.showDecorationSelected ? QIcon::Selected : QIcon::Normal) : QIcon::Disabled);

        QString displayText = index.data(Qt::DisplayRole).toString();
        painter->setPen(opt.palette.color(option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
        QRect displayRect = opt.rect.adjusted(2, ds.height()+2, -2, -2);
#if QT_VERSION >= 0x040200
        painter->drawText(displayRect, Qt::AlignHCenter|Qt::AlignBottom|Qt::TextWordWrap, opt.fontMetrics.elidedText(displayText, opt.textElideMode, displayRect.width()));
#else
        painter->drawText(displayRect, Qt::AlignHCenter|Qt::AlignBottom|Qt::TextWordWrap, QAbstractItemDelegate::elidedText( opt.fontMetrics, displayRect.width(), opt.textElideMode, displayText));
#endif
    }
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const {
        QSize ds = option.decorationSize;
        int totalwidth = option.fontMetrics.width(index.data(Qt::DisplayRole).toString());
        QSize ts;
//         if(totalwidth>option.rect.width())
//             ts = QSize(totalwidth/2, 2*option.fontMetrics.height());
//         else
        ts = QSize(totalwidth, option.fontMetrics.height());
        return QSize(qBound(ds.width(), ts.width(), option.rect.width()), ds.height()+6+ts.height());
    }
};

/**
 * move into separate file
 */
class QwwConfigWidgetPrivate : public QwwPrivate {
public:
    QwwConfigWidgetPrivate(QwwConfigWidget *p) : QwwPrivate(p),
            discarding(false), saving(false), applying(false) {}
    ~QwwConfigWidgetPrivate() {}
    QStackedWidget *stack;
    QListWidget *view;
    QHBoxLayout *layout;
    QLabel *titleLabel;
    bool discarding;
    bool saving;
    bool applying;
};

#endif
