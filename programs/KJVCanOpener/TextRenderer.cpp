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

#include "TextRenderer.h"

#include "dbstruct.h"
#include "Highlighter.h"
#include "VerseRichifier.h"
#include "ScriptureDocument.h"
#include "PersistentSettings.h"
#include "Translator.h"
#include "PhraseParser.h"

#include <QObject>

// ============================================================================

QString CTextRenderer::generateTextForBookInfo(const CBibleDatabase *pBibleDatabase,
											   const CRelIndex &ndx,
											   TextRenderOptionFlags flagsTRO)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), (flagsTRO & TRO_Copying));

	bool bTotalColophonAnchor = (!(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoColophonAnchors));

	if (ndx.book() == 0) return QString();

	if (ndx.book() > pBibleDatabase->bibleEntry().m_nNumBk) {
		Q_ASSERT(false);
		return QString();
	}

	const CBookEntry &book = *pBibleDatabase->bookEntry(ndx.book());

	// Search for "Category:".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strCategory = QObject::tr("Category:", "Scope");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(pBibleDatabase->langID()));
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("QObject", "Category:", "Scope");
		if (!strTemp.isEmpty()) strCategory = strTemp;
	}

	CScriptureTextHtmlBuilder scriptureHTML;

	COPY_FONT_SELECTION_ENUM cfseToUse = CFSE_NONE;
	if (flagsTRO & TRO_SearchResults) cfseToUse = CFSE_SEARCH_RESULTS;
	if (flagsTRO & TRO_Copying) cfseToUse = (CPersistentSettings::instance()->copyFontSelection());
	QString strCopyFont= "font-size:medium;";
	switch (cfseToUse) {
		case CFSE_NONE:
			break;
		case CFSE_COPY_FONT:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
			break;
		case CFSE_SCRIPTURE_BROWSER:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
			break;
		case CFSE_SEARCH_RESULTS:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	double nLineHeight = 1.0;
	if (flagsTRO & TRO_ScriptureBrowser) {
		nLineHeight = CPersistentSettings::instance()->scriptureBrowserLineHeight();
	}

	RichifierRenderOptionFlags flagsRRO = RRO_None;
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)) flagsRRO |= RRO_AddWordAnchors;
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoLinkAnchors)) flagsRRO |= RRO_AddLinkAnchors;
	if (flagsTRO & TRO_InlineFootnotes) flagsRRO |= RRO_InlineFootnotes;
	if (flagsTRO & TRO_EnableUserHighlighters) flagsRRO |= RRO_EnableUserHighlighters;

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
		//											.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
		//											.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<title>%1</title><style type=\"text/css\">\n"
											"body, p, li, br, .bodyIndent { white-space: pre-line; line-height:%3; %2 }\n"
											".book { font-size:xx-large; font-weight:bold; }\n"
											".chapter { font-size:x-large; font-weight:bold; }\n"
											".verse { }\n"
											".word { }\n"
											".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".category { font-size:medium; font-weight:normal; }\n"
											".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
											"</style></head><body>\n")
										.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
										.arg(strCopyFont)																// Copy Font
										.arg(QString("%1").arg(nLineHeight*100, 0, 'f', 0) + "%"));						// Line-Height
	}

	CRelIndex ndxBookChap(ndx.book(), ndx.chapter(), 0, 0);
	CRelIndex ndxBook(ndx.book(), 0, 0, 0);

	// Print Heading for this Book:
	scriptureHTML.beginDiv(pBibleDatabase->direction(), "book");
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.beginAnchorID(ndxBook.asAnchor());
	scriptureHTML.appendLiteralText(book.m_strBkName);
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.endAnchor();
	// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
	//		at the end of the book name so people adding notes/cross-refs for the book
	//		aren't confused by it being at the beginning of the name.  But then switch
	//		back to a book reference so that book category/descriptions are properly
	//		labeled:
	if (!(flagsTRO & TRO_NoAnchors)) {
		if (!(flagsTRO & TRO_NoChapterAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChap.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
		if (!(flagsTRO & TRO_NoBookAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBook.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
	}
	scriptureHTML.endDiv();

	// Print Book Descriptions:
	if ((flagsTRO & TRO_Subtitles) && (!book.m_strDesc.isEmpty())) {
		scriptureHTML.beginDiv(pBibleDatabase->direction(), "subtitle");
		scriptureHTML.appendRawText(QString("(%1)").arg(book.m_strDesc));
		scriptureHTML.endDiv();
	}
	// Print Book Category:
	if  ((flagsTRO & TRO_Category) && (!pBibleDatabase->bookCategoryName(ndxBook).isEmpty())) {
		scriptureHTML.beginDiv(pBibleDatabase->direction(), "category");
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strCategory);
		scriptureHTML.endBold();
		scriptureHTML.appendRawText(QString(" %1").arg(pBibleDatabase->bookCategoryName(ndxBook)));
		scriptureHTML.endDiv();
	}
	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
	}
	// If we have a User Note for this book, print it too:
	if ((flagsTRO & TRO_UserNotes) &&
		(scriptureHTML.addNoteFor(pBibleDatabase, ndxBook, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
		scriptureHTML.insertHorizontalRule();

	// Add colophon for the book if it exists and we are instructed to add it:
	if ((flagsTRO & TRO_Colophons) && (book.m_bHaveColophon)) {
		// Try pseudo-verse (searchable) style first:
		scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
		scriptureHTML.beginParagraph(pBibleDatabase->direction());
		scriptureHTML.appendRawText("<span class=\"verse\">");
		if (bTotalColophonAnchor) {
			CRelIndex ndxColophon(ndxBook);
			ndxColophon.setWord(1);
			scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxColophon.asAnchor()));
		}
		scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndxBook, richifierTags, flagsRRO));
		if (bTotalColophonAnchor) {
			scriptureHTML.appendRawText("</a>");
		}
		scriptureHTML.appendRawText("</span>");
		scriptureHTML.endParagraph();
		scriptureHTML.endDiv();
	} else if (!pBibleDatabase->isVersificationRemapped()) {	// Footnote style colophon doesn't work on VersificationRemapping since footnotes are stored with KJV REF
		// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
		scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
		if ((flagsTRO & TRO_Colophons) &&
			(scriptureHTML.addFootnoteFor(pBibleDatabase, ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
			scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the colophon divison ahead of footnote
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
			scriptureHTML.flushBuffer();
			scriptureHTML.endDiv();
		}
		scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		scriptureHTML.appendRawText("</body></html>");
	}

	return scriptureHTML.getResult();
}

// ============================================================================

QString CTextRenderer::generateTextForChapter(const CBibleDatabase *pBibleDatabase,
											  qreal nIndentWidth,
											  const CRelIndex &ndx,
											  TextRenderOptionFlags flagsTRO,
											  const CBasicHighlighter *pSRHighlighter,
											  const CBasicHighlighter *pSRExclHighlighter)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), (flagsTRO & TRO_Copying));

	bool bTotalColophonAnchor = (!(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoColophonAnchors));
	bool bTotalSuperscriptionAnchor =  (!(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoSuperscriptAnchors));

	if ((ndx.book() == 0) || (ndx.chapter() == 0)) return QString();

	if (ndx.book() > pBibleDatabase->bibleEntry().m_nNumBk) {
		// Note: This condition can happen if we were given an invalid passage to
		//	render for our database, such as the made-up previews in the settings
		//	for certain Bible databases:
		return QString();
	}

	const CBookEntry &book = *pBibleDatabase->bookEntry(ndx.book());

	const CChapterEntry *pChapter = pBibleDatabase->chapterEntry(ndx);
	if (pChapter == nullptr) {
		Q_ASSERT(false);
		return QString();
	}

	if (ndx.chapter() > book.m_nNumChp) {
		Q_ASSERT(false);
		return QString();
	}

	// Search for "Category:".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strCategory = QObject::tr("Category:", "Scope");
	TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(pBibleDatabase->langID()));
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("QObject", "Category:", "Scope");
		if (!strTemp.isEmpty()) strCategory = strTemp;
	}

	// Search for "Chapter".  First try and see if we can translate it in the language of the selected Bible,
	//		but if not, try in the current language setting
	QString strChapter = QObject::tr("Chapter", "Scope");
	//TTranslatorPtr pTranslator = CTranslatorList::instance()->translator(toQtLanguageName(pBibleDatabase->langID()));
	if (!pTranslator.isNull()) {
		QString strTemp = pTranslator->translatorApp().translate("QObject", "Chapter", "Scope");
		if (!strTemp.isEmpty()) strChapter = strTemp;
	}

	CScriptureTextHtmlBuilder scriptureHTML;

	COPY_FONT_SELECTION_ENUM cfseToUse = CFSE_NONE;
	if (flagsTRO & TRO_SearchResults) cfseToUse = CFSE_SEARCH_RESULTS;
	if (flagsTRO & TRO_Copying) cfseToUse = (CPersistentSettings::instance()->copyFontSelection());
	QString strCopyFont= "font-size:medium;";
	switch (cfseToUse) {
		case CFSE_NONE:
			break;
		case CFSE_COPY_FONT:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
			break;
		case CFSE_SCRIPTURE_BROWSER:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
			break;
		case CFSE_SEARCH_RESULTS:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	double nLineHeight = 1.0;
	if (flagsTRO & TRO_ScriptureBrowser) {
		nLineHeight = CPersistentSettings::instance()->scriptureBrowserLineHeight();
	}

	RichifierRenderOptionFlags flagsRRO = RRO_None;
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)) flagsRRO |= RRO_AddWordAnchors;
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoLinkAnchors)) flagsRRO |= RRO_AddLinkAnchors;
	if (flagsTRO & TRO_UseLemmas) flagsRRO |= RRO_UseLemmas;
	if (flagsTRO & TRO_UseWordSpans) flagsRRO |= RRO_UseWordSpans;
	if (flagsTRO & TRO_InlineFootnotes) flagsRRO |= RRO_InlineFootnotes;
	if (flagsTRO & TRO_EnableUserHighlighters) flagsRRO |= RRO_EnableUserHighlighters;

