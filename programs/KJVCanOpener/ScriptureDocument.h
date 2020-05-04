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

#ifndef SCRIPTURE_DOCUMENT_H
#define SCRIPTURE_DOCUMENT_H

#include "dbstruct.h"

#include <grantlee_textdocument.h>
#include <QTextFragment>
#include <QTextDocument>
#include <QTextBlockFormat>
#include <QObject>
#include <QTextObjectInterface>
#include <QPainter>
#include <QRectF>
#include <QSizeF>
#include <QAbstractTextDocumentLayout>

// ============================================================================

class CKJPBSWordScriptureObject : public QObject, public QTextObjectInterface
{
	Q_OBJECT
	Q_INTERFACES(QTextObjectInterface)

public:
	CKJPBSWordScriptureObject(CBibleDatabase *pBibleDatabase);
	virtual ~CKJPBSWordScriptureObject();

	virtual void registerTextLayoutHandlers(QAbstractTextDocumentLayout *pDocLayout);

	virtual void drawObject(QPainter *pPainter, const QRectF &aRect, QTextDocument *pDoc, int posInDocument, const QTextFormat &aFormat);
	virtual QSizeF intrinsicSize(QTextDocument *pDoc, int posInDocument, const QTextFormat &aFormat);

private:
	CBibleDatabase *m_pBibleDatabase;			// Note: Not QSharedPointer version because the CBibleDatabase actually owns us
};

// ============================================================================

class CAbstractScriptureBuilderBase : virtual public Grantlee::AbstractMarkupBuilder
{
public:
	virtual ~CAbstractScriptureBuilderBase() { }

	virtual void beginAnchorID(const QString &strID) { Q_UNUSED(strID); }

	virtual void beginBold() { }
	virtual void endBold() { }

	virtual void beginItalics() { }
	virtual void endItalics() { }

	virtual void beginFontRelativeSize(const QString &strSize) { Q_UNUSED(strSize); }
	virtual void endFontRelativeSize() { }

	virtual void beginDiv(const QString &strClass = QString(), const QString &strStyle = QString())
	{
		Q_UNUSED(strClass);
		Q_UNUSED(strStyle);
	}
	virtual void endDiv() { }

	virtual bool addKJPBSWord(const CBibleDatabase *pBibleDatabase, const CRelIndex &relIndex) = 0;

	virtual bool addNoteFor(const CRelIndex &relNdx, bool bAddExpandAnchor, bool bForceVisible, bool bAddLeadInSpace) = 0;
	virtual bool addFootnoteFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors) = 0;

	virtual bool addCrossRefsFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors, bool bAddLeadInSpace) = 0;
	virtual void addRefLinkFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors, bool bAddLeadInSpace) = 0;
	virtual void addWWWLinkFor(const QString &strURL, bool bAddAnchors, bool bAddLeadInSpace) = 0;
};

// ============================================================================

template <class T>
class CAbstractScriptureBuilder : virtual public CAbstractScriptureBuilderBase, virtual public T
{
public:
	CAbstractScriptureBuilder();
	virtual ~CAbstractScriptureBuilder();

	virtual void appendRawText( const QString &text );
	virtual QString getResult();

	virtual void startBuffered();
	virtual void stopBuffered();
	virtual void flushBuffer(bool bStopBuffering = false);
	virtual void clearBuffer(bool bStopBuffering = false);

private:
	QString m_strBufferedText;		// Buffered text for temporarily outputting to so caller chain can insert additional text ahead of the parse stream
	bool m_bBuffered;				// True if buffering output
};

// ============================================================================

class CScriptureTextHtmlBuilder : public CAbstractScriptureBuilder<Grantlee::TextHTMLBuilder>
{
public:
	CScriptureTextHtmlBuilder();
	virtual ~CScriptureTextHtmlBuilder();

	virtual void beginAnchorID(const QString &strID);

	virtual void beginBold();
	virtual void endBold();

	virtual void beginItalics();
	virtual void endItalics();

	virtual void beginFontRelativeSize(const QString &strSize);
	virtual void endFontRelativeSize();

	virtual void beginDiv(const QString &strClass = QString(), const QString &strStyle = QString());
	virtual void endDiv();

	virtual bool addKJPBSWord(const CBibleDatabase *pBibleDatabase, const CRelIndex &relIndex);

	virtual bool addNoteFor(const CRelIndex &relNdx, bool bAddExpandAnchor = true, bool bForceVisible = false, bool bAddLeadInSpace = false);
	virtual bool addFootnoteFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors = true);

	virtual bool addCrossRefsFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors = true, bool bAddLeadInSpace = false);
	virtual void addRefLinkFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors = true, bool bAddLeadInSpace = false);
	virtual void addWWWLinkFor(const QString &strURL, bool bAddAnchors = false, bool bAddLeadInSpace = false);
};

// ============================================================================

class CScripturePlainTextBuilder : public CAbstractScriptureBuilder<Grantlee::PlainTextMarkupBuilder>
{
public:
	CScripturePlainTextBuilder();
	virtual ~CScripturePlainTextBuilder();

	virtual void beginItalics();
	virtual void endItalics();

	virtual void endDiv();

	virtual bool addKJPBSWord(const CBibleDatabase *pBibleDatabase, const CRelIndex &relIndex);

	virtual bool addNoteFor(const CRelIndex &relNdx, bool bAddExpandAnchor = true, bool bForceVisible = false, bool bAddLeadInSpace = false);
	virtual bool addFootnoteFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors = true);

	virtual bool addCrossRefsFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors = true, bool bAddLeadInSpace = false);
	virtual void addRefLinkFor(const CBibleDatabase *pBibleDatabase, const CRelIndex &relNdx, bool bAddAnchors = true, bool bAddLeadInSpace = false);
	virtual void addWWWLinkFor(const QString &strURL, bool bAddAnchors = false, bool bAddLeadInSpace = false);
};

// ============================================================================

class CScriptureTextDocumentDirector : public Grantlee::MarkupDirector
{
public:
	CScriptureTextDocumentDirector(CAbstractScriptureBuilderBase *pBuilder, const CBibleDatabase *pBibleDatabase = NULL);
	virtual ~CScriptureTextDocumentDirector();

	virtual void processCustomFragment(const QTextFragment &aFragment, const QTextDocument *pDoc);

protected:
	const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase; }

private:
	CAbstractScriptureBuilderBase *m_pBuilder;
	const CBibleDatabase *m_pBibleDatabase;
};

// ============================================================================

#endif	// SCRIPTURE_DOCUMENT_H
