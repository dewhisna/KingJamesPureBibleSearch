//
// C++ Interface: qwwconfigwidget
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCONFIGWIDGET_H
#define QWWCONFIGWIDGET_H
#ifndef WW_NO_CONFIGWIDGET

#include <QWidget>
#include <QIcon>
#include <wwglobal.h>



class QwwConfigWidgetPrivate;
class Q_WW_EXPORT QwwConfigWidget : public QWidget, QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex STORED true)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int count READ count)
public:
    QwwConfigWidget(QWidget *parent = 0);
    void addGroup(QWidget*, const QIcon &i=QIcon(), const QString &n=QString());
    void insertGroup(int index, QWidget *, const QIcon &i=QIcon(), const QString &n=QString());
    int count() const;
    void removeGroup(QWidget *);
    void removeGroup(int);
    QWidget *group(int) const;
    int currentIndex() const;
    QWidget *currentGroup() const;
    QSize iconSize() const;
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void setCurrentIndex(int);
    void setCurrentGroup(const QString &);
    void setCurrentGroup(QWidget *w);
    void setGroupIcon(int, const QIcon &);
    void setGroupLabel(int, const QString &);
    void save();
    void apply();
    void discard();
    void setIconSize(const QSize &);
signals:
    void currentIndexChanged(int);
    void saving();
    void applying(int);
    void discarding(int);
private:
//     void setItemView(QAbstractItemView *);
    WW_DECLARE_PRIVATE(QwwConfigWidget);
    Q_DISABLE_COPY(QwwConfigWidget);
};

#endif
#endif