#ifdef WORKAROUND_LITEHTML_81
	// Very kludgy hack for LiteHtml missing support for "dir" property on paragraphs.
	//	This implements the LTR/RTL logic using a flex wrapping section.  It's a hack because
	//	this code really doesn't know if it's outputting text for the LiteHtml or not.
	//	We just assume we are if we are outputting Lemmas and Anchors together.  Otherwise,
	//	we assume we are either in the QTextDocument of ScriptureBrowser (which doesn't
	//	use the lemmas) or on WebChannel (which won't have link anchors).  This code nearly
	//	works correctly, but since the paragraph text itself doesn't switch to RTL mode,
	//	then verse punctuation outside of a word doesn't render on the correct side of the
	//	word due to the anchor tags on the words.
	//	This code is horrible and needs to be deleted as soon as LiteHtml gets support for "dir":
	bool bKludge81 = ((flagsRRO & (RRO_AddLinkAnchors | RRO_UseLemmas)) == (RRO_AddLinkAnchors | RRO_UseLemmas));
#endif

	VERSE_RENDERING_MODE_ENUM vrmeMode = ((flagsTRO & TRO_Copying) ?
											  CPersistentSettings::instance()->verseRenderingModeCopying() :
											  CPersistentSettings::instance()->verseRenderingMode());

	if ((flagsTRO & TRO_VPL_Only) && (vrmeMode == VRME_FF)) vrmeMode = VRME_VPL;

	if (flagsTRO & TRO_NoIndent) {
		if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_HANGING)) {
			vrmeMode = VRME_VPL;
		} else if ((vrmeMode == VRME_VPL_DS_INDENT) || (vrmeMode == VRME_VPL_DS_HANGING)) {
			vrmeMode = VRME_VPL_DS;
		}
	}

	QString strRefCSS = QString(
			".ref:dir(ltr) { float: left; padding: 0em 0.5em %1em 0em; }\n"
			".ref:dir(rtl) { float: right; padding: 0em 0em %1em 0.5em; }\n"
		).arg(QString("%1").arg((flagsTRO & TRO_UseLemmas) ? (nLineHeight-0.5) : 0.25, 0, 'f', 2));		// .word/.ref padding bottom in em's

	QString strVerseLemma = QString(".verselemma { display: flex; flex-wrap: wrap; flex-direction: %1; }\n")
								.arg((pBibleDatabase->direction() == Qt::RightToLeft) ? "row-reverse" : "row");

#ifdef WORKAROUND_LITEHTML_81
	if (bKludge81) {
		if (pBibleDatabase->direction() == Qt::LeftToRight) {
			strRefCSS = QString(
					".ref { float: left; padding: 0em 0.5em %1em 0em; }\n"
				).arg(QString("%1").arg((nLineHeight-0.5), 0, 'f', 2));		// .word/.ref padding bottom in em's
		} else {
			strRefCSS = QString(
					".ref { float: right; padding: 0em 0em %1em 0.5em; }\n"
				).arg(QString("%1").arg((nLineHeight-0.5), 0, 'f', 2));		// .word/.ref padding bottom in em's
		}
	}
