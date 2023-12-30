/****************************************************************************
**
** Copyright (C) 2012-2023 Donna Whisnant, a.k.a. Dewtronics.
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

#include "TextNavigator.h"
#include "Highlighter.h"
#include "PhraseCursor.h"
#include "ParseSymbols.h"
#include "VerseRichifier.h"

#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextFragment>

#include <string>

// ============================================================================

int CTextNavigator::anchorPosition(const QString &strAnchorName) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if (strAnchorName.isEmpty()) return -1;

	for (QTextBlock block = m_TextDocument.begin(); block.isValid(); block = block.next()) {
		QTextCharFormat format = block.charFormat();
		if (format.isAnchor()) {
			if (format.anchorNames().contains(strAnchorName)) {
				int nPos = block.position();
				QString strText = block.text();
				for (int nPosStr = 0; nPosStr < strText.length(); ++nPosStr) {
					if (strText[nPosStr].isSpace()) {
						nPos++;
					} else {
						break;
					}
				}
				return nPos;
			}
		}
		for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); ++it) {
			QTextFragment fragment = it.fragment();
			format = fragment.charFormat();
			if (format.isAnchor()) {
				if (format.anchorNames().contains(strAnchorName)) {
					int nPos = fragment.position();
					QString strText = fragment.text();
					for (int nPosStr = 0; nPosStr < strText.length(); ++nPosStr) {
						if (strText[nPosStr].isSpace()) {
							nPos++;
						} else {
							break;
						}
					}
					return nPos;
				}
			}
		}
	}

	return -1;
}

void CTextNavigator::doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear, const CRelIndex &ndxCurrent) const
{
	// Save some time if the tag isn't anything close to what we are displaying.
	//		We'll find a verse before and a verse after the main chapter being
	//		displayed (i.e. the actual scripture browser display window).  We
	//		will precalculate our current index before the main loop:
	TPhraseTagList tagCurrentDisplay = currentChapterDisplayPhraseTagList(ndxCurrent);
	doHighlighting(aHighlighter, bClear, tagCurrentDisplay);
}

void CTextNavigator::doHighlighting(const CBasicHighlighter &aHighlighter, bool bClear, const TPhraseTagList &tagsCurrent) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	if (!aHighlighter.enabled()) return;		// Nothing to do if this highlighter isn't enabled

	CPhraseCursor myCursor(&m_TextDocument, m_pBibleDatabase.data(), true);

	myCursor.beginEditBlock();

	CHighlighterPhraseTagFwdItr itrHighlighter = aHighlighter.getForwardIterator();
	while (!itrHighlighter.isEnd()) {
		TPhraseTag tag = itrHighlighter.nextTag();
		CRelIndex ndxRel = tag.relIndex();
		if (!ndxRel.isSet()) continue;

		// Save some time if the tag isn't anything close to what we are displaying.
		//		Check for intersection of the highlight tag with our display:
		if ((tagsCurrent.isSet()) && (!tagsCurrent.intersects(m_pBibleDatabase.data(), tag))) continue;

		unsigned int nTagCount = tag.count();
		if (nTagCount) --nTagCount;					// Make nTagCount the number of positions to move, not number words

		uint32_t ndxNormalStart = m_pBibleDatabase->NormalizeIndex(ndxRel);
		uint32_t ndxNormalEnd = ndxNormalStart + nTagCount;
		int nStartPos = -1;
		while ((nStartPos == -1) && (ndxNormalStart <= ndxNormalEnd)) {
			nStartPos = anchorPosition(ndxRel.asAnchor());
			if (nStartPos == -1) {
				ndxNormalStart++;
				ndxRel = m_pBibleDatabase->DenormalizeIndex(ndxNormalStart);
				// Safeguard incase we run off the end.  This is needed, for example, if
				//		on the last word of the Bible (Rev 22:21 [12]) with the cursor
				//		tracker visible and the user hits Alt-PgUp to go to the previous
				//		chapter.  It will run off the end looking for that word and
				//		not finding it because it's already scrolled out of view.  And
				//		then will run off the end of the text:
				if (!ndxRel.isSet()) ndxNormalStart = ndxNormalEnd+1;
			}
		}
		if (nStartPos == -1) continue;				// Note: Some highlight lists have tags not in this browser document

		while ((nStartPos != -1) && (ndxNormalStart <= ndxNormalEnd)) {
			myCursor.setPosition(nStartPos);
			int nWordEndPos = nStartPos + m_pBibleDatabase->wordAtIndex(ndxNormalStart, WTE_RENDERED).size();
#ifdef WORKAROUND_QTBUG_100879
			// Count the extra ZWSP added for the workaround:
			if (StringParse::firstCharSize(m_pBibleDatabase->wordAtIndex(ndxNormalStart, WTE_RENDERED)).m_bHasMarks) {
				++nWordEndPos;
			}
#endif

			// If this is a continuous highlighter, instead of stopping at the end of the word,
			//		we'll find the end of the last word of this verse that we'll be highlighting
			//		so that we will highlight everything in between and also not have to search
			//		for start/end of words within the verse.  We can't do more than one verse
			//		because of notes and cross-references:
			if ((aHighlighter.isContinuous()) & (ndxNormalStart != ndxNormalEnd)) {
				CRelIndex ndxCurrentWord(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart));
				const CVerseEntry *pCurrentWordVerseEntry = m_pBibleDatabase->verseEntry(ndxCurrentWord);
				Q_ASSERT(pCurrentWordVerseEntry != nullptr);
				if (pCurrentWordVerseEntry) {
					unsigned int nVrsWordCount = pCurrentWordVerseEntry->m_nNumWrd;
					uint32_t ndxNormalLastVerseWord = ndxNormalStart + nVrsWordCount - ndxCurrentWord.word();
					if (ndxNormalLastVerseWord > ndxNormalEnd) ndxNormalLastVerseWord = ndxNormalEnd;
					CRelIndex ndxLastVerseWord(m_pBibleDatabase->DenormalizeIndex(ndxNormalLastVerseWord));
					int nNextLastVerseWordPos = anchorPosition(ndxLastVerseWord.asAnchor());
					if (nNextLastVerseWordPos != -1) {
						nNextLastVerseWordPos += m_pBibleDatabase->wordAtIndex(ndxNormalLastVerseWord, WTE_RENDERED).size();
#ifdef WORKAROUND_QTBUG_100879
						// Count the extra ZWSP added for the workaround:
						if (StringParse::firstCharSize(m_pBibleDatabase->wordAtIndex(ndxNormalLastVerseWord, WTE_RENDERED)).m_bHasMarks) {
							++nNextLastVerseWordPos;
						}
#endif
					} else {
						Q_ASSERT(false);
					}
					Q_ASSERT(nWordEndPos <= nNextLastVerseWordPos);
					nWordEndPos = nNextLastVerseWordPos;
					ndxNormalStart = ndxNormalLastVerseWord;
				}
			}

			if (nStartPos < nWordEndPos) {
				if (myCursor.moveCursorCharRight(QTextCursor::KeepAnchor)) {
					QTextCharFormat fmtNew = aHighlighter.textCharFormat(myCursor.charFormat(), bClear);
					myCursor.setPosition(nWordEndPos, QTextCursor::KeepAnchor);
					myCursor.mergeCharFormat(fmtNew);
					myCursor.clearSelection();
				} else {
					Q_ASSERT(false);
				}
			}

			++ndxNormalStart;
			nStartPos = anchorPosition(m_pBibleDatabase->DenormalizeIndex(ndxNormalStart).asAnchor());
		}
	}

	myCursor.endEditBlock();
}

TPhraseTagList CTextNavigator::currentChapterDisplayPhraseTagList(const CRelIndex &ndxCurrent) const
{
	TPhraseTagList tagsCurrentDisplay;

	if ((ndxCurrent.isSet()) && (ndxCurrent.book() != 0) && (ndxCurrent.book() <= m_pBibleDatabase->bibleEntry().m_nNumBk)) {
		// Note: If this is a colophon reference, find the corresponding last chapter:
		CRelIndex ndxDisplay = CRelIndex(ndxCurrent.book(), ((ndxCurrent.chapter() != 0) ? ndxCurrent.chapter() : m_pBibleDatabase->bookEntry(ndxCurrent.book())->m_nNumChp), 0, 1);
		// This can happen if the versification of the reference doesn't match the active database:
		if (m_pBibleDatabase->NormalizeIndex(ndxDisplay) == 0) return TPhraseTagList();

		// Main Chapter:
		const CChapterEntry *pChapter = m_pBibleDatabase->chapterEntry(ndxDisplay);
		Q_ASSERT(pChapter != nullptr);
		tagsCurrentDisplay.append((TPhraseTag(ndxDisplay, pChapter->m_nNumWrd)));

		// Verse Before:
		CRelIndex ndxVerseBefore = m_pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndxDisplay.book(), ndxDisplay.chapter(), 1, 1), true);	// Calculate one verse prior to the first verse of this book/chapter
		if (ndxVerseBefore.isSet()) {
			const CVerseEntry *pVerseBefore = m_pBibleDatabase->verseEntry(ndxVerseBefore);
			Q_ASSERT(pVerseBefore != nullptr);
			tagsCurrentDisplay.append(TPhraseTag(ndxVerseBefore, pVerseBefore->m_nNumWrd));
		}

		// Verse After:
		CRelIndex ndxVerseAfter = m_pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndxDisplay.book(), ndxDisplay.chapter(), 1, 1), false);	// Calculate first verse of next chapter
		if (ndxVerseAfter.isSet()) {
			const CVerseEntry *pVerseAfter = m_pBibleDatabase->verseEntry(ndxVerseAfter);
			Q_ASSERT(pVerseAfter != nullptr);
			tagsCurrentDisplay.append(TPhraseTag(ndxVerseAfter, pVerseAfter->m_nNumWrd));
		}

		// If this book has a colophon and this is the last chapter of that book, we
		//	need to add it as well:
		const CBookEntry *pBook = m_pBibleDatabase->bookEntry(ndxDisplay.book());
		Q_ASSERT(pBook != nullptr);
		if ((pBook->m_bHaveColophon) && (ndxDisplay.chapter() == pBook->m_nNumChp)) {
			const CVerseEntry *pBookColophon = m_pBibleDatabase->verseEntry(CRelIndex(ndxDisplay.book(), 0, 0, 0));
			Q_ASSERT(pBookColophon != nullptr);
			tagsCurrentDisplay.append(TPhraseTag(CRelIndex(ndxDisplay.book(), 0, 0, 1), pBookColophon->m_nNumWrd));
		}

		// If the ndxVerseBefore is in a different book, check that book to see if
		//	it has a colophon and if so, add it so that we will render highlighting
		//	and other markup for it:
		if ((ndxVerseBefore.isSet()) && (ndxVerseBefore.book() != ndxDisplay.book())) {
			const CBookEntry *pBookVerseBefore = m_pBibleDatabase->bookEntry(ndxVerseBefore.book());
			Q_ASSERT(pBookVerseBefore != nullptr);
			if (pBookVerseBefore->m_bHaveColophon) {
				const CVerseEntry *pPrevBookColophon = m_pBibleDatabase->verseEntry(CRelIndex(ndxVerseBefore.book(), 0, 0, 0));
				Q_ASSERT(pPrevBookColophon != nullptr);
				tagsCurrentDisplay.append(TPhraseTag(CRelIndex(ndxVerseBefore.book(), 0, 0, 1), pPrevBookColophon->m_nNumWrd));
			}
		}
	}

	return tagsCurrentDisplay;
}

QString CTextNavigator::setDocumentToBookInfo(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO)
{
	if (((flagsTRO & TRO_InnerHTML) == 0) &&
		((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.clear();
	}
	if (flagsTRO & TRO_NoQTextDocument) {
		flagsTRO |= TRO_NoIndent;
	}

	QString strHtml = CTextRenderer::generateTextForBookInfo(m_pBibleDatabase.data(), ndx, flagsTRO);

	if (!strHtml.isEmpty() && ((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.setHtml(strHtml);
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		emit changedDocumentText();
	}

	return strHtml;
}

QString CTextNavigator::setDocumentToChapter(const CRelIndex &ndx, TextRenderOptionFlags flagsTRO,
											 const CBasicHighlighter *pSRHighlighter,
											 const CBasicHighlighter *pSRExclHighlighter)
{
	if (((flagsTRO & TRO_InnerHTML) == 0) &&
		((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.clear();
	}
	if (flagsTRO & TRO_NoQTextDocument) {
		flagsTRO |= TRO_NoIndent;
	}

	QString strHtml = CTextRenderer::generateTextForChapter(m_pBibleDatabase.data(), m_TextDocument.indentWidth(), ndx,
															flagsTRO, pSRHighlighter, pSRExclHighlighter);

	if (!strHtml.isEmpty() && ((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.setHtml(strHtml);
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		emit changedDocumentText();
	}

	return strHtml;
}

QString CTextNavigator::setDocumentToVerse(const CRelIndex &ndx, const TPhraseTagList &tagsToInclude,
										   TextRenderOptionFlags flagsTRO, const CBasicHighlighter *pSRHighlighter,
										   const CBasicHighlighter *pSRExclHighlighter)
{
	if (((flagsTRO & TRO_InnerHTML) == 0) &&
		((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.clear();
	}
	if (flagsTRO & TRO_NoQTextDocument) {
		flagsTRO |= TRO_NoIndent;
	}

	QString strHtml = CTextRenderer::generateTextForVerse(m_pBibleDatabase.data(), ndx, tagsToInclude, flagsTRO,
															pSRHighlighter, pSRExclHighlighter);

	if (!strHtml.isEmpty() && ((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.setHtml(strHtml);
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		emit changedDocumentText();
	}

	return strHtml;
}

QString CTextNavigator::setDocumentToFormattedVerses(const TPhraseTagList &lstPhraseTags, TextRenderOptionFlags flagsTRO)
{
	return setDocumentToFormattedVerses(TPassageTagList(m_pBibleDatabase.data(), lstPhraseTags), flagsTRO);
}

QString CTextNavigator::setDocumentToFormattedVerses(const TPassageTagList &lstPassageTags, TextRenderOptionFlags flagsTRO)
{
	if (((flagsTRO & TRO_InnerHTML) == 0) &&
		((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.clear();
	}
	if (flagsTRO & TRO_NoQTextDocument) {
		flagsTRO |= TRO_NoIndent;
	}

	QString strHtml = CTextRenderer::generateTextForFormattedVerses(m_pBibleDatabase.data(), m_TextDocument.indentWidth(), lstPassageTags, flagsTRO);

	if (!strHtml.isEmpty() && ((flagsTRO & TRO_NoQTextDocument) == 0)) {
		m_TextDocument.setHtml(strHtml);
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		emit changedDocumentText();
	}

	return strHtml;
}

void CTextNavigator::clearDocument()
{
	m_TextDocument.clear();
	emit changedDocumentText();
}

CSelectionPhraseTagList CTextNavigator::getSelection(const CPhraseCursor &aCursor, bool bRecursion) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	TPhraseTag tag;

	CPhraseCursor myCursor(aCursor);
	//	myCursor.beginEditBlock();
	int nPosFirst = qMin(myCursor.anchor(), myCursor.position());
	int nPosLast = qMax(myCursor.anchor(), myCursor.position());
	int nPosFirstWordStart = nPosFirst;
	CRelIndex nIndexFirst;				// First Word anchor tag
	CRelIndex nIndexLast;				// Last Word anchor tag
	QString strAnchorName;

#ifdef DEBUG_CURSOR_SELECTION
	int nPosCursorStart = -1;
	int nPosCursorEnd = -1;
#endif

	// Find first word anchor:
	myCursor.setPosition(nPosFirst);
	// See if our first character is a space.  If so, don't include it because it's
	//		most likely the single space between words.  If so, we would end up
	//		starting the selection at the preceding word and while that's technically
	//		correct, it's confusing to the user who probably didn't mean for that
	//		to happen:
	myCursor.moveCursorWordStart();
	nPosFirstWordStart = myCursor.position();
	while ((myCursor.position() <= (nPosLast+1)) && (!nIndexFirst.isSet())) {
		strAnchorName = myCursor.charFormat().anchorNames().value(0);

		nIndexFirst = CRelIndex(strAnchorName);
		// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
		//		text from a verse.  We must be in a special tag section of heading:
		if (nIndexFirst.word() == 0) {
			nIndexFirst = CRelIndex();
		}
		if (!myCursor.moveCursorCharRight()) break;
	}
#ifdef DEBUG_CURSOR_SELECTION
	if (nIndexFirst.isSet()) nPosCursorStart = myCursor.position() - 2;		// -2 -> One for the extra moveCursorCharRight and one for the anchor character position
#endif

	// Find last word anchor:
	int nPosOfIndexLast = nPosLast;
	CRelIndex nIndexLastDetected;
	myCursor.setPosition(nPosLast);
	while ((myCursor.moveCursorCharLeft()) && (myCursor.charUnderCursorIsSeparator())) { }	// Note: Always move left at least one character so we don't pickup the start of the next word (short-circuit order!)
	myCursor.moveCursorWordEnd();
	bool bFoundHit = false;
	uint32_t nNormPrev = 0;
	CRelIndex ndxPrev;
	while (myCursor.position() >= nPosFirstWordStart) {
		strAnchorName = myCursor.charFormat().anchorNames().value(0);
		CRelIndex ndxCurrent = CRelIndex(strAnchorName);

		uint32_t nNormCurrent = m_pBibleDatabase->NormalizeIndex(ndxCurrent);

		if (nNormCurrent != 0) {
			// Make sure words selected are consecutive words (i.e. that we don't have a "hidden" colophon between them or something):
			//		Note: nNormPrev will be equal to nNormCurrent when we are on the first word of the line with how our
			//		paragraphs/blocks work...
			if ((ndxCurrent.word() != 0) && (nNormPrev != (nNormCurrent+1)) && (nNormPrev != nNormCurrent)) {
				bFoundHit = false;
			}

			// If we are moving into or out of a colophon or superscription, treat it as a
			//		discontinuity and break the selection into multiple parts:
			if ((ndxCurrent.word() != 0) && (ndxPrev.isSet()) &&
				(((ndxCurrent.chapter() != 0) && (ndxPrev.chapter() == 0)) ||
				 ((ndxCurrent.chapter() == 0) && (ndxPrev.chapter() != 0)) ||
				 ((ndxCurrent.verse() != 0) && (ndxPrev.verse() == 0)) ||
				 ((ndxCurrent.verse() == 0) && (ndxPrev.verse() != 0)))) {
				bFoundHit = false;
			}

			// If we haven't hit an anchor for an actual word within a verse, we can't be selecting
			//		text from a verse.  We must be in a special tag section of heading:
			if ((ndxCurrent.word() == 0) || (ndxCurrent < nIndexFirst)) {
				ndxCurrent = CRelIndex();
			} else {
				nNormPrev = nNormCurrent;
				ndxPrev = ndxCurrent;
			}

			if (!bFoundHit) {
				nIndexLast = ndxCurrent;
				bFoundHit = nIndexLast.isSet();
				if (bFoundHit) {
					nPosOfIndexLast = myCursor.position();
					nIndexLastDetected = nIndexLast;
				}
			}
		}

		if (!myCursor.moveCursorCharLeft()) break;
	}
#ifdef DEBUG_CURSOR_SELECTION
	if (nIndexLast.isSet()) {
		myCursor.moveCursorWordRight();
		while ((myCursor.moveCursorCharLeft()) && (myCursor.charUnderCursorIsSeparator())) { }	// Note: Always move left at least one character so we don't pickup the start of the next word (short-circuit order!)
		nPosCursorEnd = myCursor.position() + 1;			// +1 -> One for the extra moveCursorCharLeft
	}
#endif

	if (nIndexLastDetected.isSet()) {
		myCursor.setPosition(nPosOfIndexLast);
		myCursor.moveCursorWordEnd();
		nPosOfIndexLast = myCursor.position()+1;
	} else {
		nPosOfIndexLast = nPosLast+1;
	}

	// Handle single-word selection:
	if (!nIndexLast.isSet()) nIndexLast = nIndexFirst;

	// If the cursor is floating in "no man's land" in a special tag area or footnote text or
	//		something, then find the closest matching tag to the left.  This is the same as
	//		the current position tracking:
	if (!nIndexFirst.isSet()) {
		myCursor.setPosition(nPosFirst);
		while (!nIndexFirst.isSet()) {
			nIndexFirst = CRelIndex(myCursor.charFormat().anchorNames().value(0));
			if (!myCursor.moveCursorCharLeft()) break;
		}
	}

	//	myCursor.endEditBlock();

	uint32_t ndxNormFirst = m_pBibleDatabase->NormalizeIndex(nIndexFirst);
	uint32_t ndxNormLast = m_pBibleDatabase->NormalizeIndex(nIndexLast);
	unsigned int nWordCount = 0;

	if ((ndxNormFirst != 0) && (ndxNormLast != 0)) {
		Q_ASSERT(ndxNormLast >= ndxNormFirst);
		nWordCount = (ndxNormLast - ndxNormFirst + 1);
	}

	tag.relIndex() = nIndexFirst;
	tag.count() = ((nPosFirst != nPosLast) ? nWordCount : 0);

#ifdef DEBUG_CURSOR_SELECTION
	QString strPhrase;
	if ((nPosCursorStart != -1) && (nPosCursorEnd != -1)) {
		CPhraseCursor myCursor(aCursor);
		myCursor.beginEditBlock();
		myCursor.setPosition(nPosCursorStart);
		myCursor.setPosition(nPosCursorEnd, QTextCursor::KeepAnchor);
		strPhrase = myCursor.selectedText();
		myCursor.endEditBlock();
	}

	qDebug("\"%s\"", strPhrase.toUtf8().data());
	qDebug("%s %d", m_pBibleDatabase->PassageReferenceText(tag.relIndex()).toUtf8().data(), tag.count());
#endif

	CSelectionPhraseTagList lstSelectTags;
	if ((!bRecursion) || (bRecursion && tag.haveSelection() && (tag.relIndex().word() != 0) && (nIndexLastDetected.isSet()))) lstSelectTags.append(tag);

	if ((nPosOfIndexLast < nPosLast) && (nPosFirst != nPosLast) && (nIndexLastDetected.isSet())) {
		myCursor.setPosition(nPosOfIndexLast);
		myCursor.setPosition(nPosLast, QTextCursor::KeepAnchor);
		lstSelectTags.append(getSelection(myCursor, true));
	}

	return lstSelectTags;
}

CSelectedPhraseList CTextNavigator::getSelectedPhrases(const CPhraseCursor &aCursor) const
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	CSelectedPhraseList lstSelectedPhrases;

	CSelectionPhraseTagList lstSelections = getSelection(aCursor);
	if (lstSelections.size() == 0) lstSelections.append(TPhraseTag());

#ifdef DEBUG_SELECTED_PHRASE
	qDebug("%d Phrases Selected", lstSelections.size());
#endif
	for (int ndxSel = 0; ndxSel < lstSelections.size(); ++ndxSel) {
		QString strPhrase;
		TPhraseTag tagSel = lstSelections.at(ndxSel);

		int nCount = tagSel.count();
		CRelIndex ndxRel = tagSel.relIndex();
		// If there is no selection (i.e. count is 0), and we are only in a book or chapter
		//		marker, don't return any selected phrase text:
		if ((nCount == 0) && (((ndxRel.chapter() == 0) || (ndxRel.verse() == 0)) && (ndxRel.word() == 0))) ndxRel.clear();
		if (ndxRel.isSet()) {
			// So the we'll start at the beginning of the "next verse", if we are only
			//		at the begging of the book or chapter, move to the start of the verse.
			//		In theory this won't happen because of how getSelection() currently
			//		works, but in case we ever change it for some reason:
			if ((ndxRel.chapter() == 0) && (ndxRel.word() == 0)) ndxRel.setChapter(1);
			if ((ndxRel.verse() == 0) && (ndxRel.word() == 0)) ndxRel.setVerse(1);
		}
		if (nCount == 0) nCount = 1;
		static const CVerseTextPlainRichifierTags tagsRichifier;
		while ((nCount > 0) && (ndxRel.isSet())) {
			const CVerseEntry *pVerse = m_pBibleDatabase->verseEntry(ndxRel);
			Q_ASSERT(pVerse != nullptr);
			strPhrase += CVerseTextRichifier::parse(ndxRel, m_pBibleDatabase.data(), pVerse, tagsRichifier, RichifierRenderOptionFlags(), &nCount);
			if (nCount) {
				// Goto the first word of this verse and add the number of words in this verse to get to start of next verse:
				ndxRel.setWord(1);
				ndxRel = m_pBibleDatabase->DenormalizeIndex(m_pBibleDatabase->NormalizeIndex(ndxRel) + m_pBibleDatabase->verseEntry(ndxRel)->m_nNumWrd);
				ndxRel.setWord(0);				// Process entire verse on next pass
				strPhrase += "  ";
			}
		}

		// Clean stray transChangeAddedBegin() marks:
		strPhrase = strPhrase.trimmed();
		if (strPhrase.endsWith(tagsRichifier.transChangeAddedBegin())) strPhrase = strPhrase.left(strPhrase.size() - tagsRichifier.transChangeAddedBegin().size());
		strPhrase = strPhrase.trimmed();

		lstSelectedPhrases.append(CSelectedPhrase(m_pBibleDatabase, tagSel, strPhrase));

#ifdef DEBUG_SELECTED_PHRASE
		qDebug("    \"%s\"", strPhrase.toUtf8().data());
		qDebug("    %s %d", m_pBibleDatabase->PassageReferenceText(tagSel.relIndex()).toUtf8().data(), tagSel.count());
#endif
	}

	return lstSelectedPhrases;
}

void CTextNavigator::removeAnchors()
{
	Q_ASSERT(!m_pBibleDatabase.isNull());

	// Note: I discovered in this that just moving the cursor one character
	//		to the right at a time and looking for anchors wasn't sufficient.
	//		Not totally sure why, but it seems like some are kept on the
	//		block level and not in a corresponding text fragment.  So, I
	//		cobbled this up, pattered after our anchorPosition function, which
	//		was patterned after the Qt code for doing this:

	CPhraseCursor myCursor(&m_TextDocument, m_pBibleDatabase.data(), true);
	myCursor.beginEditBlock();

	for (QTextBlock block = m_TextDocument.begin(); block.isValid(); block = block.next()) {
		QTextCharFormat format = block.charFormat();
		if (format.isAnchor()) {
			format.setAnchorNames(QStringList());
			format.setAnchor(false);
			myCursor.setPosition(block.position());
			myCursor.setPosition(block.position()+1, QTextCursor::KeepAnchor);
			myCursor.setCharFormat(format);
			// This one is a linked list instead of an iterator, so no need to reset
			//	any iterators here
		}
		for (QTextBlock::Iterator it = block.begin(); !it.atEnd(); /* increment inside loop */ ) {
			QTextFragment fragment = it.fragment();
			format = fragment.charFormat();
			if (format.isAnchor()) {
				format.setAnchorNames(QStringList());
				format.setAnchor(false);
				myCursor.setPosition(fragment.position());
				myCursor.setPosition(fragment.position()+1, QTextCursor::KeepAnchor);
				myCursor.setCharFormat(format);
				// Note: The above affects the fragment iteration list and
				//	if we don't reset our loop here, we'll segfault with an
				//	invalid iterator:
				it = block.begin();
			} else {
				++it;
			}
		}
	}
	myCursor.endEditBlock();
}

// ============================================================================

