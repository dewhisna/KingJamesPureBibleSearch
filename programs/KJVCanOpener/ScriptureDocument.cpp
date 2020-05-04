/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ScriptureDocument.h"
#include "ScriptureTextFormatProperties.h"

#include "UserNotesDatabase.h"

#include <assert.h>

// ============================================================================

CKJPBSWordScriptureObject::CKJPBSWordScriptureObject(CBibleDatabase *pBibleDatabase)
	:	QObject(NULL),
		m_pBibleDatabase(pBibleDatabase)
{
	assert(pBibleDatabase != NULL);
}

CKJPBSWordScriptureObject::~CKJPBSWordScriptureObject()
{

}

void CKJPBSWordScriptureObject::registerTextLayoutHandlers(QAbstractTextDocumentLayout *pDocLayout)
{
	assert(pDocLayout != NULL);
	pDocLayout->registerHandler(USEROBJ_KJPBS_WORD, this);
}

void CKJPBSWordScriptureObject::drawObject(QPainter *pPainter, const QRectF &aRect, QTextDocument *pDoc, int posInDocument, const QTextFormat &aFormat)
{
	// TODO : COMPLETE
}

QSizeF CKJPBSWordScriptureObject::intrinsicSize(QTextDocument *pDoc, int posInDocument, const QTextFormat &aFormat)
{
	// TODO : COMPLETE

	return QSizeF();
}

// ============================================================================

template <class T>
CAbstractScriptureBuilder<T>::CAbstractScriptureBuilder()
	:	m_bBuffered(false)
{

}

template <class T>
CAbstractScriptureBuilder<T>::~CAbstractScriptureBuilder()
{

}

template <class T>
void CAbstractScriptureBuilder<T>::appendRawText( const QString &text )
{
	if (m_bBuffered) {
		m_strBufferedText.append(text);
	} else {
		T::appendRawText(text);
	}
}

template <class T>
QString CAbstractScriptureBuilder<T>::getResult()
{
	flushBuffer(true);
	return T::getResult();
}

template <class T>
void CAbstractScriptureBuilder<T>::startBuffered()
{
	m_bBuffered = true;
}

template <class T>
void CAbstractScriptureBuilder<T>::stopBuffered()
{
	m_bBuffered = false;
}

template <class T>
void CAbstractScriptureBuilder<T>::flushBuffer(bool bStopBuffering)
{
	if (!m_strBufferedText.isEmpty()) {
		T::appendRawText(m_strBufferedText);
	}
	clearBuffer(bStopBuffering);
}

template <class T>
void CAbstractScriptureBuilder<T>::clearBuffer(bool bStopBuffering)
{
	m_strBufferedText.clear();
	if (bStopBuffering) m_bBuffered = false;
}

// ============================================================================

template class CAbstractScriptureBuilder<Grantlee::TextHTMLBuilder>;
template class CAbstractScriptureBuilder<Grantlee::PlainTextMarkupBuilder>;

// ============================================================================

CScriptureTextHtmlBuilder::CScriptureTextHtmlBuilder()
{

}

CScriptureTextHtmlBuilder::~CScriptureTextHtmlBuilder()
{

}

void CScriptureTextHtmlBuilder::beginAnchorID(const QString &strID)
{
	if (!strID.isEmpty()) {
		appendRawText(QString::fromLatin1("<a id=\"%1\">").arg(strID));
	}
}

void CScriptureTextHtmlBuilder::beginBold()
{
	appendRawText(QString::fromLatin1("<b>"));
}

void CScriptureTextHtmlBuilder::endBold()
{
	appendRawText(QString::fromLatin1("</b>"));
}

void CScriptureTextHtmlBuilder::beginItalics()
{
	appendRawText(QString::fromLatin1("<i>"));
}

void CScriptureTextHtmlBuilder::endItalics()
{
	appendRawText(QString::fromLatin1("</i>"));
}

void CScriptureTextHtmlBuilder::beginFontRelativeSize(const QString &strSize)
{
	appendRawText( QString::fromLatin1( "<span style=\"font-size:%1;\">" ).arg(strSize) );
}

