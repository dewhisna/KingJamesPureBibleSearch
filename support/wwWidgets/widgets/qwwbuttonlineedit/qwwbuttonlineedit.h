//
// C++ Interface: QwwButtonLineEdit
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#if defined(QT_NO_TOOLBUTTON) || defined(QT_NO_LINEEDIT)
#define WW_NO_BUTTONLINEEDIT
#endif

#if !defined(WW_NO_BUTTONLINEEDIT)
#ifndef __QWWBUTTONLINEEDIT_H
#define __QWWBUTTONLINEEDIT_H

#include <QLineEdit>
#include <wwglobal.h>
#include <QIcon>

class QResizeEvent;
class QActionEvent;
class QToolButton;

class QwwButtonLineEditPrivate;

class Q_WW_EXPORT QwwButtonLineEdit : public QLineEdit, public QwwPrivatable {
  Q_OBJECT
  Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
  Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
  Q_ENUMS(Position)
  Q_PROPERTY(Position buttonPosition READ buttonPosition WRITE setButtonPosition)
  Q_PROPERTY(bool buttonVisible READ buttonIsVisible WRITE setButtonVisible)
  Q_PROPERTY(QString regExp READ regExp WRITE setRegExp)
  Q_PROPERTY(Qt::FocusPolicy buttonFocusPolicy READ buttonFocusPolicy WRITE setButtonFocusPolicy)
#if QT_VERSION < 0x040700
  Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
#endif
public:
    enum Position { None = 0, RightOutside = 1, RightInside = 2, LeftOutside = 3, LeftInside = 4 };
    QwwButtonLineEdit(QWidget *parent=0);
    QwwButtonLineEdit(const QString &contents, QWidget *parent=0);
    QIcon icon() const;
    bool autoRaise() const;
    Position buttonPosition() const;
    void setButtonPosition(Position pos);
    bool buttonIsVisible() const;
    QString regExp() const;
    Qt::FocusPolicy buttonFocusPolicy() const;
    void setButtonFocusPolicy(Qt::FocusPolicy);
#if QT_VERSION < 0x040700
    QString placeholderText() const;
#endif
public slots:
    void setRegExp(const QString &);
    void setRegExp(const QRegExp &);
    void setIcon(const QIcon &);
    void setAutoRaise(bool);
    void setButtonVisible(bool);
#if QT_VERSION < 0x040700
    void setPlaceholderText(const QString &);
#endif
signals:
    void buttonClicked();
    void validatorChanged(const QString&);
protected:
    QwwButtonLineEdit(QwwButtonLineEditPrivate &priv, QWidget *parent);
#if QT_VERSION < 0x040700
    void paintEvent(QPaintEvent *);
#endif
    void resizeEvent(QResizeEvent *ev);
    void actionEvent(QActionEvent *ev);
    QToolButton *button() const;
private:
  WW_DECLARE_PRIVATE(QwwButtonLineEdit);
};

#endif
#endif
