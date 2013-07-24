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

#ifndef WW_NO_NUMPAD

#include "qwwnumpad.h"
#include <QLayout>
#include <QButtonGroup>
#include <QToolButton>
#include "wwglobal_p.h"

class QwwNumPadPrivate : public QwwPrivate {
public:
    QwwNumPadPrivate(QwwNumPad *pub) : QwwPrivate(pub){}
    void _q_clicked(int);
private:
    WW_DECLARE_PUBLIC(QwwNumPad);
};

void QwwNumPadPrivate::_q_clicked(int id) {
    Q_Q(QwwNumPad);
    if (id>=0 && id<10) {
        emit q->numberClicked(id);
        emit q->keyClicked(QString::number(id));
        return;
    }
    if (id==10) {
        emit q->asteriskClicked();
        emit q->keyClicked("*");
        return;
    }
    if (id==11) {
        emit q->hashClicked();
        emit q->keyClicked("#");
        return;
    }
}

/*!
 *  \class  QwwNumPad
 *  \brief  The QwwNumPad widget provides a numeric keypad widget.
 *  \inmodule wwWidgets
 *  
 *  \image qwwnumpad.png QwwNumPad
 */
/*!
 * \fn      void QwwNumPad::numberClicked(int key);
 * \brief   numeric \a key clicked
 */
/*!
 * \fn      void QwwNumPad::keyClicked(QString key);
 * \brief   text of the \a key clicked
 */
/*!
 * \fn      void QwwNumPad::hashClicked();
 * \brief   '#' key clicked
 */
/*!
 * \fn      void QwwNumPad::asteriskClicked();
 * \brief   '*' key clicked
 *
 */

/*!
 * Constructs a numpad widget with a given \a parent.
 */
QwwNumPad::QwwNumPad(QWidget *parent) : QWidget(parent), QwwPrivatable(new QwwNumPadPrivate(this)) {
    QGridLayout *l = new QGridLayout(this);
    l->setSpacing(2);
    l->setMargin(2);
    QButtonGroup *group = new QButtonGroup(this);
    for (int i=0;i<9;i++) {
        QToolButton *b = new QToolButton;
        b->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        b->setObjectName(QString("b%1").arg(i+1));
        b->setText(QString::number(i+1));
        l->addWidget(b, i/3, i%3);
        group->addButton(b, i+1);
    }
    QToolButton *b = new QToolButton;
    b->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    b->setObjectName("b0");
    b->setText("0");
    l->addWidget(b, 3, 1);
    group->addButton(b, 0);

    b = new QToolButton;
    b->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    b->setObjectName("bAster");
    b->setText("*");
    l->addWidget(b, 3, 0);
    group->addButton(b, 10);

    b = new QToolButton;
    b->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    b->setText("#");
    b->setObjectName("bHash");
    l->addWidget(b, 3, 2);
    group->addButton(b, 11);
    connect(group, SIGNAL(buttonClicked(int)), SLOT(_q_clicked(int)));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}



/*!
 * \brief Performs an animated click.
 *
 * The \a button is pressed immediately, and released \a msec milliseconds later
 * (the default is 100 ms).
 *
 * Calling this function again on the same button before it was released will reset
 * the release timer.
 * All signals associated with a click are emitted as appropriate.
 * This function does nothing if the button is disabled.
 */
void QwwNumPad::animateClick(const QString & button, int msec) {
    if (button.size()!=1) return;
    char key = button[0].toAscii();
    QList<QToolButton*> items;
    QString name;
    switch (key) {
    case '#':
        name = "bHash";
        break;
    case '*':
        name = "bAster";
        break;
    default:
        name = QString("b%1").arg(key);
    }
    QToolButton *buttonPtr = qFindChild<QToolButton*>(this, name);
    if (!buttonPtr) return;
    buttonPtr->animateClick(msec);
}

#include "moc_qwwnumpad.cpp"

#endif