#endif

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n.subtitle { font-size:12pt; font-weight:normal; }\n.category { font-size:12pt; font-weight:normal; }\n</style></head><body>\n")
		//											.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n.subtitle { font-size:medium; font-weight:normal; }\n.category { font-size:medium; font-weight:normal; }\n</style></head><body>\n")
		//											.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<title>%1</title><style type=\"text/css\">\n"
											"body, p, li, br, .bodyIndent { white-space: pre-line; line-height:%3; %2 }\n"
											".book { font-size:xx-large; font-weight:bold; }\n"
											".chapter { font-size:x-large; font-weight:bold; }\n"
											".verse { display: inline-block; }\n"
											"%4"
											".word { display: inline-block; padding: 0em 0.5em %5em 0em; }\n"
											"%6"
											".stack { display: block; }\n"
											".main { font-size:large; }\n"
											".interlinear { font-size:large; }\n"
											".strongs { font-size:medium; }\n"
											".morph { font-size:medium; }\n"
											".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".category { font-size:medium; font-weight:normal; }\n"
											".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
											"</style></head><body>\n")
										.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
										.arg(strCopyFont)																// Copy Font
										.arg(QString("%1").arg((flagsTRO & TRO_UseLemmas) ? 125.0 : nLineHeight*100, 0, 'f', 0) + "%")		// Line-Height
										.arg(strVerseLemma)
										.arg(QString("%1").arg((flagsTRO & TRO_UseLemmas) ? (nLineHeight-0.5) : 0.25, 0, 'f', 2))			// .word/.ref padding bottom in em's
										.arg(strRefCSS));																// Ref CSS for Kludge
	}

	CRelIndex relPrev = pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndx.book(), ndx.chapter(), 1, 1), true);	// Calculate one verse prior to the first verse of this book/chapter
	CRelIndex relNext = pBibleDatabase->calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndx.book(), ndx.chapter(), 1, 1), false);	// Calculate first verse of next chapter

	// Print last verse of previous chapter if available:
	if ((!(flagsTRO & TRO_SuppressPrePostChapters)) && (relPrev.isSet())) {
		relPrev.setWord(0);
		const CBookEntry &bookPrev = *pBibleDatabase->bookEntry(relPrev.book());
		scriptureHTML.beginParagraph(pBibleDatabase->direction());

		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING)) {
			scriptureHTML.beginIndent(1, -nIndentWidth);
		}
		if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.beginIndent(0, nIndentWidth);
		}

		scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));

		scriptureHTML.appendRawText("<span class=\"ref\">");
		if ((flagsTRO & TRO_UseLemmas) || (flagsTRO & TRO_UseWordSpans)) {
			scriptureHTML.appendRawText("<span class=\"word\">");
		}
		if (flagsTRO & TRO_UseLemmas) {
			scriptureHTML.appendRawText("<span class=\"stack main\">");
		}
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(relPrev.asAnchor());
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(QString("%1 ").arg(relPrev.verse()));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();
		if (flagsTRO & TRO_UseLemmas) {
			scriptureHTML.appendRawText("</span>");	// Stack
			scriptureHTML.appendRawText("<span class=\"stack interlinear\">&nbsp;</span>");
			scriptureHTML.appendRawText("<span class=\"stack strongs\">&nbsp;</span>");
			scriptureHTML.appendRawText("<span class=\"stack morph\">&nbsp;</span>");
		}
		if ((flagsTRO & TRO_UseLemmas) || (flagsTRO & TRO_UseWordSpans)) {
			scriptureHTML.appendRawText("</span>");	// Word
		}
		scriptureHTML.appendRawText("</span>");	// Ref

		scriptureHTML.appendRawText(pBibleDatabase->richVerseText(relPrev,
																	richifierTags,
																	flagsRRO,
																	pSRHighlighter,
																	pSRExclHighlighter));
		scriptureHTML.appendRawText("</span>");	// Verse

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(pBibleDatabase, relPrev, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
		}
		// And Notes:
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(pBibleDatabase, relPrev, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true);

		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING) ||
			(vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.endIndent();
		}

		scriptureHTML.endParagraph();

		// If we have a footnote or user note for this book and this is the end of the last chapter,
		//		print it too:
		if (relPrev.chapter() == bookPrev.m_nNumChp) {
			if ((flagsTRO & TRO_Colophons) && (bookPrev.m_bHaveColophon)) {
				// Try pseudo-verse (searchable) style first:
				scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
				scriptureHTML.beginParagraph(pBibleDatabase->direction());
				scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));
				if (bTotalColophonAnchor) {
					CRelIndex ndxColophon(relPrev.book(), 0, 0, 1);
					scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxColophon.asAnchor()));
				}
				scriptureHTML.appendRawText(pBibleDatabase->richVerseText(CRelIndex(relPrev.book(), 0, 0, 0),
																			richifierTags,
																			flagsRRO,
																			pSRHighlighter,
																			pSRExclHighlighter));
				if (bTotalColophonAnchor) {
					scriptureHTML.appendRawText("</a>");
				}
				scriptureHTML.appendRawText("</span>");
				scriptureHTML.endParagraph();
				scriptureHTML.endDiv();
			} else if (!pBibleDatabase->isVersificationRemapped()) {	// Footnote style colophon doesn't work on VersificationRemapping since footnotes are stored with KJV REF
				// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
				scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
				if ((flagsTRO & TRO_Colophons) &&
					(scriptureHTML.addFootnoteFor(pBibleDatabase, CRelIndex(relPrev.book(),0,0,0), (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
					scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the colophon divison ahead of footnote
					scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
					scriptureHTML.flushBuffer();
					scriptureHTML.endDiv();
				}
				scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
			}

			if (flagsTRO & TRO_UserNotes)
				scriptureHTML.addNoteFor(pBibleDatabase, CRelIndex(relPrev.book(),0,0,0), (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));
			// No extra <hr> as we have one below for the whole chapter anyway
		}
	}

	if (!(flagsTRO & TRO_SuppressPrePostChapters)) scriptureHTML.insertHorizontalRule();

	CRelIndex ndxBookChap(ndx.book(), ndx.chapter(), 0, 0);
	CRelIndex ndxBook(ndx.book(), 0, 0, 0);
	CRelIndex ndxBookChapter1(ndx.book(), 1, 0, 0);

	// Print Heading for this Book:
	scriptureHTML.beginDiv(pBibleDatabase->direction(), "book");
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.beginAnchorID(ndxBook.asAnchor());
	scriptureHTML.appendLiteralText(book.m_strBkName);
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.endAnchor();
	// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
	//		at the end of the book name so people adding notes/cross-refs for the book
	//		aren't confused by it being at the beginning of the name.  But then switch
	//		back to a book reference so that book category/descriptions are properly
	//		labeled:
	if (!(flagsTRO & TRO_NoAnchors)) {
		if (!(flagsTRO & TRO_NoChapterAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChap.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
		if (!(flagsTRO & TRO_NoBookAnchors)) {
			scriptureHTML.beginAnchorID(QString("%1").arg(ndxBook.asAnchor()));
			scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
			scriptureHTML.endAnchor();
		}
	}
	scriptureHTML.endDiv();
	// If this is the first chapter of the book:
	if (pBibleDatabase->NormalizeIndex(ndxBookChapter1) == pBibleDatabase->NormalizeIndex(ndxBookChap)) {
		// Print Book Descriptions:
		if ((flagsTRO & TRO_Subtitles) && (!book.m_strDesc.isEmpty())) {
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "subtitle");
			scriptureHTML.appendRawText(QString("(%1)").arg(book.m_strDesc));
			scriptureHTML.endDiv();
		}
		// Print Book Category:
		if  ((flagsTRO & TRO_Category) && (!pBibleDatabase->bookCategoryName(ndxBook).isEmpty())) {
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "category");
			scriptureHTML.beginBold();
			scriptureHTML.appendLiteralText(strCategory);
			scriptureHTML.endBold();
			scriptureHTML.appendRawText(QString(" %1").arg(pBibleDatabase->bookCategoryName(ndxBook)));
			scriptureHTML.endDiv();
		}
		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
		}
		// If we have a User Note for this book, print it too:
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(pBibleDatabase, ndxBook, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
			scriptureHTML.insertHorizontalRule();
	}

	// Print Heading for this Chapter:
	scriptureHTML.beginDiv(pBibleDatabase->direction(), "chapter");
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.beginAnchorID(ndxBookChap.asAnchor());
	scriptureHTML.appendLiteralText(QString("%1 %2").arg(strChapter).arg(ndx.chapter()));
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.endAnchor();
	scriptureHTML.endDiv();
	// If we have a chapter Footnote for this chapter, print it too:
	if ((flagsTRO & TRO_Superscriptions) && (pChapter->m_bHaveSuperscription)) {
		// Try pseudo-verse (searchable) style first:
		scriptureHTML.beginDiv(pBibleDatabase->direction(), "superscription");
		scriptureHTML.beginParagraph(pBibleDatabase->direction());
		scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));
		if (bTotalSuperscriptionAnchor) {
			CRelIndex ndxSuperscription(ndxBookChap);
			ndxSuperscription.setWord(1);
			scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxSuperscription.asAnchor()));
		}
		scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndxBookChap,
																	richifierTags,
																	flagsRRO,
																	pSRHighlighter,
																	pSRExclHighlighter));
		if (bTotalSuperscriptionAnchor) {
			scriptureHTML.appendRawText("</a>");
		}
		scriptureHTML.appendRawText("</span>");
		scriptureHTML.endParagraph();
		scriptureHTML.endDiv();
	} else if (!pBibleDatabase->isVersificationRemapped()) {	// Footnote style superscription doesn't work on VersificationRemapping since footnotes are stored with KJV REF
		// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
		scriptureHTML.startBuffered();			// Start buffering so we can insert superscription division if there is a footnote
		if ((flagsTRO & TRO_Superscriptions) &&
			(scriptureHTML.addFootnoteFor(pBibleDatabase, ndxBookChap, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
			scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the superscription divison ahead of footnote
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "superscription");
			scriptureHTML.flushBuffer();
			scriptureHTML.endDiv();
		}
		scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
	}

	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxBookChap, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
	}

	// If we have a chapter User Note for this chapter, print it too:
	if ((flagsTRO & TRO_UserNotes) &&
		(scriptureHTML.addNoteFor(pBibleDatabase, ndxBookChap, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
		scriptureHTML.insertHorizontalRule();

	// Print the Chapter Text:
	bool bParagraph = false;
	bool bInIndent = false;
	bool bNeedLeadSpace = false;
	CRelIndex ndxVerse;
	bool bVPLNeedsLineBreak = false;
	bool bStartedText = false;
	for (unsigned int ndxVrs=0; ndxVrs<pChapter->m_nNumVrs; ++ndxVrs) {
		if (((vrmeMode == VRME_VPL) || (vrmeMode == VRME_VPL_DS)) &&
			(bParagraph)) bVPLNeedsLineBreak = true;

		ndxVerse = CRelIndex(ndx.book(), ndx.chapter(), ndxVrs+1, 0);
		const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndxVerse);
		if (pVerse == nullptr) {
			Q_ASSERT(false);
			continue;
		}
		if ((!bStartedText) && (pVerse->m_nNumWrd == 0)) continue;			// Don't print verses that are empty if we haven't started printing anything for the chapter yet

		if (pVerse->m_nPilcrow != CVerseEntry::PTE_NONE) {
			if (bParagraph) {
				scriptureHTML.endParagraph();
				bParagraph=false;
			}
			if (flagsTRO & TRO_UseLemmas) scriptureHTML.addLineBreak();		// Force extra break on lemmas when there's a paragraph marker
			bVPLNeedsLineBreak = (flagsTRO & TRO_UseLemmas);				// And still require a break on lemmas for paragraphs, since the LiteHtml lemma markup doesn't add space like QTextBrowser does here
		}
		if (!bParagraph) {
			scriptureHTML.beginParagraph(pBibleDatabase->direction());
			bParagraph = true;
			bNeedLeadSpace = false;
		}

		if ((!bInIndent) && ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING))) {
			scriptureHTML.beginIndent(1, -nIndentWidth);
			bInIndent = true;
		}
		if ((!bInIndent) && ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT))) {
			scriptureHTML.beginIndent(0, nIndentWidth);
			bInIndent = true;
		}

		if (bVPLNeedsLineBreak) {
			scriptureHTML.addLineBreak();
			if (vrmeMode == VRME_VPL_DS) scriptureHTML.addLineBreak();
			bVPLNeedsLineBreak = false;
		} else if ((pVerse->m_nPilcrow != CVerseEntry::PTE_NONE) && (vrmeMode == VRME_VPL_DS)) {
			scriptureHTML.addLineBreak();
		}
		//		if (((vrmeMode == VRME_VPL) || (vrmeMode == VRME_VPL_DS)) && (pVerse->m_nPilcrow != CVerseEntry::PTE_NONE)) {
		//			scriptureHTML.addLineBreak();
		//		}

		scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));

		scriptureHTML.appendRawText("<span class=\"ref\">");
		if ((flagsTRO & TRO_UseLemmas) || (flagsTRO & TRO_UseWordSpans)) {
			scriptureHTML.appendRawText("<span class=\"word\">");
		}
		if (flagsTRO & TRO_UseLemmas) {
			scriptureHTML.appendRawText("<span class=\"stack main\">");
		}
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(ndxVerse.asAnchor());
		scriptureHTML.beginBold();
		if ((bNeedLeadSpace) && (vrmeMode == VRME_FF)) scriptureHTML.appendLiteralText(" ");
		scriptureHTML.appendLiteralText(QString("%1 ").arg(ndxVrs+1));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();
		if (flagsTRO & TRO_UseLemmas) {
			scriptureHTML.appendRawText("</span>");	// Stack
			scriptureHTML.appendRawText("<span class=\"stack interlinear\">&nbsp;</span>");
			scriptureHTML.appendRawText("<span class=\"stack strongs\">&nbsp;</span>");
			scriptureHTML.appendRawText("<span class=\"stack morph\">&nbsp;</span>");
		}
		if ((flagsTRO & TRO_UseLemmas) || (flagsTRO & TRO_UseWordSpans)) {
			scriptureHTML.appendRawText("</span>");	// Word
		}
		scriptureHTML.appendRawText("</span>");	// Ref

		scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndxVerse,
																	richifierTags,
																	flagsRRO,
																	pSRHighlighter,
																	pSRExclHighlighter));
		scriptureHTML.appendRawText("</span>");	// Verse

		bStartedText = true;
		bNeedLeadSpace = true;

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxVerse, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
		}

		// Output notes for this verse, but make use of the buffer in case we need to end the paragraph tag:
		scriptureHTML.startBuffered();
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(pBibleDatabase, ndxVerse, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true))) {
			if (bInIndent) {
				scriptureHTML.endIndent();
				bInIndent = false;
			}
			if (bParagraph) {
				scriptureHTML.stopBuffered();	// Switch to direct output to end the paragraph ahead of the note
				scriptureHTML.endParagraph();
				bParagraph = false;
			}
			// Do an extra horizontal break if not at the end of the chapter:
			if (ndxVrs != (pChapter->m_nNumVrs - 1)) {
				scriptureHTML.flushBuffer(true);		// Flush our note, stop buffering (call below is redundant in this one case)
				scriptureHTML.insertHorizontalRule();	//	but is needed so we can output this <hr>
			}

			if ((vrmeMode == VRME_VPL_DS) ||
				(vrmeMode == VRME_VPL_DS_HANGING) ||
				(vrmeMode == VRME_VPL_DS_INDENT)) scriptureHTML.addLineBreak();
			bNeedLeadSpace = false;
		}
		scriptureHTML.flushBuffer(true);		// Stop buffering and flush

		if (bInIndent) {
			if ((vrmeMode == VRME_VPL_DS_HANGING) || (vrmeMode == VRME_VPL_DS_INDENT)) {
				scriptureHTML.addLineBreak();
				scriptureHTML.addLineBreak();
			}
			scriptureHTML.endIndent();
			bInIndent = false;
		}

		ndxVerse.setWord(pVerse->m_nNumWrd);		// At end of loop, ndxVerse will be index of last word we've output...
	}
	if (bInIndent) {
		scriptureHTML.endIndent();
		bInIndent = false;
	}
	if (bParagraph) {
		scriptureHTML.endParagraph();
		bParagraph = false;
	}

	// If we have a footnote or user note for this book and this is the end of the last chapter,
	//		print it too:
	if (ndx.chapter() == book.m_nNumChp) {
		if ((flagsTRO & TRO_Colophons) && (book.m_bHaveColophon)) {
			// Try pseudo-verse (searchable) style first:
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
			scriptureHTML.beginParagraph(pBibleDatabase->direction());
			scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));
			if (bTotalColophonAnchor) {
				CRelIndex ndxColophon(ndxBook);
				ndxColophon.setWord(1);
				scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxColophon.asAnchor()));
			}
			scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndxBook,
																		richifierTags,
																		flagsRRO,
																		pSRHighlighter,
																		pSRExclHighlighter));
			if (bTotalColophonAnchor) {
				scriptureHTML.appendRawText("</a>");
			}
			scriptureHTML.appendRawText("</span>");
			scriptureHTML.endParagraph();
			scriptureHTML.endDiv();
		} else if (!pBibleDatabase->isVersificationRemapped()) {	// Footnote style colophon doesn't work on VersificationRemapping since footnotes are stored with KJV REF
			// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
			scriptureHTML.startBuffered();			// Start buffering so we can insert colophon division if there is a footnote
			if ((flagsTRO & TRO_Colophons) &&
				(scriptureHTML.addFootnoteFor(pBibleDatabase, ndxBook, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
				scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the colophon divison ahead of footnote
				scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
				scriptureHTML.flushBuffer();
				scriptureHTML.endDiv();
			}
			scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
		}
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(pBibleDatabase, CRelIndex(ndx.book(),0,0,0), (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));
		// No extra <hr> as we have one below for the whole chapter anyway
	}

	if (!(flagsTRO & TRO_SuppressPrePostChapters)) scriptureHTML.insertHorizontalRule();

	// Print first verse of next chapter if available:
	if ((!(flagsTRO & TRO_SuppressPrePostChapters)) && (relNext.isSet())) {
		relNext.setWord(0);
		CRelIndex ndxBookChapNext(relNext.book(), relNext.chapter(), 0, 0);
		CRelIndex ndxBookNext(relNext.book(), 0, 0, 0);
		const CBookEntry &bookNext = *pBibleDatabase->bookEntry(relNext.book());
		const CChapterEntry *pChapterNext = pBibleDatabase->chapterEntry(ndxBookChapNext);
		Q_ASSERT(pChapterNext != nullptr);

		// Print Heading for this Book:
		if (relNext.book() != ndx.book()) {
			// Print Heading for this Book:
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "book");
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.beginAnchorID(ndxBookNext.asAnchor());
			scriptureHTML.appendLiteralText(bookNext.m_strBkName);
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors)) scriptureHTML.endAnchor();
			// Put tiny Book/Chapter anchor at top for a hit-target for scrolling.  But put it
			//		at the end of the book name so people adding notes/cross-refs for the book
			//		aren't confused by it being at the beginning of the name.  But then switch
			//		back to a book reference so that book category/descriptions are properly
			//		labeled:
			if (!(flagsTRO & TRO_NoAnchors)) {
				if (!(flagsTRO & TRO_NoChapterAnchors)) {
					scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookChapNext.asAnchor()));
					scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
					scriptureHTML.endAnchor();
				}
				if (!(flagsTRO & TRO_NoBookAnchors)) {
					scriptureHTML.beginAnchorID(QString("%1").arg(ndxBookNext.asAnchor()));
					scriptureHTML.appendRawText(QChar(0x200B));		// Use zero-space space as it doesn't count as space in positioning so selection works correctly!  Ugh!
					scriptureHTML.endAnchor();
				}
			}
			scriptureHTML.endDiv();
			// Print Book Descriptions for first chapter of book:
			if ((flagsTRO & TRO_Subtitles) && (!bookNext.m_strDesc.isEmpty()) && (relNext.chapter() == 1)) {
				scriptureHTML.beginDiv(pBibleDatabase->direction(), "subtitle");
				scriptureHTML.appendRawText(QString("(%1)").arg(bookNext.m_strDesc));
				scriptureHTML.endDiv();
			}
			// Print Book Category for first chapter of book:
			if ((flagsTRO & TRO_Category) && (!pBibleDatabase->bookCategoryName(ndxBookNext).isEmpty()) && (relNext.chapter() == 1)) {
				scriptureHTML.beginDiv(pBibleDatabase->direction(), "category");
				scriptureHTML.beginBold();
				scriptureHTML.appendLiteralText(strCategory);
				scriptureHTML.endBold();
				scriptureHTML.appendRawText(QString(" %1").arg(pBibleDatabase->bookCategoryName(ndxBookNext)));
				scriptureHTML.endDiv();
			}
			// Add CrossRefs:
			if (flagsTRO & TRO_CrossRefs) {
				scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxBookNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
			}
			// If we have a User Note for this book, print it too:
			if ((flagsTRO & TRO_UserNotes) &&
				(scriptureHTML.addNoteFor(pBibleDatabase, ndxBookNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
				scriptureHTML.insertHorizontalRule();
		}
		// Print Heading for this Chapter:
		scriptureHTML.beginDiv(pBibleDatabase->direction(), "chapter");
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.beginAnchorID(ndxBookChapNext.asAnchor());
		scriptureHTML.appendLiteralText(QString("%1 %2").arg(strChapter).arg(relNext.chapter()));
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.endAnchor();
		scriptureHTML.endDiv();

		// If we have a chapter note for this chapter, print it too:
		if ((flagsTRO & TRO_Superscriptions) && (pChapterNext->m_bHaveSuperscription)) {
			// Try pseudo-verse (searchable) style first:
			scriptureHTML.beginDiv(pBibleDatabase->direction(), "superscription");
			scriptureHTML.beginParagraph(pBibleDatabase->direction());
			scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));
			if (bTotalSuperscriptionAnchor) {
				CRelIndex ndxSuperscription(ndxBookChapNext);
				ndxSuperscription.setWord(1);
				scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxSuperscription.asAnchor()));
			}
			scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndxBookChapNext,
																		richifierTags,
																		flagsRRO,
																		pSRHighlighter,
																		pSRExclHighlighter));
			if (bTotalSuperscriptionAnchor) {
				scriptureHTML.appendRawText("</a>");
			}
			scriptureHTML.appendRawText("</span>");
			scriptureHTML.endParagraph();
			scriptureHTML.endDiv();
		} else if (!pBibleDatabase->isVersificationRemapped()) {	// Footnote style superscription doesn't work on VersificationRemapping since footnotes are stored with KJV REF
			// If pseudo-verse doesn't exist, drop back to try old "footnote" style:
			scriptureHTML.startBuffered();			// Start buffering so we can insert superscription division if there is a footnote
			if ((flagsTRO & TRO_Superscriptions) &&
				(scriptureHTML.addFootnoteFor(pBibleDatabase, ndxBookChapNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoFootnoteAnchors))))) {
				scriptureHTML.stopBuffered();		// Stop the buffering so we can insert the superscription divison ahead of footnote
				scriptureHTML.beginDiv(pBibleDatabase->direction(), "superscription");
				scriptureHTML.flushBuffer();
				scriptureHTML.endDiv();
			}
			scriptureHTML.flushBuffer(true);		// Flush and stop buffering, if we haven't already
		}

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxBookChapNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)));
		}

		// If we have a chapter User Note for this chapter, print it too:
		if ((flagsTRO & TRO_UserNotes) &&
			(scriptureHTML.addNoteFor(pBibleDatabase, ndxBookChapNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible))))
			scriptureHTML.insertHorizontalRule();

		scriptureHTML.beginParagraph(pBibleDatabase->direction());

		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING)) {
			scriptureHTML.beginIndent(1, -nIndentWidth);
		}
		if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.beginIndent(0, nIndentWidth);
		}

		scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));

		scriptureHTML.appendRawText("<span class=\"ref\">");
		if ((flagsTRO & TRO_UseLemmas) || (flagsTRO & TRO_UseWordSpans)) {
			scriptureHTML.appendRawText("<span class=\"word\">");
		}
		if (flagsTRO & TRO_UseLemmas) {
			scriptureHTML.appendRawText("<span class=\"stack main\">");
		}
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(relNext.asAnchor());
		scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(QString("%1 ").arg(relNext.verse()));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();
		if (flagsTRO & TRO_UseLemmas) {
			scriptureHTML.appendRawText("</span>");	// Stack
			scriptureHTML.appendRawText("<span class=\"stack interlinear\">&nbsp;</span>");
			scriptureHTML.appendRawText("<span class=\"stack strongs\">&nbsp;</span>");
			scriptureHTML.appendRawText("<span class=\"stack morph\">&nbsp;</span>");
		}
		if ((flagsTRO & TRO_UseLemmas) || (flagsTRO & TRO_UseWordSpans)) {
			scriptureHTML.appendRawText("</span>");	// Word
		}
		scriptureHTML.appendRawText("</span>");	// Ref

		scriptureHTML.appendRawText(pBibleDatabase->richVerseText(relNext,
																	richifierTags,
																	flagsRRO,
																	pSRHighlighter,
																	pSRExclHighlighter));
		scriptureHTML.appendRawText("</span>");	// Verse

		// Add CrossRefs:
		if (flagsTRO & TRO_CrossRefs) {
			scriptureHTML.addCrossRefsFor(pBibleDatabase, relNext, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
		}
		// And Notes:
		if (flagsTRO & TRO_UserNotes)
			scriptureHTML.addNoteFor(pBibleDatabase, relNext, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible), true);

		if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING) ||
			(vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
			scriptureHTML.endIndent();
		}

		scriptureHTML.endParagraph();
	}

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		scriptureHTML.appendRawText("</body></html>");
	}

	return scriptureHTML.getResult();
}

