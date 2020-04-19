/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef CSV_H
#define CSV_H

#include <QList>
#include <QStringList>
#include <QIODevice>
#include <QBuffer>
#include <QTextStream>
#include <QScopedPointer>

// ============================================================================

class CCSVStream {
public:
	CCSVStream();
	CCSVStream(QIODevice *pIOD);
	CCSVStream(QString *pStr, QIODevice::OpenMode nFilemode);

	QIODevice *device() const { return m_stream.device(); }
	void setDevice(QIODevice *pIOD) { m_stream.setDevice(pIOD); }
	void unsetDevice() { m_stream.setDevice(NULL); }

	bool atEndOfStream() const { return m_stream.atEnd(); }
	bool atEndOfLine() const { return m_bEndOfLine; }
	bool atEnding() const { return atEndOfStream() || atEndOfLine(); }
	bool atBeginningOfLine() const { return m_bBeginningOfLine; }

	// Excel seems to do identical quoting for tab-delimited text and
	//	seemingly other delimiters like '|', so allow changing the delimiter:
	QChar delimiter() const { return m_chrDelim; }
	void setDelimiter(QChar ch) { m_chrDelim = ch; }

	CCSVStream &endLine();
	static QString escape(const QString &aString, bool bForceQuote = false, QChar chrDelim = L',');

	// template overloads allowing this class to handle any data types that QTextStream can:
	template <class T> CCSVStream &operator <<(const T &val) {
		QBuffer tmpBuff;
		tmpBuff.open(QIODevice::WriteOnly);
		QTextStream aStream(&tmpBuff);
		aStream.setCodec("UTF-16");
		aStream << val;
		tmpBuff.close();
		return (*this << QString((QChar *)tmpBuff.buffer().data(), tmpBuff.buffer().size() / sizeof(QChar)));
	}
	template <class T> CCSVStream &operator >>(T &val) {
		QString strTemp;
		CCSVStream &retVal = (*this >> strTemp);
		QByteArray data(reinterpret_cast<const char *>(strTemp.utf16()), strTemp.size() * sizeof(ushort));
		QBuffer tmpBuff(&data);
		tmpBuff.open(QIODevice::ReadOnly);
		QTextStream aStream(&tmpBuff);
		aStream.setCodec("UTF-16");
		aStream >> val;
		tmpBuff.close();
		return retVal;
	}

	CCSVStream &operator <<(const QString &aString);			// Write individual fields
	CCSVStream &operator >>(QString &aString);					// Read individual fields
	CCSVStream &operator <<(const QStringList &aStringList);	// Write individual rows
	CCSVStream &operator >>(QStringList &aStringList);			// Read individual rows

	QList<QStringList> readAll();								// Read entire file
	void writeAll(const QList<QStringList> &lstStringLists);	// Write entire file

private:
	QTextStream m_stream;
	bool m_bEndOfLine;					// Used primarily for reading
	bool m_bBeginningOfLine;			// Used primarily for writing
	QString m_strUngetBuff;
	QChar m_chrDelim;
};

// ============================================================================

extern CCSVStream &endl(CCSVStream &aStream);		// insert EOL ('\n') into stream
extern CCSVStream &flush(CCSVStream &aStream);		// flush stream output

typedef CCSVStream& (*TCSVFUNC)(CCSVStream&);		// manipulator function
inline CCSVStream& operator<<(CCSVStream &aStream, TCSVFUNC aFunction)
{
	return (*aFunction)(aStream);
}

// ============================================================================

// This class allows a stack-scope to create a new CSVStream to assign to the
//		main object scoped value, making it accessible to other member functions,
//		and yet be guaranteed to be cleaned up, both for the local stack and
//		the member object -- i.e. it's a way to clean up the member object without
//		resorting to signals and slots and QObject...

class CScopedCSVStream {
public:
	CScopedCSVStream(QScopedPointer<CCSVStream> &scopedCSV, CCSVStream *pCSVStream)
		:	m_scopedCSV(scopedCSV)
	{
		m_scopedCSV.reset(pCSVStream);
	}

	~CScopedCSVStream()
	{
		m_scopedCSV.reset();
	}

private:
	QScopedPointer<CCSVStream> &m_scopedCSV;
};

// ============================================================================

#endif // CSV_H
