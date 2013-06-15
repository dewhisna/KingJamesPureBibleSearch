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

#include "Highlighter.h"
#include "VerseListModel.h"

#include <QVariant>
#include <QBrush>
#include <QTextFormat>

//
// UserProperties:
//
#define USERPROP_FOREGROUND_BRUSH		0			//		+0		Foreground Brush
#define USERPROP_BACKGROUND_BRUSH		1			//		+1		Background Brush
#define USERPROP_UNDERLINE_COLOR		2			//		+2		Underline Color
#define USERPROP_UNDERLINE_STYLE		3			//		+3		Underline Style

// ============================================================================

CHighlighterPhraseTagFwdItr::CHighlighterPhraseTagFwdItr(const CVerseListModel *pVerseListModel)
	:	m_pVerseListModel(pVerseListModel),
		m_lstPhraseTags(m_lstDummyPhraseTags)
{
	assert(m_pVerseListModel != NULL);
	m_itrVerses = m_pVerseListModel->verseList().constBegin();
	while (m_itrVerses != m_pVerseListModel->verseList().constEnd()) {
		m_itrTags = m_itrVerses->phraseTags().constBegin();
		if (m_itrTags != m_itrVerses->phraseTags().constEnd()) break;
		++m_itrVerses;
	}
}

CHighlighterPhraseTagFwdItr::CHighlighterPhraseTagFwdItr(const TPhraseTagList &lstTags)
	:	m_pVerseListModel(NULL),
		m_lstPhraseTags(lstTags)
{
	m_itrTags = m_lstPhraseTags.constBegin();
}

TPhraseTag CHighlighterPhraseTagFwdItr::nextTag()
{
	TPhraseTag nRetVal = (!isEnd() ? *m_itrTags : TPhraseTag());

	if (m_pVerseListModel) {
		if (m_itrVerses != m_pVerseListModel->verseList().constEnd()) {
			++m_itrTags;
			if (m_itrTags != m_itrVerses->phraseTags().constEnd()) return nRetVal;
		}
		++m_itrVerses;
		while (m_itrVerses != m_pVerseListModel->verseList().constEnd()) {
			m_itrTags = m_itrVerses->phraseTags().constBegin();
			if (m_itrTags != m_itrVerses->phraseTags().constEnd()) break;
			++m_itrVerses;
		}
	} else {
		if (m_itrTags != m_lstPhraseTags.constEnd()) ++m_itrTags;
	}

	return nRetVal;
}

bool CHighlighterPhraseTagFwdItr::isEnd() const
{
	if (m_pVerseListModel) {
		return (m_itrVerses == m_pVerseListModel->verseList().constEnd());
	} else {
		return (m_itrTags == m_lstPhraseTags.constEnd());
	}
}

// ============================================================================

CSearchResultHighlighter::CSearchResultHighlighter(CVerseListModel *pVerseListModel, QObject *parent)
	:	CBasicHighlighter(parent),
		m_pVerseListModel(pVerseListModel)
{
	assert(pVerseListModel);

	connect(pVerseListModel, SIGNAL(destroyed()), this, SLOT(verseListModelDestroyed()));
	connect(pVerseListModel, SIGNAL(verseListChanged()), this, SLOT(verseListChanged()));
}

CSearchResultHighlighter::CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags, QObject *parent)
	:	CBasicHighlighter(parent),
		  m_pVerseListModel(NULL)
{
	m_myPhraseTags.setPhraseTags(lstPhraseTags);
}

CSearchResultHighlighter::CSearchResultHighlighter(const TPhraseTag &aTag, QObject *parent)
	:	CBasicHighlighter(parent),
		m_pVerseListModel(NULL)
{
	TPhraseTagList lstTags;
	lstTags.append(aTag);
	m_myPhraseTags.setPhraseTags(lstTags);
}

CSearchResultHighlighter::~CSearchResultHighlighter()
{
	if (m_pVerseListModel) {
		m_pVerseListModel->disconnect(this);
		m_pVerseListModel = NULL;
	}
}

void CSearchResultHighlighter::doHighlighting(QTextCharFormat &aFormat, bool bClear) const
{
	if ((!bClear) && (enabled())) {
		aFormat.setProperty(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH, QVariant(aFormat.foreground()));
		aFormat.setForeground(QBrush(QColor("blue")));				// TODO : Get properties from global settings!
	} else {
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH))
			aFormat.setForeground(aFormat.property(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH).value<QBrush>());
	}
}

void CSearchResultHighlighter::verseListChanged()
{
	assert(m_pVerseListModel != NULL);
	if (m_pVerseListModel == NULL) return;

	emit phraseTagsChanged();
}

void CSearchResultHighlighter::verseListModelDestroyed()
{
	if (m_pVerseListModel) {
		m_pVerseListModel->disconnect(this);
		m_pVerseListModel = NULL;
	}

	// Note: This switches us back to the internal PhraseTags list, which should
	//			be empty.  We could assert that it is, but we may decide later on
	//			to make use of this feature and allow connecting/disconnecting
	//			with arbitrary verse lists.

	emit phraseTagsChanged();
}

CHighlighterPhraseTagFwdItr CSearchResultHighlighter::getForwardIterator() const
{
	if (m_pVerseListModel) {
		return CHighlighterPhraseTagFwdItr(m_pVerseListModel);
	} else {
		return CHighlighterPhraseTagFwdItr(m_myPhraseTags.phraseTags());
	}
}

bool CSearchResultHighlighter::isEmpty() const
{
	if (m_pVerseListModel) {
		return (m_pVerseListModel->verseList().isEmpty());		// Our highlighter PhraseTags could technically be empty and not trigger this, but for the purposes of this function this highlighter "isn't empty" if we have verses
	} else {
		return m_myPhraseTags.phraseTags().isEmpty();
	}
}

// ============================================================================

Q_DECLARE_METATYPE(QTextCharFormat::UnderlineStyle)

void CCursorFollowHighlighter::doHighlighting(QTextCharFormat &aFormat, bool bClear) const
{
	if ((!bClear) && (enabled())) {
		aFormat.setProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR, QVariant(aFormat.underlineColor()));
		aFormat.setProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE, QVariant(aFormat.underlineStyle()));
		aFormat.setUnderlineColor(QColor("blue"));							// TODO : Get properties from global settings!
		aFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);		// TODO : Get properties from global settings!
	} else {
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR))
			aFormat.setUnderlineColor(aFormat.property(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR).value<QColor>());
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE))
			aFormat.setUnderlineStyle(aFormat.property(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE).value<QTextCharFormat::UnderlineStyle>());
	}
}

CHighlighterPhraseTagFwdItr CCursorFollowHighlighter::getForwardIterator() const
{
	return CHighlighterPhraseTagFwdItr(m_myPhraseTags.phraseTags());
}

bool CCursorFollowHighlighter::isEmpty() const
{
	return m_myPhraseTags.phraseTags().isEmpty();
}

const TPhraseTagList &CCursorFollowHighlighter::phraseTags() const
{
	return m_myPhraseTags.phraseTags();
}

void CCursorFollowHighlighter::setPhraseTags(const TPhraseTagList &lstPhraseTags)
{
	m_myPhraseTags.setPhraseTags(lstPhraseTags);
	emit phraseTagsChanged();
}

void CCursorFollowHighlighter::clearPhraseTags()
{
	m_myPhraseTags.setPhraseTags(TPhraseTagList());
	emit phraseTagsChanged();
}

// ============================================================================