// ============================================================================

QString CTextRenderer::generateTextForVerse(const CBibleDatabase *pBibleDatabase,
											const CRelIndex &ndx,
											const TPhraseTagList &tagsToInclude,
											TextRenderOptionFlags flagsTRO,
											const CBasicHighlighter *pSRHighlighter,
											const CBasicHighlighter *pSRExclHighlighter)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), (flagsTRO & TRO_Copying));

	if (ndx.book() == 0) return QString();

	if (ndx.book() > pBibleDatabase->bibleEntry().m_nNumBk) {
		Q_ASSERT(false);
		return QString();
	}

	if (ndx.chapter() > pBibleDatabase->bookEntry(ndx.book())->m_nNumChp) {
		Q_ASSERT(false);
		return QString();
	}

	const CChapterEntry *pChapter = pBibleDatabase->chapterEntry(ndx);		// Note: Will be null on colophon

	if (((pChapter != nullptr) && (ndx.verse() > pChapter->m_nNumVrs)) ||
		((pChapter == nullptr) && (ndx.verse() != 0))) {
		Q_ASSERT(false);
		return QString();
	}

	CRelIndex ndxVerse = ndx;
	ndxVerse.setWord(0);			// Create special index to make sure we use a verse only reference

	CRelIndex ndxSuperColo = ndx;
	ndxSuperColo.setWord(1);

	CScriptureTextHtmlBuilder scriptureHTML;

	COPY_FONT_SELECTION_ENUM cfseToUse = CFSE_NONE;
	if (flagsTRO & TRO_SearchResults) cfseToUse = CFSE_SEARCH_RESULTS;
	if (flagsTRO & TRO_Copying) cfseToUse = (CPersistentSettings::instance()->copyFontSelection());
	QString strCopyFont= "font-size:medium;";
	switch (cfseToUse) {
		case CFSE_NONE:
			break;
		case CFSE_COPY_FONT:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
			break;
		case CFSE_SCRIPTURE_BROWSER:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
			break;
		case CFSE_SEARCH_RESULTS:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	double nLineHeight = 1.0;
	if (flagsTRO & TRO_ScriptureBrowser) {
		nLineHeight = CPersistentSettings::instance()->scriptureBrowserLineHeight();
	}

	RichifierRenderOptionFlags flagsRRO = RRO_None;
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)) flagsRRO |= RRO_AddWordAnchors;
	if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoLinkAnchors)) flagsRRO |= RRO_AddLinkAnchors;
	if (flagsTRO & TRO_UseLemmas) flagsRRO |= RRO_UseLemmas;
	if (flagsTRO & TRO_UseWordSpans) flagsRRO |= RRO_UseWordSpans;
	if (flagsTRO & TRO_InlineFootnotes) flagsRRO |= RRO_InlineFootnotes;
	if (flagsTRO & TRO_EnableUserHighlighters) flagsRRO |= RRO_EnableUserHighlighters;

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n")
		//							.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		//		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><title>%1</title><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n")
		//							.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx))));		// Document Title
		scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
											"<title>%1</title><style type=\"text/css\">\n"
											"body, p, li, br, .bodyIndent { white-space: pre-wrap; line-height:%3; %2 }\n"
											".book { font-size:xx-large; font-weight:bold; }\n"
											".chapter { font-size:x-large; font-weight:bold; }\n"
											".verse { }\n"
											".verselemma { }\n"
											".word { }\n"
											".stack { }\n"
											".main { }\n"
											".interlinear { }\n"
											".strongs { }\n"
											".morph { }\n"
											".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".category { font-size:medium; font-weight:normal; }\n"
											".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
											".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
											"</style></head><body>\n")
										.arg(scriptureHTML.escape(pBibleDatabase->PassageReferenceText(ndx)))			// Document Title
										.arg(strCopyFont)																// Copy Font
										.arg(QString("%1").arg(nLineHeight*100, 0, 'f', 0) + "%"));						// Line-Height
	}

	if (flagsTRO & TRO_AddDividerLineBefore) scriptureHTML.insertHorizontalRule();

	scriptureHTML.beginParagraph(pBibleDatabase->direction());

	bool bExtended = false;			// True if result extends to multiple verses

	do {
		const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndxVerse);
		if (pVerse == nullptr) {
			Q_ASSERT(false);
			return QString();
		}

		// Print Book/Chapter for this verse:

		if (bExtended) scriptureHTML.appendRawText(QString("  "));

		if (!bExtended || (ndxVerse.book() != ndx.book())) {
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.beginAnchorID(CRelIndex(ndxVerse.book(), ndxVerse.chapter(), 0, 0).asAnchor());
			scriptureHTML.beginBold();
			if (bExtended) scriptureHTML.appendLiteralText(QString("("));
			scriptureHTML.appendLiteralText(pBibleDatabase->bookEntry(ndxVerse.book())->m_strBkName);
			scriptureHTML.endBold();
			if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoBookAnchors) && !(flagsTRO & TRO_NoChapterAnchors)) scriptureHTML.endAnchor();
		}

		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.beginAnchorID(ndxVerse.asAnchor());
		scriptureHTML.beginBold();
		if (bExtended) {
			if (ndxVerse.book() == ndx.book()) {
				scriptureHTML.appendLiteralText(QString("("));
			} else {
				scriptureHTML.appendLiteralText(QString(" "));
			}
		}

		if (bExtended && (ndxVerse.book() == ndx.book()) && (ndxVerse.chapter() == ndx.chapter())) {
			if (ndxVerse.verse() != 0) {
				scriptureHTML.appendLiteralText(QString("%1").arg(ndxVerse.verse()));
			} else {
				if (ndxVerse.chapter() == 0) {
					scriptureHTML.appendLiteralText(pBibleDatabase->translatedColophonString());
				} else {
					scriptureHTML.appendLiteralText(pBibleDatabase->translatedSuperscriptionString());
				}
			}
		} else {
			if (!bExtended) scriptureHTML.appendLiteralText(QString(" "));
			if (ndxVerse.verse() != 0) {
				scriptureHTML.appendLiteralText(QString("%1:%2").arg(ndxVerse.chapter()).arg(ndxVerse.verse()));
			} else {
				if (ndxVerse.chapter() == 0) {
					scriptureHTML.appendLiteralText(pBibleDatabase->translatedColophonString());
				} else {
					scriptureHTML.appendLiteralText(QString("%1 %2").arg(ndxVerse.chapter()).arg(pBibleDatabase->translatedSuperscriptionString()));
				}
			}
		}
		if (bExtended) scriptureHTML.appendLiteralText(QString(")"));
		scriptureHTML.appendLiteralText(QString(" "));
		scriptureHTML.endBold();
		if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoVerseAnchors)) scriptureHTML.endAnchor();

		// Print this Verse Text:

		bool bTotalColophonAnchor = (ndxSuperColo.isColophon() && !(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoColophonAnchors));
		bool bTotalSuperscriptionAnchor =  (ndxSuperColo.isSuperscription() && !(flagsTRO & TRO_NoAnchors) && (flagsTRO & TRO_NoWordAnchors) && !(flagsTRO & TRO_NoSuperscriptAnchors));

		//
		// Note: The problem with applying a special colophon/superscription style with
		//		a <div> causes it to be separated as its own paragraph rather than
		//		inline like normal verses.  Can we do this with a span or something?
		//		More importantly, do we even want to do it?  As we do lose our
		//		transChange markup in all of the italics...
		//
		//		if (ndxVerse.verse() == 0) {
		//			if (ndxVerse.chapter() == 0) {
		//				scriptureHTML.beginDiv(pBibleDatabase->direction(), "colophon");
		//			} else {
		//				scriptureHTML.beginDiv(pBibleDatabase->direction(), "superscription");
		//			}
		//		}
		scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));
		if (bTotalColophonAnchor || bTotalSuperscriptionAnchor) {
			scriptureHTML.appendRawText(QString("<a id=\"%1\">").arg(ndxSuperColo.asAnchor()));
		}
		scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndxVerse,
																	richifierTags,
																	flagsRRO,
																	pSRHighlighter,
																	pSRExclHighlighter));
		if (bTotalColophonAnchor || bTotalSuperscriptionAnchor) {
			scriptureHTML.appendRawText("</a>");
		}
		scriptureHTML.appendRawText("</span>");
		//		if (ndxVerse.verse() == 0) {
		//			scriptureHTML.endDiv();
		//		}

		// Calculate the next verse index so we can see if it intersects our
		//	results of this verse entry (i.e. spills to next verse)
		ndxVerse = pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, ndxVerse, false);
		ndxVerse.setWord(0);
		ndxSuperColo = pBibleDatabase->calcRelIndex(0, 1, 0, 0, 0, ndxSuperColo, false);
		bExtended = true;
	} while (tagsToInclude.intersects(pBibleDatabase, TPhraseTag(ndxVerse)));

	// Add CrossRefs:
	if (flagsTRO & TRO_CrossRefs) {
		scriptureHTML.addCrossRefsFor(pBibleDatabase, ndxVerse, (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoCrossRefAnchors)), true);
	}

	scriptureHTML.endParagraph();

	if (flagsTRO & TRO_UserNotes)
		scriptureHTML.addNoteFor(pBibleDatabase, ndxVerse, (flagsTRO & TRO_UserNoteExpandAnchors), (flagsTRO & TRO_UserNotesForceVisible));

	if ((flagsTRO & TRO_InnerHTML) == 0) {
		scriptureHTML.appendRawText("</body></html>");
	}

	return scriptureHTML.getResult();
}

