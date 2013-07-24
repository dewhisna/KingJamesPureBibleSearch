#ifndef WW_NO_RICHTEXTEDIT
#include "qwwrichtextedit.h"
#include <QKeyEvent>
#include <QToolBar>
#include <QResizeEvent>
#include <QAction>
#include <QFontComboBox>
#include <QTextList>
#include <QScrollBar>
#include <QMenu>
#include "wwglobal_p.h"

#ifndef WW_NO_COLORBUTTON
#include "qwwcolorbutton.h"
#endif

/*!
 *  \class QwwRichTextEdit
 *  \brief The QwwRichTextEdit widget provides a drop-in replacement for QTextEdit
 *         that contains a toolbar for editing rich text data.
 *  \inmodule wwWidgets
 *  \mainclass
 *
 *
 *
 */
/*!
 * \enum QwwRichTextEdit::Option
 *
 * \value NoOptions     No options are set
 * \value Style         Text style
 * \value Alignment     Item alignment
 * \value FontFamily    Font family
 * \value FontSize      Font size
 * \value Color         Text color
 */
/*!
 * \enum QwwRichTextEdit::Action
 *
 * \value BoldAction        Makes the text bold
 * \value ItalicAction      Makes the text italic
 * \value UnderlineAction   Makes the text underlined
 * \value LeftAlignAction   Makes the text left aligned
 * \value RightAlignAction  Makes the text right aligned
 * \value AlignCenterAction Makes the text aligned to the centre
 * \value JustifyAction     Makes the text justified
 * \value ColorAction       Changes the color of the text
 * \value FontFamilyAction  Changes the font family of the text
 * \value FontSizeAction    Changes the font size of the text
 */
/*!
 * \property QwwRichTextEdit::options
 * \brief This property holds the active set of options
 */

/*!
 *  Constructs a rich text edit with a given \a parent
 */
QwwRichTextEdit::QwwRichTextEdit(QWidget *parent) : QTextEdit(parent){
    currentList = 0;
    m_options = Style | Alignment | FontFamily | FontSize | Color;
    tb = new QToolBar(this);
    tb->setIconSize(QSize(24,24));

#ifndef WW_NO_COLORBUTTON
    colorButton = new QwwColorButton(this);
    colorButton->setShowName(false);
    colorButton->setFlat(true);
    colorButton->setFocusPolicy(Qt::NoFocus);
    m_actions[ColorAction] = tb->addWidget(colorButton);
    connect(colorButton, SIGNAL(colorPicked(QColor)), this, SLOT(setColor(QColor)));
#endif
    fcb = new QFontComboBox(this);
    m_actions[FontFamilyAction] = tb->addWidget(fcb);
    connect(fcb, SIGNAL(activated(int)), this, SLOT(setFont_h()));
    fsp = new QComboBox(this);
    m_actions[FontSizeAction] = tb->addWidget(fsp);
    connect(fsp, SIGNAL(activated(const QString &)), this, SLOT(setFont_h()));
    foreach(int s, QFontDatabase::standardSizes())
    fsp->addItem(QString::number(s));

    m_actions[BoldAction] = new QAction(wwWidgets::icon("format-text-bold", QIcon(QPixmap(":/trolltech/formeditor/images/win/textbold.png"))), "Bold", this);
    m_actions[BoldAction]->setCheckable(true);
    tb->addAction(m_actions[BoldAction]);
    connect(m_actions[BoldAction], SIGNAL(triggered(bool)), this, SLOT(setBold(bool)));
    m_actions[ItalicAction] = new QAction(wwWidgets::icon("format-text-italic", QPixmap(":/trolltech/formeditor/images/win/textitalic.png")), "Italic", this);
    m_actions[ItalicAction]->setCheckable(true);
    tb->addAction(m_actions[ItalicAction]);

    connect(m_actions[ItalicAction], SIGNAL(triggered(bool)), this, SLOT(setItalic(bool)));
    m_actions[UnderlineAction] = new QAction(wwWidgets::icon("format-text-underline", QPixmap(":/trolltech/formeditor/images/win/textunder.png")), "Underline", this);
    m_actions[UnderlineAction]->setCheckable(true);
    tb->addAction(m_actions[UnderlineAction]);

    cB = tb->addSeparator();
    connect(m_actions[UnderlineAction], SIGNAL(triggered(bool)), this, SLOT(setUnderline(bool)));
    al = new QAction(wwWidgets::icon("format-justify-left", QPixmap(":/trolltech/formeditor/images/win/textleft.png")), "Align left", this);
    ar = new QAction(wwWidgets::icon("format-justify-right", QPixmap(":/trolltech/formeditor/images/win/textright.png")), "Align right", this);
    ac = new QAction(wwWidgets::icon("format-justify-center", QPixmap(":/trolltech/formeditor/images/win/textcenter.png")), "Center", this);
    aj = new QAction(wwWidgets::icon("format-justify-fill", QPixmap(":/trolltech/formeditor/images/win/textjustify.png")), "Justify", this);
    QActionGroup *alignmentGroup = new QActionGroup(this);
    al->setCheckable(true);
    ar->setCheckable(true);
    ac->setCheckable(true);
    aj->setCheckable(true);
    alignmentGroup->addAction(al);
    alignmentGroup->addAction(ar);
    alignmentGroup->addAction(ac);
    alignmentGroup->addAction(aj);
    tb->addAction(al);
    tb->addAction(ac);
    tb->addAction(ar);
    tb->addAction(aj);
    tb->addSeparator();

    connect(alignmentGroup, SIGNAL(triggered(QAction *)), this, SLOT(changeAlignment(QAction*)));

    li = new QAction(wwWidgets::icon("format-list-unordered"), "List", this);
    tb->addAction(li);
    li->setCheckable(true);
    connect(li, SIGNAL(toggled(bool)), this, SLOT(setList(bool)));



    setViewportMargins(0, tb->sizeHint().height()+1, 0, 0);
    setContentsMargins(0, tb->sizeHint().height(), 0, 0);
    connect(this, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)),
            this, SLOT(updateCurrentCharFormat(const QTextCharFormat &)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(updateCurrentBlockFormat()));
    updateCurrentBlockFormat();
    updateCurrentCharFormat(textCursor().charFormat());
  }