void CScriptureTextHtmlBuilder::endFontRelativeSize()
{
	appendRawText( QLatin1String( "</span>" ) );
}

void CScriptureTextHtmlBuilder::beginDiv(const QString &strClass, const QString &strStyle)
{
	if (!strClass.isEmpty()) {
		if (!strStyle.isEmpty()) {
			appendRawText(QString::fromLatin1("<div class=\"%1\" style=\"%2\">").arg(strClass).arg(strStyle));
		} else {
			appendRawText(QString::fromLatin1("<div class=\"%1\">").arg(strClass));
		}
	} else {
		if (!strStyle.isEmpty()) {
			appendRawText(QString::fromLatin1("<div style=\"%1\">").arg(strStyle));
		} else {
			appendRawText(QString::fromLatin1("<div>"));
		}
	}
}

void CScriptureTextHtmlBuilder::endDiv()
{
	appendRawText(QString::fromLatin1("</div>\n"));
}

bool CScriptureTextHtmlBuilder::addKJPBSWord(const CBibleDatabase *pBibleDatabase, const CRelIndex &relIndex)
{
	if (!relIndex.isSet()) return false;
	assert(pBibleDatabase != NULL);
	QString strWordAt = pBibleDatabase->wordAtIndex(relIndex);
	appendRawText(QString("<KJPBSWord RelIndex=\"%1\" />").arg(relIndex.asAnchor()));			// TODO: Embed Text in Object???
	return (!strWordAt.isEmpty());
}

bool CScriptureTextHtmlBuilder::addNoteFor(const CRelIndex &relNdx, bool bAddExpandAnchor, bool bForceVisible, bool bAddLeadInSpace)
{
	assert(!g_pUserNotesDatabase.isNull());

	if (g_pUserNotesDatabase->existsNoteFor(relNdx)) {
		CUserNoteEntry userNote = g_pUserNotesDatabase->noteFor(relNdx);
		if (bForceVisible || userNote.isVisible()) {
			if (bAddExpandAnchor) {
				beginAnchor(QString("N%1").arg(relNdx.asAnchor()));
				appendLiteralText(QString::fromLatin1("[-]"));
				endAnchor();
				insertHorizontalRule();
			}
			if (!userNote.keywordList().isEmpty()) {
				beginParagraph();
				beginFontRelativeSize("xx-small");
				appendLiteralText(QObject::tr("Keywords:", "Scope") + " ");
				beginItalics();
				appendLiteralText(userNote.keywordList().join(", "));
				endItalics();
				endFontRelativeSize();
				endParagraph();
			} else {
				if (!bAddExpandAnchor) addLineBreak();
			}
			appendRawText(userNote.htmlText());
		} else {
			if (bAddExpandAnchor) {
				if (bAddLeadInSpace) appendLiteralText(" ");
				beginAnchor(QString("N%1").arg(relNdx.asAnchor()));
				appendLiteralText(QString::fromLatin1("[+]"));
				endAnchor();
			}
		}
		return (bForceVisible | userNote.isVisible());
	} else {
		return false;
	}
}

bool CScriptureTextHtmlBuilder::addFootnoteFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors)
{
	assert(pBibleDatabase != NULL);

	const CFootnoteEntry *pFootnote = pBibleDatabase->footnoteEntry(relNdx);
	if (pFootnote) {
		beginParagraph();
		if (bAddAnchors) beginAnchorID(relNdx.asAnchor());
		appendRawText(pFootnote->htmlText(pBibleDatabase));
		if (bAddAnchors) endAnchor();
		endParagraph();
		return true;
	} else {
		return false;
	}
}