// ============================================================================

QString CTextRenderer::generateTextForFormattedVerses(const CBibleDatabase *pBibleDatabase,
													  qreal nIndentWidth,
													  const TPhraseTagList &lstPhraseTags,
													  TextRenderOptionFlags flagsTRO)
{
	return generateTextForFormattedVerses(pBibleDatabase, nIndentWidth, TPassageTagList(pBibleDatabase, lstPhraseTags), flagsTRO);
}

static QString referenceStartingDelimiter()
{
	switch (CPersistentSettings::instance()->referenceDelimiterMode()) {
		case RDME_NO_DELIMITER:
			return QString();
		case RDME_SQUARE_BRACKETS:
			return QString("[");
		case RDME_CURLY_BRACES:
			return QString("{");
		case RDME_PARENTHESES:
			return QString("(");
		default:
			Q_ASSERT(false);
			break;
	}
	return QString();
}

static QString referenceEndingDelimiter()
{
	switch (CPersistentSettings::instance()->referenceDelimiterMode()) {
		case RDME_NO_DELIMITER:
			return QString();
		case RDME_SQUARE_BRACKETS:
			return QString("]");
		case RDME_CURLY_BRACES:
			return QString("}");
		case RDME_PARENTHESES:
			return QString(")");
		default:
			Q_ASSERT(false);
			break;
	}
	return QString();
}

