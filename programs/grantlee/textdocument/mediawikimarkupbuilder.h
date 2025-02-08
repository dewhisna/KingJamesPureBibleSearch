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
Modifications Copyright (C) 2013-2025, Donna Whisnant, a.k.a. Dewtronics

-------------------------------------------------------------------------------

*/

#ifndef GRANTLEE_MEDIAWIKIMARKUPBUILDER_H
#define GRANTLEE_MEDIAWIKIMARKUPBUILDER_H

#include "abstractmarkupbuilder.h"

namespace Grantlee
{

/**
  @brief Creates MediaWiki markup from a QTextDocument
*/
class MediaWikiMarkupBuilder : public AbstractMarkupBuilder
{
public:

  /**
    Creates a new MediaWikiMarkupBuilder
  */
  MediaWikiMarkupBuilder();
  virtual ~MediaWikiMarkupBuilder();

  virtual void beginStrong() override;
  virtual void endStrong() override;
  virtual void beginEmph() override;
  virtual void endEmph() override;
  virtual void beginUnderline() override;
  virtual void endUnderline() override;
  virtual void beginStrikeout() override;
  virtual void endStrikeout() override;

  virtual void endParagraph() override;
  virtual void addNewline() override;

  virtual void addLineBreak() override;

  virtual void beginAnchor( const QString &href = QString(), const QString &name = QString() ) override;
  virtual void endAnchor() override;

  virtual void beginHeader( int level ) override;
  virtual void endHeader( int level ) override;

  virtual void beginList( QTextListFormat::Style type ) override;

  virtual void endList() override;

  virtual void beginListItem() override;
  virtual void endListItem() override;

  virtual void appendLiteralText( const QString &text ) override;

  /**
    Escapes @p text appropriately for MediaWiki.
  */
  virtual const QString escape( const QString &s ) const override;

  virtual void appendRawText( const QString &text ) override;

  virtual QString getResult() override;

private:
  QList<QTextListFormat::Style> currentListItemStyles;

  QString m_text;
};

}

#endif