bool CScriptureTextHtmlBuilder::addCrossRefsFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors, bool bAddLeadInSpace)
{
	assert(pBibleDatabase != NULL);
	assert(!g_pUserNotesDatabase.isNull());

	const TCrossReferenceMap mapCrossRefs = g_pUserNotesDatabase->crossRefsMap().createScopedMap(pBibleDatabase);
	if (mapCrossRefs.haveCrossReferencesFor(relNdx)) {
		const TRelativeIndexSet refs = mapCrossRefs.crossReferencesFor(relNdx);
		bool bNext = false;
		if (bAddLeadInSpace) appendLiteralText(" ");
		appendLiteralText("[");
		for (TRelativeIndexSet::const_iterator itrRefs = refs.begin(); itrRefs != refs.end(); ++itrRefs) {
			if (bNext) appendLiteralText(", ");
			if (bAddAnchors) beginAnchor(QString("R%1").arg(itrRefs->asAnchor()));
			appendLiteralText(pBibleDatabase->PassageReferenceAbbrText(*itrRefs));
			if (bAddAnchors) endAnchor();
			bNext = true;
		}
		appendLiteralText("]");
		return true;
	} else {
		return false;
	}
}

void CScriptureTextHtmlBuilder::addRefLinkFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors, bool bAddLeadInSpace)
{
	assert(pBibleDatabase != NULL);

	if (bAddLeadInSpace) appendLiteralText(" ");
	appendLiteralText("[");
	if (bAddAnchors) beginAnchor(QString("R%1").arg(relNdx.asAnchor()));
	appendLiteralText(pBibleDatabase->PassageReferenceAbbrText(relNdx));
	if (bAddAnchors) endAnchor();
	appendLiteralText("]");
}

void CScriptureTextHtmlBuilder::addWWWLinkFor(const QString &strURL, bool bAddAnchors, bool bAddLeadInSpace)
{
	if (bAddLeadInSpace) appendLiteralText(" ");
	appendLiteralText("[");
	if (bAddAnchors) beginAnchor(QString("%1").arg(strURL));
	appendLiteralText(strURL);
	if (bAddAnchors) endAnchor();
	appendLiteralText("]");
}


// ============================================================================

CScripturePlainTextBuilder::CScripturePlainTextBuilder()
{

}

CScripturePlainTextBuilder::~CScripturePlainTextBuilder()
{

}

void CScripturePlainTextBuilder::beginItalics()
{
	appendLiteralText(QString::fromLatin1("["));
}

void CScripturePlainTextBuilder::endItalics()
{
	appendLiteralText(QString::fromLatin1("]"));
}

void CScripturePlainTextBuilder::endDiv()
{
	appendLiteralText(QString::fromLatin1("\n"));
}

bool CScripturePlainTextBuilder::addKJPBSWord(const CBibleDatabase *pBibleDatabase, const CRelIndex &relIndex)
{
	if (!relIndex.isSet()) return false;
	assert(pBibleDatabase != NULL);
	QString strWordAt = pBibleDatabase->wordAtIndex(relIndex);
	appendRawText(strWordAt);
	return (!strWordAt.isEmpty());
}

bool CScripturePlainTextBuilder::addNoteFor(const CRelIndex &relNdx, bool bAddExpandAnchor, bool bForceVisible, bool bAddLeadInSpace)
{
	assert(!g_pUserNotesDatabase.isNull());

	if (g_pUserNotesDatabase->existsNoteFor(relNdx)) {
		CUserNoteEntry userNote = g_pUserNotesDatabase->noteFor(relNdx);
		if (bForceVisible || userNote.isVisible()) {
			if (bAddExpandAnchor) {
				beginAnchor(QString("N%1").arg(relNdx.asAnchor()));
				appendLiteralText(QString::fromLatin1("[-]"));
				endAnchor();
				insertHorizontalRule();
			}
			if (!userNote.keywordList().isEmpty()) {
				beginParagraph();
				beginFontRelativeSize("xx-small");
				appendLiteralText(QObject::tr("Keywords:", "Scope") + " ");
				beginItalics();
				appendLiteralText(userNote.keywordList().join(", "));
				endItalics();
				endFontRelativeSize();
				endParagraph();
			} else {
				if (!bAddExpandAnchor) addLineBreak();
			}
			appendRawText(userNote.plainText());
		} else {
			if (bAddExpandAnchor) {
				if (bAddLeadInSpace) appendLiteralText(" ");
				beginAnchor(QString("N%1").arg(relNdx.asAnchor()));
				appendLiteralText(QString::fromLatin1("[+]"));
				endAnchor();
			}
		}
		return (bForceVisible | userNote.isVisible());
	} else {
		return false;
	}
}

