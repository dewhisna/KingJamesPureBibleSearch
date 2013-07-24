//
// C++ Interface: qwwrichtextbutton
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef QWWRICHTEXTBUTTON_H
#define QWWRICHTEXTBUTTON_H

#ifndef WW_NO_RICHTEXTBUTTON

#include <QAbstractButton>
#include <wwglobal.h>
class QTextDocument;
class QStyleOptionButton;

class QwwRichTextButtonPrivate;

class Q_WW_EXPORT QwwRichTextButton : public QAbstractButton, public QwwPrivatable {
    Q_OBJECT
    Q_PROPERTY(QString text READ html WRITE setHtml)
    Q_PROPERTY(QString html READ html WRITE setHtml USER true)
public:
    QwwRichTextButton(QWidget *parent=0);
    QwwRichTextButton(QTextDocument *d, QWidget *parent=0);
    void setDocument(QTextDocument *doc);
    QTextDocument *document() const;
    ~QwwRichTextButton();
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    inline QString text() const { return html(); }
    QString html() const;
    int heightForWidth ( int w ) const;
public slots:
    void setHtml(const QString &);
    void setText(const QString &t);
protected:
    void paintEvent(QPaintEvent*);
private:
    WW_DECLARE_PRIVATE(QwwRichTextButton);
    Q_PRIVATE_SLOT(d_func(), void _q_documentSizeChanged(const QSizeF &));
    Q_PRIVATE_SLOT(d_func(), void _q_documentDestroyed());
};
#endif
#endif