QString CTextRenderer::generateTextForFormattedVerses(const CBibleDatabase *pBibleDatabase,
													  qreal nIndentWidth,
													  const TPassageTagList &lstPassageTags,
													  TextRenderOptionFlags flagsTRO)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	CVerseTextRichifierTags richifierTags;
	richifierTags.setFromPersistentSettings(*CPersistentSettings::instance(), (flagsTRO & TRO_Copying));

	if ((lstPassageTags.isEmpty()) || (lstPassageTags.verseCount() == 0)) return QString();

	typedef QPair<CRelIndex, CRelIndex> TRelIndexPair;
	typedef QList<TRelIndexPair> TRelIndexPairList;

	QString strPassageReferenceRange;
	TRelIndexPairList lstFirstLastIndexes;

	CRelIndex ndxFirst;
	CRelIndex ndxLast;

	// Build list of overall first/last indexes and establish
	//	our outer-most first and last for the whole list:
	for (int ndx = 0; ndx < lstPassageTags.size(); ++ndx) {
		TPassageTag tagPassage = lstPassageTags.at(ndx);
		if (!tagPassage.isSet()) continue;
		CRelIndex ndxLocalFirst = pBibleDatabase->calcRelIndex(tagPassage.relIndex(), CBibleDatabase::RIME_Absolute);
		if (!ndxLocalFirst.isSet()) continue;	// Above absolute calculation can deem the reference invalid
		if (ndxLocalFirst != tagPassage.relIndex()) continue;		// If for some reason the above absolute calculation (normalization) changed our reference (i.e. incomplete text database), toss it as it isn't in this database anyway
		Q_ASSERT(ndxLocalFirst.word() == 1);		// Passages should always begin with the first word of a verse.  Plus this must point to first word so normalize will work correctly
		CRelIndex ndxLocalLast;
		if ((ndxLocalFirst.isColophon()) || (ndxLocalFirst.isSuperscription())) {
			ndxLocalLast = ndxLocalFirst;
		} else {
			ndxLocalLast = pBibleDatabase->calcRelIndex(0, tagPassage.verseCount()-1, 0, 0, 0, ndxLocalFirst);		// Add number of verses to find last verse to output
		}
		if (!ndxLocalLast.isSet()) continue;	// Note: If the passage tag we were given is totally outside of the text of the Bible Database, the calculate ndxLocalLast won't be set, so toss this entry
		Q_ASSERT(ndxLocalLast.word() == 1);		// Note: When we calculate next verse, we'll automatically resolve to the first word.  Leave it at 1st word so our loop compare will work

		if ((ndxLocalFirst.isColophon()) && (!CPersistentSettings::instance()->copyColophons())) continue;
		if ((ndxLocalFirst.isSuperscription()) && (!CPersistentSettings::instance()->copySuperscriptions())) continue;

		if (!strPassageReferenceRange.isEmpty()) strPassageReferenceRange += "; ";
		strPassageReferenceRange += tagPassage.PassageReferenceRangeText(pBibleDatabase);

		lstFirstLastIndexes.append(TRelIndexPair(ndxLocalFirst, ndxLocalLast));

		if (!ndxFirst.isSet()) ndxFirst = ndxLocalFirst;
		ndxLast = ndxLocalLast;
	}
	if (!ndxFirst.isSet() || !ndxLast.isSet()) return QString();		// If passage totally outside Bible Database, we have nothing to render

	CScriptureTextHtmlBuilder scriptureHTML;

	QString strCopyFont= "font-size:medium;";
	switch (CPersistentSettings::instance()->copyFontSelection()) {
		case CFSE_NONE:
			break;
		case CFSE_COPY_FONT:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontCopyFont().family()).arg(CPersistentSettings::instance()->fontCopyFont().pointSize());
			break;
		case CFSE_SCRIPTURE_BROWSER:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontScriptureBrowser().family()).arg(CPersistentSettings::instance()->fontScriptureBrowser().pointSize());
			break;
		case CFSE_SEARCH_RESULTS:
			strCopyFont = QString("font-family:'%1'; font-size:%2pt;").arg(CPersistentSettings::instance()->fontSearchResults().family()).arg(CPersistentSettings::instance()->fontSearchResults().pointSize());
			break;
		default:
			Q_ASSERT(false);
			break;
	}

	RichifierRenderOptionFlags flagsRRO = RRO_None;
	//if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoWordAnchors)) flagsRRO |= RRO_AddWordAnchors;	-- NoAnchors here, regardless of options
	//if (!(flagsTRO & TRO_NoAnchors) && !(flagsTRO & TRO_NoLinkAnchors)) flagsRRO |= RRO_AddLinkAnchors;	-- NoAnchors here, regardless of options
	if (flagsTRO & TRO_UseLemmas) flagsRRO |= RRO_UseLemmas;
	if (flagsTRO & TRO_UseWordSpans) flagsRRO |= RRO_UseWordSpans;
	if (flagsTRO & TRO_InlineFootnotes) flagsRRO |= RRO_InlineFootnotes;
	if (flagsTRO & TRO_EnableUserHighlighters) flagsRRO |= RRO_EnableUserHighlighters;

	VERSE_RENDERING_MODE_ENUM vrmeMode = ((flagsTRO & TRO_Copying) ?
											  CPersistentSettings::instance()->verseRenderingModeCopying() :
											  CPersistentSettings::instance()->verseRenderingMode());

	if ((flagsTRO & TRO_VPL_Only) && (vrmeMode == VRME_FF)) vrmeMode = VRME_VPL;

	if (flagsTRO & TRO_NoIndent) {
		if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_HANGING)) {
			vrmeMode = VRME_VPL;
		} else if ((vrmeMode == VRME_VPL_DS_INDENT) || (vrmeMode == VRME_VPL_DS_HANGING)) {
			vrmeMode = VRME_VPL_DS;
		}
	}

	//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
	//								"<html><head><title>%1</title><style type=\"text/css\">\n"
	//								"body, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n"
	//								".book { font-size:24pt; font-weight:bold; }\n"
	//								".chapter { font-size:18pt; font-weight:bold; }\n"
	//								".verse { }\n"
	//								".verselemma { }\n"
	//								".word { }\n"
	//								".stack { }\n"
	//								".main { }\n"
	//								".interlinear { }\n"
	//								".strongs { }\n"
	//								".morph { }\n"
	//								"</style></head><body>\n")
	//						.arg(scriptureHTML.escape(strPassageReferenceRange));										// Document Title

	//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
	//								"<html><head><title>%1</title><style type=\"text/css\">\n"
	//								"body, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n"
	//								".book { font-size:xx-large; font-weight:bold; }\n"
	//								".chapter { font-size:x-large; font-weight:bold; }\n"
	//								".verse { }\n"
	//								".verselemma { }\n"
	//								".word { }\n"
	//								".stack { }\n"
	//								".main { }\n"
	//								".interlinear { }\n"
	//								".strongs { }\n"
	//								".morph { }\n"
	//								"</style></head><body>\n")
	//						.arg(scriptureHTML.escape(strPassageReferenceRange));										// Document Title

	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
										"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n"
										"<title>%1</title><style type=\"text/css\">\n"
										"body, p, li, br, .bodyIndent { white-space: pre-wrap; %2 }\n"
										".book { font-size:xx-large; font-weight:bold; }\n"
										".chapter { font-size:x-large; font-weight:bold; }\n"
										".verse { }\n"
										".verselemma { }\n"
										".word { }\n"
										".stack { }\n"
										".main { }\n"
										".interlinear { }\n"
										".strongs { }\n"
										".morph { }\n"
										"</style></head><body>\n")
									.arg(scriptureHTML.escape(strPassageReferenceRange))								// Document Title
									.arg(strCopyFont));																	// Copy Font

	QString strReference;

	QString strBookNameFirst = (CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ? pBibleDatabase->bookNameAbbr(ndxFirst) : pBibleDatabase->bookName(ndxFirst));
	strReference += referenceStartingDelimiter();
	if ((ndxFirst.book() == ndxLast.book()) && (!ndxFirst.isColophon()) && (!ndxFirst.isSuperscription()) && (!ndxLast.isColophon()) && (!ndxLast.isSuperscription())) {
		if (ndxFirst.chapter() == ndxLast.chapter()) {
			if (ndxFirst.verse() == ndxLast.verse()) {
				strReference += (CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ?
									 pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true) :
									 pBibleDatabase->PassageReferenceText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true));
			} else {
				if (ndxFirst.chapter() != 0) {
					strReference += QString("%1 %2:%3-%4")
										.arg(strBookNameFirst)
										.arg(ndxFirst.chapter())
										.arg((!ndxFirst.isSuperscription()) ? QString("%1").arg(ndxFirst.verse()) : pBibleDatabase->translatedSuperscriptionString())
										.arg((!ndxLast.isSuperscription()) ? QString("%1").arg(ndxLast.verse()) : pBibleDatabase->translatedSuperscriptionString());
				} else {
					Q_ASSERT(false);		// Colophons (chapter==0) can't have superscriptions or verses
				}
			}
		} else {
			strReference += QString("%1 %2-%3")
								.arg(strBookNameFirst)
								.arg(pBibleDatabase->PassageReferenceText(CRelIndex(0, ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true))
								.arg(pBibleDatabase->PassageReferenceText(CRelIndex(0, ndxLast.chapter(), ndxLast.verse(), (((ndxLast.isColophon()) || (ndxLast.isSuperscription())) ? 1 : 0)), true));
		}
	} else {
		strReference += QString("%1-%2")
							.arg((CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ?
									  pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true) :
									  pBibleDatabase->PassageReferenceText(CRelIndex(ndxFirst.book(), ndxFirst.chapter(), ndxFirst.verse(), (((ndxFirst.isColophon()) || (ndxFirst.isSuperscription())) ? 1 : 0)), true)))
							.arg((CPersistentSettings::instance()->referencesUseAbbreviatedBookNames() ?
									  pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndxLast.book(), ndxLast.chapter(), ndxLast.verse(), (((ndxLast.isColophon()) || (ndxLast.isSuperscription())) ? 1 : 0)), true) :
									  pBibleDatabase->PassageReferenceText(CRelIndex(ndxLast.book(), ndxLast.chapter(), ndxLast.verse(), (((ndxLast.isColophon()) || (ndxLast.isSuperscription())) ? 1 : 0)), true)));
	}
	strReference += referenceEndingDelimiter();

	bool bInIndent = false;

	scriptureHTML.beginParagraph(pBibleDatabase->direction());

	if ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING)) {
		scriptureHTML.beginIndent(1, -nIndentWidth);
		bInIndent = true;
	}
	if ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT)) {
		scriptureHTML.beginIndent(0, nIndentWidth);
		bInIndent = true;
	}

	if (!CPersistentSettings::instance()->referencesAtEnd()) {
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strReference);
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.endBold();
		if (CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE) {
			scriptureHTML.addLineBreak();
		} else {
			scriptureHTML.appendLiteralText(" ");
		}
	}

	if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_COMPLETE_REFERENCE) ||
		(vrmeMode == VRME_FF)) {
		scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
	}

	CRelIndex ndxPrev = ndxFirst;
	if (CPersistentSettings::instance()->referencesAtEnd()) {
		// If printing the reference at the end, for printing of the initial verse number:
		ndxPrev.setVerse(-1);
		ndxPrev.setWord(-1);
	}
	for (int nIndexPair = 0; nIndexPair < lstFirstLastIndexes.size(); ++nIndexPair) {
		for (CRelIndex ndx = lstFirstLastIndexes.at(nIndexPair).first; ((ndx <= lstFirstLastIndexes.at(nIndexPair).second) && (ndx.isSet())); /* Increment inside */) {
			if (((vrmeMode == VRME_VPL) ||
				 (vrmeMode == VRME_VPL_DS) ||
				 (vrmeMode == VRME_VPL_DS_HANGING) ||
				 (vrmeMode == VRME_VPL_DS_INDENT)) &&
				(ndx != ndxFirst)) {
				scriptureHTML.addLineBreak();
				if ((vrmeMode == VRME_VPL_DS) ||
					(vrmeMode == VRME_VPL_DS_HANGING) ||
					(vrmeMode == VRME_VPL_DS_INDENT)) scriptureHTML.addLineBreak();
			}

			if ((bInIndent) && (ndx != ndxFirst)) {
				scriptureHTML.endIndent();
				bInIndent = false;
			}

			if ((!bInIndent) && ((vrmeMode == VRME_VPL_HANGING) || (vrmeMode == VRME_VPL_DS_HANGING))) {
				scriptureHTML.beginIndent(1, -nIndentWidth);
				bInIndent = true;
			}
			if ((!bInIndent) && ((vrmeMode == VRME_VPL_INDENT) || (vrmeMode == VRME_VPL_DS_INDENT))) {
				scriptureHTML.beginIndent(0, nIndentWidth);
				bInIndent = true;
			}

			if ((ndx.book() != ndxPrev.book()) &&
				(CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_COMPLETE_REFERENCE)) {
				if (vrmeMode == VRME_FF) {
					scriptureHTML.appendLiteralText("  ");
				}
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.beginBold();
				scriptureHTML.appendLiteralText(QString("%1%2%3")
													.arg(referenceStartingDelimiter())
													.arg(CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames() ?
															 pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true) :
															 pBibleDatabase->PassageReferenceText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true))
													.arg(referenceEndingDelimiter()));
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.endBold();
				scriptureHTML.appendLiteralText(" ");
			} else if ((ndx.chapter() != ndxPrev.chapter()) || (ndx.verse() != ndxPrev.verse()) ||
					   (CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE)) {
				if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_NO_NUMBER) &&
					(vrmeMode != VRME_VPL) && (vrmeMode != VRME_VPL_DS) &&
					(ndx != ndxFirst)) {
					scriptureHTML.appendLiteralText("  ");
				}
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.beginBold();

				QString strBookChapterVerse = QString("%1%2%3")
												  .arg(referenceStartingDelimiter())
												  .arg(CPersistentSettings::instance()->verseNumbersUseAbbreviatedBookNames() ?
														   pBibleDatabase->PassageReferenceAbbrText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true) :
														   pBibleDatabase->PassageReferenceText(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true))
												  .arg(referenceEndingDelimiter());
				QString strChapterVerse = pBibleDatabase->PassageReferenceText(CRelIndex((((ndx.isColophon()) || (ndx.isSuperscription())) ? ndx.book() : 0), ndx.chapter(), ndx.verse(), (((ndx.isColophon()) || (ndx.isSuperscription())) ? 1 : 0)), true);
				QString strVerse = ((!ndx.isSuperscription()) ? QString("%1").arg(ndx.verse()) : pBibleDatabase->translatedSuperscriptionString());
				switch (CPersistentSettings::instance()->verseNumberDelimiterMode()) {
					case RDME_NO_NUMBER:
						break;
					case RDME_NO_DELIMITER:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("%1").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("%1").arg(strVerse));
						}
						break;
					case RDME_SQUARE_BRACKETS:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("[%1]").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("[%1]").arg(strVerse));
						}
						break;
					case RDME_CURLY_BRACES:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("{%1}").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("{%1}").arg(strVerse));
						}
						break;
					case RDME_PARENTHESES:
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("(%1)").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("(%1)").arg(strVerse));
						}
						break;
					case RDME_SUPERSCRIPT:
						scriptureHTML.beginSuperscript();
						if (ndx.chapter() != ndxPrev.chapter()) {
							scriptureHTML.appendLiteralText(QString("%1").arg(strChapterVerse));
						} else {
							scriptureHTML.appendLiteralText(QString("%1").arg(strVerse));
						}
						scriptureHTML.endSuperscript();
						break;
					case RDME_COMPLETE_REFERENCE:
						scriptureHTML.appendLiteralText(QString("%1").arg(strBookChapterVerse));
						break;
					default:
						Q_ASSERT(false);
						break;
				}
				if (CPersistentSettings::instance()->verseNumbersInBold()) scriptureHTML.endBold();

				if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_NO_NUMBER) ||
					(vrmeMode == VRME_FF)) {
					scriptureHTML.appendLiteralText(" ");
				}
			}

			if ((CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE) &&
				(vrmeMode != VRME_FF)) {
				scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
			}

			scriptureHTML.appendRawText(QString("<span class=\"verse%1\">").arg((flagsTRO & TRO_UseLemmas) ? "lemma" : ""));
			scriptureHTML.appendRawText(pBibleDatabase->richVerseText(ndx, richifierTags, flagsRRO));
			scriptureHTML.appendRawText("</span>");

			if ((CPersistentSettings::instance()->verseNumberDelimiterMode() == RDME_COMPLETE_REFERENCE) &&
				(vrmeMode != VRME_FF)) {
				scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
			}

			ndxPrev = ndx;

			if ((!ndx.isColophon()) && (!ndx.isSuperscription())) {
				ndx = pBibleDatabase->calcRelIndex(0,1,0,0,0,ndx);
			} else {
				ndx.clear();
			}
		}
	}

	if ((CPersistentSettings::instance()->verseNumberDelimiterMode() != RDME_COMPLETE_REFERENCE) ||
		(vrmeMode == VRME_FF)) {
		scriptureHTML.appendLiteralText(QString("%1").arg(CPersistentSettings::instance()->addQuotesAroundVerse() ? "\"" : ""));
	}

	if (CPersistentSettings::instance()->referencesAtEnd()) {
		if ((vrmeMode == VRME_VPL) || (vrmeMode == VRME_VPL_DS)) {
			scriptureHTML.addLineBreak();
		} else {
			scriptureHTML.appendLiteralText(" ");
		}
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.beginBold();
		scriptureHTML.appendLiteralText(strReference);
		if (CPersistentSettings::instance()->referencesInBold()) scriptureHTML.endBold();
	}

	if ((bInIndent)) {
		scriptureHTML.endIndent();
		bInIndent = false;
	}

	scriptureHTML.endParagraph();
	scriptureHTML.appendRawText("</body></html>");

	return scriptureHTML.getResult();
}

