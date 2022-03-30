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

#include "CrossRefEditDlg.h"

#include "ReportError.h"
#include "myApplication.h"
#include "PersistentSettings.h"

#include "SearchResults.h"
#include "ScriptureEdit.h"
#include "PhraseNavigator.h"
#include "PassageNavigatorDlg.h"

#include <QMessageBox>
#include <QTextCursor>

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------

	// RestoreState:
	const QString constrRestoreStateGroup("RestoreState");
	const QString constrGeometryKey("Geometry");
	const QString constrWindowStateKey("WindowState");

}

// ============================================================================

CCrossRefEditDlg::CCrossRefEditDlg(CBibleDatabasePtr pBibleDatabase, CUserNotesDatabasePtr pUserNotesDatabase, QWidget *parent)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		m_pBibleDatabase(pBibleDatabase),
		m_pUserNotesDatabase(pUserNotesDatabase),
		m_pEditSourcePassage(nullptr),
		m_pCrossRefTreeView(nullptr),
		m_bIsDirty(false),
		m_bHaveGeometry(false)
{
	Q_ASSERT(!m_pBibleDatabase.isNull());
	Q_ASSERT(!m_pUserNotesDatabase.isNull());

	// Create a working copy and initialize it to the existing database:
	m_pWorkingUserNotesDatabase = QSharedPointer<CUserNotesDatabase>(new CUserNotesDatabase());
//	m_pWorkingUserNotesDatabase->setDataFrom(*(m_pUserNotesDatabase.data()));
	const TCrossReferenceMap *pRefMap = m_pUserNotesDatabase->crossRefsMap(m_pBibleDatabase.data());
	m_pWorkingUserNotesDatabase->setCrossRefsMap(m_pBibleDatabase.data(), pRefMap ? *pRefMap : TCrossReferenceMap());

	ui.setupUi(this);

	int ndx;

	// --------------------------------------------------------------

	//	Swapout the editSourcePassage from the layout with
	//		one that we can set the database on:

	ndx = ui.verticalLayoutSource->indexOf(ui.editSourcePassage);
	Q_ASSERT(ndx != -1);
	if (ndx == -1) return;

	m_pEditSourcePassage = new CScriptureEdit(m_pBibleDatabase, this);
	m_pEditSourcePassage->setObjectName("editSourcePassage");
	m_pEditSourcePassage->setMinimumSize(QSize(200, 150));
	QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	sizePolicy1.setHorizontalStretch(10);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(m_pEditSourcePassage->sizePolicy().hasHeightForWidth());
	m_pEditSourcePassage->setSizePolicy(sizePolicy1);
	m_pEditSourcePassage->setMouseTracking(true);
	m_pEditSourcePassage->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	m_pEditSourcePassage->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pEditSourcePassage->setAcceptDrops(false);
	m_pEditSourcePassage->setUndoRedoEnabled(false);
	m_pEditSourcePassage->setTabChangesFocus(true);
	m_pEditSourcePassage->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse);
	m_pEditSourcePassage->setContextMenuPolicy(Qt::DefaultContextMenu);
	m_pEditSourcePassage->setToolTip(tr("Source Passage for Reference", "MainMenu"));

	delete ui.editSourcePassage;
	ui.editSourcePassage = nullptr;
	ui.verticalLayoutSource->insertWidget(ndx, m_pEditSourcePassage);


	// --------------------------------------------------------------

	//	Swapout the treeCrossRefs from the layout with
	//		one that we can set the database on:

	ndx = ui.verticalLayoutRefList->indexOf(ui.treeCrossRefs);
	Q_ASSERT(ndx != -1);
	if (ndx == -1) return;

	m_pCrossRefTreeView = new CSearchResultsTreeView(m_pBibleDatabase, m_pWorkingUserNotesDatabase, this);
	m_pCrossRefTreeView->setObjectName("treeViewCrossRefs");
	QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy2.setHorizontalStretch(5);
	sizePolicy2.setVerticalStretch(0);
	sizePolicy2.setHeightForWidth(m_pCrossRefTreeView->sizePolicy().hasHeightForWidth());
	m_pCrossRefTreeView->setSizePolicy(sizePolicy2);
	m_pCrossRefTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
	m_pCrossRefTreeView->setToolTip(tr("Cross Reference Passages Linked to the Source Reference", "MainMenu"));
	m_pCrossRefTreeView->setViewMode(CVerseListModel::VVME_CROSSREFS);
	m_pCrossRefTreeView->setDisplayMode(CVerseListModel::VDME_RICHTEXT);

	delete ui.treeCrossRefs;
	ui.treeCrossRefs = nullptr;
	ui.verticalLayoutRefList->insertWidget(ndx, m_pCrossRefTreeView);

	connect(m_pCrossRefTreeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_crossRefTreeViewContextMenuRequested(const QPoint &)));
	connect(m_pCrossRefTreeView, SIGNAL(currentItemChanged()), this, SLOT(en_crossRefTreeViewCurrentItemChanged()));
	connect(m_pCrossRefTreeView, SIGNAL(selectionListChanged()), this, SLOT(en_crossRefTreeViewSelectionListChanged()));
	connect(m_pCrossRefTreeView, SIGNAL(activated(const QModelIndex &)), this, SLOT(en_crossRefTreeViewEntryActivated(const QModelIndex &)));

	// --------------------------------------------------------------

	connect(ui.buttonSelectSourceRef, SIGNAL(clicked()), this, SLOT(en_SelectSourceReferenceClicked()));
	connect(ui.buttonAddRef, SIGNAL(clicked()), this, SLOT(en_AddReferenceClicked()));
	connect(ui.buttonDeleteRef, SIGNAL(clicked()), this, SLOT(en_DelReferenceClicked()));

	// --------------------------------------------------------------

