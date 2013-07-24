//
// C++ Interface: qwwloginbox
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QWWLOGINBOX_H
#define QWWLOGINBOX_H

#ifndef WW_NO_LOGINBOX

#include <QWidget>
#include <wwglobal.h>
// #include <QNetworkProxy>

class QwwLoginBoxPrivate;
class Q_WW_EXPORT QwwLoginBox : public QWidget, public QwwPrivatable {
    Q_OBJECT
    Q_FLAGS(Fields);
    Q_PROPERTY(Fields fields READ fields WRITE setFields)
    Q_PROPERTY(QString host READ host WRITE setHost)
    Q_PROPERTY(int port READ port WRITE setPort)
    Q_PROPERTY(QString user READ user)
    Q_PROPERTY(bool rememberPassword READ rememberPassword WRITE setRememberPassword)
    Q_PROPERTY(bool proxyEnabled READ proxyIsEnabled WRITE setProxyEnabled)
//     Q_PROPERTY(QString proxyHost READ proxyHost WRITE setProxyHost)
//     Q_PROPERTY(int proxyPort READ proxyPort WRITE setProxyPort)
//     Q_PROPERTY(QString proxyUser READ proxyUser WRITE setProxyUser)
//     Q_PROPERTY(QString proxyPassword READ proxyPassword WRITE setProxyPassword)

public:
    enum Field {
        NoFields=0x0,
        HostField=0x1,
        PortField=0x2,
        UserField=0x4,
        PasswordField=0x8,
        RepeatPasswordField=0x10,
        RememberPasswordField=0x20,
        ProxyField=0x40
    };
    QwwLoginBox(QWidget *parent = 0);
    Q_DECLARE_FLAGS(Fields, Field);
    Fields fields() const;
    void setFields(Fields f);
    QString host() const;
    int port() const;
    void setPort(int p);
    QString user() const;
    QString password() const;
    bool isRepeatCorrect() const;
    bool isProxyRepeatCorrect() const;
    bool proxyIsEnabled() const;
    bool rememberPassword() const;
    void setProxyEnabled(bool );
    void setRememberPassword(bool r);
public slots:
    void setHost(const QString &);
    void setUser(const QString &user, const QString &pass=QString::null);
private:
    WW_DECLARE_PRIVATE(QwwLoginBox);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QwwLoginBox::Fields)

#endif
#endif
