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

#include "texthtmlbuilder.h"

#include <QtCore/QList>
#include <QtGui/QTextDocument>

namespace Grantlee
{

class TextHTMLBuilderPrivate
{
public:
  TextHTMLBuilderPrivate( TextHTMLBuilder *b )
    : q_ptr( b )
  {

  }

  QList<QTextListFormat::Style> currentListItemStyles;
  QString m_text;

  TextHTMLBuilder *q_ptr;

  Q_DECLARE_PUBLIC( TextHTMLBuilder )

};

}

using namespace Grantlee;

TextHTMLBuilder::TextHTMLBuilder()
  : AbstractMarkupBuilder(), d_ptr( new TextHTMLBuilderPrivate( this ) )
{

}

TextHTMLBuilder::~TextHTMLBuilder()
{
  delete d_ptr;
}

void TextHTMLBuilder::beginStrong()
{
  appendRawText( QLatin1String( "<strong>" ) );
}

void TextHTMLBuilder::endStrong()
{
  appendRawText( QLatin1String( "</strong>" ) );
}

void TextHTMLBuilder::beginEmph()
{
  appendRawText( QLatin1String( "<em>" ) );
}

void TextHTMLBuilder::endEmph()
{
  appendRawText( QLatin1String( "</em>" ) );
}

void TextHTMLBuilder::beginUnderline()
{
  appendRawText( QLatin1String( "<u>" ) );
}

void TextHTMLBuilder::endUnderline()
{
  appendRawText( QLatin1String( "</u>" ) );
}

void TextHTMLBuilder::beginStrikeout()
{
  appendRawText( QLatin1String( "<s>" ) );
}

void TextHTMLBuilder::endStrikeout()
{
  appendRawText( QLatin1String( "</s>" ) );
}

void TextHTMLBuilder::beginForeground( const QBrush &brush )
{
  appendRawText( QString::fromLatin1( "<span style=\"color:%1;\">" ).arg( brush.color().name() ) );
}

void TextHTMLBuilder::endForeground()
{
  appendRawText( QLatin1String( "</span>" ) );
}

void TextHTMLBuilder::beginBackground( const QBrush &brush )
{
  appendRawText( QString::fromLatin1( "<span style=\"background-color:%1;\">" ).arg( brush.color().name() ) );
}

void TextHTMLBuilder::endBackground()
{
  appendRawText( QLatin1String( "</span>" ) );
}

void TextHTMLBuilder::beginAnchor( const QString &href, const QString &name )
{
  if ( !href.isEmpty() ) {
    if ( !name.isEmpty() ) {
	  appendRawText( QString::fromLatin1( "<a href=\"%1\" name=\"%2\">" ).arg( href ).arg( name ) );
    } else {
	  appendRawText( QString::fromLatin1( "<a href=\"%1\">" ).arg( href ) );
    }
  } else {
    if ( !name.isEmpty() ) {
	  appendRawText( QString::fromLatin1( "<a name=\"%1\">" ).arg( name ) );
    }
  }
}

void TextHTMLBuilder::endAnchor()
{
  appendRawText( QLatin1String( "</a>" ) );
}

void TextHTMLBuilder::beginFont(const QString &family, int size)
{
  appendRawText( QString::fromLatin1( "<span style=\"font-family:%1; font-size:%2pt;\">" ).arg( family ).arg( QString::number( size ) ) );
}

void TextHTMLBuilder::endFont()
{
  appendRawText( QLatin1String( "</span>" ) );
}

void TextHTMLBuilder::beginFontFamily( const QString &family )
{
  appendRawText( QString::fromLatin1( "<span style=\"font-family:%1;\">" ).arg( family ) );
}

void TextHTMLBuilder::endFontFamily()
{
  appendRawText( QLatin1String( "</span>" ) );
}

void TextHTMLBuilder::beginFontPointSize( int size )
{
  appendRawText( QString::fromLatin1( "<span style=\"font-size:%1pt;\">" ).arg( QString::number( size ) ) );
}

void TextHTMLBuilder::endFontPointSize()
{
  appendRawText( QLatin1String( "</span>" ) );
}

