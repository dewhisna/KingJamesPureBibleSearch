#include "qwwclearlineedit.h"
#if !defined(WW_NO_BUTTONLINEEDIT) && !defined(WW_NO_CLEARLINEEDIT)
#include <QPaintEvent>
#include <QToolButton>
#include <QEvent>
#include "qwwbuttonlineedit_p.h"

/*!
  \class QwwClearLineEditPrivate
  \internal
 */
class QwwClearLineEditPrivate : public QwwButtonLineEditPrivate {
public:
    QwwClearLineEditPrivate(QwwClearLineEdit *c) : QwwButtonLineEditPrivate(c) {
        vis = false;
    }
    bool vis;
    void _q_textChanged(const QString &s) {
        Q_Q(QwwClearLineEdit);
        if(s.isEmpty())
            q->setButtonPosition(QwwButtonLineEdit::None);
        else
            q->setButtonPosition(QwwButtonLineEdit::RightInside);
        //q->setButtonVisible(!s.isEmpty());
    }
    void _q_clearRequested() {
        Q_Q(QwwClearLineEdit);
        if (!q->isReadOnly())
            q->clear();
    }
    void retranslateUi(){
        Q_Q(QwwClearLineEdit);
        q->button()->setToolTip(q->tr("Clear"));
    }

    WW_DECLARE_PUBLIC(QwwClearLineEdit);
};


/*!
 *  \class QwwClearLineEdit
 *  \brief A line edit widget that has an embedded button for clearing its contents.
 *  \mainclass
 *
 *
 */

/*!
 * \brief Constructs a clear line edit with a given \a parent
 *
 */
QwwClearLineEdit::QwwClearLineEdit(QWidget * parent) : QwwButtonLineEdit(*new QwwClearLineEditPrivate(this), parent) {
    //setButtonPosition(RightInside);
    setButtonPosition(None);
    setIcon(QPixmap(":/closetab.png"));
    setAutoRaise(true);
    //setButtonVisible(false);
    setButtonFocusPolicy(Qt::NoFocus);
    connect(this, SIGNAL(buttonClicked()), this, SLOT(_q_clearRequested()));
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(_q_textChanged(const QString &)));
    Q_D(QwwClearLineEdit);
    d->retranslateUi();
}




/*!
 \internal
 */
void QwwClearLineEdit::paintEvent(QPaintEvent *ev) {
    Q_D(QwwClearLineEdit);
    // hack to make sure the button is disabled when the widget is made read only
    if (!d->vis && isReadOnly()) {
        // widget became read only

        button()->setVisible(false);
        d->vis = true;
    } else if (d->vis && !isReadOnly()) {
        // widget stopped being read only

        button()->setVisible(!text().isEmpty());
        d->vis = false;
    }
    QwwButtonLineEdit::paintEvent(ev);
}

/*!
 * \reimp
 */
void QwwClearLineEdit::setButtonVisible(bool v) {
    if (v && isReadOnly())
        return;
    QwwButtonLineEdit::setButtonVisible(v);
}

/*!
 \internal
 */
void QwwClearLineEdit::changeEvent(QEvent *event) {
    if (event->type()==QEvent::EnabledChange) {
        setButtonVisible(isEnabled() && !text().isEmpty());
    }
    if(event->type()==QEvent::LanguageChange) {
        Q_D(QwwClearLineEdit);
        d->retranslateUi();
    }
    QwwButtonLineEdit::changeEvent(event);
}

#include "moc_qwwclearlineedit.cpp"
#endif
