#ifndef WINDOW_H
#define WINDOW_H

#include <QwwButtonLineEdit>

class ToUpperLineEdit : public QwwButtonLineEdit {
    Q_OBJECT
public:
    ToUpperLineEdit(QWidget *parent = 0);
public slots:
    void toUpper();
private slots:
    void updateButton(const QString &txt);
};

class Window : public QWidget
{
    Q_OBJECT
public:
    Window(QWidget *parent=0);
};

#endif // WINDOW_H
