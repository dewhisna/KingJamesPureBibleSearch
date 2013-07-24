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
#include "qwwfilechooser.h"
#if !defined(WW_NO_FILECHOOSER)

#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
#include <QCompleter>
#endif
#include <QToolButton>

#include <QDirModel>
#include <QString>
#include <QShortcut>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>
#include "../qwwbuttonlineedit/qwwbuttonlineedit_p.h"
#include "wwglobal_p.h"

class QwwFileChooserPrivate : public QwwButtonLineEditPrivate {
public:
    QwwFileChooserPrivate(QwwFileChooser *c) : QwwButtonLineEditPrivate(c) {
        model = 0;
        native = true;
        fileMode = QFileDialog::AnyFile;
        acceptMode = QFileDialog::AcceptOpen;
    }
    void setModel(QAbstractItemModel *m) {
        Q_Q(QwwFileChooser);
        if (m==model)
            return;
        if (m) {
            model = m;
        } else {
            model = new QDirModel(q);
        }
#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
        completer->setModel(model);
#endif
    }
    QAbstractItemModel *model;
    QCompleter *completer;
//     QToolButton *tb;
    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;
    bool native;
    QString filter;
    QIcon icon;
    void _q_textChanged(const QString & s) {
      Q_Q(QwwFileChooser);
      QFileIconProvider prov;
      icon = prov.icon(QFileInfo(s));
      q->update();
    }

private:
    WW_DECLARE_PUBLIC(QwwFileChooser);
};

/*!
 *  \class QwwFileChooser
 *  \brief A widget for choosing a file or directory from the file system
 *  \mainclass
 *
 *  \image qwwfilechooser.png QwwFileChooser
 *         The widget is able to work in two modes - open (existing files or directories -- for loading files)
 *         and save (for saving files).
 */
/*!
 * \fn QwwFileChooser::~QwwFileChooser()
 * \internal
 */

/*!
 * Constructs a file chooser with a given \a parent.
 * 
 */
QwwFileChooser::QwwFileChooser(QWidget *parent) : QwwButtonLineEdit(*new QwwFileChooserPrivate(this), parent) {
    Q_D(QwwFileChooser);
#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
    d->completer = new QCompleter(this);
    setCompleter(d->completer);
    QStyleOption opt;
    opt.initFrom(this);
    int mar = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this);
    int siz = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
#if QT_VERSION >=0x040500
    setTextMargins(siz+2, 0, 0, 0);
#else
    setStyleSheet(QString("QwwFileChooser {padding-left: %1px;}").arg(mar+siz+2));
#endif

//     connect(this, SIGNAL(textEdited(const QString&)), d->completer, SLOT(setCompletionPrefix(const QString&)));
#endif
    setModel(new QDirModel(this));
    setButtonPosition(RightOutside);
    connect(this, SIGNAL(buttonClicked()), this, SLOT(chooseFile()));
    setAutoRaise(true);
    setAcceptMode(QFileDialog::AcceptOpen);

    QShortcut *sc = new QShortcut(QKeySequence("Ctrl+Space"), this);
    connect(sc, SIGNAL(activated()), d->completer, SLOT(complete()));
    connect(this, SIGNAL(textChanged(const QString&)), this, SLOT(_q_textChanged(const QString&)));
}

#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
/*!
 * \property QwwFileChooser::completionMode
 * \brief This property holds the current completion mode of the widget.
 */
QCompleter::CompletionMode QwwFileChooser::completionMode() const {
    Q_D(const QwwFileChooser);
    return d->completer->completionMode();
}

void QwwFileChooser::setCompletionMode(QCompleter::CompletionMode m) {
    Q_D(QwwFileChooser);
    d->completer->setCompletionMode(m);
}
#endif


/*!
 * \brief Sets the model used by the widget to \a m.
 * 
 */
void QwwFileChooser::setModel(QAbstractItemModel *m) {
    Q_D(QwwFileChooser);
    d->setModel(m);
}

/*!
 * \property QwwFileChooser::fileMode
 * \brief This property holds the current file mode of the widget.
 */
QFileDialog::FileMode QwwFileChooser::fileMode() const {
    Q_D(const QwwFileChooser);
    return d->fileMode;
}

void QwwFileChooser::setFileMode(QFileDialog::FileMode m) {
    Q_D(QwwFileChooser);
    d->fileMode = m;
}

/*!
 * \property QwwFileChooser::acceptMode
 * \brief This property holds the current accept mode of the widget.
 */
QFileDialog::AcceptMode QwwFileChooser::acceptMode() const {
    Q_D(const QwwFileChooser);
    return d->acceptMode;
}

