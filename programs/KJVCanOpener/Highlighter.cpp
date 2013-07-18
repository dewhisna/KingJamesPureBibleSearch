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
#include "PersistentSettings.h"
#include "UserNotesDatabase.h"

#include <QVariant>
#include <QBrush>
#include <QTextFormat>
#include <QIcon>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QMenu>

// ============================================================================

#define NUM_HIGHLIGHTER_TOOLBAR_BUTTONS 4


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
	m_itrVerses = m_pVerseListModel->searchResults().verseMap().constBegin();
	while (m_itrVerses != m_pVerseListModel->searchResults().verseMap().constEnd()) {
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
		if (m_itrVerses != m_pVerseListModel->searchResults().verseMap().constEnd()) {
			++m_itrTags;
			if (m_itrTags != m_itrVerses->phraseTags().constEnd()) return nRetVal;
		}
		++m_itrVerses;
		while (m_itrVerses != m_pVerseListModel->searchResults().verseMap().constEnd()) {
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
		return (m_itrVerses == m_pVerseListModel->searchResults().verseMap().constEnd());
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
		if (!aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH)) {
			aFormat.setProperty(QTextFormat::UserProperty + USERPROP_FOREGROUND_BRUSH, QVariant(aFormat.foreground()));
		}
		aFormat.setForeground(QBrush(CPersistentSettings::instance()->colorSearchResults()));
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
		return (m_pVerseListModel->searchResults().verseMap().isEmpty());		// Our highlighter PhraseTags could technically be empty and not trigger this, but for the purposes of this function this highlighter "isn't empty" if we have verses
	} else {
		return m_myPhraseTags.phraseTags().isEmpty();
	}
}

// ============================================================================

Q_DECLARE_METATYPE(QTextCharFormat::UnderlineStyle)

void CCursorFollowHighlighter::doHighlighting(QTextCharFormat &aFormat, bool bClear) const
{
	if ((!bClear) && (enabled())) {
		if (!aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR)) {
			aFormat.setProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_COLOR, QVariant(aFormat.underlineColor()));
		}
		if (!aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE)) {
			aFormat.setProperty(QTextFormat::UserProperty + USERPROP_UNDERLINE_STYLE, QVariant(aFormat.underlineStyle()));
		}
		aFormat.setUnderlineColor(CPersistentSettings::instance()->colorCursorFollow());
		aFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);		// TODO : Get properties from global settings! ??
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

void CUserDefinedHighlighter::doHighlighting(QTextCharFormat &aFormat, bool bClear) const
{
	assert(g_pUserNotesDatabase != NULL);
	const TUserDefinedColor highlighterDefinition = g_pUserNotesDatabase->highlighterDefinition(m_strUserDefinedHighlighterName);

	if ((!bClear) && (enabled()) &&
		(highlighterDefinition.isValid()) &&
		(highlighterDefinition.m_bEnabled)) {
		if (!aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_BACKGROUND_BRUSH)) {
			aFormat.setProperty(QTextFormat::UserProperty + USERPROP_BACKGROUND_BRUSH, QVariant(aFormat.background()));
		}
		aFormat.setBackground(QBrush(highlighterDefinition.m_color));
	} else {
		if (aFormat.hasProperty(QTextFormat::UserProperty + USERPROP_BACKGROUND_BRUSH))
			aFormat.setBackground(aFormat.property(QTextFormat::UserProperty + USERPROP_BACKGROUND_BRUSH).value<QBrush>());
	}
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

CHighlighterButtons *CHighlighterButtons::g_pHighlighterButtons = NULL;

CHighlighterButtons::CHighlighterButtons(QToolBar *pParent)
	:	m_pActionGroupHighlighterTools(NULL)
{
	assert(g_pHighlighterButtons == NULL);
	assert(pParent != NULL);
	assert(g_pUserNotesDatabase != NULL);

	g_pHighlighterButtons = this;

	m_pActionGroupHighlighterTools = new QActionGroup(pParent);
	m_pActionGroupHighlighterTools->setExclusive(false);

	m_lstButtons.clear();
	m_lstActionGroups.clear();
	for (int ndx = 0; ndx < NUM_HIGHLIGHTER_TOOLBAR_BUTTONS; ++ndx) {
		QMenu *pHighlighterMenu = new QMenu(pParent);
		QToolButton *pButtonHighlighter = new QToolButton(pParent);
		m_lstButtons.append(pButtonHighlighter);
		m_lstActionGroups.append(NULL);					// Set initial list to NULL so our setHighlighterList() function will create it
		pButtonHighlighter->setMenu(pHighlighterMenu);
//		pButtonHighlighter->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
//		pButtonHighlighter->setText(tr("#%1").arg(ndx+1));
		pButtonHighlighter->setToolButtonStyle(Qt::ToolButtonIconOnly);
		pButtonHighlighter->setPopupMode(QToolButton::MenuButtonPopup);
		QAction *pActionToolButton = m_pActionGroupHighlighterTools->addAction(tr("&Highlight/Unhighlight Passage with Tool #%1").arg(ndx+1));
		pActionToolButton->setToolTip(tr("Highlighter Tool #%1").arg(ndx+1));
		pActionToolButton->setStatusTip(tr("Highlight/Unhighlight the selected passage with Highlighter Tool #%1").arg(ndx+1));
#ifndef Q_OS_MAC
		switch (ndx) {
			case 0:
				pActionToolButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
				break;
			case 1:
				pActionToolButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_J));
				break;
			case 2:
				pActionToolButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_K));
				break;
			case 3:
				pActionToolButton->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
				break;
			default:
				break;
		}
