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

#ifndef GRANTLEE_BBCODEBUILDER_H
#define GRANTLEE_BBCODEBUILDER_H

#include "abstractmarkupbuilder.h"

namespace Grantlee
{

/**
  @brief Builder to create BBCode from a QTextDocument
*/
class BBCodeBuilder : public AbstractMarkupBuilder
{
public:

  /**
    Creates a new BBCodeBuilder.
  */
  BBCodeBuilder();

  virtual ~BBCodeBuilder();

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

  // Background colour not supported by BBCode.

  virtual void beginAnchor( const QString &href = QString(), const QString &name = QString() ) override;
  virtual void endAnchor() override;

  // Font not supported by BBCode.

  // Font family not supported by BBCode.

  /**
    Begin an element of font size @p size. Note that this size is in pixels, and must be converted before
    it is suitable for use in BBCode.
    @param size The size of font to begin.
  */
  virtual void beginFontPointSize( int size ) override;
  virtual void endFontPointSize() override;

  virtual void beginParagraph( Qt::LayoutDirection d = Qt::LayoutDirectionAuto, Qt::Alignment a = Qt::AlignLeft, qreal top = 0.0, qreal bottom = 0.0, qreal left = 0.0, qreal right = 0.0 ) override;
  virtual void endParagraph() override;

  virtual void beginIndent( int nBlockIndent = 0, qreal nTextIndent = 0.0, const QString &strClass = QLatin1String("bodyIndent") ) override;
  virtual void endIndent() override;

  virtual void addNewline() override;

  virtual void addLineBreak() override;

  virtual void insertImage( const QString &src, qreal width, qreal height ) override;

  virtual void beginList( QTextListFormat::Style type ) override;

  virtual void endList() override;


  virtual void beginListItem() override;

  virtual void beginSuperscript() override;

  virtual void endSuperscript() override;

  virtual void beginSubscript() override;

  virtual void endSubscript() override;


  virtual void beginTable( qreal, qreal, const QString & ) override;

  virtual void beginTableRow() override;


  virtual void appendLiteralText( const QString &text ) override;

  /**
    Escapes @p text appropriately for BBCode.
  */
  virtual const QString escape( const QString &text ) const override;

  virtual void appendRawText( const QString &text ) override;

  virtual QString getResult() override;

private:
  QList<QTextListFormat::Style> m_currentListItemStyles;

  QString m_text;

  Qt::Alignment m_currentAlignment;

};

}

#endif
