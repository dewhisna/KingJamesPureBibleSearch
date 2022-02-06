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
Modifications Copyright 2013-2020, Donna Whisnant, a.k.a. Dewtronics

-------------------------------------------------------------------------------

*/

#ifndef GRANTLEE_PLAINTEXTMARKUPBUILDER_H
#define GRANTLEE_PLAINTEXTMARKUPBUILDER_H


#ifdef Q_OS_WIN
#ifdef _MSC_VER			// Warning disabling is only needed on Microsoft Compiler
#pragma warning( disable : 4250 )
#endif
#endif

#define LETTERSINALPHABET 26
#define DIGITSOFFSET 10

#include "abstractmarkupbuilder.h"
#include "grantlee_gui_export.h"
#include "markupdirector.h"

class QBrush;

namespace Grantlee
{

class PlainTextMarkupBuilderPrivate;

/// @headerfile plaintextmarkupbuilder.h grantlee/plaintextmarkupbuilder.h

/**
  @brief The PlainTextHTMLMarkupBuilder creates a simple marked up plain text document.

  This class creates a simple plain text markup.

  Text that may be represented as

  @code
    A paragraph with <b>bold</b> text, <i>italic</i> text, and <u>underlined</u> text.
  @endcode

  would be output as

  @code
    A paragraph with *bold* text /italic/ text, and _underlined_ text.
  @endcode

  The markup is intended to be simple, plain and easily human readable. No markup is created for different font-familiy, font-size, foreground or background colors.

  Lists are marked up by preceding the list element with '*' for disc, 'o' for circle, 'X' for square, or a letter or number. Lists are also indented if nested.
  eg:

  @code
    A. One
    B. Two
      o Three
      o Four
        \* Five
        \* Six
    C. Seven
  @endcode

  External references such as external urls and images are represented in the body text as a reference, and references are maintained at the bottom of the output.

  Eg,
  @code
    Here is a link to <a href="http://www.kde.org">KDE</a> and the <a href="http://pim.kde.org">KDEPIM project</a>.
  @endcode

  becomes:

  @code
    Here is a link to KDE[1], and the KDEPIM project[2].

    ---- References ----
    [1] http://www.kde.org
    [2] http://pim.kde.org
  @endcode

  @author Stephen Kelly <steveire@gmail.com>
*/
class GRANTLEE_GUI_EXPORT PlainTextMarkupBuilder : virtual public AbstractMarkupBuilder
{
public:
  /** Construct a new PlainTextHTMLMarkupBuilder. */
  PlainTextMarkupBuilder();

  virtual ~PlainTextMarkupBuilder();

  virtual void beginStrong() override;
  virtual void endStrong() override;
  virtual void beginEmph() override;
  virtual void endEmph() override;
  virtual void beginUnderline() override;
  virtual void endUnderline() override;
  virtual void beginStrikeout() override;
  virtual void endStrikeout() override;

  virtual void beginAnchor( const QString &href = QString(), const QString &name = QString() ) override;

  virtual void endAnchor() override;

  virtual void beginForeground( const QBrush &brush ) override;

  virtual void endForeground() override;

  virtual void beginBackground( const QBrush &brush ) override;

  virtual void endBackground() override;

  virtual void beginFont(const QString &family, int size) override;

  virtual void endFont() override;

  virtual void beginFontFamily( const QString &family ) override;

  virtual void endFontFamily() override;

  virtual void beginFontPointSize( int size ) override;

  virtual void endFontPointSize() override;

  virtual void beginParagraph( Qt::Alignment a = Qt::AlignLeft, qreal top = 0.0, qreal bottom = 0.0, qreal left = 0.0, qreal right = 0.0 ) override;
  virtual void endParagraph() override;

  virtual void beginIndent( int nBlockIndent = 0, qreal nTextIndent = 0.0, const QString &strClass = QLatin1String("bodyIndent") ) override;
  virtual void endIndent() override;

  virtual void addNewline() override;

  virtual void addLineBreak() override;

  virtual void insertHorizontalRule( int width = -1 ) override;

  virtual void insertImage( const QString &src, qreal width, qreal height ) override;

  virtual void beginList( QTextListFormat::Style style ) override;

  virtual void endList() override;

  virtual void beginListItem() override;

  virtual void endListItem() override;

  virtual void beginSuperscript() override;

  virtual void endSuperscript() override;

  virtual void beginSubscript() override;

  virtual void endSubscript() override;

  virtual void beginTable( qreal cellpadding, qreal cellspacing, const QString &width ) override;

  virtual void beginTableRow() override;

  virtual void beginTableHeaderCell( const QString &width, int colSpan, int rowSpan ) override;

  virtual void beginTableCell( const QString &width, int colSpan, int rowSpan ) override;

  virtual void endTable() override;

  virtual void endTableRow() override;

  virtual void endTableHeaderCell() override;

  virtual void endTableCell() override;

  virtual void beginHeader( int level ) override;

  virtual void endHeader( int level ) override;

  virtual void appendLiteralText( const QString &text ) override;

  virtual const QString escape( const QString &s ) const override;

  virtual void appendRawText( const QString &text ) override;

  /**
    Adds a reference to @p reference to the internal list of references in the document.
  */
  int addReference( const QString &reference );

  /**
    Returns the finalised plain text markup, including references at the end.
  */
  virtual QString getResult() override;

private:
  PlainTextMarkupBuilderPrivate * const d_ptr;
  Q_DECLARE_PRIVATE( PlainTextMarkupBuilder )

};

}

#endif