void TextHTMLBuilder::beginParagraph(Qt::LayoutDirection d, Qt::Alignment al, qreal topMargin, qreal bottomMargin, qreal leftMargin, qreal rightMargin )
{
  // Don't put paragraph tags inside li tags. Qt bug reported.
//     if (currentListItemStyles.size() != 0)
//     {
  QString styleString;
  if ( topMargin != 0 ) {
    styleString.append( QString::fromLatin1( "margin-top:%1;" ).arg( topMargin ) );
  }
  if ( bottomMargin != 0 ) {
    styleString.append( QString::fromLatin1( "margin-bottom:%1;" ).arg( bottomMargin ) );
  }
  if ( leftMargin != 0 ) {
    styleString.append( QString::fromLatin1( "margin-left:%1;" ).arg( leftMargin ) );
  }
  if ( rightMargin != 0 ) {
    styleString.append( QString::fromLatin1( "margin-right:%1;" ).arg( rightMargin ) );
  }

  // Using == doesn't work here.
  // Using bitwise comparison because an alignment can contain a vertical and a horizontal part.
  if (((al & Qt::AlignRight) && (d == Qt::LeftToRight)) ||
	  ((al & Qt::AlignLeft) && (d == Qt::RightToLeft))) {
	appendRawText( QLatin1String( "<p align=\"right\"" ) );
  } else if ( al & Qt::AlignHCenter ) {
	appendRawText( QLatin1String( "<p align=\"center\"" ) );
  } else if ( al & Qt::AlignJustify ) {
	appendRawText( QLatin1String( "<p align=\"justify\"" ) );
  } else if (((al & Qt::AlignLeft) && (d == Qt::LeftToRight)) ||
			 ((al & Qt::AlignRight) && (d == Qt::RightToLeft))) {
	appendRawText( QLatin1String( "<p align=\"left\"" ) );
  } else {
	appendRawText( QLatin1String( "<p" ) );
  }

  if (d == Qt::LeftToRight) {
	appendRawText( QLatin1String( " dir=\"ltr\"" ) );
  } else if (d == Qt::RightToLeft) {
	appendRawText( QLatin1String( " dir=\"rtl\"" ) );
  }

  if ( !styleString.isEmpty() ) {
	appendRawText( QLatin1String( " style=\"" ) + styleString + QLatin1Char( '\"' ) );
  }
  appendRawText( QLatin1String( ">" ) );
//     }
}

void TextHTMLBuilder::endParagraph()
{
  appendRawText( QLatin1String( "</p>\n" ) );
}

void TextHTMLBuilder::beginIndent( int nBlockIndent, qreal nTextIndent, const QString &strClass )
{
  if (strClass.isEmpty()) {
	  appendRawText(QString::fromLatin1("<div style=\"-qt-block-indent:%1; text-indent:%2px;\" >").arg(nBlockIndent).arg(nTextIndent));
  } else {
	  appendRawText(QString::fromLatin1("<div class=\"%1\" style=\"-qt-block-indent:%2; text-indent:%3px;\" >").arg(strClass).arg(nBlockIndent).arg(nTextIndent));
  }
}

void TextHTMLBuilder::endIndent()
{
  appendRawText(QString::fromLatin1("</div>"));
}

void TextHTMLBuilder::beginHeader( int level )
{
  switch ( level ) {
  case 1:
	appendRawText( QLatin1String( "<h1>" ) );
    break;
  case 2:
	appendRawText( QLatin1String( "<h2>" ) );
    break;
  case 3:
	appendRawText( QLatin1String( "<h3>" ) );
    break;
  case 4:
	appendRawText( QLatin1String( "<h4>" ) );
    break;
  case 5:
	appendRawText( QLatin1String( "<h5>" ) );
    break;
  case 6:
	appendRawText( QLatin1String( "<h6>" ) );
    break;
  default:
    break;
  }
}

void TextHTMLBuilder::endHeader( int level )
{
  switch ( level ) {
  case 1:
	appendRawText( QLatin1String( "</h1>" ) );
    break;
  case 2:
	appendRawText( QLatin1String( "</h2>" ) );
    break;
  case 3:
	appendRawText( QLatin1String( "</h3>" ) );
    break;
  case 4:
	appendRawText( QLatin1String( "</h4>" ) );
    break;
  case 5:
	appendRawText( QLatin1String( "</h5>" ) );
    break;
  case 6:
	appendRawText( QLatin1String( "</h6>" ) );
    break;
  default:
    break;
  }
}

void TextHTMLBuilder::addNewline()
{
  appendRawText( QLatin1String( "<p>&nbsp;</p>\n" ) );
}

void TextHTMLBuilder::addLineBreak()
{
  appendRawText( QLatin1String( "<br>\n") );
}

void TextHTMLBuilder::insertHorizontalRule( int width )
{
  if ( width != -1 ) {
	appendRawText( QString::fromLatin1( "<hr width=\"%1\" />\n" ).arg( width ) );
  }
  appendRawText( QLatin1String( "<hr />\n" ) );
}

void TextHTMLBuilder::insertImage( const QString &src, qreal width, qreal height )
{
  appendRawText( QString::fromLatin1( "<img src=\"%1\" " ).arg( src ) );
  if ( width != 0 ) appendRawText( QString::fromLatin1( "width=\"%2\" " ).arg( width ) );
  if ( height != 0 ) appendRawText( QString::fromLatin1( "height=\"%2\" " ).arg( height ) );
  appendRawText( QLatin1String( "/>" ) );
}

