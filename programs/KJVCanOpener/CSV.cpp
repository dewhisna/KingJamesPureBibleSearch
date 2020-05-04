/****************************************************************************
**
** Copyright (C) 2012-2020 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

#include "CSV.h"
#include <QFile>

#include <assert.h>

// ============================================================================

CCSVStream &endl(CCSVStream &aStream)
{
	return aStream.endLine();
}

CCSVStream &flush(CCSVStream &aStream)
{
	if (aStream.device()) {
		QFile *pFile = qobject_cast<QFile *>(aStream.device());
		assert(pFile != NULL);
		pFile->flush();
	}
	return aStream;
}

// ============================================================================

CCSVStream::CCSVStream()
	:	m_bEndOfLine(false),
		m_bBeginningOfLine(true),
		m_chrDelim(L',')
{
	m_stream.setCodec("UTF-8");
}

CCSVStream::CCSVStream(QIODevice *pIOD)
	:	m_stream(pIOD),
		m_bEndOfLine(false),
		m_bBeginningOfLine(true),
		m_chrDelim(L',')
{
	m_stream.setCodec("UTF-8");
}

CCSVStream::CCSVStream(QString *pStr, QIODevice::OpenMode nFilemode)
	:	m_stream(pStr, nFilemode),
		m_bEndOfLine(false),
		m_bBeginningOfLine(true),
		m_chrDelim(L',')
{
	m_stream.setCodec("UTF-8");
}

CCSVStream &CCSVStream::endLine()
{
	m_stream << endl;
	m_bBeginningOfLine = true;
	return *this;
}

// Based on the field quoting rules of http://www.creativyst.com/Doc/Articles/CSV/CSV01.htm
// conforms to RFC4180 (definition of text/csv) except for ignoring unquoted leading whitespace
QString CCSVStream::escape(const QString &aString, bool bForceQuote, QChar chrDelim)
{
	bool bQuoted = false;
	QString strOut;

	if ((!aString.isEmpty()) && ((aString.at(0) == ' ') || (aString.at(0) == '\t')))
		bQuoted = true;		// Leading whitespace must to be quoted

	for (int i=0; i<aString.size(); ++i) {
		QChar ch = aString.at(i);
		if (ch == '"') {
			strOut.append(ch);		// Insert quote an extra time to escape
			bQuoted = true;
		} else if ((ch == chrDelim) || (ch == '\n')) {
			bQuoted = true;
		}
		strOut.append(ch);
	}
	if ((bForceQuote) || (bQuoted) || (strOut.isEmpty() && (!aString.isNull()))) {
		strOut.prepend('"');
		strOut.append('"');
	}
	return strOut;
}

CCSVStream &CCSVStream::operator <<(const QString &aString)
{
	bool bForceQuote = false;

	// Excel will (rather stupidly) detect a file as SYLK instead of CSV
	// if it begins with the characters ID. We'll force quoting of the cell
	// (so it starts with "ID instead) if this looks like a potential problem.
	// See http://support.microsoft.com/kb/323626
	if ((m_stream.device()) && (m_stream.device()->pos() == 0) && (aString.left(2) == "ID"))
		bForceQuote = true;

	QString strOut = escape(aString, bForceQuote, m_chrDelim);

	if (!m_bBeginningOfLine) m_stream << m_chrDelim;
	m_bBeginningOfLine = false;

	m_stream << strOut;
	return *this;
}

CCSVStream &CCSVStream::operator >>(QString &aString)
{
	m_bEndOfLine = false;
	aString = QString();

	bool bQuoted = false;
	bool bQuoteLiteral = false;
	bool bNonWhitespaceSeen = false;
	bool bEndOfField = ((m_stream.atEnd()) && (m_strUngetBuff.isEmpty()));

	QChar ch;
	while (!bEndOfField) {
		if (m_strUngetBuff.size()) {
			ch = m_strUngetBuff.at(0);
			m_strUngetBuff = m_strUngetBuff.mid(1);
		} else {
			m_stream >> ch;
		}

		if (!bNonWhitespaceSeen) {		// Quoting can only begin at the first character
			if ((ch == ' ') || (ch == '\t')) {
				continue; // ignore all leading whitespace
			} else {
				bNonWhitespaceSeen = true;
			}
			if (ch == '"') {
				aString = "";		// not a null field anymore, but still an empty string
				bQuoted = true;
				continue;
			}
		}

		if (ch == '"') {
			if (!bQuoted) {
				// no special treatment in this case
			} else if (bQuoteLiteral) {
				bQuoteLiteral = false;
			} else {
				bQuoteLiteral = true;
				continue;
			}
		} else if (bQuoteLiteral) {		// Un-doubled " in quoted string, that's the end of the quoted portion
			bQuoted = false;
			bQuoteLiteral = true;
		}

		if (!bQuoted) {
			// handle linefeed variations - CR or CRLF or LF all get consumed in one step, and returned as '\n'
			if (ch == '\x0D') {			// CR
				m_stream >> ch;
				if (ch == '\x0A') {		// windows-style CRLF
					ch = '\n';
				} else {				// Non-LF, guess it was a Mac-style bare CR
					m_strUngetBuff.prepend(ch);
					ch = '\n';
				}
			} else if (ch == '\x0A') {	// Unix-style LF
				ch = '\n';
			}
		}

		if ((!bQuoted) && (ch == m_chrDelim)) {
			bEndOfField = true;
		} else if (((!bQuoted) && (ch == '\n')) || ((m_stream.atEnd()) && (m_strUngetBuff.isEmpty()))) {
			if (ch != '\n') aString.append(ch);
			bEndOfField = true;
			m_bEndOfLine = true;
		} else {
			aString.append(ch);
		}
	}
	return *this;
}

CCSVStream &CCSVStream::operator <<(const QStringList &aStringList)
{
	for (QStringList::const_iterator itr = aStringList.begin(); itr != aStringList.end(); ++itr)
		*this << *itr;
	return (*this << endl);
}

CCSVStream &CCSVStream::operator >>(QStringList &aStringList)
{
	aStringList.clear();
	do {
		QString strTemp;
		*this >> strTemp;
		aStringList.append(strTemp);
	} while(!atEndOfLine());
	return *this;
}

QList<QStringList> CCSVStream::readAll()
{
	QList<QStringList> lstStringLists;
	while (!atEndOfStream()) {
		QStringList aStringList;
		*this >> aStringList;
		lstStringLists.append(aStringList);
	}
	return lstStringLists;
}

void CCSVStream::writeAll(const QList<QStringList> &lstStringLists)
{
	foreach(const QStringList aList, lstStringLists) {
		*this << aList;
	}
}

// ============================================================================