void QwwFileChooser::setAcceptMode(QFileDialog::AcceptMode m) {
    Q_D(QwwFileChooser);

    d->acceptMode = m;
    if (d->acceptMode==QFileDialog::AcceptOpen) {
        setIcon(wwWidgets::icon("document-open", QPixmap(":/fileopenicon")));
    } else {
        setIcon(wwWidgets::icon("document-save", QPixmap(":/filesaveicon")));
    }
}

/*!
 * \property QwwFileChooser::usesNativeFileDialog
 * \brief This property holds information whether the widget uses a native file dialog for choosing files (or directories).
 */
bool QwwFileChooser::usesNativeFileDialog() const {
    Q_D(const QwwFileChooser);
    return d->native;
}

void QwwFileChooser::setUsesNativeFileDialog(bool n) {
    Q_D(QwwFileChooser);
    d->native = n;
}

/*!
 * \property QwwFileChooser::filter
 * \brief This property holds information about the currently chosen filter.
 */
const QString & QwwFileChooser::filter() const {
    Q_D(const QwwFileChooser);
    return d->filter;
}

void QwwFileChooser::setFilter(const QString &f) {
    Q_D(QwwFileChooser);
    d->filter = f;
}

/*!
 * \brief This slot opens a file dialog for choosing one or more entries from the file system
 *        according to currently active mode.
 *
 * \sa acceptMode
 */
void QwwFileChooser::chooseFile() {
    if (isReadOnly())
        return;
    Q_D(QwwFileChooser);
    QString path;
    bool ok=true;
    if (d->native) {
        switch (d->fileMode) {
        case QFileDialog::AnyFile:
            if (d->acceptMode==QFileDialog::AcceptOpen)
                path = QFileDialog::getOpenFileName(this, tr("Choose file"), text(), filter());
            else
                path = QFileDialog::getSaveFileName(this, tr("Choose file"), text(), filter());
            break;
        case QFileDialog::ExistingFile:
            if (d->acceptMode==QFileDialog::AcceptOpen)
                path = QFileDialog::getOpenFileName(this, tr("Choose file"), text(), filter());
            break;
        case QFileDialog::Directory:
            path = QFileDialog::getExistingDirectory(this, tr("Choose directory"), text(), 0);
            break;
        case QFileDialog::DirectoryOnly:
            path = QFileDialog::getExistingDirectory(this, tr("Choose directory"), text());
            break;
        case QFileDialog::ExistingFiles:
            QStringList s = QFileDialog::getOpenFileNames(this, tr("Choose files"), text(), filter());
            path = s.isEmpty() ? QString::null : s.at(0);
            break;
        }
    } else {
        QFileDialog dlg(this);
        dlg.setAcceptMode(d->acceptMode);
        dlg.setFileMode(d->fileMode);
        dlg.setDirectory(text());
        dlg.setFilter(filter());
        if (dlg.exec()) {
            QStringList s = dlg.selectedFiles();
            path = s.isEmpty() ? QString::null : s.at(0);
        }
    }
    if (ok && !path.isNull()) {
        setText(path);
#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
        d->completer->setCompletionPrefix(path);
#endif
    }
}

/*!
 * \internal
 */
QSize QwwFileChooser::sizeHint() const {
    Q_D(const QwwFileChooser);
    return QLineEdit::sizeHint() + QSize(button()->sizeHint().width(), 0);
}

/*!
 * \internal
 */
QSize QwwFileChooser::minimumSizeHint() const {
    Q_D(const QwwFileChooser);
    return QLineEdit::minimumSizeHint() + QSize(button()->sizeHint().width(), 0);
}

/*!
 * \internal
 */
void QwwFileChooser::paintEvent(QPaintEvent * e) {
    Q_D(QwwFileChooser);
    QwwButtonLineEdit::paintEvent(e);
    QPainter p(this);
    QStyleOption opt;
    opt.initFrom(this);
    int mar = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, &opt, this);
    int siz = style()->pixelMetric(QStyle::PM_SmallIconSize, &opt, this);
    d->icon.paint(&p, QRect(contentsRect().left()+mar+1, mar+1, siz, siz), Qt::AlignCenter, opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled);
}

// void QwwFileChooser::setText(const QString & t) {
// #if QT_VERSION >=0x040200
//     Q_D(QwwFileChooser);
//     d->completer->setCompletionPrefix(t);
// #endif
//     QLineEdit::setText(t);
// }

#include "moc_qwwfilechooser.cpp"
#endif
