#ifndef __qwwtipdialog_h
#define __qwwtipdialog_h

#if defined(WW_NO_TEXTBROWSER) || defined(WW_NO_PUSHBUTTON)
#define WW_NO_TIPWIDGET
#endif

#ifndef WW_NO_TIPWIDGET

#include <QWidget>
#include <QStringList>

class QAbstractItemModel;
class QTextBrowser;
class QPushButton;
class QCheckBox;
#include <QPersistentModelIndex>
#include <QFrame>
#include <wwglobal.h>

class QwwTipWidgetPrivate;
/**
 *
 *
 *
 */
class Q_WW_EXPORT QwwTipWidget : public QWidget, public QwwPrivatable {
  Q_OBJECT
  Q_PROPERTY(QFrame::Shape canvasFrameShape
            READ canvasFrameShape WRITE setCanvasFrameShape)
  Q_PROPERTY(bool checkVisible
            READ checkIsVisible WRITE setCheckVisible)
  Q_PROPERTY(bool closeVisible
            READ closeIsVisible WRITE setCloseVisible)
  Q_PROPERTY(QStringList tips
            READ tips WRITE setTips)
  Q_PROPERTY(int currentTip
            READ currentTip WRITE setCurrentTip)
  Q_PROPERTY(bool tipsEnabled
            READ tipsEnabled WRITE setTipsEnabled)
public:
  QwwTipWidget(const QStringList &list, QWidget *parent=0);
  int currentTip() const;
  bool tipsEnabled() const;
  QWidget *headerWidget() const;
  void setHeaderWidget(QWidget *);
  const QPushButton *nextButton() const;
  const QPushButton *prevButton() const;
  const QPushButton *closeButton() const;
  const QTextBrowser *tipCanvas() const;
  bool checkIsVisible() const;
  bool closeIsVisible() const;
  QFrame::Shape canvasFrameShape() const;
  const QStringList tips() const;
public slots:
  void nextTip();
  void prevTip();
  void setTipsEnabled(bool);
  void setCheckVisible(bool);
  void setCheckHidden(bool);
  void setCloseVisible(bool);
  void setCloseHidden(bool);
  void setCanvasFrameShape(QFrame::Shape);
  void setTips(const QStringList &);
  void setCurrentTip(int);
signals:
  void tipChanged(int);
  void closed();
protected:
  void changeEvent(QEvent *);
private:
  WW_DECLARE_PRIVATE(QwwTipWidget);
  Q_PRIVATE_SLOT(d_func(), void showTip());
};


#endif
#endif
