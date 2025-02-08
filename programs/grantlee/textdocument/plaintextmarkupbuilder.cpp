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

#include "plaintextmarkupbuilder.h"

namespace Grantlee
{

class PlainTextMarkupBuilderPrivate
{
public:
  PlainTextMarkupBuilderPrivate( PlainTextMarkupBuilder *b )
      : q_ptr( b ) {
  }

  /**
    Get a letter string to represent a number.

    The numbers 1-26 are represented by a-z, and 27-52 by aa-az, 53-79 by ba-bz etc.

    @param The number to convert
    @return The letter string representation of the number.
  */
  QString getLetterString( int itemNumber );

  QString getRomanString(int itemNumber);

  /**
    Gets a block of references in the body of the text.
    This is an ordered list of links and images in the text.
  */
  QString getReferences();

  QStringList m_urls;
  QList<QTextListFormat::Style> currentListItemStyles;
  QList<int> currentListItemNumbers;

  QString activeLink;

  QString m_text;

  PlainTextMarkupBuilder *q_ptr;

  Q_DECLARE_PUBLIC( PlainTextMarkupBuilder )

};

}

using namespace Grantlee;

QString PlainTextMarkupBuilderPrivate::getRomanString(int item)
{
  QString result;
  //Code based to gui/text/qtextlist.cpp
  if (item < 5000) {
      QString romanNumeral;

      // works for up to 4999 items
      QString romanSymbols = QLatin1String("iiivixxxlxcccdcmmmm");
      int c[] = { 1, 4, 5, 9, 10, 40, 50, 90, 100, 400, 500, 900, 1000 };
      int n = item;
      for (int i = 12; i >= 0; n %= c[i], i--) {
          int q = n / c[i];
          if (q > 0) {
              int startDigit = i + (i+3)/4;
              int numDigits;
              if (i % 4) {
                  // c[i] == 4|5|9|40|50|90|400|500|900
                  if ((i-2) % 4) {
                      // c[i] == 4|9|40|90|400|900 => with subtraction (IV, IX, XL, XC, ...)
                      numDigits = 2;
                  }
                  else {
                      // c[i] == 5|50|500 (V, L, D)
                      numDigits = 1;
                  }
              }
              else {
                  // c[i] == 1|10|100|1000 (I, II, III, X, XX, ...)
                  numDigits = q;
              }

              romanNumeral.append(romanSymbols.mid(startDigit, numDigits));
          }
      }
      result = romanNumeral;
  } else {
      result = QLatin1String("?");
  }
  return result;
}

QString PlainTextMarkupBuilderPrivate::getLetterString( int itemNumber )
{
  QString letterString;
  while ( true ) {
    // Create the letter string by prepending one char at a time.
    // The itemNumber is converted to a number in the base 36 (number of letters in the
    // alphabet plus 10) after being increased by 10 (to pass out the digits 0 to 9).
    letterString.prepend( QString::fromLatin1( "%1" ).arg(( itemNumber % LETTERSINALPHABET ) + DIGITSOFFSET,
                          0, // no padding while building this string.
                          LETTERSINALPHABET + DIGITSOFFSET ) );
    if (( itemNumber >= LETTERSINALPHABET ) ) {
      itemNumber = itemNumber / LETTERSINALPHABET;
      itemNumber--;
    } else {
      break;
    }
  }
  return letterString;
}

QString PlainTextMarkupBuilderPrivate::getReferences()
{
  QString refs;
  if ( !m_urls.isEmpty() ) {
    refs.append( QLatin1String( "\n--------\n" ) );

    int index = 1;
    while ( !m_urls.isEmpty() ) {
      refs.append( QString::fromLatin1( "[%1] %2\n" ).arg( index++ ).arg( m_urls.takeFirst() ) );
    }
  }
  return refs;
}

PlainTextMarkupBuilder::PlainTextMarkupBuilder()
  : d_ptr( new PlainTextMarkupBuilderPrivate( this ) )
{
}

PlainTextMarkupBuilder::~PlainTextMarkupBuilder()
{
  delete d_ptr;
}

void PlainTextMarkupBuilder::beginStrong()
{
  appendRawText( QLatin1String( "*" ) );
}

void PlainTextMarkupBuilder::endStrong()
{
  appendRawText( QLatin1String( "*" ) );
}

void PlainTextMarkupBuilder::beginEmph()
{
  appendRawText( QLatin1String( "[" ) );
}

