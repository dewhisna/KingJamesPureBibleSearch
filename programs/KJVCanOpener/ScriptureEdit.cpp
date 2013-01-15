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

#include "ScriptureEdit.h"

#include "dbstruct.h"
#include "KJVPassageNavigatorDlg.h"
#include "MimeHelper.h"
#include "PersistentSettings.h"

#include <assert.h>

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QByteArray>
#include <QDataStream>
#include <QString>
#include <QEvent>
#include <QHelpEvent>


// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------
	// Find Dialog:
	const QString constrFindDialogGroup("FindDialog");
}

// ============================================================================

template <class T, class U>
CScriptureText<T,U>::CScriptureText(QWidget *parent)
	:	T(parent),
		m_pFindDialog(NULL),
		m_bDoingPopup(false),
		m_navigator(*this, T::useToolTipEdit()),
		m_bDoPlainCopyOnly(false),
		m_pEditMenu(NULL),
		m_pActionCopy(NULL),
		m_pActionCopyPlain(NULL),
		m_pActionCopyRaw(NULL),
		m_pActionCopyVeryRaw(NULL),
		m_pActionCopyVerses(NULL),
		m_pActionCopyVersesPlain(NULL),
		m_pActionCopyReferenceDetails(NULL),
		m_pActionCopyPassageStatistics(NULL),
		m_pActionCopyEntirePassageDetails(NULL),
		m_pActionSelectAll(NULL),
		m_pActionFind(NULL),
		m_pActionFindNext(NULL),
		m_pActionFindPrev(NULL),
		m_pStatusAction(NULL)
{
	T::setMouseTracking(true);
	T::installEventFilter(this);

	T::viewport()->setCursor(QCursor(Qt::ArrowCursor));

	m_HighlightTimer.stop();

	// FindDialog:
	m_pFindDialog = new FindDialog(this);
	m_pFindDialog->setModal(false);
	m_pFindDialog->setTextEdit(this);

	T::connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(on_cursorPositionChanged()));
	T::connect(this, SIGNAL(selectionChanged()), this, SLOT(on_selectionChanged()));
	T::connect(&m_navigator, SIGNAL(changedDocumentText()), &m_Highlighter, SLOT(clearPhraseTags()));
	T::connect(&m_HighlightTimer, SIGNAL(timeout()), this, SLOT(clearHighlighting()));

	m_pEditMenu = new QMenu("&Edit", this);
	m_pEditMenu->setStatusTip("Scripture Text Edit Operations");
	m_pActionCopy = m_pEditMenu->addAction("&Copy as shown", this, SLOT(on_copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	m_pActionCopy->setStatusTip("Copy selected passage browser text, as shown, to the clipboard");
	m_pActionCopy->setEnabled(false);
	T::connect(this, SIGNAL(copyAvailable(bool)), m_pActionCopy, SLOT(setEnabled(bool)));
	m_pActionCopyPlain = m_pEditMenu->addAction("Copy as shown (&plain)", this, SLOT(on_copyPlain()));
	m_pActionCopyPlain->setStatusTip("Copy selected passage browser text, as shown but without colors and fonts, to the clipboard");
	m_pActionCopyPlain->setEnabled(false);
	T::connect(this, SIGNAL(copyAvailable(bool)), m_pActionCopyPlain, SLOT(setEnabled(bool)));
	m_pActionCopyRaw = m_pEditMenu->addAction("Copy Raw &Text (No headings)", this, SLOT(on_copyRaw()), QKeySequence(Qt::CTRL + Qt::Key_T));
	m_pActionCopyRaw->setStatusTip("Copy selected passage browser text as raw phrase words to the clipboard");
	m_pActionCopyRaw->setEnabled(false);
	T::connect(this, SIGNAL(copyRawAvailable(bool)), m_pActionCopyRaw, SLOT(setEnabled(bool)));
	m_pActionCopyVeryRaw = m_pEditMenu->addAction("Copy Very Ra&w Text (No punctuation)", this, SLOT(on_copyVeryRaw()), QKeySequence(Qt::CTRL + Qt::Key_W));
	m_pActionCopyVeryRaw->setStatusTip("Copy selected passage browser text as very raw (no punctuation) phrase words to the clipboard");
	m_pActionCopyVeryRaw->setEnabled(false);
	T::connect(this, SIGNAL(copyRawAvailable(bool)), m_pActionCopyVeryRaw, SLOT(setEnabled(bool)));
	m_pEditMenu->addSeparator();
	m_pActionCopyVerses = m_pEditMenu->addAction("Copy as &Verses", this, SLOT(on_copyVerses()));
	m_pActionCopyVerses->setStatusTip("Copy selected passage browser text as Formatted Verses to the clipboard");
	m_pActionCopyVerses->setEnabled(false);
	T::connect(this, SIGNAL(copyRawAvailable(bool)), m_pActionCopyVerses, SLOT(setEnabled(bool)));
	m_pActionCopyVersesPlain = m_pEditMenu->addAction("Copy as Verses (plai&n)", this, SLOT(on_copyVersesPlain()));
	m_pActionCopyVersesPlain->setStatusTip("Copy selected passage browser text as Formatted Verses, but without colors and fonts, to the clipboard");
	m_pActionCopyVersesPlain->setEnabled(false);
	T::connect(this, SIGNAL(copyRawAvailable(bool)), m_pActionCopyVersesPlain, SLOT(setEnabled(bool)));
	m_pEditMenu->addSeparator();
	m_pActionCopyReferenceDetails = m_pEditMenu->addAction("Copy &Reference Details (Word/Phrase)", this, SLOT(on_copyReferenceDetails()), QKeySequence(Qt::CTRL + Qt::Key_R));
	m_pActionCopyReferenceDetails->setStatusTip("Copy the Word/Phrase Reference Details in the passage browser to the clipboard");
	m_pActionCopyPassageStatistics = m_pEditMenu->addAction("Copy Passage Stat&istics (Book/Chapter/Verse)", this, SLOT(on_copyPassageStatistics()), QKeySequence(Qt::CTRL + Qt::Key_I));
	m_pActionCopyPassageStatistics->setStatusTip("Copy the Book/Chapter/Verse Passage Statistics in the passage browser to the clipboard");
	m_pActionCopyEntirePassageDetails = m_pEditMenu->addAction("Copy Entire Passage Detai&ls", this, SLOT(on_copyEntirePassageDetails()), QKeySequence(Qt::CTRL + Qt::Key_L));
	m_pActionCopyEntirePassageDetails->setStatusTip("Copy both the Word/Phrase Reference Detail and Book/Chapter/Verse Statistics in the passage browser to the clipboard");
	m_pEditMenu->addSeparator();
	m_pActionSelectAll = m_pEditMenu->addAction("Select &All", this, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	m_pActionSelectAll->setStatusTip("Select all current passage browser text");
	m_pEditMenu->addSeparator();
	m_pActionFind = m_pEditMenu->addAction("&Find...", this, SLOT(on_findDialog()), QKeySequence(Qt::CTRL + Qt::Key_F));
	m_pActionFind->setStatusTip("Find text within the passage browser");
	m_pActionFind->setEnabled(T::useFindDialog());
	m_pActionFindNext = m_pEditMenu->addAction("Find &Next", m_pFindDialog, SLOT(findNext()), QKeySequence(Qt::Key_F3));
	m_pActionFindNext->setStatusTip("Find next occurrence of text within the passage browser");
	m_pActionFindNext->setEnabled(T::useFindDialog());
	m_pActionFindPrev = m_pEditMenu->addAction("Find &Previous", m_pFindDialog, SLOT(findPrev()), QKeySequence(Qt::SHIFT + Qt::Key_F3));
	m_pActionFindPrev->setStatusTip("Find previous occurrence of text within the passage browser");
	m_pActionFindPrev->setEnabled(T::useFindDialog());

//	T::connect(ui->actionReplace, SIGNAL(triggered()), this, SLOT(findReplaceDialog()));

	m_pStatusAction = new QAction(this);
}

template<class T, class U>
CScriptureText<T,U>::~CScriptureText()
{

}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::savePersistentSettings(const QString &strGroup)
{
	QSettings &settings(CPersistentSettings::instance()->settings());
	m_pFindDialog->writeSettings(settings, groupCombine(strGroup, constrFindDialogGroup));
}

template<class T, class U>
void CScriptureText<T,U>::restorePersistentSettings(const QString &strGroup)
{
	QSettings &settings(CPersistentSettings::instance()->settings());
	m_pFindDialog->readSettings(settings, groupCombine(strGroup, constrFindDialogGroup));
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::on_findDialog()
{
	if (haveSelection()) {
		m_pFindDialog->setTextToFind(m_selectedPhrase.first.phraseRaw());
	}
	if (m_pFindDialog->isVisible()) {
		m_pFindDialog->activateWindow();
	} else {
		m_pFindDialog->show();
	}
}

// ----------------------------------------------------------------------------

template<class T, class U>
void CScriptureText<T,U>::clearHighlighting()
{
	if (!m_bDoingPopup) {
		m_navigator.doHighlighting(m_Highlighter, true);
		m_Highlighter.clearPhraseTags();
		m_HighlightTimer.stop();
	}
}

template<class T, class U>
bool CScriptureText<T,U>::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == this) {
		switch (ev->type()) {
			case QEvent::Wheel:
			case QEvent::ActivationChange:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			case QEvent::FocusOut:
			case QEvent::FocusIn:
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonDblClick:
			case QEvent::Leave:
				return false;
			default:
				break;
		}
	}

	return U::eventFilter(obj, ev);
}

template<class T, class U>
bool CScriptureText<T,U>::event(QEvent *ev)
{
	if (ev->type() == QEvent::FocusIn) emit T::activatedScriptureText();

	switch (ev->type()) {
		case QEvent::ToolTip:
			{
				if ((!U::hasFocus()) || (!haveDetails())) {
					ev->ignore();
					return true;
				}

//				QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(ev);
//				if (m_navigator.handleToolTipEvent(pHelpEvent, m_Highlighter, m_selectedPhrase.second)) {
//					m_HighlightTimer.stop();
//				} else {
//					pHelpEvent->ignore();
//				}
//				return true;
			}
			break;

		// User input and window activation makes tooltips sleep
		case QEvent::Wheel:
		case QEvent::ActivationChange:
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::FocusOut:
		case QEvent::FocusIn:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
			// Unfortunately, there doesn't seem to be any event we can hook to to determine
			//		when the ToolTip disappears.  Looking at the Qt code, it looks to be on
			//		a 2 second timeout.  So, we'll do a similar timeout here for the highlight:
			if ((!m_bDoingPopup) && (!m_Highlighter.getHighlightTags().isEmpty()) && (!m_HighlightTimer.isActive()))
				m_HighlightTimer.start(2000);
			break;
		case QEvent::Leave:
			if ((!m_bDoingPopup) && (!m_Highlighter.getHighlightTags().isEmpty())) {
				m_HighlightTimer.start(20);
			}
			break;
		default:
			break;
	}

	return U::event(ev);
}

template<class T, class U>
bool CScriptureText<T,U>::haveDetails() const
{
	QString strToolTip = m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second);
	return (!strToolTip.isEmpty());
}

