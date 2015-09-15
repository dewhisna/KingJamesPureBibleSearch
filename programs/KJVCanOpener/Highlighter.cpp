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
#include "ScriptureTextFormatProperties.h"
#include "VerseListModel.h"
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"

#include <QVariant>
#include <QBrush>
#include <QTextFormat>
#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#endif

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

#define NUM_HIGHLIGHTER_TOOLBAR_BUTTONS 4

// ============================================================================

CHighlighterPhraseTagFwdItr::CHighlighterPhraseTagFwdItr(const i_TVerseListModelResults *pvlmResults)
	:	m_pvlmResults(pvlmResults),
		m_lstPhraseTags(m_lstDummyPhraseTags)
{
	assert(pvlmResults != NULL);
	m_itrVerses = m_pvlmResults->data.verseMap().constBegin();
	while (m_itrVerses != m_pvlmResults->data.verseMap().constEnd()) {
		m_itrTags = m_itrVerses->phraseTags().constBegin();
		if (m_itrTags != m_itrVerses->phraseTags().constEnd()) break;
		++m_itrVerses;
	}
}

CHighlighterPhraseTagFwdItr::CHighlighterPhraseTagFwdItr(const TPhraseTagList &lstTags)
	:	m_pvlmResults(NULL),
		m_lstPhraseTags(lstTags)
{
	m_itrTags = m_lstPhraseTags.constBegin();
}

CHighlighterPhraseTagFwdItr::~CHighlighterPhraseTagFwdItr()
{
	if (m_pvlmResults) {
		delete m_pvlmResults;
		m_pvlmResults = NULL;
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
	assert(pVerseListModel);

	connect(pVerseListModel, SIGNAL(destroyed()), this, SLOT(verseListModelDestroyed()));
	connect(pVerseListModel, SIGNAL(verseListChanged()), this, SLOT(verseListChanged()));
}

CSearchResultHighlighter::CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags, bool bExcludedResults, QObject *parent)
	:	CBasicHighlighter(parent),
		  m_pVerseListModel(NULL),
		  m_bExcludedResults(bExcludedResults)
{
	m_myPhraseTags.setPhraseTags(lstPhraseTags);
}

CSearchResultHighlighter::CSearchResultHighlighter(const TPhraseTag &aTag, bool bExcludedResults, QObject *parent)
	:	CBasicHighlighter(parent),
		m_pVerseListModel(NULL),
		m_bExcludedResults(bExcludedResults)
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

QTextCharFormat CSearchResultHighlighter::doHighlighting(const QTextCharFormat &aFormat, bool bClear) const
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

bool CSearchResultHighlighter::intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	assert(pBibleDatabase != NULL);

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

QTextCharFormat CCursorFollowHighlighter::doHighlighting(const QTextCharFormat &aFormat, bool bClear) const
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

QTextCharFormat CUserDefinedHighlighter::doHighlighting(const QTextCharFormat &aFormat, bool bClear) const
{
	assert(!g_pUserNotesDatabase.isNull());
	const TUserDefinedColor highlighterDefinition = g_pUserNotesDatabase->highlighterDefinition(m_strUserDefinedHighlighterName);

	QTextCharFormat fmtNew;

	if ((!bClear) && (enabled()) &&
		(highlighterDefinition.isValid()) &&
		(highlighterDefinition.m_bEnabled)) {
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

void CUserDefinedHighlighter::clearPhraseTags()
{
	m_myPhraseTags.setPhraseTags(TPhraseTagList());
	emit phraseTagsChanged();
}

// ============================================================================
// ============================================================================

#if !defined(OSIS_PARSER_BUILD) && !defined(KJV_SEARCH_BUILD) && !defined(KJV_DIFF_BUILD)

CHighlighterButtons::CHighlighterButtons(QObject *pParent)
	:	QObject(pParent),
		m_pActionGroupHighlighterTools(NULL)
{
	assert(pParent != NULL);
	assert(!g_pUserNotesDatabase.isNull());

	m_pActionGroupHighlighterTools = new QActionGroup(pParent);
	m_pActionGroupHighlighterTools->setExclusive(false);

	m_lstButtons.clear();
	m_lstActionGroups.clear();
	for (int ndx = 0; ndx < NUM_HIGHLIGHTER_TOOLBAR_BUTTONS; ++ndx) {
		QList<QKeySequence> lstShortcuts;
		lstShortcuts.reserve(2);
#ifndef Q_OS_MAC
		switch (ndx) {
			case 0:
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_H));
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_H));
				break;
			case 1:
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_J));
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_J));
				break;
			case 2:
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_K));
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_K));
				break;
			case 3:
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::Key_L));
				lstShortcuts.append(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
				break;
			default:
				break;
		}