/*!
 * \brief Changes alignment of the text to the one specified by action \a a
 */
void QwwRichTextEdit::changeAlignment(QAction *a){
    QTextCursor cur = textCursor();
    QTextBlockFormat fmt;
    if(a==al)
      fmt.setAlignment(Qt::AlignLeft);
    else if(a==ar)
      fmt.setAlignment(Qt::AlignRight);
    else if(a==ac)
      fmt.setAlignment(Qt::AlignCenter);
    else if(a==aj)
      fmt.setAlignment(Qt::AlignJustify);
    cur.mergeBlockFormat(fmt);
    setTextCursor(cur);
  }

/*!
 * \brief   Changes the bold attribute of the selected text to \a v
 */
void QwwRichTextEdit::setBold(bool v){
    QTextCursor cur = textCursor();
    QTextCharFormat fmt;
//    fmt.setFontWeight(QFont::Bold);
//    cur.mergeCharFormat(fmt);
     fmt.setFontWeight(!v ? QFont::Normal : QFont::Bold);
     cur.mergeCharFormat(fmt);
    setTextCursor(cur);
}

/*!
 * \brief   Changes the italic attribute of the selected text to \a v
 */
void QwwRichTextEdit::setItalic(bool v){
    QTextCursor cur = textCursor();
    QTextCharFormat fmt;
    fmt.setFontItalic(v);
    cur.mergeCharFormat(fmt);
    setTextCursor(cur);
}

/*!
 * \brief   Changes the underline attribute of the selected text to \a v
 */
void QwwRichTextEdit::setUnderline(bool v){
    QTextCursor cur = textCursor();
    QTextCharFormat fmt;
    fmt.setFontUnderline(v);
    cur.mergeCharFormat(fmt);
    setTextCursor(cur);
  }

/*!
 * \brief   Changes the color of the selected text to \a c
 */
void QwwRichTextEdit::setColor(const QColor &c)
{
    QTextCursor cur = textCursor();
    QTextCharFormat fmt;
    fmt.setForeground(c);
    cur.mergeCharFormat(fmt);
    setTextCursor(cur);
}

/*!
 * \internal
 */
void QwwRichTextEdit::setFont_h(){
    QFont f = fcb->currentFont();
    f.setPointSize(fsp->currentText().toInt());
    setFont(f);
    setFocus();
}

/*!
 * \brief   Changes the font of the selected text to \a f
 */
void QwwRichTextEdit::setFont(const QFont &f){
    QTextCursor cur = textCursor();
    QTextCharFormat fmt;
    fmt.setFontFamily(f.family());
    fmt.setFontPointSize(f.pointSize());
    cur.mergeCharFormat(fmt);
    setTextCursor(cur);
    if(fcb->currentFont().family()!=f.family())
        fcb->setCurrentFont(f);
    if(f.pointSize()!=fsp->currentText().toInt()){
      fsp->setCurrentIndex(fsp->findText(QString::number(f.pointSize())));
    }
  }

/*!
 * \brief   Changes the list attribute of the selected text to \a v
 */
void QwwRichTextEdit::setList(bool v){
    QTextCursor cur = textCursor();
    if(v){
      QTextListFormat listFormat;
      listFormat.setStyle(QTextListFormat::ListDisc);
      currentList = cur.createList(listFormat);
    } else {
      cur.setBlockFormat(QTextBlockFormat());
  //    cur.movePosition(QTextCursor::NextBlock);
//      cur.insertBlock(QTextBlockFormat());
      setTextCursor(cur);
      currentList = 0;
    }
  }