template<class T, class U>
void CScriptureText<T,U>::showDetails()
{
	U::ensureCursorVisible();
	if (m_navigator.handleToolTipEvent(m_Highlighter, m_tagLast, m_selectedPhrase.second))
		m_HighlightTimer.stop();
}

template<>
void CScriptureText<i_CScriptureEdit, QTextEdit>::mouseDoubleClickEvent(QMouseEvent *ev)
{
	begin_popup();

	CRelIndex ndxLast = m_navigator.ResolveCursorReference(cursorForPosition(ev->pos()));
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
	m_navigator.highlightTag(m_Highlighter, m_tagLast);
	if (ndxLast.isSet()) emit gotoIndex(m_tagLast);

	end_popup();
}

template<>
void CScriptureText<i_CScriptureBrowser, QTextBrowser>::mouseDoubleClickEvent(QMouseEvent *ev)
{
	QTextBrowser::mouseDoubleClickEvent(ev);
}

template<class T, class U>
void CScriptureText<T,U>::on_passageNavigator()
{
	begin_popup();

	// This now works exclusively by edit cursor position, not the mouse position from
	//		hovering as it used to when there was no selection.  This is so the menu
	//		Ctrl-G shortcut to activate this will make sense and be consistent across
	//		the entire app.

	TPhraseTag tagSel = m_selectedPhrase.second;
	if (!tagSel.first.isSet()) tagSel.first = m_tagLast.first;
	if (tagSel.second == 0) tagSel.second = ((tagSel.first.word() != 0) ? 1 : 0);			// Simulate single word selection if nothing actually selected, but only if there is a word

	// Cap the number of words to those remaining in this verse so
	//		we don't spend all day highlighting junk:
	TPhraseTag tagHighlight = tagSel;
	CRefCountCalc Wrd(CRefCountCalc::RTE_WORD, tagHighlight.first);
	tagHighlight.second = qMin(Wrd.ofVerse().second - Wrd.ofVerse().first + 1, tagHighlight.second);

	m_Highlighter.setEnabled(true);
	m_navigator.highlightTag(m_Highlighter, tagHighlight);
	CKJVPassageNavigatorDlg dlg(T::parentWidget());
//	dlg.navigator().startRelativeMode(tagSel, false);
	dlg.navigator().startAbsoluteMode(tagSel);
	if (dlg.exec() == QDialog::Accepted) {
		emit T::gotoIndex(dlg.passage());
	}

	end_popup();
}

