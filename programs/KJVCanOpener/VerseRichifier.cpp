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
#include "ParseSymbols.h"

#include "PersistentSettings.h"
#include "PhraseEdit.h"
#include "Highlighter.h"

#include <QRegExp>

#define OUTPUT_HEBREW_PS119 1
#define PSALMS_BOOK_NUM 19

// ============================================================================
// ============================================================================

static QString psalm119HebrewPrefix(const CRelIndex &ndx, bool bAddAnchors)
{
	if ((ndx.book() != PSALMS_BOOK_NUM) || (ndx.chapter() != 119) || (((ndx.verse()-1)%8) != 0)) return QString();

	QString strHebrewPrefix;

	// Add special Start tag so KJVBrowser can know to ignore the special Hebrew text insertion during highlighting:
	if (bAddAnchors) strHebrewPrefix += QString("<a id=\"A%1\"> </a>").arg(ndx.asAnchor());

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

	if (bAddAnchors) {
		// Add special End tag so KJVBrowser can know to ignore the special Hebrew text insertion during highlighting:
		strHebrewPrefix += QString("<a id=\"B%1\"> </a>").arg(ndx.asAnchor());
	} else {
		// Otherwise, just add a space:
		strHebrewPrefix += " ";
	}

	return strHebrewPrefix;
}

// ============================================================================
// ============================================================================

CVerseTextRichifier::CVerseTextRichifier(const QChar &chrMatchChar, const QString &strXlateText, const CVerseTextRichifier *pRichNext)
	:	m_pRichNext(pRichNext),
		m_chrMatchChar(chrMatchChar),
		m_pVerse(NULL),
		m_strXlateText(strXlateText),
		m_bAddAnchors(false),
		m_bUseLemmas(false)
{

}

CVerseTextRichifier::CVerseTextRichifier(const QChar &chrMatchChar, const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext, bool bAddAnchors, bool bUseLemmas)
	:	m_pRichNext(pRichNext),
		m_chrMatchChar(chrMatchChar),
		m_pVerse(pVerse),
		m_bAddAnchors(bAddAnchors),
		m_bUseLemmas(bUseLemmas)
{
	assert(pVerse != NULL);
}

CVerseTextRichifier::~CVerseTextRichifier()
{

}

void CVerseTextRichifier::writeLemma(CRichifierBaton &parseBaton) const
{
	if (!m_bUseLemmas) return;
	if (!parseBaton.m_bUsesHTML) return;

	// Note: This finishes off the word itself too:
	if (parseBaton.m_bOutput && parseBaton.m_bUsesHTML) {
		if (parseBaton.m_pCurrentLemma) {
			parseBaton.m_strVerseText.append(QString("</span><span class=\"stack\">%1&nbsp;</span><span class=\"stack\">%2&nbsp;</span><span class=\"stack\">%3&nbsp;</span>")
												.arg(parseBaton.m_pCurrentLemma->text().join(' '))
												.arg(parseBaton.m_pCurrentLemma->strongs().join(' '))
												.arg(parseBaton.m_pCurrentLemma->morph().join(' ')));
		} else {
			parseBaton.m_strVerseText.append(QString("</span><span class=\"stack\">&nbsp;</span><span class=\"stack\">&nbsp;</span><span class=\"stack\">&nbsp;</span>"));
		}
	}
}

