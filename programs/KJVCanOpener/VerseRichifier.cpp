/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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
#include "TextRenderer.h"
#include "UserNotesDatabase.h"

#include "BibleLayout.h"

#if QT_VERSION >= 0x050000
#include <QRegularExpression>
#else
#include <QRegExp>
#endif

#define OUTPUT_HEBREW_PS119 1

// ============================================================================
// ============================================================================

static QString psalm119HebrewPrefix(const CRelIndex &ndx, bool bAddAnchors)
{
	if ((ndx.book() != PSALMS_BOOK_NUM) || (ndx.chapter() != 119) || (((ndx.verse()-1)%8) != 0)) return QString();
	CRelIndex ndxRelVerse(ndx.book(), ndx.chapter(), ndx.verse(), 0);		// Relative as verse only

	QString strHebrewPrefix;

	// Add special Start tag so BrowserWidget can know to ignore the special Hebrew text insertion during highlighting:
	if (bAddAnchors) strHebrewPrefix += QString("<a id=\"A%1\"> </a>").arg(ndxRelVerse.asAnchor());

#if (OUTPUT_HEBREW_PS119)
	switch ((ndxRelVerse.verse()-1)/8) {
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

	switch ((ndxRelVerse.verse()-1)/8) {
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
		// Add special End tag so BrowserWidget can know to ignore the special Hebrew text insertion during highlighting:
		strHebrewPrefix += QString("<a id=\"B%1\"> </a>").arg(ndxRelVerse.asAnchor());
	} else {
		// Otherwise, just add a space:
		strHebrewPrefix += " ";
	}

	return strHebrewPrefix;
}

// ============================================================================
// ============================================================================

CVerseTextRichifier::CVerseTextRichifier(CRichifierBaton &parseBaton,
											CVerseTextRichifierTags::VERSE_TEMPLATE_TAGS_ENUM nMatchChar,
											const CVerseTextRichifier *pRichNext)
	:	m_parseBaton(parseBaton),
		m_pRichNext(pRichNext),
		m_vtteMatch(nMatchChar),
		m_pVerse(nullptr)
{
	static const struct {
		QChar m_chrMatchChar;
		FXlateText m_fncXlateText;
	} lstMatchHandler[CVerseTextRichifierTags::VTTE_COUNT] = {
		{ 'w', [](const CRichifierBaton &)->QString { Q_ASSERT(false); return QString(); } },					// VTTE_w - Word
		{ 'M', [](const CRichifierBaton &baton)->QString {														// VTTE_M - Ps119 Hebrew Prefix
			return ((baton.m_tags.addRichPs119HebrewPrefix() && (baton.m_pBibleDatabase->langID() != LIDE_HEBREW))
						? psalm119HebrewPrefix(baton.m_ndxCurrent, baton.renderOption(RRO_AddWordAnchors) && baton.usesHTML())
						: "");
		} },
		{ 'J', [](const CRichifierBaton &baton)->QString { return baton.m_tags.wordsOfJesusBegin(); } },		// VTTE_J - Words of Jesus Begin
		{ 'j', [](const CRichifierBaton &baton)->QString { return baton.m_tags.wordsOfJesusEnd(); } },			// VTTE_j - Words of Jesus End
		{ 'T', [](const CRichifierBaton &baton)->QString { return baton.m_tags.transChangeAddedBegin(); } },	// VTTE_T - TransChangeAdded Begin
		{ 't', [](const CRichifierBaton &baton)->QString { return baton.m_tags.transChangeAddedEnd(); } },		// VTTE_t - TransChangeAdded End
		{ 'D', [](const CRichifierBaton &baton)->QString { return baton.m_tags.divineNameBegin(); } },			// VTTE_D - Divine Name Begin
		{ 'd', [](const CRichifierBaton &baton)->QString { return baton.m_tags.divineNameEnd(); } },			// VTTE_d - Divine Name End
		{ 'A', [](const CRichifierBaton &baton)->QString {														// VTTE_A - Anchor Begin
			if (baton.usesHTML()) {
				return QString("<a id=\"%1\">").arg(baton.m_ndxCurrent.asAnchor());
			} else return QString();
		} },
		{ 'a', [](const CRichifierBaton &baton)->QString { return QString(baton.usesHTML() ? "</a>" : ""); } },	// VTTE_a - Anchor End
		{ 'N', [](const CRichifierBaton &baton)->QString { return baton.m_tags.inlineNoteBegin(); } },			// VTTE_N - Inline Note Begin
		{ 'n', [](const CRichifierBaton &baton)->QString { return baton.m_tags.inlineNoteEnd(); } },			// VTTE_n - Inline Note End
		{ 'R', [](const CRichifierBaton &baton)->QString { return baton.m_tags.searchResultsBegin(); } },		// VTTE_R - Search Results Begin
		{ 'r', [](const CRichifierBaton &baton)->QString { return baton.m_tags.searchResultsEnd(); } },			// VTTE_r - Search Results End
		{ 'E', [](const CRichifierBaton &baton)->QString { return baton.m_tags.searchResultsExcludedBegin(); } },	// VTTE_E - Search Results Excluded Begin
		{ 'e', [](const CRichifierBaton &baton)->QString { return baton.m_tags.searchResultsExcludedEnd(); } },		// VTTE_e - Search Results Excluded End
		{ 'H', [](const CRichifierBaton &)->QString { Q_ASSERT(false); return QString(); } },					// VTTE_H - User Highlighter Begin (for completeness -- but is processed by parse)
		{ 'h', [](const CRichifierBaton &)->QString { Q_ASSERT(false); return QString(); } },					// VTTE_h - User Highlighter End (for completeness -- but is processed by parse)
		{ 'L', [](const CRichifierBaton &)->QString { Q_ASSERT(false); return QString(); } },					// VTTE_L - Lemma Begin (for completeness -- not used in richifier, see KJVDataParse)
		{ 'l', [](const CRichifierBaton &)->QString { Q_ASSERT(false); return QString(); } },					// VTTE_l - Lemma End (for completeness -- not used in richifier, see KJVDataParse)
	};

	Q_ASSERT(nMatchChar < CVerseTextRichifierTags::VTTE_COUNT);
	m_chrMatchChar = lstMatchHandler[nMatchChar].m_chrMatchChar;
	m_fncXlateText = lstMatchHandler[nMatchChar].m_fncXlateText;
}

CVerseTextRichifier::CVerseTextRichifier(CRichifierBaton &parseBaton,
											CVerseTextRichifierTags::VERSE_TEMPLATE_TAGS_ENUM nMatchChar,
											const CVerseEntry *pVerse, const CVerseTextRichifier *pRichNext)
	:	m_parseBaton(parseBaton),
		m_pRichNext(pRichNext),
		m_vtteMatch(nMatchChar),
		m_chrMatchChar('w'),
		m_pVerse(pVerse)
{
	Q_ASSERT(nMatchChar == CVerseTextRichifierTags::VTTE_w);
	Q_ASSERT(pVerse != nullptr);
}

CVerseTextRichifier::~CVerseTextRichifier()
{

}

void CVerseTextRichifier::startLemma(const CRelIndex &ndxWord) const
{
	if (!m_parseBaton.m_bOutput) return;
	if (!m_parseBaton.renderOption(RRO_UseLemmas)) return;
	if (!m_parseBaton.usesHTML()) return;

	m_parseBaton.m_strVerseText.append(QString("<span class=\"stack main\">"));
	m_parseBaton.m_pCurrentLemma = m_parseBaton.m_pBibleDatabase->lemmaEntry(ndxWord);
}

void CVerseTextRichifier::finishLemma() const
{
	if (!m_parseBaton.m_bOutput) return;
	if (!m_parseBaton.renderOption(RRO_UseLemmas)) return;
	if (!m_parseBaton.usesHTML()) return;

	// Note: This finishes off the main span too:
	if (m_parseBaton.m_pCurrentLemma) {
		QStringList lstStrongLinks;
		if (m_parseBaton.renderOption(RRO_AddLinkAnchors)) {
			lstStrongLinks.reserve(m_parseBaton.m_pCurrentLemma->strongs().size());
			for (auto const &entry : m_parseBaton.m_pCurrentLemma->strongs()) {
				lstStrongLinks.append(QString("<a href=\"strong://%1\">%1</a>").arg(entry));
			}
		}
		QStringList lstMorphology;
		lstMorphology.reserve(m_parseBaton.m_pCurrentLemma->count());
		for (auto const &entry : m_parseBaton.m_pCurrentLemma->morph()) {
			CMorphEntry morph = m_parseBaton.m_pBibleDatabase->lookupMorphology(entry.m_nSource, entry.m_strEntryKey);
			if (!morph.description().isEmpty()) {
				lstMorphology.append(QString("<span title=\"%1\">%2</span>").arg(morph.description()).arg(entry.m_strEntryKey));
			} else {
				if (!entry.m_strEntryKey.isEmpty()) {
					lstMorphology.append(entry.m_strEntryKey);
				} else {
					lstMorphology.append("&nbsp;");
				}
			}
		}

		QString strInterlinear = m_parseBaton.m_pCurrentLemma->text().join(QChar(' '));
		QString strStrongs = m_parseBaton.renderOption(RRO_AddLinkAnchors) ? lstStrongLinks.join(QChar(' ')) : m_parseBaton.m_pCurrentLemma->strongs().join(QChar(' '));
		QString strMorphology = lstMorphology.join(QChar(' '));

		if (!strInterlinear.isEmpty()) {
			m_parseBaton.m_strVerseText.append(QString("</span><span class=\"stack interlinear\">%1&nbsp;</span><span class=\"stack strongs\">%2&nbsp;</span><span class=\"stack morph\">%3&nbsp;</span>")
												   .arg(strInterlinear)
												   .arg(strStrongs)
												   .arg(strMorphology)
											   );
		} else {
			m_parseBaton.m_strVerseText.append(QString("</span><span class=\"stack strongs\">%1&nbsp;</span><span class=\"stack morph\">%2&nbsp;</span><span class=\"stack interlinear\">&nbsp;</span>")
												   .arg(strStrongs)
												   .arg(strMorphology)
											   );
		}

		m_parseBaton.m_pCurrentLemma = nullptr;
	} else {
		m_parseBaton.m_strVerseText.append(QString("</span><span class=\"stack interlinear\">&nbsp;</span><span class=\"stack strongs\">&nbsp;</span><span class=\"stack morph\">&nbsp;</span>"));
	}
}

void CVerseTextRichifier::startWordSpan() const
{
	if (!m_parseBaton.m_bOutput) return;
	if (!m_parseBaton.renderOption(RRO_UseLemmas) && !m_parseBaton.renderOption(RRO_UseWordSpans)) return;
	if (!m_parseBaton.usesHTML()) return;

	m_parseBaton.m_strVerseText.append(QString("<span class=\"word\">"));
}

void CVerseTextRichifier::finishWordSpan() const
{
	if (!m_parseBaton.m_bOutput) return;
	if (!m_parseBaton.renderOption(RRO_UseLemmas) && !m_parseBaton.renderOption(RRO_UseWordSpans)) return;
	if (!m_parseBaton.usesHTML()) return;

	m_parseBaton.m_strVerseText.append(QString("</span>"));		// End word span
}

void CVerseTextRichifier::pushWordToVerseText(const QString &strWord) const
{
	CVerseTextRichifierTags::VerseWordTypeFlags nWordTypes = CVerseTextRichifierTags::VerseWordTypeFlags(CVerseTextRichifierTags::VWT_None);
	if (m_parseBaton.m_bInTransChangeAdded) nWordTypes |= CVerseTextRichifierTags::VWT_TransChangeAdded;
	if (m_parseBaton.m_bInWordsOfJesus) nWordTypes |= CVerseTextRichifierTags::VWT_WordsOfJesus;
	if (m_parseBaton.m_bInSearchResult) nWordTypes |= CVerseTextRichifierTags::VWT_SearchResult;
	if (m_parseBaton.m_bInSearchResultExcl) nWordTypes |= CVerseTextRichifierTags::VWT_SearchResultExcluded;

	StringParse::TFirstCharSize fcs = StringParse::firstCharSize(strWord);

#ifdef WORKAROUND_QTBUG_100879
	// In QTBUG-100879, the first mark composed character inside anchors
	//	doesn't render correctly.  But it does if there's a ZWSP (zero
	//	width space).  Therefore, if the next character is composed with
	//	marks and we are rendering anchors, output a "&#x200B;":
	if (m_parseBaton.m_bOutput && m_parseBaton.usesHTML() && m_parseBaton.renderOption(RRO_AddWordAnchors) && fcs.m_bHasMarks) {
		m_parseBaton.m_strVerseText.append(QChar(0x200B));
	}
#endif

	if (!m_parseBaton.m_strDivineNameFirstLetterParseText.isEmpty()) {
		if (m_parseBaton.m_bOutput) {
			m_parseBaton.m_strVerseText.append(strWord.left(fcs.m_nSize)
											+ m_parseBaton.m_strDivineNameFirstLetterParseText
											+ strWord.mid(fcs.m_nSize));
		}
		m_parseBaton.m_strDivineNameFirstLetterParseText.clear();
		nWordTypes |= CVerseTextRichifierTags::VWT_DivineName;
	} else {
		if (m_parseBaton.m_bOutput) m_parseBaton.m_strVerseText.append(strWord);
	}

	m_parseBaton.m_tags.wordCallback(strWord, nWordTypes);
}

void CVerseTextRichifier::startHighlighter(const CBasicHighlighter &highlighter, bool &bInHighlighterFlag) const
{
	// Note: We always have to check verse intersection and handle enter/exit of
	//		bInHighlighterFlag since we are called to parse twice -- once for the
	//		begin tags and once for the end tags.  Otherwise we don't know when
	//		to start/stop and which to output:
	if ((m_parseBaton.m_bOutput) &&
		(!bInHighlighterFlag) &&
		(highlighter.intersects(m_parseBaton.m_pBibleDatabase, TPhraseTag(m_parseBaton.m_ndxCurrent)))) {
		m_parseBaton.m_strPrewordStack.append((m_vtteMatch == CVerseTextRichifierTags::VTTE_H) ? highlighter.htmlBegin() : m_fncXlateText(m_parseBaton));
		bInHighlighterFlag = true;
	}
}

void CVerseTextRichifier::finishHighlighter(const CBasicHighlighter &highlighter, bool &bInHighlighterFlag) const
{
	if ((m_parseBaton.m_bOutput) &&
		(bInHighlighterFlag) &&
		((!highlighter.isContinuous()) ||
		 (!m_parseBaton.m_tagVerse.intersects(m_parseBaton.m_pBibleDatabase, TPhraseTag(m_parseBaton.m_ndxCurrent))) ||	// Continuous highlighters can't span past end of verse
		 (m_parseBaton.renderOption(RRO_UseLemmas)) ||
		 (m_parseBaton.renderOption(RRO_UseWordSpans)) ||
		 (!highlighter.intersects(m_parseBaton.m_pBibleDatabase, TPhraseTag(m_parseBaton.m_ndxCurrent))))) {
		m_parseBaton.m_strVerseText.append((m_vtteMatch == CVerseTextRichifierTags::VTTE_h) ? highlighter.htmlEnd() : m_fncXlateText(m_parseBaton));
		bInHighlighterFlag = false;
	}
}

void CVerseTextRichifier::parse(const QString &strNodeIn) const
{
	if (m_chrMatchChar.isNull()) {
		Q_ASSERT(false);			// This state should never happen
		m_parseBaton.m_strVerseText.append(strNodeIn);
		return;
	}

	QStringList lstSplit;

	if (m_pVerse != nullptr) {
		lstSplit.reserve(m_pVerse->m_nNumWrd + 1);
		lstSplit = m_parseBaton.m_strTemplate.split(m_chrMatchChar);
		Q_ASSERT(static_cast<unsigned int>(lstSplit.size()) == (m_pVerse->m_nNumWrd + 1));
		Q_ASSERT(strNodeIn.isNull());
	} else {
		lstSplit = strNodeIn.split(m_chrMatchChar);
	}
	Q_ASSERT(lstSplit.size() != 0);

	bool bStartedVerseOutput = false;		// Set to true when we first start writing output on this verse

	for (int i=0; i<lstSplit.size(); ++i) {
		CRelIndex ndxPreviousWord = m_parseBaton.m_ndxCurrent;		// Used for checking the intersection of current lemma with previous word/lemma
		if (m_pVerse != nullptr) {
			// Use +1 here for word because during the first word being processed,
			//	i will be 0 and all calls to other parse stages will be expecting
			//	to parse the first word -- this avoids adding 1 to the index everywhere.
			//	For word output, however, it will actually be for the next word
			//	and ndxPreviousWord will be the current word (since word output
			//	actually starts for i>0):
			m_parseBaton.m_ndxCurrent.setWord(i+1);

			bool bOldOutputStatus = m_parseBaton.m_bOutput;
			bool bNewOutputStatus = (static_cast<unsigned int>(i) >= m_parseBaton.m_nStartWord);
			if ((m_parseBaton.m_pWordCount != nullptr) && ((*m_parseBaton.m_pWordCount) == 0)) bNewOutputStatus = false;
			if (bOldOutputStatus && !bNewOutputStatus) {
				// If we transitioned out of output and lemma, finish writing
				//	the lemma and close it out.  This special case is needed
				//	because these functions check for m_parseBaton.m_bOutput,
				//	so we can't transition that until writing is complete:
				finishLemma();
				finishWordSpan();
				bStartedVerseOutput = false;
			}

			m_parseBaton.m_bOutput = bNewOutputStatus;
		}
		if (i > 0) {
			if (m_pVerse != nullptr) {
				// If this is the first word, move any preword template text into the
				//		PrewordStack for this word span so they are rendered correctly
				//		in the lemma stacks (see below):
				if ((i == 1) && m_parseBaton.m_bOutput) {
					m_parseBaton.m_strPrewordStack.append(m_parseBaton.m_strVerseText);
					m_parseBaton.m_strVerseText.clear();
				}
				QString strWord = m_parseBaton.m_pBibleDatabase->wordAtIndex(m_pVerse->m_nWrdAccum + i, WTE_RENDERED);

				bool bWasInLemma = (m_parseBaton.m_pCurrentLemma != nullptr);
				if (!bStartedVerseOutput ||
					!bWasInLemma ||
					(bWasInLemma && !m_parseBaton.m_pCurrentLemma->tag().intersects(m_parseBaton.m_pBibleDatabase, TPhraseTag(ndxPreviousWord)))) {
					// If we are currently in a lemma that is ending or if we are
					//	in an empty lemma group, then end it:
					if (bStartedVerseOutput) {
						finishLemma();		// Finish current lemma (write empty lemma if needed)
						finishWordSpan();	// Finish word span if needed
					}

					// Transition to verse output if needed and/or start next lemma group:
					if (m_parseBaton.m_bOutput) {
						bStartedVerseOutput = true;

						startWordSpan();
						startLemma(ndxPreviousWord);		// See note above regarding ndxPreviousWord and ndxCurrent
					}
				}	// Otherwise, we are still in a Lemma continuation and need to continue to output it...

				if (m_parseBaton.m_bOutput) m_parseBaton.m_strVerseText.append(m_parseBaton.m_strPrewordStack);
				m_parseBaton.m_strPrewordStack.clear();

				pushWordToVerseText(strWord);

				if ((m_parseBaton.m_bOutput) && (m_parseBaton.m_pWordCount != nullptr) && ((*m_parseBaton.m_pWordCount) > 0)) --(*m_parseBaton.m_pWordCount);
			} else {
				if (m_chrMatchChar == QChar('D')) {
					m_parseBaton.m_strDivineNameFirstLetterParseText = m_fncXlateText(m_parseBaton);
				} else if (m_chrMatchChar == QChar('R')) {
					Q_ASSERT(m_parseBaton.m_pSRHighlighter != nullptr);
					startHighlighter(*m_parseBaton.m_pSRHighlighter, m_parseBaton.m_bInSearchResult);
				} else if (m_chrMatchChar == QChar('r')) {
					Q_ASSERT(m_parseBaton.m_pSRHighlighter != nullptr);
					finishHighlighter(*m_parseBaton.m_pSRHighlighter, m_parseBaton.m_bInSearchResult);
				} else if (m_chrMatchChar == QChar('E')) {
					Q_ASSERT(m_parseBaton.m_pSRExclHighlighter != nullptr);
					startHighlighter(*m_parseBaton.m_pSRExclHighlighter, m_parseBaton.m_bInSearchResultExcl);
				} else if (m_chrMatchChar == QChar('e')) {
					Q_ASSERT(m_parseBaton.m_pSRExclHighlighter != nullptr);
					finishHighlighter(*m_parseBaton.m_pSRExclHighlighter, m_parseBaton.m_bInSearchResultExcl);
				} else if (m_chrMatchChar == QChar('H')) {
					Q_ASSERT(m_parseBaton.m_pUserHighlighters != nullptr);		// Main parse() function shouldn't add "H" tags if this is null
					// Process start tags forward:
					for (THighlighterTagMap::const_iterator itrHighlighters = m_parseBaton.m_pUserHighlighters->cbegin();
									itrHighlighters != m_parseBaton.m_pUserHighlighters->cend(); ++itrHighlighters) {
						CUserDefinedHighlighter highlighter(itrHighlighters->first, itrHighlighters->second);
						startHighlighter(highlighter, m_parseBaton.m_mapInHighlighter[itrHighlighters->first]);
					}
				} else if (m_chrMatchChar == QChar('h')) {
					Q_ASSERT(m_parseBaton.m_pUserHighlighters != nullptr);		// Main parse() function shouldn't add "H" tags if this is null
					// Process end tags reverse:
					THighlighterTagMap::const_iterator itrHighlighters = m_parseBaton.m_pUserHighlighters->cend();
					do {
						--itrHighlighters;
						CUserDefinedHighlighter highlighter(itrHighlighters->first, itrHighlighters->second);
						finishHighlighter(highlighter, m_parseBaton.m_mapInHighlighter[itrHighlighters->first]);
					} while(itrHighlighters != m_parseBaton.m_pUserHighlighters->cbegin());
				} else if (m_chrMatchChar == QChar('N')) {
					if (m_parseBaton.m_ndxCurrent.word() > 1) m_parseBaton.m_strPrewordStack.append(' ');
					m_parseBaton.m_strPrewordStack.append(m_fncXlateText(m_parseBaton));		// Opening '('
					const CFootnoteEntry *pFootnote = m_parseBaton.m_pBibleDatabase->footnoteEntry(m_parseBaton.m_ndxCurrent);
					Q_ASSERT(pFootnote != nullptr);
					m_parseBaton.m_strPrewordStack.append(pFootnote->text());
				} else if (m_chrMatchChar == QChar('n')) {
					m_parseBaton.m_strPrewordStack.append(m_fncXlateText(m_parseBaton));		// Closing ')'
					m_parseBaton.m_strPrewordStack.append(' ');			// Add separator.  Note that we will trim baton whitespace at the end anyway
				} else {
					if (m_chrMatchChar == QChar('T')) {
						m_parseBaton.m_bInTransChangeAdded = true;
					} else if (m_chrMatchChar == QChar('t')) {
						m_parseBaton.m_bInTransChangeAdded = false;
					} else if (m_chrMatchChar == QChar('J')) {
						m_parseBaton.m_bInWordsOfJesus = true;
					} else if (m_chrMatchChar == QChar('j')) {
						m_parseBaton.m_bInWordsOfJesus = false;
					}

					if (m_parseBaton.m_bOutput) {
						if (isStartOperator()) {
							m_parseBaton.m_strPrewordStack.append(m_fncXlateText(m_parseBaton));
						} else {
							m_parseBaton.m_strVerseText.append(m_fncXlateText(m_parseBaton));
						}
					}
				}
			}
		}
		if (m_pRichNext) {
			m_pRichNext->parse(lstSplit.at(i));
		} else {
			if (m_parseBaton.m_bOutput) m_parseBaton.m_strVerseText.append(lstSplit.at(i));
		}
	}
	if (m_pVerse != nullptr) {
		if (bStartedVerseOutput) {
			// Finish any lemma or word span
			finishLemma();
			finishWordSpan();
		}

		// Push any remaining stack:
		if (m_parseBaton.m_bOutput) m_parseBaton.m_strVerseText.append(m_parseBaton.m_strPrewordStack);
		m_parseBaton.m_strPrewordStack.clear();
	}
}

QString CVerseTextRichifier::parse(const CRelIndex &ndxRelative, const CBibleDatabase *pBibleDatabase,
									const CVerseEntry *pVerse, const CVerseTextRichifierTags &tags,
									RichifierRenderOptionFlags flagsRRO, int *pWordCount,
									const CBasicHighlighter *pSRHighlighter, const CBasicHighlighter *pSRExclHighlighter)
{
	Q_ASSERT(pBibleDatabase != nullptr);
	Q_ASSERT(pVerse != nullptr);

	CRelIndex ndxRelVerse(ndxRelative.book(), ndxRelative.chapter(), ndxRelative.verse(), 0);	// Relative as verse only

	QString strTemplate = pVerse->m_strTemplate;

	// --------------------------------

	// If the template is already inserting inline footnotes for verses, don't
	//	add extra ones and double them up:
	if (strTemplate.contains("N", Qt::CaseInsensitive)) flagsRRO &= ~RRO_InlineFootnotes;

	// --------------------------------

	// Convert WordsOfJesus, TransChangeAdded, and DivineName to per-word entities
	//	so that displaying works correctly in per-word fields for stacking:
	QStringList lstWords = strTemplate.split('w');
	QList<int> lstWordsOfJesus;			// Counts at this point to convert to flags
	QList<int> lstTransChangeAdded;
	QList<int> lstDivineName;			// Note: Divine name processed here specifically for anchor tag pairing of per-word entity
	int nInWordsOfJesus = 0;
	int nInTransChangeAdded = 0;
	int nInDivineName = 0;
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
			} else if (lstWords.at(ndxWord).at(nChar) == 'D') {
				++nInDivineName;
			} else if (lstWords.at(ndxWord).at(nChar) == 'd') {
				--nInDivineName;
			}
		}
		lstWordsOfJesus.append(nInWordsOfJesus);
		lstTransChangeAdded.append(nInTransChangeAdded);
		lstDivineName.append(nInDivineName);

		// Remove the symbols we've parsed out:
#if QT_VERSION >= 0x050000
		lstWords[ndxWord].remove(QRegularExpression("[JjTtDd]"));
#else
		lstWords[ndxWord].remove(QRegExp("[JjTtDd]"));
#endif
	}

	strTemplate.clear();

	ndxRelVerse.setWord(1);
	if ((flagsRRO & RRO_InlineFootnotes) && (pBibleDatabase->footnoteEntry(ndxRelVerse))) {
		strTemplate.append("Nn ");
	}
	for (int ndxWord = 1; ndxWord < lstWords.size(); ++ndxWord) {
		if (lstWordsOfJesus.at(ndxWord-1)) {
			strTemplate.append('J');
		}
		if (ndxWord == 1) {
			strTemplate.append(lstWords.at(0));
		}
		if (flagsRRO & RRO_AddWordAnchors) {
			strTemplate.append('A');
		}
		if (lstTransChangeAdded.at(ndxWord-1)) {
			strTemplate.append('T');
		}
		if (lstDivineName.at(ndxWord-1)) {
			strTemplate.append('D');
		}

		strTemplate.append('w');

		if (lstDivineName.at(ndxWord-1)) {
			strTemplate.append('d');
		}
		if (lstTransChangeAdded.at(ndxWord-1)) {
			strTemplate.append('t');
		}
		if (flagsRRO & RRO_AddWordAnchors) {
			strTemplate.append('a');
		}

		strTemplate.append(lstWords.at(ndxWord));

		if (lstWordsOfJesus.at(ndxWord-1)) {
			strTemplate.append('j');
		}

		ndxRelVerse.setWord(ndxWord+1);
		if ((flagsRRO & RRO_InlineFootnotes) && (pBibleDatabase->footnoteEntry(ndxRelVerse))) {
			strTemplate.append(" Nn");
		}
	}

	// --------------------------------

	// Search Results Highlighter after other tags above so that its
	//	color takes precedence over Words of Jesus color, etc:
	if ((pSRHighlighter != nullptr) &&
		(pSRHighlighter->enabled())) {
		strTemplate.replace(QChar('w'), "Rwr");
	}

	// Similar for Search Results Excluded Highlighter:
	if ((pSRExclHighlighter != nullptr) &&
		(pSRExclHighlighter->enabled())) {
		strTemplate.replace(QChar('w'), "Ewe");
	}

	// UserHighlighters last so paint order is correct:
	const THighlighterTagMap *pUserHighlighters = nullptr;
	if (flagsRRO & RRO_EnableUserHighlighters) {
		Q_ASSERT(!g_pUserNotesDatabase.isNull());
		pUserHighlighters = g_pUserNotesDatabase->highlighterTagsFor(pBibleDatabase);
		if (pUserHighlighters) {
			if (!pUserHighlighters->empty()) {
				strTemplate.replace(QChar('w'), "Hwh");
			} else {
				// If we have no actual highlighters in the map, disable processing
				//	or we will assert in the reverse iterator logic
				pUserHighlighters = nullptr;
			}
		}
	}

	ndxRelVerse.setWord(1);
	CRichifierBaton baton(tags, pBibleDatabase, ndxRelative, TPhraseTag(ndxRelVerse, pVerse->m_nNumWrd),
							strTemplate, flagsRRO, pWordCount, pSRHighlighter, pSRExclHighlighter, pUserHighlighters);
	if (((pVerse->m_nPilcrow == CVerseEntry::PTE_MARKER) || (pVerse->m_nPilcrow == CVerseEntry::PTE_MARKER_ADDED)) &&
		(ndxRelative.word() <= 1) &&
		(tags.showPilcrowMarkers())) {
		baton.m_strPrewordStack.append(g_chrPilcrow);
		baton.m_strPrewordStack.append(QChar(' '));
	}

	// Note: While it would be most optimum to reverse this and
	//		do the verse last so we don't have to call the entire
	//		tree for every word, we can't reverse it because doing
	//		so then creates sub-lists of 'w' tags and then we
	//		no longer know where we are in the list.
	CVerseTextRichifier rich_h(baton, CVerseTextRichifierTags::VTTE_h);
	CVerseTextRichifier rich_H(baton, CVerseTextRichifierTags::VTTE_H, &rich_h);
	CVerseTextRichifier rich_e(baton, CVerseTextRichifierTags::VTTE_e, &rich_H);
	CVerseTextRichifier rich_E(baton, CVerseTextRichifierTags::VTTE_E, &rich_e);
	CVerseTextRichifier rich_r(baton, CVerseTextRichifierTags::VTTE_r, &rich_E);
	CVerseTextRichifier rich_R(baton, CVerseTextRichifierTags::VTTE_R, &rich_r);
	CVerseTextRichifier rich_n(baton, CVerseTextRichifierTags::VTTE_n, &rich_R);
	CVerseTextRichifier rich_N(baton, CVerseTextRichifierTags::VTTE_N, &rich_n);
	CVerseTextRichifier rich_a(baton, CVerseTextRichifierTags::VTTE_a, &rich_N);
	CVerseTextRichifier rich_A(baton, CVerseTextRichifierTags::VTTE_A, &rich_a);
	CVerseTextRichifier rich_d(baton, CVerseTextRichifierTags::VTTE_d, &rich_A);
	CVerseTextRichifier rich_D(baton, CVerseTextRichifierTags::VTTE_D, &rich_d);				// D/d must be after J/j and T/t for font start/stop to work correctly with special first-letter text mode
	CVerseTextRichifier rich_t(baton, CVerseTextRichifierTags::VTTE_t, &rich_D);
	CVerseTextRichifier rich_T(baton, CVerseTextRichifierTags::VTTE_T, &rich_t);
	CVerseTextRichifier rich_j(baton, CVerseTextRichifierTags::VTTE_j, &rich_T);
	CVerseTextRichifier rich_J(baton, CVerseTextRichifierTags::VTTE_J, &rich_j);
	CVerseTextRichifier rich_M(baton, CVerseTextRichifierTags::VTTE_M, &rich_J);
	CVerseTextRichifier richVerseText(baton, CVerseTextRichifierTags::VTTE_w, pVerse, &rich_M);

	richVerseText.parse();

	return baton.m_strVerseText.trimmed();
}

// ============================================================================
// ============================================================================

void CVerseTextRichifierTags::setFromPersistentSettings(const CPersistentSettings &aPersistentSettings, bool bCopyOptions)
{
	setWordsOfJesusTagsByColor(aPersistentSettings.colorWordsOfJesus());
	setSearchResultsTagsByColor(aPersistentSettings.colorSearchResults());
	setSearchResultsExcludedTagsByColor(aPersistentSettings.colorSearchResults());

	if (bCopyOptions) {
		switch (aPersistentSettings.transChangeAddWordMode()) {
			case TCAWME_NO_MARKING:
				setTransChangeAddedTags(QString(), QString());
				break;
			case TCAWME_ITALICS:
				setTransChangeAddedTags(QString("<i>"), QString("</i>"));
				break;
			case TCAWME_BRACKETS:
				setTransChangeAddedTags(QString("["), QString("]"));
				break;
			default:
				Q_ASSERT(false);
				break;
		}
	}

	setShowPilcrowMarkers(bCopyOptions ? aPersistentSettings.copyPilcrowMarkers() : aPersistentSettings.showPilcrowMarkers());
}

// ============================================================================
// ============================================================================

