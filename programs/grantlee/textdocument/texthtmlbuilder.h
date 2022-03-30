/*
  This file is part of the Grantlee template system.

  Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version
  2.1 of the Licence, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.

-------------------------------------------------------------------------------

Modified for the behaviour desired in King James Pure Bible Search.
Modifications Copyright (C) 2013-2022, Donna Whisnant, a.k.a. Dewtronics

-------------------------------------------------------------------------------

*/

#ifndef GRANTLEE_TEXTHTMLBUILDER_H
#define GRANTLEE_TEXTHTMLBUILDER_H

#ifdef Q_CC_MSVC
#pragma warning( disable : 4250 )
#endif

#include "abstractmarkupbuilder.h"
#include "grantlee_gui_export.h"
#include "markupdirector.h"


namespace Grantlee
{

class TextHTMLBuilderPrivate;

/// @headerfile texthtmlbuilder.h grantlee/texthtmlbuilder.h

/**
  @brief The TextHTMLBuilder creates a clean html markup output.

  This class creates html output which is as minimal as possible and restricted to the rich text features supported in Qt. (http://doc.trolltech.com/4.4/richtext-html-subset.htm)

  The output contains only the body content, not the head element or other metadata.

  eg:

  @code
    <p>
      This is some <strong>formatted content</strong> in a paragraph.
    </p>
  @endcode

  instead of the content produced by qt:

  @code
    <!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
    <html><head><meta name="qrichtext" content="1" /><meta http-equiv="Content-Type" content="text/html; charset=UTF-8" /><style type="text/css">
    p, li { white-space: pre-wrap; }
    </style></head><body style=" font-family:'Sans Serif'; font-size:10pt; font-weight:400; font-style:normal;">
    <p style=" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;">This is some <span style=" font-weight:600;">formatted content</span> in a paragraph. </p></body></html>
  @endcode

  Such tags should be created separately. For example:

  @code
    AbstractMarkupBuilder *b = new TextHTMLBuilder();
    MarkupDirector *md = new MarkupDirector(b);
    md->constructContent();
    QString cleanHtml("<head>\n<title>%1</title>\n</head>\n<body>%2</body>\n</html>")
                      .arg(document.metaInformation(QTextDocument::DocumentTitle))
                      .arg(b->getOutput());
    QFile.write(cleanHtml);
  @endcode

  Font formatting information on elements is represented by individual span elements.
  eg:
  @code
    <span style"color:blue;"><span style="background-color:red;">Blue text on red background</span></span>
  @endcode
  instead of
  @code
    <span style="color:blue;background-color:red;">Blue text on red background</span>
  @endcode
  It my be possible to change this if necessary.

  @author Stephen Kelly <steveire@gmail.com>
*/
class GRANTLEE_GUI_EXPORT TextHTMLBuilder : virtual public AbstractMarkupBuilder
{
public:

  /**
    Creates a new TextHTMLBuilder.
  */
  TextHTMLBuilder();
  virtual ~TextHTMLBuilder();

  virtual void beginStrong() override;
  virtual void endStrong() override;
  virtual void beginEmph() override;
  virtual void endEmph() override;
  virtual void beginUnderline() override;
  virtual void endUnderline() override;
  virtual void beginStrikeout() override;
  virtual void endStrikeout() override;
  virtual void beginForeground( const QBrush &brush ) override;
  virtual void endForeground() override;
  virtual void beginBackground( const QBrush &brush ) override;
  virtual void endBackground() override;
  virtual void beginAnchor( const QString &href = QString(), const QString &name = QString() ) override;
  virtual void endAnchor() override;

  // Maybe this stuff should just be added to a list, and then when I add literal text,
  // add some kind of style attribute in one span instead of many.
  virtual void beginFont(const QString &family, int size) override;
  virtual void endFont() override;

  virtual void beginFontFamily( const QString &family ) override;
  virtual void endFontFamily() override;

  /**
    Begin a new font point size
    @param size The new size to begin.
  */
  virtual void beginFontPointSize( int size ) override;
  virtual void endFontPointSize() override;

  /**
    Begin a new paragraph
    @param al The new paragraph alignment
    @param topMargin The new paragraph topMargin
    @param bottomMargin The new paragraph bottomMargin
    @param leftMargin The new paragraph leftMargin
    @param rightMargin The new paragraph rightMargin
  */
  virtual void beginParagraph( Qt::LayoutDirection d = Qt::LayoutDirectionAuto, Qt::Alignment al = Qt::AlignLeft, qreal topMargin = 0.0, qreal bottomMargin = 0.0, qreal leftMargin = 0.0, qreal rightMargin = 0.0 ) override;
  virtual void endParagraph() override;

  virtual void beginIndent( int nBlockIndent = 0, qreal nTextIndent = 0.0, const QString &strClass = QLatin1String("bodyIndent") ) override;
  virtual void endIndent() override;

  /**
    Begin a new header element.
    @param level The new level to begin.
  */
  virtual void beginHeader( int level ) override;

  /**
    End a header element.
    @param level The new level to end.
  */
  virtual void endHeader( int level ) override;

  virtual void addNewline() override;

  virtual void addLineBreak() override;

  virtual void insertHorizontalRule( int width = -1 ) override;

  virtual void insertImage( const QString &src, qreal width, qreal height ) override;

  virtual void beginList( QTextListFormat::Style type ) override;

  virtual void endList() override;

  virtual void beginListItem() override;
  virtual void endListItem() override;

  virtual void beginSuperscript() override;

  virtual void endSuperscript() override;

  virtual void beginSubscript() override;

  virtual void endSubscript() override;


  virtual void beginTable( qreal cellpadding, qreal cellspacing, const QString &width ) override;

  virtual void beginTableRow() override;
  virtual void beginTableHeaderCell( const QString &width, int colspan, int rowspan ) override;

  virtual void beginTableCell( const QString &width, int colspan, int rowspan ) override;

  virtual void endTable() override;
  virtual void endTableRow() override;
  virtual void endTableHeaderCell() override;
  virtual void endTableCell() override;

  /**
    Reimplemented from AbstractMarkupBuilder.

    This implementation escapes the text before appending so that

    @verbatim
      A sample <b>bold</b> word.
    @endverbatim

    becomes

    @verbatim
      A sample &lt;b&gt;bold&lt;/b&gt; word.
    @endverbatim
  */
  virtual void appendLiteralText( const QString &text ) override;

  virtual const QString escape( const QString &s ) const override;

  /**
    Append @p text without escaping.

    This is useful if extending MarkupDirector
  */
  virtual void appendRawText( const QString &text ) override;

  virtual QString getResult() override;

private:
  TextHTMLBuilderPrivate *d_ptr;
  Q_DECLARE_PRIVATE( TextHTMLBuilder )

};

}

#endif