#else
		switch (ndx) {
			case 0:
				lstShortcuts.append(QKeySequence(Qt::META + Qt::Key_H));
				lstShortcuts.append(QKeySequence(Qt::META + Qt::SHIFT + Qt::Key_H));
				break;
			case 1:
				lstShortcuts.append(QKeySequence(Qt::META + Qt::Key_J));
				lstShortcuts.append(QKeySequence(Qt::META + Qt::SHIFT + Qt::Key_J));
				break;
			case 2:
				lstShortcuts.append(QKeySequence(Qt::META + Qt::Key_K));
				lstShortcuts.append(QKeySequence(Qt::META + Qt::SHIFT + Qt::Key_K));
				break;
			case 3:
				lstShortcuts.append(QKeySequence(Qt::META + Qt::Key_L));
				lstShortcuts.append(QKeySequence(Qt::META + Qt::SHIFT + Qt::Key_L));
				break;
			default:
				break;
		}
#endif
		if (lstShortcuts.size() != 2) {
			assert(false);
			continue;
		}

		CHighlighterAction *pActionToolButton = new CHighlighterAction(lstShortcuts, m_pActionGroupHighlighterTools);
		m_pActionGroupHighlighterTools->addAction(pActionToolButton);
		pActionToolButton->setEnabled(false);		// Will get enabled on proper focus-in to Search Results and/or Scripture Browser
		pActionToolButton->setData(ndx);		// Data is our Highlighter Tool Index
		connect(pActionToolButton, SIGNAL(highlightTriggered(QAction*, bool)), this, SLOT(en_highlighterToolTriggered(QAction*, bool)));

		TToolButtonPtr pButtonHighlighter = new CHighlighterWidgetAction(pActionToolButton, pParent);
		m_lstButtons.append(pButtonHighlighter);
		m_lstActionGroups.append(NULL);					// Set initial list to NULL so our setHighlighterList() function will create it

		setHighlighterList(ndx);
	}

	setHighlighterTips(false);

	connect(g_pUserNotesDatabase.data(), SIGNAL(changedHighlighters()), this, SLOT(en_changedHighlighters()));
	connect(m_pActionGroupHighlighterTools, SIGNAL(triggered(QAction*)), this, SLOT(en_highlighterToolTriggered(QAction*)));		// This connect needed for click activate vs. shortcut activated
}

CHighlighterButtons::~CHighlighterButtons()
{

}

void CHighlighterButtons::setHighlighterTips(bool bSearchResultsActive)
{
	for (int ndx = 0; ndx < m_lstButtons.size(); ++ndx) {
		CHighlighterAction *pActionToolButton = m_lstButtons.at(ndx)->buttonAction();

		QList<QKeySequence> lstShortcuts = pActionToolButton->shortcuts();
		if (lstShortcuts.size() != 2) {
			assert(false);
		} else {
			QString strActionText;
			if (bSearchResultsActive) {
				strActionText += tr("&Highlight/Unhighlight Verse (Search Results) with Tool #%1", "MainMenu").arg(ndx+1);
			} else {
				strActionText += tr("&Highlight/Unhighlight Passage with Tool #%1", "MainMenu").arg(ndx+1);
			}
			strActionText += "\t" + lstShortcuts.at(0).toString(QKeySequence::NativeText);
			if (bSearchResultsActive) strActionText += " (" + lstShortcuts.at(1).toString(QKeySequence::NativeText) + ")";
			pActionToolButton->setText(strActionText);
		}

		QString strToolTip = tr("Highlighter Tool #%1", "MainMenu").arg(ndx+1);
		if (bSearchResultsActive) {
			strToolTip += "\n" + tr("Ctrl+Click to highlight only the Search Result Text", "MainMenu");
		}
		pActionToolButton->setToolTip(strToolTip);

		pActionToolButton->setStatusTip(tr("Highlight/Unhighlight the selected passage with Highlighter Tool #%1", "MainMenu").arg(ndx+1));
	}
}

void CHighlighterButtons::addHighlighterButtonsToToolBar(QToolBar *pToolBar)
{
	assert(pToolBar != NULL);
	for (int ndx = 0; ndx < m_lstButtons.size(); ++ndx) {
		// Originally had this addWidget call.  However, addWidget creates a new QWidgetAction
		//		which takes ownership of the specified widget, which is undesirable since we
		//		are sharing it across multiple toolbars and handling the parenting and object
		//		cleanup.  Therefore, we will use our own derived QWidgetAction, parent that to
		//		our application, and use addAction here instead:
		//	pToolBar->addWidget(m_lstButtons.at(ndx).data());
		pToolBar->addAction(m_lstButtons.at(ndx));
	}
}

void CHighlighterButtons::enterConfigurationMode()
{
	disconnect(g_pUserNotesDatabase.data(), SIGNAL(changedHighlighters()), this, SLOT(en_changedHighlighters()));
}

void CHighlighterButtons::leaveConfigurationMode()
{
	connect(g_pUserNotesDatabase.data(), SIGNAL(changedHighlighters()), this, SLOT(en_changedHighlighters()));
	setHighlighterLists();
}

void CHighlighterButtons::en_changedHighlighters()
{
	setHighlighterLists();
}

void CHighlighterButtons::setHighlighterLists()
{
	for (int ndx = 0; ndx < m_lstButtons.size(); ++ndx) {
		setHighlighterList(ndx);
	}
}

