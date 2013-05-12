/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "VerseRichifier.h"
#include "dbstruct.h"
#include "../KJVCanOpener/ParseSymbols.h"

#define OUTPUT_HEBREW_PS119 1
#define PSALMS_BOOK_NUM 19

// ============================================================================
// ============================================================================

static QString psalm119HebrewPrefix(const CRelIndex &ndx, bool bAddAnchors)
{
	if ((ndx.book() != PSALMS_BOOK_NUM) || (ndx.chapter() != 119) || (((ndx.verse()-1)%8) != 0)) return QString();

	QString strHebrewPrefix;

	// Add special Start tag so KJVBrowser can know to ignore the special Hebrew text insertion during highlighting:
	if (bAddAnchors) strHebrewPrefix += QString("<a id=\"\"A%1\"\"> </a>").arg(ndx.asAnchor());

#if (OUTPUT_HEBREW_PS119)
	switch ((ndx.verse()-1)/8) {
		case 0:
			// ALEPH
			strHebrewPrefix += QChar(0x005D0);
			break;
		case 1:
			// BETH
			strHebrewPrefix += QChar(0x005D1);
			break;
		case 2:
			// GIMEL
			strHebrewPrefix += QChar(0x005D2);
			break;
		case 3:
			// DALETH
			strHebrewPrefix += QChar(0x005D3);
			break;
		case 4:
			// HE
			strHebrewPrefix += QChar(0x005D4);
			break;
		case 5:
			// VAU
			strHebrewPrefix += QChar(0x005D5);
			break;
		case 6:
			// ZAIN
			strHebrewPrefix += QChar(0x005D6);
			break;
		case 7:
			// CHETH
			strHebrewPrefix += QChar(0x005D7);
			break;
		case 8:
			// TETH
			strHebrewPrefix += QChar(0x005D8);
			break;
		case 9:
			// JOD
			strHebrewPrefix += QChar(0x005D9);
			break;
		case 10:
			// CAPH
			strHebrewPrefix += QChar(0x005DB);		// Using nonfinal-CAPH
			break;
		case 11:
			// LAMED
			strHebrewPrefix += QChar(0x005DC);
			break;
		case 12:
			// MEM
			strHebrewPrefix += QChar(0x005DE);		// Using nonfinal-Mem
			break;
		case 13:
			// NUN
			strHebrewPrefix += QChar(0x005E0);		// Using nonfinal-Nun
			break;
		case 14:
			// SAMECH
			strHebrewPrefix += QChar(0x005E1);
			break;
		case 15:
			// AIN
			strHebrewPrefix += QChar(0x005E2);
			break;
		case 16:
			// PE
			strHebrewPrefix += QChar(0x005E4);		// Using nonfinal-Pe
			break;
		case 17:
			// TZADDI
			strHebrewPrefix += QChar(0x005E6);		// Using nonfinal-Tzaddi
			break;
		case 18:
			// KOPH
			strHebrewPrefix += QChar(0x005E7);
			break;
		case 19:
			// RESH
			strHebrewPrefix += QChar(0x005E8);
			break;
		case 20:
			// SCHIN
			strHebrewPrefix += QChar(0x005E9);
			break;
		case 21:
			// TAU
			strHebrewPrefix += QChar(0x005EA);
			break;
	}
	strHebrewPrefix += " ";
#endif

	switch ((ndx.verse()-1)/8) {
		case 0:
			strHebrewPrefix += "(ALEPH).";
			break;
		case 1:
			strHebrewPrefix += "(BETH).";
			break;
		case 2:
			strHebrewPrefix += "(GIMEL).";
			break;
		case 3:
			strHebrewPrefix += "(DALETH).";
			break;
		case 4:
			strHebrewPrefix += "(HE).";
			break;
		case 5:
			strHebrewPrefix += "(VAU).";
			break;
		case 6:
			strHebrewPrefix += "(ZAIN).";
			break;
		case 7:
			strHebrewPrefix += "(CHETH).";
			break;
		case 8:
			strHebrewPrefix += "(TETH).";
			break;
		case 9:
			strHebrewPrefix += "(JOD).";
			break;
		case 10:
			strHebrewPrefix += "(CAPH).";
			break;
		case 11:
			strHebrewPrefix += "(LAMED).";
			break;
		case 12:
			strHebrewPrefix += "(MEM).";
			break;
		case 13:
			strHebrewPrefix += "(NUN).";
			break;
		case 14:
			strHebrewPrefix += "(SAMECH).";
			break;
		case 15:
			strHebrewPrefix += "(AIN).";
			break;
		case 16:
			strHebrewPrefix += "(PE).";
			break;
		case 17:
			strHebrewPrefix += "(TZADDI).";
			break;
		case 18:
			strHebrewPrefix += "(KOPH).";
			break;
		case 19:
			strHebrewPrefix += "(RESH).";
			break;
		case 20:
			strHebrewPrefix += "(SCHIN).";
			break;
		case 21:
			strHebrewPrefix += "(TAU).";
			break;
	}

	// Add special End tag so KJVBrowser can know to ignore the special Hebrew text insertion during highlighting:
	if (bAddAnchors) strHebrewPrefix += QString("<a id=\"\"B%1\"\"> </a>").arg(ndx.asAnchor());

	return strHebrewPrefix;
}