void PlainTextMarkupBuilder::endEmph()
{
  appendRawText( QLatin1String( "]" ) );
}

void PlainTextMarkupBuilder::beginUnderline()
{
  appendRawText( QLatin1String( "_" ) );
}

void PlainTextMarkupBuilder::endUnderline()
{
  appendRawText( QLatin1String( "_" ) );
}

void PlainTextMarkupBuilder::beginStrikeout()
{
  appendRawText( QLatin1String( "-" ) );
}

void PlainTextMarkupBuilder::endStrikeout()
{
  appendRawText( QLatin1String( "-" ) );
}

void PlainTextMarkupBuilder::beginAnchor( const QString &href, const QString &name )
{
  Q_D( PlainTextMarkupBuilder );
  Q_UNUSED( name );
  if ( !d->m_urls.contains( href ) ) {

    d->m_urls.append( href );
  }
  d->activeLink = href;
}

void PlainTextMarkupBuilder::endAnchor()
{
  Q_D( PlainTextMarkupBuilder );
  appendRawText( QString::fromLatin1( "[%1]" ).arg( d->m_urls.indexOf( d->activeLink ) + 1 ) );
}

void PlainTextMarkupBuilder::addNewline()
{
  appendRawText( QLatin1String( "\n" ) );
}

void PlainTextMarkupBuilder::addLineBreak()
{
  appendRawText( QLatin1String( "\n" ) );
}

void PlainTextMarkupBuilder::insertHorizontalRule( int width )
{
  Q_UNUSED( width )

  appendRawText( QLatin1String( "--------------------\n" ) );
}

int PlainTextMarkupBuilder::addReference( const QString& reference )
{
  Q_D( PlainTextMarkupBuilder );

  if ( !d->m_urls.contains( reference ) )
    d->m_urls.append( reference );
  return d->m_urls.indexOf( reference ) + 1;
}

void PlainTextMarkupBuilder::insertImage( const QString &src, qreal width, qreal height )
{
  Q_UNUSED( width )
  Q_UNUSED( height )

  int ref = addReference( src );

  appendRawText( QString::fromLatin1( "[%1]" ).arg( ref ) );
}

void PlainTextMarkupBuilder::beginList( QTextListFormat::Style style )
{
  Q_D( PlainTextMarkupBuilder );
  d->currentListItemStyles.append( style );
  d->currentListItemNumbers.append( 0 );
}

void PlainTextMarkupBuilder::endList()
{
  Q_D( PlainTextMarkupBuilder );
  if ( !d->currentListItemNumbers.isEmpty() ) {
    d->currentListItemStyles.removeLast();
    d->currentListItemNumbers.removeLast();
  }
}

void PlainTextMarkupBuilder::beginListItem()
{
  Q_D( PlainTextMarkupBuilder );
  for ( int i = 0; i < d->currentListItemNumbers.size(); i++ ) {
	appendRawText( QLatin1String( "    " ) );
  }

  int itemNumber = d->currentListItemNumbers.last();

  switch ( d->currentListItemStyles.last() ) {
  case QTextListFormat::ListDisc:
	appendRawText( QLatin1String( " *  " ) );
    break;
  case QTextListFormat::ListCircle:
	appendRawText( QLatin1String( " o  " ) );
    break;
  case QTextListFormat::ListSquare:
	appendRawText( QLatin1String( " -  " ) );
    break;
  case QTextListFormat::ListDecimal:
	appendRawText( QString::fromLatin1( " %1. " ).arg( itemNumber + 1 ) );
    break;
  case QTextListFormat::ListLowerAlpha:
	appendRawText( QString::fromLatin1( " %1. " ).arg( d->getLetterString( itemNumber ) ) );
    break;
  case QTextListFormat::ListUpperAlpha:
	appendRawText( QString::fromLatin1( " %1. " ).arg( d->getLetterString( itemNumber ).toUpper() ) );
    break;
  case QTextListFormat::ListLowerRoman:
	appendRawText( QString::fromLatin1( " %1. " ).arg( d->getRomanString( itemNumber +1 ) ) );
    break;
  case QTextListFormat::ListUpperRoman:
	appendRawText( QString::fromLatin1( " %1. " ).arg( d->getRomanString( itemNumber +1 ).toUpper() ) );
    break;
  default:
    break;
  }
}

