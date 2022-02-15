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

#include "HighlighterButtons.h"
#include "UserNotesDatabase.h"

#include <QToolBar>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

// ============================================================================

constexpr int NUM_HIGHLIGHTER_TOOLBAR_BUTTONS = 4;

// ============================================================================

CHighlighterButtons::CHighlighterButtons(QObject *pParent)
	:	QObject(pParent),
		m_pActionGroupHighlighterTools(nullptr)
{
	Q_ASSERT(pParent != nullptr);
	Q_ASSERT(!g_pUserNotesDatabase.isNull());

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
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_H));
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
				break;
			case 1:
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_J));
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J));
				break;
			case 2:
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_K));
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K));
				break;
			case 3:
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::Key_L));
				lstShortcuts.append(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_L));
				break;
			default:
				break;
		}
#else
		switch (ndx) {
			case 0:
				lstShortcuts.append(QKeySequence(Qt::META | Qt::Key_H));
				lstShortcuts.append(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_H));
				break;
			case 1:
				lstShortcuts.append(QKeySequence(Qt::META | Qt::Key_J));
				lstShortcuts.append(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_J));
				break;
			case 2:
				lstShortcuts.append(QKeySequence(Qt::META | Qt::Key_K));
				lstShortcuts.append(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_K));
				break;
			case 3:
				lstShortcuts.append(QKeySequence(Qt::META | Qt::Key_L));
				lstShortcuts.append(QKeySequence(Qt::META | Qt::SHIFT | Qt::Key_L));
				break;
			default:
				break;
		}
#endif
		if (lstShortcuts.size() != 2) {
			Q_ASSERT(false);
			continue;
		}

		CHighlighterAction *pActionToolButton = new CHighlighterAction(lstShortcuts, m_pActionGroupHighlighterTools);
		m_pActionGroupHighlighterTools->addAction(pActionToolButton);
		pActionToolButton->setEnabled(false);		// Will get enabled on proper focus-in to Search Results and/or Scripture Browser
		pActionToolButton->setData(ndx);		// Data is our Highlighter Tool Index
		connect(pActionToolButton, SIGNAL(highlightTriggered(QAction*, bool)), this, SLOT(en_highlighterToolTriggered(QAction*, bool)));

		TToolButtonPtr pButtonHighlighter = new CHighlighterWidgetAction(pActionToolButton, pParent);
		m_lstButtons.append(pButtonHighlighter);
		m_lstActionGroups.append(nullptr);					// Set initial list to NULL so our setHighlighterList() function will create it

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
			Q_ASSERT(false);
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
	Q_ASSERT(pToolBar != nullptr);
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
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	Q_ASSERT((ndx >= 0) && (ndx < m_lstButtons.size()));
	Q_ASSERT(m_lstButtons.size() == m_lstActionGroups.size());
	Q_ASSERT(m_lstButtons.at(ndx) != nullptr);
	if (m_lstButtons.at(ndx) == nullptr) return;
	QString strHighlighter = strUserDefinedHighlighterName;
	if (m_lstActionGroups.at(ndx) == nullptr) {
		m_lstActionGroups[ndx] = new QActionGroup(this);
	} else {
		if (strHighlighter.isEmpty()) {
			QAction *pCurrentAction = m_lstActionGroups.at(ndx)->checkedAction();
			if (pCurrentAction != nullptr) {
				strHighlighter = pCurrentAction->text();
			}
		}
		delete m_lstActionGroups[ndx];
		m_lstActionGroups[ndx] = new QActionGroup(this);
	}
	m_lstActionGroups[ndx]->setExclusive(true);

	Q_ASSERT(m_lstButtons[ndx]->menu() != nullptr);
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
	if (m_lstActionGroups[ndx]->checkedAction() == nullptr) strHighlighter.clear();			// If we didn't check something, we didn't have a matching highlighter
	setHighlighterPreview(ndx, strHighlighter);
	connect(m_lstActionGroups[ndx], SIGNAL(triggered(QAction*)), this, SLOT(en_highlighterSelectionChanged(QAction*)));
}

void CHighlighterButtons::setHighlighterPreview(int ndx, const QString &strUserDefinedHighlighterName)
{
	Q_ASSERT(!g_pUserNotesDatabase.isNull());
	Q_ASSERT((ndx >= 0) && (ndx < m_lstButtons.size()));

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
	Q_ASSERT(pAction != nullptr);
	int ndx = pAction->data().toInt();
	Q_ASSERT((ndx >= 0) && (ndx < m_lstButtons.size()));

	setHighlighterPreview(ndx, pAction->text());
}

QString CHighlighterButtons::highlighter(int ndx) const
{
	if (ndx == -1) return QString();

	Q_ASSERT((ndx >= 0) && (ndx < m_lstActionGroups.size()));
	if ((ndx < 0) || (ndx >= m_lstActionGroups.size())) return QString();

	QAction *pCurrentAction = m_lstActionGroups[ndx]->checkedAction();
	if (pCurrentAction == nullptr) return QString();

	return pCurrentAction->text();
}

void CHighlighterButtons::en_highlighterToolTriggered(QAction *pAction, bool bSecondary)
{
	Q_ASSERT(pAction != nullptr);
	int ndx = pAction->data().toInt();
	Q_ASSERT((ndx >= 0) && (ndx < m_lstButtons.size()));

	emit highlighterToolTriggered(ndx, m_lstButtons.at(ndx)->controlActivation() || bSecondary);
}

// ============================================================================
