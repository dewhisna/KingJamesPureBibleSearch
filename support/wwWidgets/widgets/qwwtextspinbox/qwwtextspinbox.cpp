//
// C++ Implementation: QwwTextSpinBox
//
// Description:
//
//
// Author: Witold Wysota <wwwidgets@wysota.eu.org>, (C) 2008-2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "qwwtextspinbox.h"
#if !defined(WW_NO_TEXTSPINBOX)
#include "wwglobal_p.h"

class QwwTextSpinBoxPrivate : public QwwPrivate {
public:
    QwwTextSpinBoxPrivate(QwwTextSpinBox *pub) : QwwPrivate(pub) {}
    QStringList strings;
private:
    WW_DECLARE_PUBLIC(QwwTextSpinBox);
};

/*!
 * \class QwwTextSpinBox
 * \inmodule wwWidgets
 * \brief The QwwTextSpinBox widget provides a spin box with configurable set of texts
 *
 */
/*!
 * \fn      void QwwTextSpinBox::stringsChanged(const QStringList &strings)
 * \brief   This signal is emitted when the list of strings associated with the widget changes to \a strings.
 */

/*!
 * Constrcuts a text spin box with a given \a parent.
 */
QwwTextSpinBox::QwwTextSpinBox(QWidget * parent) : QSpinBox(parent), QwwPrivatable(new QwwTextSpinBoxPrivate(this)) {
    Q_D(QwwTextSpinBox);
    setRange(0,d->strings.size());
}

/*!
 * Constructs a text spin box with a given \a parent and a list of \a strings as its contents
 */
QwwTextSpinBox::QwwTextSpinBox(const QStringList & strings, QWidget * parent) : QSpinBox(parent), QwwPrivatable(new QwwTextSpinBoxPrivate(this)) {
    Q_D(QwwTextSpinBox);
    d->strings = strings;
    setRange(0,d->strings.size());
}

/*!
 * \brief       Sets \a strings as new list of strings for the widget.
 */
void QwwTextSpinBox::setStrings(const QStringList & strings) {
    Q_D(QwwTextSpinBox);
    if (strings==d->strings) return;
    d->strings = strings;
    setRange(0, d->strings.size()-1);
    emit stringsChanged(strings);
    interpretText();
}

/*!
 * \property    QwwTextSpinBox::strings
 * \brief       This property holds a list of strings for the widget.
 */
const QStringList & QwwTextSpinBox::strings() const {
    Q_D(const QwwTextSpinBox);
    return d->strings;
}

/*!
 * \internal
 */
QString QwwTextSpinBox::textFromValue(int value) const {
    Q_D(const QwwTextSpinBox);
    if (d->strings.size() <= value)
        return "";
    return d->strings.at(value);
}

/*!
 * \internal
 */
int QwwTextSpinBox::valueFromText(const QString & text) const {
    Q_D(const QwwTextSpinBox);
    return d->strings.indexOf(text);
}

/*!
 * \internal
 */
QValidator::State QwwTextSpinBox::validate(QString & input, int & pos) const {
    foreach(QString str, strings()) {
        if (str==input) return QValidator::Acceptable;
        if (str.contains(input)) return QValidator::Intermediate;
    }
    return QValidator::Invalid;
}
#endif