/*!
 * \brief Updates actions to reflect the format specified by \a fmt
 */
void QwwRichTextEdit::updateCurrentCharFormat(const QTextCharFormat &fmt){
    m_actions[BoldAction]->setChecked(fmt.fontWeight()==QFont::Bold);
    m_actions[ItalicAction]->setChecked(fmt.fontItalic());
    m_actions[UnderlineAction]->setChecked(fmt.fontUnderline());

    //if(fmt.font().family()!=fcb->currentFont().family()){
    //  fcb->setCurrentFont(fmt.font());
    //}
}

/*!
 * \brief Updates the format of the selected text
 */
void QwwRichTextEdit::updateCurrentBlockFormat(){
    QTextBlockFormat fmt = textCursor().blockFormat();
    switch(fmt.alignment()){
      case Qt::AlignLeft: al->setChecked(true); break;
      case Qt::AlignRight: ar->setChecked(true); break;
      case Qt::AlignCenter: ac->setChecked(true); break;
      case Qt::AlignJustify: aj->setChecked(true); break;
      default: al->setChecked(true);
    }
    if(!textCursor().hasSelection())
      li->setChecked(textCursor().currentList()!=0);
//    currentList = textCursor().currentList();
    QTextCharFormat cf = textCursor().charFormat();
    if(cf.font().family()!=fcb->currentFont().family()){
      fcb->setCurrentFont(cf.font());
    }
    if(cf.font().pointSize()!=fsp->currentText().toInt()){
      fsp->setCurrentIndex(fsp->findText(QString::number(cf.font().pointSize())));
    }
#ifndef WW_NO_COLORBUTTON
    if(cf.foreground().color()!=colorButton->currentColor() && !textCursor().hasSelection())
        colorButton->setCurrentColor(cf.foreground().color());
#endif
}

/*!
 * \internal
 */
bool QwwRichTextEdit::event(QEvent *e){
    if(e->type()!=QEvent::KeyPress)
      return QTextEdit::event(e);
    QKeyEvent *ke = (QKeyEvent*)e;
    currentList = textCursor().currentList();
    if(currentList){
      if(ke->key()==Qt::Key_Tab || ke->key() ==Qt::Key_Backtab){
        QTextCursor cur = textCursor();
        QTextListFormat listFormat = currentList->format();
        listFormat.setIndent(currentList->format().indent()+ (ke->key()== Qt::Key_Tab ? 1 : -1));
//        listFormat.setStyle(QTextListFormat::ListDisc);
        currentList = cur.createList(listFormat);
        return true;
      }
    }
    return QTextEdit::event(e);
}

/*!
 * \internal
 */
void QwwRichTextEdit::resizeEvent(QResizeEvent *re){
    tb->setGeometry(1,1,width()-verticalScrollBar()->sizeHint().width()-1, tb->sizeHint().height());
    QTextEdit::resizeEvent(re);
    QRect geo = verticalScrollBar()->geometry();
    geo.setTop(tb->geometry().bottom()+2);
    verticalScrollBar()->setGeometry(geo);
}

void QwwRichTextEdit::setOptions(QwwRichTextEdit::Options opt)
{
    m_options = opt;
  m_actions[BoldAction]->setVisible(m_options & Style);
  m_actions[ItalicAction]->setVisible(m_options & Style);
  m_actions[UnderlineAction]->setVisible(m_options & Style);
cB->setVisible(m_options & Style);

  ac->setVisible(m_options & Alignment);
  ar->setVisible(m_options & Alignment);
  al->setVisible(m_options & Alignment);
  aj->setVisible(m_options & Alignment);

  m_actions[FontFamilyAction]->setVisible(m_options & FontFamily);
  m_actions[FontSizeAction]->setVisible(m_options & FontSize);

  #ifndef WW_NO_COLORBUTTON
  m_actions[ColorAction]->setVisible(m_options & Color);
  #endif
}

QwwRichTextEdit::Options QwwRichTextEdit::options() const
{
    return m_options;
}

/*!
 * \brief Returns the QAction object associated with action \a act
 */
QAction * QwwRichTextEdit::toolBarAction(Action act) const
{
    return m_actions[act];
}

/*!
 * \internal
 */
void QwwRichTextEdit::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu *menu = createStandardContextMenu ( event->pos() );
    QMenu *formatMenu = new QMenu(tr("Style"));

    formatMenu->addAction(m_actions[BoldAction]);
    formatMenu->addAction(m_actions[ItalicAction]);
    formatMenu->addAction(m_actions[UnderlineAction]);
    menu->insertMenu(menu->actions()[9], formatMenu);
    QMenu *alignMenu = new QMenu(tr("Align"));
    alignMenu->addAction(al);
    alignMenu->addAction(ac);
    alignMenu->addAction(ar);
    alignMenu->addAction(aj);
    menu->insertMenu(menu->actions()[10], alignMenu);
    menu->exec(event->pos());
    delete menu;
}

#endif
