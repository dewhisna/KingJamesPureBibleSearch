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
Modifications Copyright 2013, Donna Whisnant, a.k.a. Dewtronics

-------------------------------------------------------------------------------

*/

#include "mediawikimarkupbuilder.h"

using namespace Grantlee;

MediaWikiMarkupBuilder::MediaWikiMarkupBuilder()
{
}

MediaWikiMarkupBuilder::~MediaWikiMarkupBuilder()
{
}

void MediaWikiMarkupBuilder::beginStrong()
{
  appendRawText( QLatin1String( "'''" ) );
}
void MediaWikiMarkupBuilder::endStrong()
{
  appendRawText( QLatin1String( "'''" ) );
}
void MediaWikiMarkupBuilder::beginEmph()
{
  appendRawText( QLatin1String( "''" ) );
}
void MediaWikiMarkupBuilder::endEmph()
{
  appendRawText( QLatin1String( "''" ) );
}
void MediaWikiMarkupBuilder::beginUnderline()
{
  appendRawText( QLatin1String( "<u>" ) );
}
void MediaWikiMarkupBuilder::endUnderline()
{
  appendRawText( QLatin1String( "</u>" ) );
}
void MediaWikiMarkupBuilder::beginStrikeout()
{
  appendRawText( QLatin1String( "<s>" ) );
}
void MediaWikiMarkupBuilder::endStrikeout()
{
  appendRawText( QLatin1String( "</s>" ) );
}

void MediaWikiMarkupBuilder::endParagraph()
{
  appendRawText( QLatin1String( "\n" ) );
}
void MediaWikiMarkupBuilder::addNewline()
{
  appendRawText( QLatin1String( "\n" ) );
}

void MediaWikiMarkupBuilder::beginAnchor( const QString &href, const QString &name )
{
  Q_UNUSED( name );
  appendRawText( QString::fromLatin1( "[%1 " ).arg( href ) );
}
void MediaWikiMarkupBuilder::endAnchor()
{
  appendRawText( QLatin1String( "]" ) );
}

void MediaWikiMarkupBuilder::beginHeader( int level )
{
  switch ( level ) {
  case 1:
	appendRawText( QLatin1String( "= " ) );
    break;
  case 2:
	appendRawText( QLatin1String( "== " ) );
    break;
  case 3:
	appendRawText( QLatin1String( "=== " ) );
    break;
  case 4:
	appendRawText( QLatin1String( "==== " ) );
    break;
  case 5:
	appendRawText( QLatin1String( "===== " ) );
    break;
  case 6:
	appendRawText( QLatin1String( "====== " ) );
    break;
  default:
    break;
  }
}

void MediaWikiMarkupBuilder::endHeader( int level )
{
  switch ( level ) {
  case 1:
	appendRawText( QLatin1String( " =\n" ) );
    break;
  case 2:
	appendRawText( QLatin1String( " ==\n" ) );
    break;
  case 3:
	appendRawText( QLatin1String( " ===\n" ) );
    break;
  case 4:
	appendRawText( QLatin1String( " ====\n" ) );
    break;
  case 5:
	appendRawText( QLatin1String( " =====\n" ) );
    break;
  case 6:
	appendRawText( QLatin1String( " ======\n" ) );
    break;
  default:
    break;
  }
}

void MediaWikiMarkupBuilder::beginList( QTextListFormat::Style type )
{
  currentListItemStyles.append( type );
  switch ( type ) {
  case QTextListFormat::ListDisc:
  case QTextListFormat::ListCircle:
  case QTextListFormat::ListSquare:
  case QTextListFormat::ListDecimal:
  case QTextListFormat::ListLowerAlpha:
  case QTextListFormat::ListUpperAlpha:
	appendRawText( QLatin1String( "\n" ) );
    break;
  default:
    break;
  }
}

void MediaWikiMarkupBuilder::endList()
{
  appendRawText( QLatin1String( "\n" ) );
  currentListItemStyles.removeLast();
}

void MediaWikiMarkupBuilder::beginListItem()
{
  switch ( currentListItemStyles.last() ) {
  case QTextListFormat::ListDisc:
  case QTextListFormat::ListCircle:
  case QTextListFormat::ListSquare:
	appendRawText( QLatin1String( "* " ) );  // Unordered lists are all disc type in MediaWikiMarkup.
    break;
  case QTextListFormat::ListDecimal:
  case QTextListFormat::ListLowerAlpha:
  case QTextListFormat::ListUpperAlpha:
	appendRawText( QLatin1String( "# " ) );
    break;
  default:
    break;
  }
}

void MediaWikiMarkupBuilder::endListItem()
{
  appendRawText( QLatin1String( "\n" ) );
}

void MediaWikiMarkupBuilder::appendLiteralText( const QString &text )
{
  appendRawText( escape( text ) );
}

const QString MediaWikiMarkupBuilder::escape( const QString &s ) const
{
  if ( s.contains( QLatin1Char( '<' ) ) ) {    // TODO: This could contain more. "''" and "[" for example
    return QLatin1String( "<nowiki>" ) + s + QLatin1String( "</nowiki>" );
  }
  return s;
}

void MediaWikiMarkupBuilder::appendRawText(const QString &text)
{
	m_text.append(text);
}

QString MediaWikiMarkupBuilder::getResult()
{
  QString ret = m_text;
  m_text.clear();
  return ret;
}
