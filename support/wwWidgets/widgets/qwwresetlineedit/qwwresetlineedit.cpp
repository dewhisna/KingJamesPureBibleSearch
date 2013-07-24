
#include "qwwresetlineedit.h"
#if !defined(WW_NO_RESETLINEEDIT)
#include <QToolButton>
#include <QEvent>

#include "qwwbuttonlineedit_p.h"

class QwwResetLineEditPrivate : public QwwButtonLineEditPrivate {
public:
    QwwResetLineEditPrivate(QwwResetLineEdit *c) : QwwButtonLineEditPrivate(c) {

    }
    QString stdText;
    void _q_textChanged(const QString &txt) {
        Q_Q(QwwResetLineEdit);
        if (!q->isModified()) {
            // setText called
            stdText = q->text();
            q->setButtonVisible(false);
        } else {
            q->setButtonVisible(stdText != txt);
        }
    }
    void retranslateUi() {
        Q_Q(QwwResetLineEdit);
        button->setToolTip(q->tr("Reset to default"));
    }

    WW_DECLARE_PUBLIC(QwwResetLineEdit);
};

/*!
 *  \class QwwResetLineEdit
 *  \brief The QwwResetLineEdit widget provides a line edit that has an embedded button
 *         for resetting the contents of the widget to its initial state.
 *  \inmodule wwWidgets
 */

/*!
 * Constructs a reset line edit with a given \a parent.
 */
QwwResetLineEdit::QwwResetLineEdit(QWidget * parent) : QwwButtonLineEdit(*new QwwResetLineEditPrivate(this), parent) {
    setButtonPosition(RightInside);
    setIcon(QPixmap(":/wrap.png"));
    setAutoRaise(true);
    setButtonVisible(false);
    setButtonFocusPolicy(Qt::NoFocus);
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(_q_textChanged(const QString&)));
    connect(this, SIGNAL(buttonClicked()), this, SLOT(resetText()));
    Q_D(QwwResetLineEdit);
    d->retranslateUi();

}


/*!
 * \brief   Resets text to its initial state
 * \sa      defaultText
 */
void QwwResetLineEdit::resetText() {
    Q_D(QwwResetLineEdit);
    setText(d->stdText);
}

/*!
 * \property    QwwResetLineEdit::defaultText
 * \brief       This property holds the default text of the widget
 *
 *              The "default" text is one set using QLineEdit::setText(). The text that is put
 *              into the widget by the user (using keyboard) is considered modified and will be
 *              reset to the last text set programatically using setText().
 *
 * \sa          resetText(), QLineEdit::setText()
 */
const QString & QwwResetLineEdit::defaultText() const {
    Q_D(const QwwResetLineEdit);
    return d->stdText;
}

/*!
 * \internal
 */
void QwwResetLineEdit::changeEvent(QEvent * e) {
    Q_D(QwwResetLineEdit);
    if (e->type()==QEvent::EnabledChange) {
        setButtonVisible(isEnabled() && d->stdText!=text());
    }
    if(e->type()==QEvent::LanguageChange) {
        d->retranslateUi();
    }
    QwwButtonLineEdit::changeEvent(e);
}

#include "moc_qwwresetlineedit.cpp"
#endif