#ifndef Q_OS_MAC
	setWindowModality(Qt::WindowModal);		// Only block our parentCanOpener, not the whole app
#endif
}

CCrossRefEditDlg::~CCrossRefEditDlg()
{

}

// ============================================================================

void CCrossRefEditDlg::writeSettings(QSettings &settings, const QString &prefix)
{
#ifdef PRESERVE_DIALOG_GEOMETRY
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	if (m_bHaveGeometry) settings.setValue(constrGeometryKey, saveGeometry());
	settings.endGroup();
#endif
}

void CCrossRefEditDlg::readSettings(QSettings &settings, const QString &prefix)
{
#ifdef PRESERVE_DIALOG_GEOMETRY
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	if (!settings.value(constrGeometryKey).toByteArray().isEmpty()) {
		restoreGeometry(settings.value(constrGeometryKey).toByteArray());
		m_bHaveGeometry = true;
	}
	settings.endGroup();
#endif
}

// ============================================================================

void CCrossRefEditDlg::setSourcePassage(const TPassageTag &tag)
{
	CRelIndex ndxRel = tag.relIndex();
	ndxRel.setWord(0);			// Make sure we have only a book, chapter, or verse
	m_tagSourcePassage = TPassageTag(ndxRel, tag.verseCount());			// Warning: TPassageTag word will always be set to 1 (not 0)!
	TPhraseTagList tagList(TPhraseTag(m_pBibleDatabase.data(), tag));

	ui.editSourceRefDesc->setText(m_pBibleDatabase->PassageReferenceText(ndxRel));

	if (ndxRel.verse()) {
		m_pEditSourcePassage->navigator().setDocumentToVerse(ndxRel, tagList, CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible | CPhraseNavigator::TRO_ScriptureBrowser);
	} else if (ndxRel.chapter()) {
		m_pEditSourcePassage->navigator().setDocumentToChapter(ndxRel, (defaultDocumentToChapterFlags | CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible | CPhraseNavigator::TRO_SuppressPrePostChapters | CPhraseNavigator::TRO_ScriptureBrowser) & ~CPhraseNavigator::TRO_UserNoteExpandAnchors);
		QTextCursor txtCursor;
		txtCursor = m_pEditSourcePassage->textCursor();
		txtCursor.movePosition(QTextCursor::Start);
		m_pEditSourcePassage->setTextCursor(txtCursor);
	} else {
		m_pEditSourcePassage->navigator().setDocumentToBookInfo(ndxRel, defaultDocumentToBookInfoFlags | CPhraseNavigator::TRO_NoAnchors | CPhraseNavigator::TRO_AllUserNotesVisible | CPhraseNavigator::TRO_ScriptureBrowser);
	}

	// Update working database from source database:
//	m_pWorkingUserNotesDatabase->setDataFrom(*(m_pUserNotesDatabase.data()));
	const TCrossReferenceMap *pRefMap = m_pUserNotesDatabase->crossRefsMap(m_pBibleDatabase.data());
	m_pWorkingUserNotesDatabase->setCrossRefsMap(m_pBibleDatabase.data(), pRefMap ? *pRefMap : TCrossReferenceMap());
	m_pCrossRefTreeView->setSingleCrossRefSourceIndex(ndxRel);
	m_bIsDirty = false;
}