// ============================================================================

QString CTextRenderer::getToolTip(TIP_EDIT_TYPE_ENUM nTipType, const CBibleDatabase *pBibleDatabase, const TPhraseTag &tag, const CSelectionPhraseTagList &selection, TOOLTIP_TYPE_ENUM nToolTipType, bool bPlainText)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	QString strToolTip;

	for (int ndxSel = 0; ((ndxSel < selection.size()) || (ndxSel == 0)); ++ndxSel) {
		bool bUseTag = !selection.haveSelection();

		bool bHaveSelection = (bUseTag ? false : selection.at(ndxSel).haveSelection());
		const CRelIndex &ndxReference(bUseTag ? tag.relIndex() : selection.at(ndxSel).relIndex());
		unsigned int nCount = (bUseTag ? tag.count() : selection.at(ndxSel).count());

		if (ndxReference.isSet()) {
			if (!strToolTip.isEmpty()) {
				if (!bPlainText) {
					strToolTip += "</pre><hr /><hr /><pre>";
				} else {
					strToolTip += "\n------------------------------------------------------------\n";
				}
			}
			if ((strToolTip.isEmpty()) && (!bPlainText)) strToolTip += "<html><body><pre>";
			if ((nToolTipType == TTE_COMPLETE) ||
				(nToolTipType == TTE_REFERENCE_ONLY)) {
				if (!bHaveSelection) {
					if (ndxReference.word() != 0) {
						uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndxReference);
						if ((ndxNormal != 0) && (ndxNormal <= pBibleDatabase->bibleEntry().m_nNumWrd)) {
							strToolTip += QObject::tr("Word:", "Statistics") + " \"" + pBibleDatabase->wordAtIndex(ndxNormal, WTE_RENDERED) + "\"\n";
						}
					}
					strToolTip += pBibleDatabase->SearchResultToolTip(ndxReference,
																	  (nTipType == TETE_DETAILS) ? RIMASK_ALL : RIMASK_HEADING);
				} else {
					strToolTip += QObject::tr("Phrase:", "Statistics") + " \"";
					uint32_t ndxNormal = pBibleDatabase->NormalizeIndex(ndxReference);
					if (ndxNormal != 0) {
						unsigned int ndx;
						for (ndx = 0; ((ndx < qMin(7u, nCount)) && ((ndxNormal + ndx) <= pBibleDatabase->bibleEntry().m_nNumWrd)); ++ndx) {
							if (ndx) strToolTip += " ";
							strToolTip += pBibleDatabase->wordAtIndex(ndxNormal + ndx, WTE_RENDERED);
						}
						if ((ndx == 7u) && (nCount > 7u)) strToolTip += " ...";
					} else {
						Q_ASSERT(false);
						strToolTip += "???";
					}
					strToolTip += "\"\n";
					strToolTip += pBibleDatabase->SearchResultToolTip(ndxReference,
																	  (nTipType == TETE_DETAILS) ? RIMASK_ALL : RIMASK_HEADING, nCount);
				}
			}
			if (nTipType == TETE_DETAILS) {
				if ((nToolTipType == TTE_COMPLETE) ||
					(nToolTipType == TTE_STATISTICS_ONLY)) {
					if (ndxReference.book() != 0) {
						Q_ASSERT(ndxReference.book() <= pBibleDatabase->bibleEntry().m_nNumBk);
						if (ndxReference.book() <= pBibleDatabase->bibleEntry().m_nNumBk) {
							if (nToolTipType == TTE_COMPLETE) {
								if (!bPlainText) {
									strToolTip += "</pre><hr /><pre>";
								} else {
									strToolTip += "--------------------\n";
								}
							}
							strToolTip += QString("\n%1 ").arg(pBibleDatabase->bookName(ndxReference)) +
											QObject::tr("contains:", "Statistics") + "\n"
											"    " + QObject::tr("%n Chapter(s)", "Statistics", pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) + "\n"
											"    " + QObject::tr("%n Verse(s)", "Statistics", pBibleDatabase->bookEntry(ndxReference.book())->m_nNumVrs) + "\n"
											"    " + QObject::tr("%n Word(s)", "Statistics", pBibleDatabase->bookEntry(ndxReference.book())->m_nNumWrd) + "\n";
							if (ndxReference.chapter() != 0) {
								Q_ASSERT(ndxReference.chapter() <= pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp);
								if (ndxReference.chapter() <= pBibleDatabase->bookEntry(ndxReference.book())->m_nNumChp) {
									strToolTip += QString("\n%1 %2 ").arg(pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()) +
													QObject::tr("contains:", "Statistics") + "\n"
													"    " + QObject::tr("%n Verse(s)", "Statistics", pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) + "\n"
													"    " + QObject::tr("%n Word(s)", "Statistics", pBibleDatabase->chapterEntry(ndxReference)->m_nNumWrd) + "\n";
									if ((!bHaveSelection) && (ndxReference.verse() != 0)) {
										Q_ASSERT(ndxReference.verse() <= pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs);
										if (ndxReference.verse() <= pBibleDatabase->chapterEntry(ndxReference)->m_nNumVrs) {
											strToolTip += QString("\n%1 %2:%3 ").arg(pBibleDatabase->bookName(ndxReference)).arg(ndxReference.chapter()).arg(ndxReference.verse()) +
																QObject::tr("contains:", "Statistics") + "\n"
																"    " + QObject::tr("%n Word(s)", "Statistics", pBibleDatabase->verseEntry(ndxReference)->m_nNumWrd) + "\n";
										}
									}
								}
							}
						}
					}
					if (bHaveSelection) {
						strToolTip += "\n" + QObject::tr("%n Word(s) Selected", "Statistics", nCount) + "\n";
					}
				}
			} else if (nTipType == TETE_GEMATRIA) {
#ifdef USE_GEMATRIA
				if (TBibleDatabaseList::useGematria()) {
					strToolTip += CGematriaCalc::tooltip(pBibleDatabase, TPhraseTag(ndxReference, nCount), bPlainText);
				}
#endif
			}
		}
	}
	if ((!strToolTip.isEmpty()) && (!bPlainText)) strToolTip += "</pre></body></html>";

	return strToolTip;
}

