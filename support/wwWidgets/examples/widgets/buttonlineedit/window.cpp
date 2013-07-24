#include "window.h"
#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include <QwwClearLineEdit>
#include <QwwResetLineEdit>

ToUpperLineEdit::ToUpperLineEdit(QWidget *parent) : QwwButtonLineEdit(parent){
    connect(this, SIGNAL(buttonClicked()), this, SLOT(toUpper()));
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(updateButton(const QString &)));
    updateButton(QString());
}

void ToUpperLineEdit::toUpper(){
    QString txt = text();
    setText(txt.toUpper());
}

void ToUpperLineEdit::updateButton(const QString &txt){
    button()->setDisabled(txt.isEmpty());
}


Window::Window(QWidget *parent) : QWidget(parent)
{
    QGridLayout * l = new QGridLayout(this);
    QLabel *lab = new QLabel("QwwClearLineEdit:");
    l->addWidget(lab, 0, 0);
    QwwClearLineEdit *clearLE = new QwwClearLineEdit;
    clearLE->setText("Click to clear");
    l->addWidget(clearLE, 0, 1);
    lab = new QLabel("QwwResetLineEdit:");
    l->addWidget(lab, 1, 0);
    QwwResetLineEdit *resetLE = new QwwResetLineEdit;
    resetLE->setText("This is default text");
    l->addWidget(resetLE, 1, 1);
    ToUpperLineEdit *ble = new ToUpperLineEdit;
    ble->setButtonPosition(QwwButtonLineEdit::RightOutside);
    l->addWidget(ble, 2, 0, 1, 2);
}