template<class T, class U>
void CScriptureText<T,U>::contextMenuEvent(QContextMenuEvent *ev)
{
	begin_popup();

	CRelIndex ndxLast = m_navigator.ResolveCursorReference(T::cursorForPosition(ev->pos()));
	m_tagLast = TPhraseTag(ndxLast, (ndxLast.isSet() ? 1 : 0));
	m_navigator.highlightTag(m_Highlighter, m_tagLast);
	QMenu menu;
	menu.addAction(m_pActionCopy);
	menu.addAction(m_pActionCopyPlain);
	menu.addAction(m_pActionCopyRaw);
	menu.addAction(m_pActionCopyVeryRaw);
	menu.addSeparator();
	menu.addAction(m_pActionCopyVerses);
	menu.addAction(m_pActionCopyVersesPlain);
	menu.addSeparator();
	menu.addAction(m_pActionCopyReferenceDetails);
	menu.addAction(m_pActionCopyPassageStatistics);
	menu.addAction(m_pActionCopyEntirePassageDetails);
	menu.addSeparator();
	menu.addAction(m_pActionSelectAll);
	if (T::useFindDialog()) {
		menu.addSeparator();
		menu.addAction(m_pActionFind);
		menu.addAction(m_pActionFindNext);
		menu.addAction(m_pActionFindPrev);
	}
	menu.addSeparator();
	QAction *pActionNavigator = menu.addAction("Passage &Navigator...");
	pActionNavigator->setEnabled(T::connect(pActionNavigator, SIGNAL(triggered()), this, SLOT(on_passageNavigator())));
	pActionNavigator->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
	menu.addSeparator();
	QAction *pActionDetails = menu.addAction("View &Details...");
	pActionDetails->setEnabled(haveDetails());
	pActionDetails->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	T::connect(pActionDetails, SIGNAL(triggered()), this, SLOT(showDetails()));
	menu.exec(ev->globalPos());

	end_popup();
}

