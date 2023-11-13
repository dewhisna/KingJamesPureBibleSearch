//
// C++ Interface: qwwtextspinbox
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWTEXTSPINBOX_H
#define QWWTEXTSPINBOX_H

#if defined(QT_NO_SPINBOX)
#define WW_NO_TEXTSPINBOX
#endif

#if !defined(WW_NO_TEXTSPINBOX)

#include <QSpinBox>
#include <wwglobal.h>
/* przerobic na model */

class QwwTextSpinBoxPrivate;
class Q_WW_EXPORT QwwTextSpinBox : public QSpinBox, public QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(QStringList strings READ strings WRITE setStrings)
public:
    QwwTextSpinBox(QWidget *parent = nullptr);
    QwwTextSpinBox(const QStringList &strings, QWidget *parent = nullptr);
    virtual QValidator::State validate ( QString & input, int & pos ) const override;
    const QStringList &strings() const;
public slots:
    void setStrings(const QStringList &s);
signals:
    void stringsChanged(const QStringList &);
protected:
    virtual QString textFromValue ( int value ) const override;
    virtual int valueFromText ( const QString & text ) const override;
private:
    WW_DECLARE_PRIVATE(QwwTextSpinBox);
};

#endif
#endif
