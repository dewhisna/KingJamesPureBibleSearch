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


#ifndef WW_NO_LOGINBOX
#include "qwwloginbox.h"
#include "ui_loginbox.h"
#include "wwglobal_p.h"

class QwwLoginBoxPrivate: public QwwPrivate {
public:
    QwwLoginBoxPrivate(QwwLoginBox *q) : QwwPrivate(q) {}
    Ui::LoginBox ui;
    QwwLoginBox::Fields fields;
    void updateFields() {
		ui.labelHost->setVisible(fields & QwwLoginBox::HostField);
		ui.host->setVisible(fields & QwwLoginBox::HostField);
		ui.labelPort->setVisible(fields & QwwLoginBox::HostField && fields & QwwLoginBox::PortField);
		ui.port->setVisible(fields & QwwLoginBox::HostField && fields & QwwLoginBox::PortField);
		ui.labelUser->setVisible(fields & QwwLoginBox::UserField);
		ui.user->setVisible(fields & QwwLoginBox::UserField);
		ui.labelPassword->setVisible(fields & QwwLoginBox::PasswordField);
		ui.password->setVisible(fields & QwwLoginBox::PasswordField);
		ui.labelRepeat->setVisible(fields & QwwLoginBox::PasswordField && fields & QwwLoginBox::RepeatPasswordField);
		ui.repeat->setVisible(fields & QwwLoginBox::PasswordField && fields & QwwLoginBox::RepeatPasswordField);

		ui.remember->setVisible(fields & QwwLoginBox::PasswordField && fields & QwwLoginBox::RememberPasswordField);

		ui.proxy->setVisible(fields & QwwLoginBox::ProxyField);
		ui.proxyRepeat->setVisible(fields & QwwLoginBox::ProxyField && fields & QwwLoginBox::RepeatPasswordField);
		ui.labelProxyRepeat->setVisible(fields & QwwLoginBox::ProxyField && fields & QwwLoginBox::RepeatPasswordField);
    }
private:
    WW_DECLARE_PUBLIC(QwwLoginBox);
};


/*!
 *  \class QwwLoginBox
 *  \brief The QwwLoginBox class provides a login form with configurable set of fields
 *         for logging into different types of services.
 *  \inmodule wwWidgets
 *
 */
/*!
 * \enum QwwLoginBox::Field
 *
 * \value NoFields			No fields are visible
 * \value HostField			Host name
 * \value PortField			Port number
 * \value UserField			User (login)
 * \value PasswordField			Password
 * \value RepeatPasswordField		Password confirmation
 * \value RememberPasswordField		"Remember password" checkbox
 * \value ProxyField			Network proxy group
 *
 *
 */

/*!
 * Constructs a login box with a given \a parent.
 */
QwwLoginBox::QwwLoginBox(QWidget * parent): QWidget(parent), QwwPrivatable(new QwwLoginBoxPrivate(this)) {
    Q_D(QwwLoginBox);
    d->ui.setupUi(this);
    d->fields = HostField|PortField|UserField|PasswordField;
    d->updateFields();
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void QwwLoginBox::setFields(QwwLoginBox::Fields f) {
    Q_D(QwwLoginBox);
    d->fields = f;
    d->updateFields();
    updateGeometry();
    update();
}

/*!
 * \property QwwLoginBox::fields
 * \brief This property holds the set of fields visible in the box.
 */
QwwLoginBox::Fields QwwLoginBox::fields() const {
    Q_D(const QwwLoginBox);
    return d->fields;
}

/*!
 * \property QwwLoginBox::host
 * \brief This property holds the host set in the box.
 */
QString QwwLoginBox::host() const {
    Q_D(const QwwLoginBox);
    return d->ui.host->text();
}

/*!
 * @brief Sets the host.
 */

void QwwLoginBox::setHost(const QString &t) {
    Q_D(QwwLoginBox);
    d->ui.host->setText(t);
}

/*!
 * \property QwwLoginBox::port
 * \brief This property holds the port set in the box.
 */
int QwwLoginBox::port() const {
    Q_D(const QwwLoginBox);
    return d->ui.port->value();
}

/**
 * @brief Sets the port.
 */
void QwwLoginBox::setPort(int p) {
    Q_D(QwwLoginBox);
    d->ui.port->setValue(p);
}

/*!
 * \brief Sets the \a user and \a password set in the box.
 */
void QwwLoginBox::setUser(const QString & user, const QString & password) {
    Q_D(QwwLoginBox);
    d->ui.user->setText(user);
    d->ui.password->setText(password);
}

/*!
 * \property QwwLoginBox::user
 * \brief This property holds the user set in the box.
 */
QString QwwLoginBox::user() const {
    Q_D(const QwwLoginBox);
    return d->ui.user->text();
}

/*!
 * \brief Returns the password set in the box.
 */
QString QwwLoginBox::password() const {
    Q_D(const QwwLoginBox);
    return d->ui.password->text();
}

/*!
 * \brief Returns whether both password fields contain the same data.
 */
bool QwwLoginBox::isRepeatCorrect() const
{
    Q_D(const QwwLoginBox);
    return !(d->fields & RepeatPasswordField) || d->ui.password->text()==d->ui.repeat->text();
}

/*!
 * \property QwwLoginBox::proxyEnabled
 * \brief The property holds whether the proxy field is enabled.
 */

bool QwwLoginBox::proxyIsEnabled() const
{
    Q_D(const QwwLoginBox);
    return d->ui.proxy->isChecked();
}

/*!
 * @brief Enables or disables the use of a network proxy field.
 */
void QwwLoginBox::setProxyEnabled(bool v)
{
    Q_D(const QwwLoginBox);
    d->ui.proxy->setChecked(v);
}

/*!
 * \brief Returns whether both password fields in the proxy section contain the same data.
 */
bool QwwLoginBox::isProxyRepeatCorrect() const
{
    Q_D(const QwwLoginBox);
    return !(d->fields & RepeatPasswordField) || d->ui.proxyPassword->text()==d->ui.proxyRepeat->text();
}

/*!
 * \property QwwLoginBox::rememberPassword
 * \brief The property holds whether the "remember password" checkbox is visible and checked
 */
bool QwwLoginBox::rememberPassword() const
{
    Q_D(const QwwLoginBox);
    return (d->fields & PasswordField && d->fields & RememberPasswordField) && d->ui.remember->isChecked();
}

/*!
 * \brief Checks or unchecks the "remember password" field depending on the value of \a r.
 */
void QwwLoginBox::setRememberPassword(bool r)
{
   Q_D(QwwLoginBox);
   d->ui.remember->setChecked(r);
}

#endif