void TextHTMLBuilder::beginList( QTextListFormat::Style type )
{
  Q_D( TextHTMLBuilder );
  d->currentListItemStyles.append( type );
  switch ( type ) {
  case QTextListFormat::ListDisc:
	appendRawText( QLatin1String( "<ul type=\"disc\">\n" ) );
    break;
  case QTextListFormat::ListCircle:
	appendRawText( QLatin1String( "\n<ul type=\"circle\">\n" ) );
    break;
  case QTextListFormat::ListSquare:
	appendRawText( QLatin1String( "\n<ul type=\"square\">\n" ) );
    break;
  case QTextListFormat::ListDecimal:
	appendRawText( QLatin1String( "\n<ol type=\"1\">\n" ) );
    break;
  case QTextListFormat::ListLowerAlpha:
	appendRawText( QLatin1String( "\n<ol type=\"a\">\n" ) );
    break;
  case QTextListFormat::ListUpperAlpha:
	appendRawText( QLatin1String( "\n<ol type=\"A\">\n" ) );
    break;
  case QTextListFormat::ListLowerRoman:
	appendRawText( QLatin1String("\n<ol type=\"i\">\n") );
    break;
  case QTextListFormat::ListUpperRoman:
	appendRawText( QLatin1String("\n<ol type=\"I\">\n") );
    break;
  default:
    break;
  }
}
void TextHTMLBuilder::endList()
{
  Q_D( TextHTMLBuilder );
  switch ( d->currentListItemStyles.last() ) {
  case QTextListFormat::ListDisc:
  case QTextListFormat::ListCircle:
  case QTextListFormat::ListSquare:
	appendRawText( QLatin1String( "</ul>\n" ) );
    break;
  case QTextListFormat::ListDecimal:
  case QTextListFormat::ListLowerAlpha:
  case QTextListFormat::ListUpperAlpha:
  case QTextListFormat::ListLowerRoman:
  case QTextListFormat::ListUpperRoman:
	appendRawText( QLatin1String( "</ol>\n" ) );
    break;
  default:
    break;
  }
  d->currentListItemStyles.removeLast();
}
void TextHTMLBuilder::beginListItem()
{
  appendRawText( QLatin1String( "<li>" ) );
}

void TextHTMLBuilder::endListItem()
{
  appendRawText( QLatin1String( "</li>\n" ) );
}

void TextHTMLBuilder::beginSuperscript()
{
  appendRawText( QLatin1String( "<sup>" ) );
}

void TextHTMLBuilder::endSuperscript()
{
  appendRawText( QLatin1String( "</sup>" ) );
}

void TextHTMLBuilder::beginSubscript()
{
  appendRawText( QLatin1String( "<sub>" ) );
}

void TextHTMLBuilder::endSubscript()
{
  appendRawText( QLatin1String( "</sub>" ) );
}

void TextHTMLBuilder::beginTable( qreal cellpadding, qreal cellspacing, const QString &width )
{
  appendRawText( QString::fromLatin1( "<table cellpadding=\"%1\" cellspacing=\"%2\" width=\"%3\" border=\"1\">" )
                    .arg( cellpadding )
                    .arg( cellspacing )
                    .arg( width ) );
}

void TextHTMLBuilder::beginTableRow()
{
  appendRawText( QLatin1String( "<tr>" ) );
}

void TextHTMLBuilder::beginTableHeaderCell( const QString &width, int colspan, int rowspan )
{
  appendRawText( QString::fromLatin1( "<th width=\"%1\" colspan=\"%2\" rowspan=\"%3\">" ).arg( width ).arg( colspan ).arg( rowspan ) );
}

void TextHTMLBuilder::beginTableCell( const QString &width, int colspan, int rowspan )
{
  appendRawText( QString::fromLatin1( "<td width=\"%1\" colspan=\"%2\" rowspan=\"%3\">" ).arg( width ).arg( colspan ).arg( rowspan ) );
}

void TextHTMLBuilder::endTable()
{
  appendRawText( QLatin1String( "</table>" ) );
}

void TextHTMLBuilder::endTableRow()
{
  appendRawText( QLatin1String( "</tr>" ) );
}

void TextHTMLBuilder::endTableHeaderCell()
{
  appendRawText( QLatin1String( "</th>" ) );
}

void TextHTMLBuilder::endTableCell()
{
  appendRawText( QLatin1String( "</td>" ) );
}

void TextHTMLBuilder::appendLiteralText( const QString &text )
{
  appendRawText( escape( text ) );
}

const QString TextHTMLBuilder::escape( const QString &s ) const
{
#if QT_VERSION < 0x050000
	return Qt::escape( s );
#else
	return s.toHtmlEscaped();
#endif
}

void TextHTMLBuilder::appendRawText( const QString &text )
{
  Q_D( TextHTMLBuilder );
  d->m_text.append( text );
}

QString TextHTMLBuilder::getResult()
{
  Q_D( TextHTMLBuilder );
  QString ret = d->m_text;
  d->m_text.clear();
  return ret;
}
