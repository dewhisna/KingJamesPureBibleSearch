#ifndef __QWWCLEARLINEEDIT_H
#define __QWWCLEARLINEEDIT_H

#include "qwwbuttonlineedit.h"
#if !defined(WW_NO_BUTTONLINEEDIT) && !defined(WW_NO_CLEARLINEEDIT)


class QwwClearLineEditPrivate;
class Q_WW_EXPORT QwwClearLineEdit : public QwwButtonLineEdit {
    Q_OBJECT
public:
    QwwClearLineEdit(QWidget *parent=0);
protected:
    void paintEvent(QPaintEvent*);
    void changeEvent(QEvent *);
private slots:
    void setButtonVisible(bool vis);
private:
    Q_PRIVATE_SLOT(d_func(), void _q_textChanged(const QString &));
    Q_PRIVATE_SLOT(d_func(), void _q_clearRequested());
    WW_DECLARE_PRIVATE(QwwClearLineEdit);
};

#endif
#endif