bool CScripturePlainTextBuilder::addFootnoteFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors)
{
	assert(pBibleDatabase != NULL);

	const CFootnoteEntry *pFootnote = pBibleDatabase->footnoteEntry(relNdx);
	if (pFootnote) {
		beginParagraph();
		if (bAddAnchors) beginAnchorID(relNdx.asAnchor());
		appendRawText(pFootnote->plainText(pBibleDatabase));
		if (bAddAnchors) endAnchor();
		endParagraph();
		return true;
	} else {
		return false;
	}
}

bool CScripturePlainTextBuilder::addCrossRefsFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors, bool bAddLeadInSpace)
{
	assert(pBibleDatabase != NULL);
	assert(!g_pUserNotesDatabase.isNull());

	const TCrossReferenceMap mapCrossRefs = g_pUserNotesDatabase->crossRefsMap().createScopedMap(pBibleDatabase);
	if (mapCrossRefs.haveCrossReferencesFor(relNdx)) {
		const TRelativeIndexSet refs = mapCrossRefs.crossReferencesFor(relNdx);
		bool bNext = false;
		if (bAddLeadInSpace) appendLiteralText(" ");
		appendLiteralText("[");
		for (TRelativeIndexSet::const_iterator itrRefs = refs.begin(); itrRefs != refs.end(); ++itrRefs) {
			if (bNext) appendLiteralText(", ");
			if (bAddAnchors) beginAnchor(QString("R%1").arg(itrRefs->asAnchor()));
			appendLiteralText(pBibleDatabase->PassageReferenceAbbrText(*itrRefs));
			if (bAddAnchors) endAnchor();
			bNext = true;
		}
		appendLiteralText("]");
		return true;
	} else {
		return false;
	}
}


void CScripturePlainTextBuilder::addRefLinkFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors, bool bAddLeadInSpace)
{
	assert(pBibleDatabase != NULL);

	if (bAddLeadInSpace) appendLiteralText(" ");
	appendLiteralText("[");
	if (bAddAnchors) beginAnchor(QString("R%1").arg(relNdx.asAnchor()));
	appendLiteralText(pBibleDatabase->PassageReferenceAbbrText(relNdx));
	if (bAddAnchors) endAnchor();
	appendLiteralText("]");
}

void CScripturePlainTextBuilder::addWWWLinkFor(const QString &strURL, bool bAddAnchors, bool bAddLeadInSpace)
{
	if (bAddLeadInSpace) appendLiteralText(" ");
	appendLiteralText("[");
	if (bAddAnchors) beginAnchor(QString("%1").arg(strURL));
	appendLiteralText(strURL);
	if (bAddAnchors) endAnchor();
	appendLiteralText("]");
}

// ============================================================================

CScriptureTextDocumentDirector::CScriptureTextDocumentDirector(CAbstractScriptureBuilderBase *pBuilder, const CBibleDatabase *pBibleDatabase)
	:	MarkupDirector(pBuilder),
		m_pBuilder(pBuilder),
		m_pBibleDatabase(pBibleDatabase)
{
	assert(pBuilder != NULL);
}

CScriptureTextDocumentDirector::~CScriptureTextDocumentDirector()
{

}

void CScriptureTextDocumentDirector::processCustomFragment(const QTextFragment &aFragment, const QTextDocument *pDoc)
{
	if (aFragment.charFormat().objectType() != USEROBJ_KJPBS_WORD)
		return Grantlee::MarkupDirector::processCustomFragment(aFragment, pDoc);

	CRelIndex relIndex(aFragment.charFormat().property(USERPROP_RELINDEX).toString());
	m_pBuilder->addKJPBSWord(bibleDatabase(), relIndex);
}

// ============================================================================

