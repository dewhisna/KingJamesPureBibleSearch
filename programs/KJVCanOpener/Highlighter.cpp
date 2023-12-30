/****************************************************************************
**
** Copyright (C) 2012-2022 Donna Whisnant, a.k.a. Dewtronics.
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
#include "ScriptureTextFormatProperties.h"
#include "VerseListModel.h"
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"

#include <QVariant>
#include <QBrush>
#include <QTextFormat>

// Nasty intermediate class type defintion for CVerseListModel::TVerseListModelResults, but avoids very nasty header interdependency:
class i_TVerseListModelResults
{
public:
	i_TVerseListModelResults(const CVerseListModel::TVerseListModelResults &aData)
		:	data(aData)
	{

	}

	const CVerseListModel::TVerseListModelResults &data;
};

// ============================================================================

CHighlighterPhraseTagFwdItr::CHighlighterPhraseTagFwdItr(const i_TVerseListModelResults *pvlmResults)
	:	m_pvlmResults(pvlmResults),
		m_lstPhraseTags(m_lstDummyPhraseTags)
{
	Q_ASSERT(pvlmResults != nullptr);
	m_itrVerses = m_pvlmResults->data.verseMap().constBegin();
	while (m_itrVerses != m_pvlmResults->data.verseMap().constEnd()) {
		m_itrTags = m_itrVerses->phraseTags().constBegin();
		if (m_itrTags != m_itrVerses->phraseTags().constEnd()) break;
		++m_itrVerses;
	}
}

CHighlighterPhraseTagFwdItr::CHighlighterPhraseTagFwdItr(const TPhraseTagList &lstTags)
	:	m_pvlmResults(nullptr),
		m_lstPhraseTags(lstTags)
{
	m_itrTags = m_lstPhraseTags.constBegin();
}

CHighlighterPhraseTagFwdItr::~CHighlighterPhraseTagFwdItr()
{
	if (m_pvlmResults) {
		delete m_pvlmResults;
		m_pvlmResults = nullptr;
	}
}

TPhraseTag CHighlighterPhraseTagFwdItr::nextTag()
{
	TPhraseTag nRetVal = (!isEnd() ? *m_itrTags : TPhraseTag());

	if (m_pvlmResults) {
		if (m_itrVerses != m_pvlmResults->data.verseMap().constEnd()) {
			++m_itrTags;
			if (m_itrTags != m_itrVerses->phraseTags().constEnd()) return nRetVal;
		}
		++m_itrVerses;
		while (m_itrVerses != m_pvlmResults->data.verseMap().constEnd()) {
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
	if (m_pvlmResults) {
		return (m_itrVerses == m_pvlmResults->data.verseMap().constEnd());
	} else {
		return (m_itrTags == m_lstPhraseTags.constEnd());
	}
}

// ============================================================================

CSearchResultHighlighter::CSearchResultHighlighter(const CVerseListModel *pVerseListModel, bool bExcludedResults, QObject *parent)
	:	CBasicHighlighter(parent),
		m_pVerseListModel(pVerseListModel),
		m_bExcludedResults(bExcludedResults)
{
	Q_ASSERT(pVerseListModel);

	connect(pVerseListModel, SIGNAL(destroyed()), this, SLOT(verseListModelDestroyed()));
	connect(pVerseListModel, SIGNAL(verseListChanged()), this, SLOT(verseListChanged()));
}

CSearchResultHighlighter::CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags, bool bExcludedResults, QObject *parent)
	:	CBasicHighlighter(parent),
		  m_pVerseListModel(nullptr),
		  m_bExcludedResults(bExcludedResults)
{
	m_myPhraseTags.setPhraseTags(lstPhraseTags);
}

CSearchResultHighlighter::~CSearchResultHighlighter()
{
	if (m_pVerseListModel) {
		disconnect(m_pVerseListModel, nullptr, this, nullptr);
		m_pVerseListModel = nullptr;
	}
}

QTextCharFormat CSearchResultHighlighter::textCharFormat(const QTextCharFormat &aFormat, bool bClear) const
{
	QTextCharFormat fmtNew;

	if ((!bClear) && (enabled())) {
		if (!aFormat.hasProperty(USERPROP_FOREGROUND_BRUSH)) {
			fmtNew.setProperty(USERPROP_FOREGROUND_BRUSH, QVariant(aFormat.foreground()));
		}
		fmtNew.setForeground(QBrush(CPersistentSettings::instance()->colorSearchResults()));
		if (m_bExcludedResults) {
			if (!aFormat.hasProperty(USERPROP_FONT_STRIKE_OUT)) {
				fmtNew.setProperty(USERPROP_FONT_STRIKE_OUT, QVariant(aFormat.fontStrikeOut()));
			}
			fmtNew.setFontStrikeOut(true);
		}
	} else {
		if (aFormat.hasProperty(USERPROP_FOREGROUND_BRUSH))
			fmtNew.setForeground(aFormat.property(USERPROP_FOREGROUND_BRUSH).value<QBrush>());
		if (m_bExcludedResults) {
			if (aFormat.hasProperty(USERPROP_FONT_STRIKE_OUT))
				fmtNew.setFontStrikeOut(aFormat.property(USERPROP_FONT_STRIKE_OUT).value<bool>());
		}
	}

	return fmtNew;
}

QString CSearchResultHighlighter::htmlBegin() const
{
	return enabled() ? QString("<font color=\"%1\"%2>").arg(CPersistentSettings::instance()->colorSearchResults().name())
						   .arg(m_bExcludedResults ? QString(" style=\"text-decoration: line-through;\"") : QString())
					 : QString();
}

QString CSearchResultHighlighter::htmlEnd() const
{
	return enabled () ? QString("</font>") : QString();
}

bool CSearchResultHighlighter::intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	Q_ASSERT(pBibleDatabase != nullptr);

	if (!enabled()) return false;
	if (!aTag.relIndex().isSet()) return false;

	CHighlighterPhraseTagFwdItr aItr = getForwardIterator();
	while (!aItr.isEnd()) {
		if (aItr.nextTag().intersects(pBibleDatabase, aTag)) return true;
	}

	return false;
}

void CSearchResultHighlighter::verseListChanged()
{
	Q_ASSERT(m_pVerseListModel != nullptr);
	if (m_pVerseListModel == nullptr) return;

	emit phraseTagsChanged();
}

void CSearchResultHighlighter::verseListModelDestroyed()
{
	if (m_pVerseListModel) {
		disconnect(m_pVerseListModel, nullptr, this, nullptr);
		m_pVerseListModel = nullptr;
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
		return CHighlighterPhraseTagFwdItr(new i_TVerseListModelResults(m_pVerseListModel->searchResults(m_bExcludedResults)));		// Note: CHighlighterPhraseTagFwdItr takes ownership of i_TVerseListModelResults
	} else {
		return CHighlighterPhraseTagFwdItr(m_myPhraseTags.phraseTags());
	}
}

bool CSearchResultHighlighter::isEmpty() const
{
	if (m_pVerseListModel) {
		return (m_pVerseListModel->searchResults(false).verseMap().isEmpty());		// Our highlighter PhraseTags could technically be empty and not trigger this, but for the purposes of this function this highlighter "isn't empty" if we have verses
	} else {
		return m_myPhraseTags.phraseTags().isEmpty();
	}
}

// ============================================================================

Q_DECLARE_METATYPE(QTextCharFormat::UnderlineStyle)

QTextCharFormat CCursorFollowHighlighter::textCharFormat(const QTextCharFormat &aFormat, bool bClear) const
{
	QTextCharFormat fmtNew;

	if ((!bClear) && (enabled())) {
		if (!aFormat.hasProperty(USERPROP_UNDERLINE_COLOR)) {
			fmtNew.setProperty(USERPROP_UNDERLINE_COLOR, QVariant(aFormat.underlineColor()));
		}
		if (!aFormat.hasProperty(USERPROP_UNDERLINE_STYLE)) {
			fmtNew.setProperty(USERPROP_UNDERLINE_STYLE, QVariant(aFormat.underlineStyle()));
		}
		fmtNew.setUnderlineColor(CPersistentSettings::instance()->colorCursorFollow());
		fmtNew.setUnderlineStyle(QTextCharFormat::SingleUnderline);		// TODO : Get properties from global settings! ??
	} else {
		if (aFormat.hasProperty(USERPROP_UNDERLINE_COLOR))
			fmtNew.setUnderlineColor(aFormat.property(USERPROP_UNDERLINE_COLOR).value<QColor>());
		if (aFormat.hasProperty(USERPROP_UNDERLINE_STYLE))
			fmtNew.setUnderlineStyle(aFormat.property(USERPROP_UNDERLINE_STYLE).value<QTextCharFormat::UnderlineStyle>());
	}

	return fmtNew;
}

bool CCursorFollowHighlighter::intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	return m_myPhraseTags.phraseTags().intersects(pBibleDatabase, aTag);
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

CUserDefinedHighlighter::CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstPhraseTags, QObject *parent)
	:	CBasicHighlighter(parent),
		m_strUserDefinedHighlighterName(strUserDefinedHighlighterName)
{
	m_myPhraseTags.setPhraseTags(lstPhraseTags);

	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	const TUserDefinedColor highlighterDefinition = g_pUserNotesDatabase->highlighterDefinition(m_strUserDefinedHighlighterName);
	setEnabled(highlighterDefinition.isValid() && highlighterDefinition.m_bEnabled);
}

CUserDefinedHighlighter::CUserDefinedHighlighter(const CUserDefinedHighlighter &aUserDefinedHighlighter)
	:	CBasicHighlighter(aUserDefinedHighlighter.parent())
{
	setEnabled(aUserDefinedHighlighter.enabled());
	m_myPhraseTags.setPhraseTags(aUserDefinedHighlighter.m_myPhraseTags.phraseTags());
	m_strUserDefinedHighlighterName = aUserDefinedHighlighter.m_strUserDefinedHighlighterName;
}

QTextCharFormat CUserDefinedHighlighter::textCharFormat(const QTextCharFormat &aFormat, bool bClear) const
{
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	const TUserDefinedColor highlighterDefinition = g_pUserNotesDatabase->highlighterDefinition(m_strUserDefinedHighlighterName);

	QTextCharFormat fmtNew;

	if ((!bClear) && (enabled()) &&
		(highlighterDefinition.isValid())) {
		if (!aFormat.hasProperty(USERPROP_BACKGROUND_BRUSH)) {
			fmtNew.setProperty(USERPROP_BACKGROUND_BRUSH, QVariant(aFormat.background()));
		}
		fmtNew.setBackground(QBrush(highlighterDefinition.m_color));
	} else {
		if (aFormat.hasProperty(USERPROP_BACKGROUND_BRUSH))
			fmtNew.setBackground(aFormat.property(USERPROP_BACKGROUND_BRUSH).value<QBrush>());
	}

	return fmtNew;
}

QString CUserDefinedHighlighter::htmlBegin() const
{
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	const TUserDefinedColor highlighterDefinition = g_pUserNotesDatabase->highlighterDefinition(m_strUserDefinedHighlighterName);

	if (enabled() && highlighterDefinition.isValid()) {
		return QString("<span style=\"background-color: %1\">").arg(highlighterDefinition.m_color.name());
	}
	return QString();
}

QString CUserDefinedHighlighter::htmlEnd() const
{
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	const TUserDefinedColor highlighterDefinition = g_pUserNotesDatabase->highlighterDefinition(m_strUserDefinedHighlighterName);

	if (enabled() && highlighterDefinition.isValid()) {
		return QString("</span>");
	}
	return QString();
}

bool CUserDefinedHighlighter::intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	return m_myPhraseTags.phraseTags().intersects(pBibleDatabase, aTag);
}

CHighlighterPhraseTagFwdItr CUserDefinedHighlighter::getForwardIterator() const
{
	return CHighlighterPhraseTagFwdItr(m_myPhraseTags.phraseTags());
}

bool CUserDefinedHighlighter::isEmpty() const
{
	return m_myPhraseTags.phraseTags().isEmpty();
}

const TPhraseTagList &CUserDefinedHighlighter::phraseTags() const
{
	return m_myPhraseTags.phraseTags();
}

void CUserDefinedHighlighter::setPhraseTags(const TPhraseTagList &lstPhraseTags)
{
	m_myPhraseTags.setPhraseTags(lstPhraseTags);
	emit phraseTagsChanged();
}

// ============================================================================

