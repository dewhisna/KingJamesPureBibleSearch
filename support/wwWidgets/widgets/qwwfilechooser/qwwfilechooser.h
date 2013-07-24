//
// C++ Interface: qwwfilechooser
//
// Description:
//
//
// Author: Witold Wysota <wysota@wysota.eu.org>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#if defined(WW_NO_BUTTONLINEEDIT) || defined(QT_NO_FILEDIALOG)
#define WW_NO_FILECHOOSER
#endif

#if !defined(WW_NO_FILECHOOSER)
#ifndef __QWWFILECHOOSER_H
#define __QWWFILECHOOSER_H

#include "qwwbuttonlineedit.h"
#include <QFileDialog>
#include <wwglobal.h>

#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
#include <QCompleter>
#endif

class QToolButton;
class QwwFileChooserPrivate;
class Q_WW_EXPORT QwwFileChooser : public QwwButtonLineEdit {
  Q_OBJECT
#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
  Q_ENUMS(QCompleter::CompletionMode)
  Q_PROPERTY(QCompleter::CompletionMode completionMode READ completionMode WRITE setCompletionMode)
#endif
  Q_PROPERTY(QFileDialog::FileMode fileMode READ fileMode WRITE setFileMode)
  Q_PROPERTY(QFileDialog::AcceptMode acceptMode READ acceptMode WRITE setAcceptMode)
  Q_PROPERTY(bool usesNativeFileDialog READ usesNativeFileDialog WRITE setUsesNativeFileDialog)
  Q_PROPERTY(QString filter READ filter WRITE setFilter)
public:
  QwwFileChooser(QWidget *parent=0);
  virtual ~QwwFileChooser(){}
#if QT_VERSION >= 0x040200 && !defined(QT_NO_COMPLETER)
  QCompleter::CompletionMode completionMode() const;
  void setCompletionMode(QCompleter::CompletionMode m);
#endif
  virtual void setModel(QAbstractItemModel *m);
  QFileDialog::FileMode fileMode() const;
  void setFileMode(QFileDialog::FileMode m);
  QFileDialog::AcceptMode acceptMode() const;
  void setAcceptMode(QFileDialog::AcceptMode m);
  bool usesNativeFileDialog() const;
  void setUsesNativeFileDialog(bool n);
  const QString &filter() const;
  void setFilter(const QString &f);
  QSize sizeHint() const;
  QSize minimumSizeHint() const;
public slots:
  void chooseFile();
//   void setText(const QString &t);
protected:
  void paintEvent(QPaintEvent *e);
private:
  Q_PRIVATE_SLOT(d_func(), void _q_textChanged(const QString &s));
  WW_DECLARE_PRIVATE(QwwFileChooser);
};

#endif
#endif
