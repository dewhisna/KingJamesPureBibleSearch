#ifndef __QWWRESETLINEEDIT_H
#define __QWWRESETLINEEDIT_H

#if defined(WW_NO_BUTTONLINEEDIT)
#define WW_NO_RESETLINEEDIT
#endif

#if !defined(WW_NO_RESETLINEEDIT)


#include "qwwbuttonlineedit.h"

class QwwResetLineEditPrivate;
class Q_WW_EXPORT QwwResetLineEdit : public QwwButtonLineEdit {
    Q_OBJECT
    Q_PROPERTY(QString defaultText READ defaultText)
public:
    QwwResetLineEdit(QWidget *parent=0);
    const QString &defaultText() const;
public slots:
    void resetText();
protected:
    void changeEvent(QEvent *e);
private:
    Q_PRIVATE_SLOT(d_func(), void _q_textChanged(const QString &));
    WW_DECLARE_PRIVATE(QwwResetLineEdit);
};

#endif
#endif