void CHighlighterButtons::setHighlighterList(int ndx, const QString &strUserDefinedHighlighterName)
{
	assert(!g_pUserNotesDatabase.isNull());
	assert((ndx >= 0) && (ndx < m_lstButtons.size()));
	assert(m_lstButtons.size() == m_lstActionGroups.size());
	assert(m_lstButtons.at(ndx) != NULL);
	if (m_lstButtons.at(ndx) == NULL) return;
	QString strHighlighter = strUserDefinedHighlighterName;
	if (m_lstActionGroups.at(ndx) == NULL) {
		m_lstActionGroups[ndx] = new QActionGroup(this);
	} else {
		if (strHighlighter.isEmpty()) {
			QAction *pCurrentAction = m_lstActionGroups.at(ndx)->checkedAction();
			if (pCurrentAction != NULL) {
				strHighlighter = pCurrentAction->text();
			}
		}
		delete m_lstActionGroups[ndx];
		m_lstActionGroups[ndx] = new QActionGroup(this);
	}
	m_lstActionGroups[ndx]->setExclusive(true);

	assert(m_lstButtons[ndx]->menu() != NULL);
	const TUserDefinedColorMap mapHighlighters(g_pUserNotesDatabase->highlighterDefinitionsMap());
	for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin(); itrHighlighters != mapHighlighters.constEnd(); ++itrHighlighters) {
		if ((!itrHighlighters->isValid()) || (!itrHighlighters->m_bEnabled)) continue;
		QAction *pAction = new QAction(itrHighlighters.key(), m_lstActionGroups[ndx]);
		pAction->setData(ndx);
		pAction->setCheckable(true);
		pAction->setIcon(iconHighlighterPreview(itrHighlighters.key()));
		if (strHighlighter.compare(itrHighlighters.key()) == 0) pAction->setChecked(true);
		m_lstButtons[ndx]->menu()->addAction(pAction);
	}
	if (m_lstActionGroups[ndx]->checkedAction() == NULL) strHighlighter.clear();			// If we didn't check something, we didn't have a matching highlighter
	setHighlighterPreview(ndx, strHighlighter);
	connect(m_lstActionGroups[ndx], SIGNAL(triggered(QAction*)), this, SLOT(en_highlighterSelectionChanged(QAction*)));
}

void CHighlighterButtons::setHighlighterPreview(int ndx, const QString &strUserDefinedHighlighterName)
{
	assert(!g_pUserNotesDatabase.isNull());
	assert((ndx >= 0) && (ndx < m_lstButtons.size()));

	if (strUserDefinedHighlighterName.isEmpty()) {
		m_lstButtons[ndx]->setIcon(QIcon(":/res/highlighter_translucent_01-256.png"));
		m_pActionGroupHighlighterTools->actions()[ndx]->setIcon(QIcon(":/res/highlighter_translucent_01-256.png"));
	} else {
		QIcon iconHighlighter(iconHighlighterPreview(strUserDefinedHighlighterName));
		m_lstButtons[ndx]->setIcon(QIcon(iconHighlighter));
		m_pActionGroupHighlighterTools->actions()[ndx]->setIcon(QIcon(iconHighlighter));
	}
}

QIcon CHighlighterButtons::iconHighlighterPreview(const QString &strUserDefinedHighlighterName)
{
	QPixmap pixHighlighter(":/res/highlighter_white_01-256.png");
	QBitmap bmHighlighterMask = pixHighlighter.createMaskFromColor(QColor(255, 255, 255), Qt::MaskOutColor);		// Mask white panel
	QPainter paintHighlighter(&pixHighlighter);
	paintHighlighter.setPen(g_pUserNotesDatabase->highlighterColor(strUserDefinedHighlighterName));
	paintHighlighter.drawPixmap(pixHighlighter.rect(), bmHighlighterMask, bmHighlighterMask.rect());
	paintHighlighter.end();

	return QIcon(pixHighlighter);
}

void CHighlighterButtons::en_highlighterSelectionChanged(QAction *pAction)
{
	assert(pAction != NULL);
	int ndx = pAction->data().toInt();
	assert((ndx >= 0) && (ndx < m_lstButtons.size()));

	setHighlighterPreview(ndx, pAction->text());
}

QString CHighlighterButtons::highlighter(int ndx) const
{
	if (ndx == -1) return QString();

	assert((ndx >= 0) && (ndx < m_lstActionGroups.size()));
	if ((ndx < 0) || (ndx >= m_lstActionGroups.size())) return QString();

	QAction *pCurrentAction = m_lstActionGroups[ndx]->checkedAction();
	if (pCurrentAction == NULL) return QString();

	return pCurrentAction->text();
}

void CHighlighterButtons::en_highlighterToolTriggered(QAction *pAction, bool bSecondary)
{
	assert(pAction != NULL);
	int ndx = pAction->data().toInt();
	assert((ndx >= 0) && (ndx < m_lstButtons.size()));

	emit highlighterToolTriggered(ndx, m_lstButtons.at(ndx)->controlActivation() || bSecondary);
}

#endif

// ============================================================================