// ============================================================================
// ============================================================================

CVerseTextRichifier::CVerseTextRichifier(const CRelIndex &ndxRelative, const QChar &chrMatchChar, const QString &strXlateText, const CVerseTextRichifier *pRichNext)
	:	m_pRichNext(pRichNext),
		m_chrMatchChar(chrMatchChar),
		m_pVerse(NULL),
		m_strXlateText(strXlateText),
		m_bAddAnchors(false),
		m_ndxCurrent(ndxRelative)
{

}

CVerseTextRichifier::CVerseTextRichifier(const CRelIndex &ndxRelative, const QChar &chrMatchChar, const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext, bool bAddAnchors)
	:	m_pRichNext(pRichNext),
		m_chrMatchChar(chrMatchChar),
		m_pVerse(pVerse),
		m_bAddAnchors(bAddAnchors),
		m_ndxCurrent(ndxRelative)
{
	assert(pVerse != NULL);
}

CVerseTextRichifier::~CVerseTextRichifier()
{

}

QString CVerseTextRichifier::parse(CRichifierBaton &parseBaton, const QString &strNodeIn) const
{
	if (m_chrMatchChar.isNull()) return strNodeIn;

	QString strTemp;
	QStringList lstSplit;

	if (m_pVerse != NULL) {
		lstSplit = m_pVerse->m_strTemplate.split(m_chrMatchChar);
		assert(static_cast<unsigned int>(lstSplit.size()) == (m_pVerse->m_nNumWrd + 1));
		assert(strNodeIn.isNull());
	} else {
		lstSplit = strNodeIn.split(m_chrMatchChar);
	}
	assert(lstSplit.size() != 0);

	for (int i=0; i<lstSplit.size(); ++i) {
		if (i > 0) {
			if (m_pVerse != NULL) {
#ifdef OSIS_PARSER_BUILD
				QString strWord = m_pVerse->m_lstRichWords.at(i-1);
#else
				QString strWord = parseBaton.m_pBibleDatabase->wordAtIndex(m_pVerse->m_nWrdAccum - m_pVerse->m_nNumWrd + i);
#endif
				if (m_bAddAnchors) strTemp += QString("<a id=\"%1\">").arg(CRelIndex(m_ndxCurrent.index() + i).asAnchor());
				if (!parseBaton.m_strDivineNameFirstLetterParseText.isEmpty()) {
					strTemp += strWord.left(1)
							+ parseBaton.m_strDivineNameFirstLetterParseText
							+ strWord.mid(1);
					parseBaton.m_strDivineNameFirstLetterParseText.clear();
				} else {
					strTemp += strWord;
				}
				if (m_bAddAnchors) strTemp += "</a>";
			} else {
				if (m_chrMatchChar == QChar('D')) {
					parseBaton.m_strDivineNameFirstLetterParseText = m_strXlateText;
				} else {
					strTemp += m_strXlateText;
				}
			}
		}
		if (m_pRichNext) {
			strTemp += m_pRichNext->parse(parseBaton, lstSplit.at(i));
		} else {
			strTemp += lstSplit.at(i);
		}
	}

	return strTemp;
}

QString CVerseTextRichifier::parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse, const CVerseTextRichifierTags &tags, bool bAddAnchors)
{
	assert(pBibleDatabase != NULL);
	assert(pVerse != NULL);

	// Note: While it would be most optimum to reverse this and
	//		do the verse last so we don't have to call the entire
	//		tree for every word, we can't reverse it because doing
	//		so then creates sub-lists of 'w' tags and then we
	//		no longer know where we are in the list:
	CVerseTextRichifier rich_d(ndxRelative, 'd', tags.divineNameEnd());
	CVerseTextRichifier rich_D(ndxRelative, 'D', tags.divineNameBegin(), &rich_d);				// D/d must be last for font start/stop to work correctly with special first-letter text mode
	CVerseTextRichifier rich_t(ndxRelative, 't', tags.transChangeAddedEnd(), &rich_D);
	CVerseTextRichifier rich_T(ndxRelative, 'T', tags.transChangeAddedBegin(), &rich_t);
	CVerseTextRichifier rich_j(ndxRelative, 'j', tags.wordsOfJesusEnd(), &rich_T);
	CVerseTextRichifier rich_J(ndxRelative, 'J', tags.wordsOfJesusBegin(), &rich_j);
	CVerseTextRichifier rich_M(ndxRelative, 'M', (tags.addRichPs119HebrewPrefix() ? psalm119HebrewPrefix(ndxRelative, bAddAnchors) : ""), &rich_J);
	CVerseTextRichifier richVerseText(ndxRelative, 'w', pVerse, &rich_M, bAddAnchors);

	CRichifierBaton baton(pBibleDatabase);
	QString strTemp = richVerseText.parse(baton);
	if ((pVerse->m_nPilcrow == CVerseEntry::PTE_MARKER) ||
		(pVerse->m_nPilcrow == CVerseEntry::PTE_MARKER_ADDED))
		strTemp = g_chrPilcrow + strTemp;
	return strTemp;
}

// ============================================================================
// ============================================================================