#else
		switch (ndx) {
			case 0:
				pActionToolButton->setShortcut(QKeySequence(Qt::META + Qt::Key_H));
				break;
			case 1:
				pActionToolButton->setShortcut(QKeySequence(Qt::META + Qt::Key_J));
				break;
			case 2:
				pActionToolButton->setShortcut(QKeySequence(Qt::META + Qt::Key_K));
				break;
			case 3:
				pActionToolButton->setShortcut(QKeySequence(Qt::META + Qt::Key_L));
				break;
			default:
				break;
		}
#endif
		pActionToolButton->setData(ndx);		// Data is our Highlighter Tool Index
		pButtonHighlighter->setDefaultAction(pActionToolButton);

		setHighlighterList(ndx);
		pParent->addWidget(pButtonHighlighter);
	}

	connect(g_pUserNotesDatabase.data(), SIGNAL(changedHighlighters()), this, SLOT(en_changedHighlighters()));
	connect(m_pActionGroupHighlighterTools, SIGNAL(triggered(QAction*)), this, SIGNAL(highlighterToolTriggered(QAction*)));
}

CHighlighterButtons::~CHighlighterButtons()
{

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
	assert(g_pUserNotesDatabase != NULL);
	assert((ndx >= 0) && (ndx < m_lstButtons.size()));
	assert(m_lstButtons.size() == m_lstActionGroups.size());
	assert(m_lstButtons.at(ndx) != NULL);
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
	const TUserDefinedColorMap &mapHighlighters(g_pUserNotesDatabase->highlighterDefinitionsMap());
	for (TUserDefinedColorMap::const_iterator itrHighlighters = mapHighlighters.constBegin(); itrHighlighters != mapHighlighters.constEnd(); ++itrHighlighters) {
		if ((!itrHighlighters->isValid()) || (!itrHighlighters->m_bEnabled)) continue;
		QAction *pAction = new QAction(itrHighlighters.key(), m_lstActionGroups[ndx]);
		pAction->setData(ndx);
		pAction->setCheckable(true);
		if (strHighlighter.compare(itrHighlighters.key()) == 0) pAction->setChecked(true);
		m_lstButtons[ndx]->menu()->addAction(pAction);
	}
	if (m_lstActionGroups[ndx]->checkedAction() == NULL) strHighlighter.clear();			// If we didn't check something, we didn't have a matching highlighter
	setHighlighterPreview(ndx, strHighlighter);
	connect(m_lstActionGroups[ndx], SIGNAL(selected(QAction*)), this, SLOT(en_highlighterSelectionChanged(QAction*)));
}

void CHighlighterButtons::setHighlighterPreview(int ndx, const QString &strUserDefinedHighlighterName)
{
	assert(g_pUserNotesDatabase != NULL);
	assert((ndx >= 0) && (ndx < m_lstButtons.size()));

	if (strUserDefinedHighlighterName.isEmpty()) {
		m_lstButtons[ndx]->setIcon(QIcon(":res/highlighter_translucent_01-256.png"));
	} else {
		QPixmap pixHighlighter(":res/highlighter_white_01-256.png");
		QBitmap bmHighlighterMask = pixHighlighter.createMaskFromColor(QColor(255, 255, 255), Qt::MaskOutColor);		// Mask white panel
		QPainter paintHighlighter(&pixHighlighter);
		paintHighlighter.setPen(g_pUserNotesDatabase->highlighterColor(strUserDefinedHighlighterName));
		paintHighlighter.drawPixmap(pixHighlighter.rect(), bmHighlighterMask, bmHighlighterMask.rect());
		paintHighlighter.end();
		m_lstButtons[ndx]->setIcon(QIcon(pixHighlighter));
	}
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

// ============================================================================