void CVerseTextRichifier::parse(CRichifierBaton &parseBaton, const QString &strNodeIn) const
{
	if (m_chrMatchChar.isNull()) {
		parseBaton.m_strVerseText.append(strNodeIn);
		return;
	}

	QStringList lstSplit;

	if (m_pVerse != NULL) {
		lstSplit.reserve(m_pVerse->m_nNumWrd + 1);
		lstSplit = parseBaton.m_strTemplate.split(m_chrMatchChar);
		assert(static_cast<unsigned int>(lstSplit.size()) == (m_pVerse->m_nNumWrd + 1));
		assert(strNodeIn.isNull());
	} else {
		lstSplit = strNodeIn.split(m_chrMatchChar);
	}
	assert(lstSplit.size() != 0);

	bool bStartedVerseOutput = false;		// Set to true when we first start writing output on this verse

	for (int i=0; i<lstSplit.size(); ++i) {
		if (m_pVerse != NULL) {
			bool bOldOutputStatus = parseBaton.m_bOutput;
			parseBaton.m_bOutput = (static_cast<unsigned int>(i) >= parseBaton.m_nStartWord);
			if ((parseBaton.m_pWordCount != NULL) && ((*parseBaton.m_pWordCount) == 0)) parseBaton.m_bOutput = false;

			if (bOldOutputStatus && !parseBaton.m_bOutput && parseBaton.m_bUsesHTML && m_bUseLemmas && (parseBaton.m_pCurrentLemma != nullptr)) {
				// If we transitioned out of output and lemma, finish writing
				//	the lemma and close it out:
				writeLemma(parseBaton);
				parseBaton.m_pCurrentLemma = nullptr;
			}
		}
		if (i > 0) {
			if (m_pVerse != NULL) {
#ifdef OSIS_PARSER_BUILD
				QString strWord = m_pVerse->m_lstRichWords.at(i-1);
#else
				QString strWord = parseBaton.m_pBibleDatabase->wordAtIndex(m_pVerse->m_nWrdAccum + i);
#endif
				parseBaton.m_ndxCurrent.setWord(i);
				bool bWasInLemma = (parseBaton.m_pCurrentLemma != nullptr);
				if (parseBaton.m_bOutput) {
					if (!bWasInLemma || !m_bUseLemmas) {
						if (bStartedVerseOutput && parseBaton.m_bUsesHTML) {
							// If not in a Lemma, we need to end this word:
							if (parseBaton.m_pCurrentLemma == nullptr) {
								if (m_bUseLemmas) {
									writeLemma(parseBaton);		// Write empty lemma
								} else {
									parseBaton.m_strVerseText.append(QString("</span>"));	// End stack span
								}
								parseBaton.m_strVerseText.append(QString("</span>"));		// End word span
							}
						}
						bStartedVerseOutput = true;

						// If not currently in a Lemma or not even processing Lemmas, we
						//	need to write the word span:
						if (parseBaton.m_bUsesHTML) {
							parseBaton.m_strVerseText.append(QString("<span class=\"word\">"));
							parseBaton.m_strVerseText.append(QString("<span class=\"stack\">"));
						}
						if (m_bUseLemmas) {
							parseBaton.m_pCurrentLemma = parseBaton.m_pBibleDatabase->lemmaEntry(parseBaton.m_ndxCurrent);
						}
					} else if (bWasInLemma && (!parseBaton.m_pCurrentLemma->tag().intersects(parseBaton.m_pBibleDatabase, TPhraseTag(parseBaton.m_ndxCurrent)))) {
						// If we were in a lemma but no longer are, we need to write the
						//	end of the current lemma:
						writeLemma(parseBaton);
						// Check for next lemma:
						parseBaton.m_pCurrentLemma = parseBaton.m_pBibleDatabase->lemmaEntry(parseBaton.m_ndxCurrent);
						if (parseBaton.m_bUsesHTML) {
							// End word span:
							parseBaton.m_strVerseText.append(QString("</span>"));
							// Start next word segment, regardless of whether or not we are in a Lemma:
							parseBaton.m_strVerseText.append(QString("<span class=\"word\">"));
							parseBaton.m_strVerseText.append(QString("<span class=\"stack\">"));
						}
					}	// Otherwise, we are still in a Lemma and need to continue to output it...

					parseBaton.m_strVerseText.append(parseBaton.m_strPrewordStack);
					parseBaton.m_strPrewordStack.clear();

					if (m_bAddAnchors && parseBaton.m_bUsesHTML) parseBaton.m_strVerseText.append(QString("<a id=\"%1\">").arg(parseBaton.m_ndxCurrent.asAnchor()));
				}
				if (!parseBaton.m_strDivineNameFirstLetterParseText.isEmpty()) {
					if (parseBaton.m_bOutput) {
						parseBaton.m_strVerseText.append(strWord.left(1)
														+ parseBaton.m_strDivineNameFirstLetterParseText
														+ strWord.mid(1));
					}
					parseBaton.m_strDivineNameFirstLetterParseText.clear();
				} else {
					if (parseBaton.m_bOutput) parseBaton.m_strVerseText.append(strWord);
				}
				if (parseBaton.m_bOutput && parseBaton.m_bUsesHTML) {
					if (m_bAddAnchors) parseBaton.m_strVerseText.append("</a>");
				}
				if ((parseBaton.m_bOutput) && (parseBaton.m_pWordCount != NULL) && ((*parseBaton.m_pWordCount) > 0)) --(*parseBaton.m_pWordCount);
			} else {
				if (m_chrMatchChar == QChar('D')) {
					parseBaton.m_strDivineNameFirstLetterParseText = m_strXlateText;
				} else if (m_chrMatchChar == QChar('R')) {
					assert(parseBaton.m_pHighlighter != NULL);
					// Note: for searchResult, we always have to check the intersection and handle
					//		enter/exit of m_bInSearchResult since we are called to parse twice -- once
					//		for the begin tags and once for the end tags.  Otherwise we don't know when
					//		to start/stop and which to output:
					CRelIndex ndxWord = parseBaton.m_ndxCurrent;
					ndxWord.setWord(ndxWord.word()+1);
					if ((parseBaton.m_bOutput) &&
						(!parseBaton.m_bInSearchResult) &&
						(parseBaton.m_pHighlighter->intersects(parseBaton.m_pBibleDatabase, TPhraseTag(ndxWord)))) {
						parseBaton.m_strPrewordStack.append(m_strXlateText);
						parseBaton.m_bInSearchResult = true;
					}
				} else if (m_chrMatchChar == QChar('r')) {
					assert(parseBaton.m_pHighlighter != NULL);
					CRelIndex ndxWord = parseBaton.m_ndxCurrent;
					ndxWord.setWord(ndxWord.word()+1);
					if ((parseBaton.m_bOutput) &&
						(parseBaton.m_bInSearchResult) &&
						((!parseBaton.m_pHighlighter->isContinuous()) ||
							(!parseBaton.m_pHighlighter->intersects(parseBaton.m_pBibleDatabase, TPhraseTag(ndxWord))))) {
						parseBaton.m_strVerseText.append(m_strXlateText);
						parseBaton.m_bInSearchResult = false;
					}
				} else if (m_chrMatchChar == QChar('N')) {
					CRelIndex ndxWord = parseBaton.m_ndxCurrent;
					ndxWord.setWord(ndxWord.word()+1);
					if (ndxWord.word() > 1) parseBaton.m_strVerseText.append(' ');
					parseBaton.m_strVerseText.append(m_strXlateText);		// Opening '('
					const CFootnoteEntry *pFootnote = parseBaton.m_pBibleDatabase->footnoteEntry(ndxWord);
					assert(pFootnote != NULL);
					parseBaton.m_strVerseText.append(pFootnote->text());
				} else if (m_chrMatchChar == QChar('n')) {
					parseBaton.m_strVerseText.append(m_strXlateText);		// Closing ')'
					parseBaton.m_strVerseText.append(' ');			// Add separator.  Note that we will trim baton whitespace at the end anyway
				} else {
					if (parseBaton.m_bOutput) {
						if (isStartOperator()) {
							parseBaton.m_strPrewordStack.append(m_strXlateText);
						} else {
							parseBaton.m_strVerseText.append(m_strXlateText);
						}
					}
				}
			}
		}
		if (m_pRichNext) {
			m_pRichNext->parse(parseBaton, lstSplit.at(i));
		} else {
			if (parseBaton.m_bOutput) parseBaton.m_strVerseText.append(lstSplit.at(i));
		}
	}
	if (m_pVerse != NULL) {
		// Push any remaining stack:
		parseBaton.m_strVerseText.append(parseBaton.m_strPrewordStack);
		parseBaton.m_strPrewordStack.clear();

		if (parseBaton.m_pCurrentLemma) {
			writeLemma(parseBaton);
			parseBaton.m_pCurrentLemma = nullptr;
			if (parseBaton.m_bUsesHTML) parseBaton.m_strVerseText.append(QString("</span>"));		// End word span
		} else if (bStartedVerseOutput && parseBaton.m_bUsesHTML) {
			// If not in a Lemma, we need to end this word:
			if (m_bUseLemmas) {
				writeLemma(parseBaton);		// Write empty lemma
			} else {
				parseBaton.m_strVerseText.append(QString("</span>"));	// End stack span
			}
			parseBaton.m_strVerseText.append(QString("</span>"));		// End word span
		}
	}
}

