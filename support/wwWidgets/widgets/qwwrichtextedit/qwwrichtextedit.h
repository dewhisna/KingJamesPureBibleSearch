#ifndef __QWWRICHTEXTEDIT_H
#define __QWWRICHTEXTEDIT_H
#ifndef WW_NO_RICHTEXTEDIT
#include <QTextEdit>
#include <QTextDocument>
#include <QTextFormat>
#include <wwglobal.h>
#include <QFlags>
#include <QWidgetAction>

class QFontComboBox;
class QToolBar;
class QComboBox;
#ifndef WW_NO_COLORBUTTON
class QwwColorButton;
#endif

/* dorobic flagi */
class Q_WW_EXPORT QwwRichTextEdit : public QTextEdit {
  Q_OBJECT
  Q_FLAGS(Options);
  Q_PROPERTY(Options options READ options WRITE setOptions)
public:
  QwwRichTextEdit(QWidget *parent=0);
    enum Option { NoOptions = 0x0, Style = 0x01, Alignment = 0x02, FontFamily = 0x04, FontSize = 0x08, Color = 0x10 };
  Q_DECLARE_FLAGS(Options, Option);
  enum Action {
    BoldAction, ItalicAction, UnderlineAction, LeftAlignAction, RightAlignAction, AlignCenterAction, JustifyAction, ColorAction, FontFamilyAction, FontSizeAction
  };
  Options options() const;
  QAction *toolBarAction(Action a) const;
public slots:
  void changeAlignment(QAction *a);
  void setBold(bool v);
  void setItalic(bool v);
  void setUnderline(bool v);
  void setFont_h();
  void setFont(const QFont &f);
  void setList(bool v);
  void setColor(const QColor &);
void setOptions(Options opt);
  void updateCurrentCharFormat(const QTextCharFormat &fmt);
  void updateCurrentBlockFormat();
protected:
  bool event(QEvent *e);
  QAction *m_actions[10];
  QAction *cB;
  QToolBar *tb;
  QAction *ac, *ar, *al, *aj;
  QAction *li;
  QFontComboBox *fcb;
  QComboBox *fsp;
  QTextList *currentList;
  Options m_options;
  void contextMenuEvent ( QContextMenuEvent * event );
#ifndef WW_NO_COLORBUTTON
  QwwColorButton *colorButton;
#endif
  void resizeEvent(QResizeEvent *re);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QwwRichTextEdit::Options)
#endif
#endif