template<class T, class U>
QMimeData *CScriptureText<T,U>::createMimeDataFromSelection() const
{
	QMimeData *mime = U::createMimeDataFromSelection();
	if (m_bDoPlainCopyOnly) {
		// Let the base class do the copy, but snag the plaintext
		//	version and render only that:
		QString strTemp = mime->text();
		mime->clear();
		mime->setText(strTemp);
	} else {
		if (mime->hasHtml()) {
			QTextDocument docCopy;
			docCopy.setHtml(mime->html());
			CPhraseNavigator navigator(docCopy);
			navigator.removeAnchors();
			mime->setHtml(docCopy.toHtml());
		}
	}
	if (haveSelection()) CMimeHelper::addPhraseTagToMimeData(mime, m_selectedPhrase.second);
	return mime;
}

template<class T, class U>
void CScriptureText<T,U>::on_cursorPositionChanged()
{
	CPhraseCursor cursor(T::textCursor());
	m_tagLast.first = m_navigator.ResolveCursorReference(cursor);
	if (!m_tagLast.first.isSet()) m_tagLast.second = 0;

	// Move start of selection tag so we can later simulate pseudo-selection of
	//		single word when nothing is really selected:
	updateSelection();
}

template<class T, class U>
void CScriptureText<T,U>::on_selectionChanged()
{
	updateSelection();
}

template<class T, class U>
void CScriptureText<T,U>::updateSelection()
{
	bool bOldSel = haveSelection();
	m_selectedPhrase = m_navigator.getSelectedPhrase();
	if (haveSelection() != bOldSel) emit T::copyRawAvailable(haveSelection());
	QString strStatusText;
	if (haveSelection()) {
		strStatusText = m_selectedPhrase.second.PassageReferenceRangeText();
	} else if (m_tagLast.first.isSet()) {
		strStatusText = m_tagLast.first.PassageReferenceText();
	}

	if (m_selectedPhrase.second.second > 0) {
		if (!strStatusText.isEmpty()) strStatusText += " : ";
		strStatusText += QString("%1 Word(s) Selected").arg(m_selectedPhrase.second.second);
	}
	T::setStatusTip(strStatusText);
	m_pStatusAction->setStatusTip(strStatusText);
	m_pStatusAction->showStatusText();

	if (!haveSelection()) {
		TPhraseTagList lstTags(m_Highlighter.getHighlightTags());
		TPhraseTag nNewSel = TPhraseTag(m_tagLast.first, 1);
		if  ((lstTags.size() == 0) || (lstTags[0] != nNewSel))
			m_navigator.highlightTag(m_Highlighter, nNewSel);
	}
	m_Highlighter.setEnabled(!haveSelection());
}

