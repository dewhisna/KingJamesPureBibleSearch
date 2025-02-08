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

#include "bbcodebuilder.h"

using namespace Grantlee;

BBCodeBuilder::BBCodeBuilder()
  : m_currentAlignment( Qt::AlignLeft )
{
}

BBCodeBuilder::~BBCodeBuilder()
{
}

void BBCodeBuilder::beginStrong()
{
  appendRawText( QLatin1String( "[B]" ) );
}
void BBCodeBuilder::endStrong()
{
  appendRawText( QLatin1String( "[/B]" ) );
}
void BBCodeBuilder::beginEmph()
{
  appendRawText( QLatin1String( "[I]" ) );
}
void BBCodeBuilder::endEmph()
{
  appendRawText( QLatin1String( "[/I]" ) );
}
void BBCodeBuilder::beginUnderline()
{
  appendRawText( QLatin1String( "[U]" ) );
}
void BBCodeBuilder::endUnderline()
{
  appendRawText( QLatin1String( "[/U]" ) );
}
void BBCodeBuilder::beginStrikeout()
{
  appendRawText( QLatin1String( "[S]" ) );
}
void BBCodeBuilder::endStrikeout()
{
  appendRawText( QLatin1String( "[/S]" ) );
}
void BBCodeBuilder::beginForeground( const QBrush &brush )
{
  appendRawText( QString::fromLatin1( "[COLOR=%1]" ).arg( brush.color().name() ) );
}
void BBCodeBuilder::endForeground()
{
  appendRawText( QLatin1String( "[/COLOR]" ) );
}

// Background colour not supported by BBCode.

void BBCodeBuilder::beginAnchor( const QString &href, const QString &name )
{
  Q_UNUSED( name )
  appendRawText( QString::fromLatin1( "[URL=%1]" ).arg( href ) );
}
void BBCodeBuilder::endAnchor()
{
  appendRawText( QLatin1String( "[/URL]" ) );
}

// Font family not supported by BBCode.

void BBCodeBuilder::beginFontPointSize( int size )
{
  appendRawText( QString::fromLatin1( "[SIZE=%1]" ).arg( QString::number( size ) ) );
}
void BBCodeBuilder::endFontPointSize()
{
  appendRawText( QLatin1String( "[/SIZE]" ) );
}

void BBCodeBuilder::beginParagraph(Qt::LayoutDirection d, Qt::Alignment a, qreal top, qreal bottom, qreal left, qreal right )
{
  Q_UNUSED( d );
  Q_UNUSED( top );
  Q_UNUSED( bottom );
  Q_UNUSED( left );
  Q_UNUSED( right );
  if ( a & Qt::AlignRight ) {
	appendRawText( QLatin1String( "\n[Right]" ) );
  } else if ( a & Qt::AlignHCenter ) {
	appendRawText( QLatin1String( "\n[CENTER]" ) );
  }
  // LEFT is also supported in BBCode, but ignored.
  m_currentAlignment = a;
}

void BBCodeBuilder::endParagraph()
{
  if ( m_currentAlignment & Qt::AlignRight ) {
	appendRawText( QLatin1String( "\n[/Right]\n" ) );
  } else if ( m_currentAlignment & Qt::AlignHCenter ) {
	appendRawText( QLatin1String( "\n[/CENTER]\n" ) );
  } else {
	appendRawText( QLatin1String( "\n" ) );
  }
  m_currentAlignment = Qt::AlignLeft;
}

void BBCodeBuilder::beginIndent( int nBlockIndent, qreal nTextIndent, const QString &strClass)
{
  Q_UNUSED( strClass );
  Q_UNUSED( nBlockIndent );
  Q_UNUSED( nTextIndent );
}

void BBCodeBuilder::endIndent()
{
}

void BBCodeBuilder::addNewline()
{
  appendRawText( QLatin1String( "\n" ) );
}

void BBCodeBuilder::addLineBreak()
{
  appendRawText( QLatin1String( "\n" ) );
}

void BBCodeBuilder::insertImage( const QString &src, qreal width, qreal height )
{
  Q_UNUSED( width );
  Q_UNUSED( height );
  appendRawText( QString::fromLatin1( "[IMG]%1[/IMG]" ).arg( src ) );
}

void BBCodeBuilder::beginList( QTextListFormat::Style type )
{
  switch ( type ) {
  case QTextListFormat::ListDisc:
  case QTextListFormat::ListCircle:
  case QTextListFormat::ListSquare:
	appendRawText( QLatin1String( "[LIST]\n" ) );   // Unordered lists are all disc type in BBCode.
    break;
  case QTextListFormat::ListDecimal:
	appendRawText( QLatin1String( "[LIST=1]\n" ) );
    break;
  case QTextListFormat::ListLowerAlpha:
	appendRawText( QLatin1String( "[LIST=a]\n" ) );
    break;
  case QTextListFormat::ListUpperAlpha:
	appendRawText( QLatin1String( "[LIST=A]\n" ) );
    break;
  default:
    break;
  }
}

void BBCodeBuilder::endList()
{
  appendRawText( QLatin1String( "[/LIST]\n" ) );
}

void BBCodeBuilder::beginListItem()
{
  appendRawText( QLatin1String( "[*] " ) );
}

void BBCodeBuilder::beginSuperscript()
{
  appendRawText( QLatin1String( "[SUP]" ) );
}

void BBCodeBuilder::endSuperscript()
{
  appendRawText( QLatin1String( "[/SUP]" ) );
}

void BBCodeBuilder::beginSubscript()
{
  appendRawText( QLatin1String( "[SUB]" ) );
}

void BBCodeBuilder::endSubscript()
{
  appendRawText( QLatin1String( "[/SUB]" ) );
}


void BBCodeBuilder::beginTable( qreal, qreal, const QString & )
{
  appendRawText( QLatin1String( "[TABLE]\n" ) );
}

void BBCodeBuilder::beginTableRow()
{
  appendRawText( QLatin1String( "[/TABLE]" ) );
}

void BBCodeBuilder::appendLiteralText( const QString &text )
{
  appendRawText( escape( text ) );
}

const QString BBCodeBuilder::escape( const QString &s ) const
{
  if ( s.contains( QLatin1Char( '[' ) ) ) {
    return QLatin1String( "[NOPARSE]" ) + s + QLatin1String( "[/NOPARSE]" );
  }
  return s;
}

void BBCodeBuilder::appendRawText(const QString &text)
{
	m_text.append(text);
}

QString BBCodeBuilder::getResult()
{
  QString ret = m_text;
  m_text.clear();
  return ret;
}