// ============================================================================

void CCrossRefEditDlg::saveCrossRefs()
{
	Q_ASSERT(!m_pUserNotesDatabase.isNull());

	const TCrossReferenceMap *pRefMap = m_pWorkingUserNotesDatabase->crossRefsMap(m_pBibleDatabase.data());
	m_pUserNotesDatabase->setCrossRefsMap(m_pBibleDatabase.data(), pRefMap ? *pRefMap : TCrossReferenceMap());
	m_bIsDirty = false;
}

void CCrossRefEditDlg::accept()
{
	saveCrossRefs();
	m_bHaveGeometry = true;
	QDialog::accept();
}

void CCrossRefEditDlg::reject()
{
	if (m_bIsDirty) {
		int nResult = displayWarning(this, windowTitle(), tr("You have made changes to this Cross Reference.  Do you wish to discard them??", "Errors"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
	}

	m_bHaveGeometry = true;
	QDialog::reject();
}

// ============================================================================

void CCrossRefEditDlg::en_crossRefTreeViewContextMenuRequested(const QPoint &pos)
{
	Q_UNUSED(pos);

	// TODO : Finish
}

void CCrossRefEditDlg::en_crossRefTreeViewCurrentItemChanged()
{

}

void CCrossRefEditDlg::en_crossRefTreeViewSelectionListChanged()
{
	QModelIndexList lstSelectedItems = m_pCrossRefTreeView->selectionModel()->selectedRows();
	ui.buttonDeleteRef->setEnabled(lstSelectedItems.size() != 0);
}

void CCrossRefEditDlg::en_crossRefTreeViewEntryActivated(const QModelIndex &index)
{
	CRelIndex ndxInitial = m_pCrossRefTreeView->vlmodel()->logicalIndexForModelIndex(index);
	Q_ASSERT(ndxInitial.isSet());
	Q_ASSERT(ndxInitial.word() == 0);
	CRelIndex ndxTarget = navigateCrossRef(ndxInitial);
	Q_ASSERT(ndxTarget.word() == 0);
	CRelIndex ndxSource = m_tagSourcePassage.relIndex();
	ndxSource.setWord(0);		// Note: This is needed because passages always begin at word 1 and our cross-refs are always indexed from 0
	if ((ndxTarget.isSet()) && (ndxInitial != ndxTarget)) {
		bool bRemove = m_pWorkingUserNotesDatabase->removeCrossReference(m_pBibleDatabase.data(), ndxSource, ndxInitial);
		Q_ASSERT(bRemove);
		bool bAdd = m_pWorkingUserNotesDatabase->setCrossReference(m_pBibleDatabase.data(), ndxSource, ndxTarget);
		Q_ASSERT(bAdd);
		if (bAdd || bRemove) m_bIsDirty = true;
	}
}

// ============================================================================

CRelIndex CCrossRefEditDlg::navigateCrossRef(const CRelIndex &ndxStart)
{
	CPassageNavigator::NAVIGATOR_REF_TYPE_ENUM nType = CPassageNavigator::NRTE_VERSE;
	if (ndxStart.verse() == 0) nType = CPassageNavigator::NRTE_CHAPTER;
	if (ndxStart.chapter() == 0) nType = CPassageNavigator::NRTE_BOOK;

	CPassageNavigatorDlgPtr pDlg(m_pBibleDatabase, this, CPassageNavigator::NRTO_Verse | CPassageNavigator::NRTO_Chapter | CPassageNavigator::NRTO_Book, nType);
	pDlg->setGotoButtonText(tr("&OK", "MainMenu"));
	TPhraseTag tagNav(ndxStart);
	pDlg->navigator().startAbsoluteMode(tagNav);
	if (pDlg->exec() != QDialog::Accepted) return CRelIndex();

	if (pDlg != nullptr) {			// Could get deleted during execution
		CRelIndex ndxTarget = pDlg->passage().relIndex();
		ndxTarget.setWord(0);			// Whole verse references only
		return ndxTarget;
	}
	return CRelIndex();
}

// ============================================================================

void CCrossRefEditDlg::en_SelectSourceReferenceClicked()
{
	if (m_bIsDirty) {
		int nResult = displayWarning(this, windowTitle(), tr("You have made changes to this Cross Reference.  Save them??", "Errors"),
																	(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes);
		if (nResult == QMessageBox::Yes) saveCrossRefs();
	}

	CRelIndex ndxTarget = navigateCrossRef(m_tagSourcePassage.relIndex());			// Note: Passage word 1 is OK for navigation
	if (ndxTarget.isSet()) {
		setSourcePassage(TPassageTag(ndxTarget));		// This will reset m_bIsDirty for us
	}
	// If user cancels out of navigator, we leave the original reference dirtiness as-is
}

void CCrossRefEditDlg::en_AddReferenceClicked()
{
	CRelIndex ndxSource = m_tagSourcePassage.relIndex();
	ndxSource.setWord(0);
	CRelIndex ndxTarget = ndxSource;
	bool bRefSet = false;
	do {
		ndxTarget = navigateCrossRef(ndxTarget);
		if (ndxTarget.isSet()) {
			if (ndxSource == ndxTarget) {
				displayWarning(this, windowTitle(), tr("You can't set a cross-reference to reference itself.", "Errors"));
			} else if (m_pWorkingUserNotesDatabase->crossRefsMap(m_pBibleDatabase.data()) &&
					   m_pWorkingUserNotesDatabase->crossRefsMap(m_pBibleDatabase.data())->haveCrossReference(ndxSource, ndxTarget)) {
				displayWarning(this, windowTitle(), tr("That cross-reference already exists.", "Errors"));
			} else if (m_pWorkingUserNotesDatabase->setCrossReference(m_pBibleDatabase.data(), ndxSource, ndxTarget)) {
				m_bIsDirty = true;
				bRefSet = true;
			}
		}
	} while ((ndxTarget.isSet()) && (!bRefSet));
}

void CCrossRefEditDlg::en_DelReferenceClicked()
{
	TRelativeIndexList lstRefsToRemove;

	// Fetch our list of selections first because once we start removing them, our selection will be changing:
	QModelIndexList lstSelectedItems = m_pCrossRefTreeView->selectionModel()->selectedRows();
	for (int ndx = 0; ndx < lstSelectedItems.size(); ++ndx) {
		if (lstSelectedItems.at(ndx).isValid()) {
			CRelIndex ndxRel = m_pCrossRefTreeView->vlmodel()->logicalIndexForModelIndex(lstSelectedItems.at(ndx));
			if (ndxRel.isSet()) lstRefsToRemove.push_back(ndxRel);
		}
	}

	bool bSomethingChanged = false;
	CRelIndex ndxSource = m_tagSourcePassage.relIndex();
	ndxSource.setWord(0);
	for (unsigned int ndx = 0; ndx < lstRefsToRemove.size(); ++ndx) {
		if (m_pWorkingUserNotesDatabase->removeCrossReference(m_pBibleDatabase.data(), ndxSource, lstRefsToRemove.at(ndx))) {
			bSomethingChanged = true;
		}
	}

	if (bSomethingChanged) m_bIsDirty = true;
}

// ============================================================================
