/****************************************************************************
**
** Copyright (C) 2013-2025 Donna Whisnant, a.k.a. Dewtronics.
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

#include "PassageReferenceResolver.h"

#include "SoundEx.h"

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#else
#include <QRegExp>
#endif
#include <QStringList>

#define PASSAGE_SOUNDEX_LENGTH 4
#define PASSAGE_SOUNDEX_MODE SoundEx::SEOME_ENHANCED

// ============================================================================

CPassageReferenceResolver::CPassageReferenceResolver(CBibleDatabasePtr pBibleDatabase, QObject *pParent)
	:	QObject(pParent),
		m_pBibleDatabase(pBibleDatabase)
{
	Q_ASSERT(!pBibleDatabase.isNull());
	buildSoundExTables();
}

TPhraseTag CPassageReferenceResolver::resolve(const QString &strPassageReference) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());
	if (m_pBibleDatabase.isNull()) return TPhraseTag();

	//	From: http://stackoverflow.com/questions/9974012/php-preg-match-bible-scripture-format
	//
	//	RegExp: "\w+\s?(\d{1,3})?(:\d{1,3})?([-–]\d{1,3})?(,\s\d{1,3}[-–]\d{1,3})?"
	//	"
	//	\w         # Match a single character that is a “word character” (letters, digits, and underscores)
	//	   +          # Between one and unlimited times, as many times as possible, giving back as needed (greedy)
	//	\s         # Match a single character that is a “whitespace character” (spaces, tabs, and line breaks)
	//	   ?          # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 1
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 2
	//	   :          # Match the character “:” literally
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 3
	//	   [-–]       # Match a single character present in the list “-–”
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	(          # Match the regular expression below and capture its match into backreference number 4
	//	   ,          # Match the character “,” literally
	//	   \s         # Match a single character that is a “whitespace character” (spaces, tabs, and line breaks)
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	   [-–]       # Match a single character present in the list “-–”
	//	   \d         # Match a single digit 0..9
	//	      {1,3}      # Between one and 3 times, as many times as possible, giving back as needed (greedy)
	//	)?         # Between zero and one times, as many times as possible, giving back as needed (greedy)
	//	"

	//
	// Modified version written by me:
	//		Returns 8 submatch parts, plus the main expression match.  capturedTexts() count will be 9.
	//			Format is:  "1 John 2:3-4,5-6 [7]"
	//			0 :	"1 John 2:3-4,5-6 [7]"
	//			1 : "1"
	//			2 : "John"
	//			3 : "2"
	//			4 : "3"
	//			5 : "4"
	//			6 : "5"
	//			7 : "6"
	//			8 : "7"
	//
	//		- If there is no pre-book number, (1) will be empty.
	//		- If there is no ":" and verse number, then (4) will be empty, for example: "1 John 2-3,4-5 [6]"
	//			0 : "1 John 2-3,4-5"
	//			1 : "1"
	//			2 : "John"
	//			3 : "2"
	//			4 : ""
	//			5 : "3"
	//			6 : "4"
	//			7 : "5"
	//			8 : "6"
	//		- (5) is empty if no initial range is given
	//		- (6) and (7) will be empty if there is no ","
	//		- (7) is empty if no secondary range is given
	//		- (8) will be the Word Number, the value in the "[]" tags, if specified, or empty if not
	//

#if QT_VERSION >= 0x050000
	QRegularExpression expReference("\\s*(\\d)?\\s*(\\w+)\\s*(\\d{1,3})?\\s*[:.]?\\s*(\\d{1,3})?\\s*[-–]?\\s*(\\d{1,3})?\\s*,?\\s*(\\d{1,3})?\\s*[-–]?\\s*(\\d{1,3})?\\s*\\[?\\s*(\\d{1,3})?\\s*\\]?", QRegularExpression::CaseInsensitiveOption);
#else
	QRegExp expReference("\\s*(\\d)?\\s*(\\w+)\\s*(\\d{1,3})?\\s*[:.]?\\s*(\\d{1,3})?\\s*[-–]?\\s*(\\d{1,3})?\\s*,?\\s*(\\d{1,3})?\\s*[-–]?\\s*(\\d{1,3})?\\s*\\[?\\s*(\\d{1,3})?\\s*\\]?", Qt::CaseInsensitive);
#endif

#define PARSENDX_ALL			0
#define PARSENDX_PREBOOK		1
#define PARSENDX_BOOK			2
#define PARSENDX_CHAPTER		3
#define PARSENDX_VERSE			4
#define PARSENDX_RANGEEND1		5
#define PARSENDX_RANGESTART2	6
#define PARSENDX_RANGEEND2		7
#define PARSENDX_WORD			8

#if QT_VERSION >= 0x050000
	QRegularExpressionMatch expRefMatch = expReference.match(strPassageReference);
	QStringList lstMatches = expRefMatch.capturedTexts();
	qsizetype nPos = expRefMatch.capturedStart();
	Q_ASSERT(expReference.captureCount() == 8);
	while (lstMatches.size() <= expReference.captureCount()) lstMatches.append(QString());
#else
	int nPos = expReference.indexIn(strPassageReference);
	QStringList lstMatches = expReference.capturedTexts();
#endif
	TPhraseTag tagResult;

#if 0
	qDebug("nPos=%d", nPos);
	for (int ndx=0; ndx<lstMatches.size(); ++ndx) {
		qDebug("Match %d : \"%s\"", ndx, lstMatches.at(ndx).toUtf8().data());
	}
#endif

	Q_ASSERT(lstMatches.size() == 9);

	if ((nPos != -1) && (lstMatches.size() == 9)) {
		CRelIndex ndxResolved;
		unsigned int nWordCount = 0;
		ndxResolved.setBook(resolveBook(lstMatches.at(PARSENDX_PREBOOK), lstMatches.at(PARSENDX_BOOK)));
		if (ndxResolved.book()) {
			const CBookEntry &book = *m_pBibleDatabase->bookEntry(ndxResolved.book());
			if (!lstMatches.at(PARSENDX_CHAPTER).isEmpty()) {
				// Resolve Chapter:
				ndxResolved.setChapter(lstMatches.at(PARSENDX_CHAPTER).toUInt());
				if ((ndxResolved.chapter() < 1) || (ndxResolved.chapter() > book.m_nNumChp)) ndxResolved.setChapter(0);
			} else {
				ndxResolved.setChapter(1);
			}
			if (ndxResolved.chapter()) {
				const CChapterEntry &chapter = *m_pBibleDatabase->chapterEntry(ndxResolved);
				if (!lstMatches.at(PARSENDX_VERSE).isEmpty()) {
					// Resolve Verse:
					ndxResolved.setVerse(lstMatches.at(PARSENDX_VERSE).toUInt());
					if ((ndxResolved.verse() < 1) || (ndxResolved.verse() > chapter.m_nNumVrs)) ndxResolved.setVerse(0);
				} else {
					ndxResolved.setVerse(1);
					ndxResolved.setWord(1);
					if (!lstMatches.at(PARSENDX_RANGEEND1).isEmpty()) {
						// Range of Chapters:
						uint32_t nLastChapter = lstMatches.at(PARSENDX_RANGEEND1).toUInt();
						if ((nLastChapter >= 1) && (nLastChapter <= book.m_nNumChp) && (nLastChapter >= ndxResolved.chapter())) {
							CRelIndex ndxLast(ndxResolved.book(), nLastChapter, 1, 1);
							const CChapterEntry &chapterLast = *m_pBibleDatabase->chapterEntry(ndxLast);
							ndxLast.setVerse(chapterLast.m_nNumVrs);
							const CVerseEntry &verseLast = *m_pBibleDatabase->verseEntry(ndxLast);
							ndxLast.setWord(verseLast.m_nNumWrd);
							nWordCount = m_pBibleDatabase->NormalizeIndex(ndxLast) - m_pBibleDatabase->NormalizeIndex(ndxResolved) + 1;
						}
					}
				}
				if (ndxResolved.verse() && (!lstMatches.at(PARSENDX_VERSE).isEmpty())) {
					const CVerseEntry &verse = *m_pBibleDatabase->verseEntry(ndxResolved);
					if (!lstMatches.at(PARSENDX_WORD).isEmpty()) {
						ndxResolved.setWord(lstMatches.at(PARSENDX_WORD).toUInt());
						if ((ndxResolved.word() < 1) || (ndxResolved.word() > verse.m_nNumWrd)) ndxResolved.setWord(0);
						nWordCount = 1;
					} else {
						ndxResolved.setWord(1);
					}
					if (!lstMatches.at(PARSENDX_RANGEEND1).isEmpty() && ndxResolved.word()) {
						// Range of Verses:
						uint32_t nLastVerse = lstMatches.at(PARSENDX_RANGEEND1).toUInt();
						if ((nLastVerse >= 1) && (nLastVerse <= chapter.m_nNumVrs) && (nLastVerse >= ndxResolved.verse())) {
							CRelIndex ndxLast = ndxResolved;
							ndxLast.setVerse(nLastVerse);
							const CVerseEntry &verseLast = *m_pBibleDatabase->verseEntry(ndxLast);
							ndxLast.setWord(verseLast.m_nNumWrd);
							nWordCount = m_pBibleDatabase->NormalizeIndex(ndxLast) - m_pBibleDatabase->NormalizeIndex(ndxResolved) + 1;
						}
					}
				}
			}
		}

		if ((ndxResolved.book() != 0) &&
			(ndxResolved.chapter() != 0) &&
			(ndxResolved.verse() != 0) &&
			(ndxResolved.word() != 0)) {
			tagResult = TPhraseTag(ndxResolved, nWordCount);
		}
	}

	return tagResult;
}

// ============================================================================

void CPassageReferenceResolver::buildSoundExTables()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());
	m_lstBookSoundEx.clear();
	m_lstBookSoundEx.reserve(m_pBibleDatabase->bibleEntry().m_nNumBk);

	for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(nBk);
		QStringList lstSoundEx;
		for (int nAbbr = 0; nAbbr <= book.m_lstBkAbbr.size(); ++nAbbr) {		// Index 0 will be used for entire book name others will index into abbr array
			QString strBookName = ((nAbbr == 0) ? book.m_strBkName.toLower() : book.m_lstBkAbbr.at(nAbbr-1).toLower());
#if QT_VERSION >= 0x050000
			strBookName.replace(QRegularExpression("\\s"), QString());
			QRegularExpression regexpPrefix("^(\\d*)?");
			QRegularExpressionMatch regexpPrefixMatch = regexpPrefix.match(strBookName);
			qsizetype nPosPrefix = regexpPrefixMatch.capturedStart();
			Q_ASSERT(nPosPrefix != -1);
			Q_ASSERT(regexpPrefixMatch.capturedTexts().size() == 2);
			QString strPrefix = regexpPrefixMatch.capturedTexts().at(1);
#else
			strBookName.replace(QRegExp("\\s"), QString());
			QRegExp regexpPrefix("^(\\d*)?");
			int nPosPrefix = regexpPrefix.indexIn(strBookName);
			Q_ASSERT(nPosPrefix != -1);
			Q_ASSERT(regexpPrefix.capturedTexts().size() == 2);
			QString strPrefix = regexpPrefix.capturedTexts().at(1);
#endif
			lstSoundEx.append(strPrefix + SoundEx::soundEx(strBookName,
																			SoundEx::languageValue(m_pBibleDatabase->langID()),
																			PASSAGE_SOUNDEX_LENGTH,
																			PASSAGE_SOUNDEX_MODE));
		}
		m_lstBookSoundEx.append(lstSoundEx);
	}
}

uint32_t CPassageReferenceResolver::resolveBook(const QString &strPreBook, const QString &strBook) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());
	QString strBookName = strPreBook.toLower() + strBook.toLower();
	QString strSoundEx = strPreBook + SoundEx::soundEx(strBookName,
																			 SoundEx::languageValue(m_pBibleDatabase->langID()),
																			 PASSAGE_SOUNDEX_LENGTH,
																			 PASSAGE_SOUNDEX_MODE);
	uint32_t nResolvedBook = 0;
	QList<uint32_t> lstResolvedBooks;
	for (unsigned int nBk = 1; nBk <= m_pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry &book = *m_pBibleDatabase->bookEntry(nBk);
		for (int nAbbr = 0; nAbbr <= book.m_lstBkAbbr.size(); ++nAbbr) {		// Index 0 will be used for entire book name others will index into abbr array
			QString strBibleBookName = ((nAbbr == 0) ? book.m_strBkName.toLower() : book.m_lstBkAbbr.at(nAbbr-1).toLower());
#if QT_VERSION >= 0x050000
			strBibleBookName.replace(QRegularExpression("\\s"), QString());
#else
			strBibleBookName.replace(QRegExp("\\s"), QString());
#endif
			if ((strSoundEx.compare(m_lstBookSoundEx.at(nBk-1).at(nAbbr)) == 0) || (strBibleBookName.startsWith(strBookName))) {
				lstResolvedBooks.append(nBk);
				break;			// Only need to add the book once
			}
		}
	}
	if (lstResolvedBooks.size() > 1) {
		bool bIsUnique = true;
		bool bExactMatch = false;
		for (int ndx = 0; ndx < lstResolvedBooks.size(); ++ndx) {
			const CBookEntry &book = *m_pBibleDatabase->bookEntry(lstResolvedBooks.at(ndx));
			for (int nAbbr = 0; nAbbr <= book.m_lstBkAbbr.size(); ++nAbbr) {		// Index 0 will be used for entire book name others will index into abbr array
				QString strBibleBookName = ((nAbbr == 0) ? book.m_strBkName.toLower() : book.m_lstBkAbbr.at(nAbbr-1).toLower());
#if QT_VERSION >= 0x050000
				strBibleBookName.replace(QRegularExpression("\\s"), QString());
#else
				strBibleBookName.replace(QRegExp("\\s"), QString());
#endif
				if (strBibleBookName.startsWith(strBookName)) {
					if ((nResolvedBook == 0) || (strBibleBookName.compare(strBookName) == 0)) {
						nResolvedBook = lstResolvedBooks.at(ndx);
						if (strBibleBookName.compare(strBookName) == 0) {
							bExactMatch = true;
							break;					// Stop if we get an exact match...
						}
					} else {
						bIsUnique = false;
					}
				}
			}
		}
		if ((!bIsUnique) && (!bExactMatch)) nResolvedBook = 0;
	} else if (lstResolvedBooks.size() == 1) {
		nResolvedBook = lstResolvedBooks.at(0);
	}

	return nResolvedBook;
}

// ============================================================================