template<class T, class U>
void CScriptureText<T,U>::on_copy()
{
	// Clear highlighting before copy so we don't have the cursor follow highlighter
	//		copied in the middle of our copied text
	m_bDoingPopup = false;
	clearHighlighting();
	T::copy();
}

template<class T, class U>
void CScriptureText<T,U>::on_copyPlain()
{
	// Clear highlighting before copy so we don't have the cursor follow highlighter
	//		copied in the middle of our copied text
	m_bDoingPopup = false;
	clearHighlighting();
	m_bDoPlainCopyOnly = true;		// Do plaintext only so user can paste into Word without changing its format, for example
	T::copy();
	m_bDoPlainCopyOnly = false;
}

template<class T, class U>
void CScriptureText<T,U>::on_copyRaw()
{
	if (!haveSelection()) return;
	QMimeData *mime = new QMimeData();
	mime->setText(m_selectedPhrase.first.phrase());
	CMimeHelper::addPhraseTagToMimeData(mime, m_selectedPhrase.second);
	QApplication::clipboard()->setMimeData(mime);
}

template<class T, class U>
void CScriptureText<T,U>::on_copyVeryRaw()
{
	if (!haveSelection()) return;
	QMimeData *mime = new QMimeData();
	mime->setText(m_selectedPhrase.first.phraseRaw());
	CMimeHelper::addPhraseTagToMimeData(mime, m_selectedPhrase.second);
	QApplication::clipboard()->setMimeData(mime);
}

template<class T, class U>
void CScriptureText<T,U>::on_copyVerses()
{
	if (haveSelection()) copyVersesCommon(false);
}

template<class T, class U>
void CScriptureText<T,U>::on_copyVersesPlain()
{
	if (haveSelection()) copyVersesCommon(true);
}

template<class T, class U>
void CScriptureText<T,U>::on_copyReferenceDetails()
{
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second, CPhraseEditNavigator::TTE_REFERENCE_ONLY, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second, CPhraseEditNavigator::TTE_REFERENCE_ONLY, false));
	CMimeHelper::addPhraseTagToMimeData(mime, selection());
	QApplication::clipboard()->setMimeData(mime);
}

template<class T, class U>
void CScriptureText<T,U>::on_copyPassageStatistics()
{
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second, CPhraseEditNavigator::TTE_STATISTICS_ONLY, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second, CPhraseEditNavigator::TTE_STATISTICS_ONLY, false));
	CMimeHelper::addPhraseTagToMimeData(mime, selection());
	QApplication::clipboard()->setMimeData(mime);
}

template<class T, class U>
void CScriptureText<T,U>::on_copyEntirePassageDetails()
{
	QMimeData *mime = new QMimeData();
	mime->setText(m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second, CPhraseEditNavigator::TTE_COMPLETE, true));
	mime->setHtml(m_navigator.getToolTip(m_tagLast, m_selectedPhrase.second, CPhraseEditNavigator::TTE_COMPLETE, false));
	CMimeHelper::addPhraseTagToMimeData(mime, selection());
	QApplication::clipboard()->setMimeData(mime);
}

template<class T, class U>
void CScriptureText<T,U>::copyVersesCommon(bool bPlainOnly)
{
	QTextDocument docFormattedVerses;
	CPhraseNavigator navigator(docFormattedVerses);
	navigator.setDocumentToFormattedVerses(m_selectedPhrase.second);

	QMimeData *mime = new QMimeData();
	mime->setText(docFormattedVerses.toPlainText());
	if (!bPlainOnly) mime->setHtml(docFormattedVerses.toHtml());
	QApplication::clipboard()->setMimeData(mime);
}

// ============================================================================

template class CScriptureText<i_CScriptureEdit, QTextEdit>;
template class CScriptureText<i_CScriptureBrowser, QTextBrowser>;