void PlainTextMarkupBuilder::endListItem()
{
  Q_D( PlainTextMarkupBuilder );
  d->currentListItemNumbers.last() = d->currentListItemNumbers.last() + 1;
  appendRawText( QLatin1String("\n") );
}

void PlainTextMarkupBuilder::beginSuperscript()
{
  appendRawText( QLatin1String( "^{" ) );
}

void PlainTextMarkupBuilder::endSuperscript()
{
  appendRawText( QLatin1String( "}" ) );
}

void PlainTextMarkupBuilder::beginSubscript()
{
  appendRawText( QLatin1String( "_{" ) );
}

void PlainTextMarkupBuilder::endSubscript()
{
  appendRawText( QLatin1String( "}" ) );
}

void PlainTextMarkupBuilder::appendLiteralText( const QString &text )
{
  appendRawText( escape(text) );
}

const QString PlainTextMarkupBuilder::escape( const QString &s ) const
{
	return s;
}

void PlainTextMarkupBuilder::appendRawText( const QString &text )
{
  Q_D( PlainTextMarkupBuilder );
  d->m_text.append( text );
}

QString PlainTextMarkupBuilder::getResult()
{
  Q_D( PlainTextMarkupBuilder );
  QString ret = d->m_text;
  ret.append( d->getReferences() );
  d->m_text.clear();
  return ret;
}

void PlainTextMarkupBuilder::beginBackground( const QBrush& brush )
{
  Q_UNUSED( brush );
}

void PlainTextMarkupBuilder::beginFont(const QString &family, int size)
{
  Q_UNUSED( family );
  Q_UNUSED( size );
}

void PlainTextMarkupBuilder::beginFontFamily( const QString& family )
{
  Q_UNUSED( family );
}

void PlainTextMarkupBuilder::beginFontPointSize( int size )
{
  Q_UNUSED( size );
}

void PlainTextMarkupBuilder::beginForeground( const QBrush& brush )
{
  Q_UNUSED( brush );
}

void PlainTextMarkupBuilder::beginHeader( int level )
{
  Q_UNUSED( level );
}

void PlainTextMarkupBuilder::beginParagraph(Qt::LayoutDirection d, Qt::Alignment a, qreal top, qreal bottom, qreal left, qreal right )
{
  Q_UNUSED( d );
  Q_UNUSED( a );
  Q_UNUSED( top );
  Q_UNUSED( bottom );
  Q_UNUSED( left );
  Q_UNUSED( right );
}

void PlainTextMarkupBuilder::endParagraph()
{
  appendRawText( QLatin1String( "\n" ) );
}

void PlainTextMarkupBuilder::beginIndent( int nBlockIndent, qreal nTextIndent, const QString &strClass )
{
  Q_UNUSED( strClass );
  Q_UNUSED( nBlockIndent );
  Q_UNUSED( nTextIndent );
}

void PlainTextMarkupBuilder::endIndent()
{
}

void PlainTextMarkupBuilder::beginTable( qreal cellpadding, qreal cellspacing, const QString& width )
{
  Q_UNUSED( cellpadding );
  Q_UNUSED( cellspacing );
  Q_UNUSED( width );
}

void PlainTextMarkupBuilder::beginTableCell( const QString& width, int colSpan, int rowSpan )
{
  Q_UNUSED( width );
  Q_UNUSED( colSpan );
  Q_UNUSED( rowSpan );
}

void PlainTextMarkupBuilder::beginTableHeaderCell( const QString& width, int colSpan, int rowSpan )
{
  Q_UNUSED( width );
  Q_UNUSED( colSpan );
  Q_UNUSED( rowSpan );
}

void PlainTextMarkupBuilder::beginTableRow()
{

}

void PlainTextMarkupBuilder::endBackground()
{

}

void PlainTextMarkupBuilder::endFont()
{

}

void PlainTextMarkupBuilder::endFontFamily()
{

}

void PlainTextMarkupBuilder::endFontPointSize()
{

}

void PlainTextMarkupBuilder::endForeground()
{

}

void PlainTextMarkupBuilder::endHeader( int level )
{
  Q_UNUSED( level )
}

void PlainTextMarkupBuilder::endTable()
{

}

void PlainTextMarkupBuilder::endTableCell()
{

}

void PlainTextMarkupBuilder::endTableHeaderCell()
{

}

void Grantlee::PlainTextMarkupBuilder::endTableRow()
{

}