QString CTextRenderer::getFootnote(const CBibleDatabase *pBibleDatabase, const CRelIndex &ndx, bool bPlainText)
{
	Q_ASSERT(pBibleDatabase != nullptr);

	// First try footnote for word itself:
	const CFootnoteEntry *pFootnote = pBibleDatabase->footnoteEntry(ndx);
	if (pFootnote == nullptr) {
		// Then try for next word location:
		pFootnote = pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), ndx.word()+1));
	}
	if ((pFootnote == nullptr) && (ndx.word() != 0)) {
		// Then see if there's a stacked up footnote off the end of the verse:
		const CVerseEntry *pVerse = pBibleDatabase->verseEntry(ndx);
		if (pVerse) {
			pFootnote = pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), pVerse->m_nNumWrd+1));
		}
	}

	QString strFootnote;
	const CFootnoteEntry *pFootnoteVerse = pBibleDatabase->footnoteEntry(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0));
	if (pFootnote == nullptr) {
		// Finally, try the entire verse itself as last resort (example: special colophon/superscription pseudo-footnotes)
		pFootnote = pFootnoteVerse;
	} else {
		// But if we had other notes after the verse note, prepend with the verse note for consistency (like colophon/superscription above)
		if (pFootnoteVerse) {
			strFootnote += bPlainText ? pFootnoteVerse->plainText(pBibleDatabase).trimmed() : pFootnoteVerse->htmlText(pBibleDatabase);
			strFootnote += "; ";	// We already know we'll be appending more text below since pFootnote isn't nullptr
		}
	}

	if (pFootnote) {
		strFootnote += bPlainText ? pFootnote->plainText(pBibleDatabase).trimmed() : pFootnote->htmlText(pBibleDatabase);
	}

	return strFootnote;
}

// ============================================================================