QString CVerseTextRichifier::parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase, const CVerseEntry *pVerse,
										const CVerseTextRichifierTags &tags, bool bAddAnchors, int *pWordCount, const CBasicHighlighter *pHighlighter, bool bUseLemmas)
{
	assert(pBibleDatabase != NULL);
	assert(pVerse != NULL);

	CRelIndex ndxRelVerse(ndxRelative.book(), ndxRelative.chapter(), ndxRelative.verse(), 0);	// Relative as verse only

	// Note: While it would be most optimum to reverse this and
	//		do the verse last so we don't have to call the entire
	//		tree for every word, we can't reverse it because doing
	//		so then creates sub-lists of 'w' tags and then we
	//		no longer know where we are in the list:
	CVerseTextRichifier rich_r('r', tags.searchResultsEnd());
	CVerseTextRichifier rich_R('R', tags.searchResultsBegin(), &rich_r);
	CVerseTextRichifier rich_n('n', tags.inlineNoteEnd(), &rich_R);
	CVerseTextRichifier rich_N('N', tags.inlineNoteBegin(), &rich_n);
	CVerseTextRichifier rich_d('d', tags.divineNameEnd(), &rich_N);
	CVerseTextRichifier rich_D('D', tags.divineNameBegin(), &rich_d);				// D/d must be last for font start/stop to work correctly with special first-letter text mode
	CVerseTextRichifier rich_t('t', tags.transChangeAddedEnd(), &rich_D);
	CVerseTextRichifier rich_T('T', tags.transChangeAddedBegin(), &rich_t);
	CVerseTextRichifier rich_j('j', tags.wordsOfJesusEnd(), &rich_T);
	CVerseTextRichifier rich_J('J', tags.wordsOfJesusBegin(), &rich_j);
	CVerseTextRichifier rich_M('M', (tags.addRichPs119HebrewPrefix() ? psalm119HebrewPrefix(ndxRelVerse, bAddAnchors && tags.usesHTML()) : ""), &rich_J);
	CVerseTextRichifier richVerseText('w', pVerse, &rich_M, bAddAnchors, bUseLemmas);

	QString strTemplate = pVerse->m_strTemplate;

	// --------------------------------

	// Convert WordsOfJesus and TransChangeAdded to per-word entities
	//	so that displaying works correctly in per-word fields for stacking:
	QStringList lstWords = strTemplate.split('w');
	QList<int> lstWordsOfJesus;			// Counts at this point to convert to flags
	QList<int> lstTransChangeAdded;
	int nInWordsOfJesus = 0;
	int nInTransChangeAdded = 0;
	for (int ndxWord = 0; ndxWord < lstWords.size(); ++ndxWord) {
		for (int nChar = 0; nChar < lstWords.at(ndxWord).size(); ++nChar) {
			if (lstWords.at(ndxWord).at(nChar) == 'J') {
				++nInWordsOfJesus;
			} else if (lstWords.at(ndxWord).at(nChar) == 'j') {
				--nInWordsOfJesus;
			} else if (lstWords.at(ndxWord).at(nChar) == 'T') {
				++nInTransChangeAdded;
			} else if (lstWords.at(ndxWord).at(nChar) == 't') {
				--nInTransChangeAdded;
			}
		}
		lstWordsOfJesus.append(nInWordsOfJesus);
		lstTransChangeAdded.append(nInTransChangeAdded);
	}

	strTemplate.clear();
	for (int ndxWord = 1; ndxWord < lstWords.size(); ++ndxWord) {
		if (lstWordsOfJesus.at(ndxWord-1)) {
			strTemplate.append('J');
		}
		if (ndxWord == 1) {
			lstWords[0].remove(QRegExp("[JjTt]"));
			strTemplate.append(lstWords.at(0));
		}
		if (lstTransChangeAdded.at(ndxWord-1)) {
			strTemplate.append('T');
		}
		strTemplate.append('w');

		if (lstTransChangeAdded.at(ndxWord-1)) {
			strTemplate.append('t');
		}
		lstWords[ndxWord].remove(QRegExp("[JjTt]"));
		strTemplate.append(lstWords.at(ndxWord));
		if (lstWordsOfJesus.at(ndxWord-1)) {
			strTemplate.append('j');
		}
	}

	// --------------------------------

	if ((pHighlighter != NULL) &&
		(pHighlighter->enabled())) {
		strTemplate.replace(QChar('w'), "Rwr");
	}

	CRichifierBaton baton(pBibleDatabase, ndxRelative, strTemplate, tags.usesHTML(), pWordCount, pHighlighter);
	if (((pVerse->m_nPilcrow == CVerseEntry::PTE_MARKER) || (pVerse->m_nPilcrow == CVerseEntry::PTE_MARKER_ADDED)) &&
		(ndxRelative.word() <= 1) &&
		(tags.showPilcrowMarkers())) {
		baton.m_strPrewordStack.append(g_chrPilcrow);
		baton.m_strPrewordStack.append(QChar(' '));
	}
	richVerseText.parse(baton);

	return baton.m_strVerseText.trimmed();
}

// ============================================================================
// ============================================================================

void CVerseTextRichifierTags::setFromPersistentSettings(const CPersistentSettings &aPersistentSettings, bool bCopyOptions)
{
	setWordsOfJesusTagsByColor(aPersistentSettings.colorWordsOfJesus());
	setSearchResultsTagsByColor(aPersistentSettings.colorSearchResults());

	if (bCopyOptions) {
		switch (aPersistentSettings.transChangeAddWordMode()) {
			case CPhraseNavigator::TCAWME_NO_MARKING:
				setTransChangeAddedTags(QString(), QString());
				break;
			case CPhraseNavigator::TCAWME_ITALICS:
				setTransChangeAddedTags(QString("<i>"), QString("</i>"));
				break;
			case CPhraseNavigator::TCAWME_BRACKETS:
				setTransChangeAddedTags(QString("["), QString("]"));
				break;
			default:
				assert(false);
				break;
		}
	}

	setShowPilcrowMarkers(bCopyOptions ? aPersistentSettings.copyPilcrowMarkers() : aPersistentSettings.showPilcrowMarkers());
}

// ============================================================================
// ============================================================================

