//
// C++ Interface: qwwcolorcombobox
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWCOLORCOMBOBOX_H
#define QWWCOLORCOMBOBOX_H

#if defined(QT_NO_COMBOBOX)
#define WW_NO_COLORCOMBOBOX
#endif

#if !defined(WW_NO_COLORCOMBOBOX)

#include <QComboBox>
#include <QModelIndex>
#include <wwglobal.h>
class QPushButton;
class QwwColorComboBoxPrivate;

/**
 *
 *
 *
 */
class Q_WW_EXPORT QwwColorComboBox : public QComboBox, QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(bool colorDialogEnabled READ isColorDialogEnabled WRITE setColorDialogEnabled)
    Q_PROPERTY(int colorCount READ colorCount)
    Q_PROPERTY(QColor currentColor READ currentColor)
    Q_PROPERTY(QStringList colors READ colors WRITE setColors)
public:
    QwwColorComboBox(QWidget *parent = 0);
    virtual ~QwwColorComboBox();
    void addColor ( const QColor & color, const QString & name );
    QColor color ( int index ) const;
    QStringList colors() const;
    void setColors(const QStringList &);
    int colorCount () const;
    QColor currentColor () const;
    void insertColor ( int index, const QColor & color, const QString & name );
    bool isColorDialogEnabled () const;
    void setColorDialogEnabled ( bool enabled = true );
    void setStandardColors();
//     void addItem ( const QString & text, const QVariant & userData = QVariant() ){}
//     void addItem ( const QIcon & icon, const QString & text, const QVariant & userData = QVariant() ){}
//     void addItems ( const QStringList & texts ){}
    virtual void showPopup() override;
public slots:
    void setCurrentColor ( const QColor & color );

signals:
    void activated(const QColor &);

protected:
    virtual bool eventFilter(QObject *o, QEvent *e) override;
    virtual void paintEvent(QPaintEvent *e) override;
    virtual void dragEnterEvent(QDragEnterEvent *) override;
    virtual void dropEvent(QDropEvent *) override;
private:
    Q_PRIVATE_SLOT(d_func(), void _q_activated(int));
    Q_PRIVATE_SLOT(d_func(), void _q_popupDialog());
    WW_DECLARE_PRIVATE(QwwColorComboBox);
};

#endif
#endif
