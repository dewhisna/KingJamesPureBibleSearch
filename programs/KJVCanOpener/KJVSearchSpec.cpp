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

#include "KJVSearchSpec.h"
#include "ui_KJVSearchSpec.h"

#include "PhraseEdit.h"
#include "PhraseListModel.h"
#include "Highlighter.h"
#include "PersistentSettings.h"
#include "BusyCursor.h"

#include <QScrollBar>
#include <QItemSelection>
#include <QTimer>

// ============================================================================

QSize CSearchPhraseScrollArea::minimumSizeHint() const
{
	return QScrollArea::minimumSizeHint();
}

QSize CSearchPhraseScrollArea::sizeHint() const
{
	return QScrollArea::sizeHint();
}

// ============================================================================

CKJVSearchSpec::CKJVSearchSpec(CBibleDatabasePtr pBibleDatabase, bool bHaveUserDatabase, QWidget *parent)
	:	QWidget(parent),
		m_pBibleDatabase(pBibleDatabase),
		m_bHaveUserDatabase(bHaveUserDatabase),
		m_pLayoutPhrases(NULL),
		m_pLastEditorActive(NULL),
		m_bDoneActivation(false),
		ui(new Ui::CKJVSearchSpec)
{
	ui->setupUi(this);

	// -------------------- Search Phrase Widgets:

	m_pLayoutPhrases = new QVBoxLayout(ui->scrollAreaWidgetContents);
	m_pLayoutPhrases->setSpacing(0);
	m_pLayoutPhrases->setContentsMargins(0, 0, 0, 0);

	ui->scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = addSearchPhrase();
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));

	ui->scrollAreaSearchPhrases->setMinimumSize(m_pLayoutPhrases->sizeHint().width() +
							ui->scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui->scrollAreaSearchPhrases->frameWidth() * 2,
							m_pLayoutPhrases->sizeHint().height() /* pFirstSearchPhraseEditor->sizeHint() */);

	ui->scrollAreaSearchPhrases->installEventFilter(this);

//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);


	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(false);

	connect(ui->widgetSearchCriteria, SIGNAL(addSearchPhraseClicked()), this, SLOT(addSearchPhrase()));
	connect(ui->widgetSearchCriteria, SIGNAL(changedSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM)), this, SLOT(on_changedSearchCriteria()));

	// Connect Pass-through:
	connect(ui->widgetSearchCriteria, SIGNAL(copySearchPhraseSummary()), this, SIGNAL(copySearchPhraseSummary()));

}

CKJVSearchSpec::~CKJVSearchSpec()
{
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		delete m_lstSearchPhraseEditors[ndx];
	}
	m_lstSearchPhraseEditors.clear();

	m_pLastEditorActive = NULL;

	delete ui;
}

// ------------------------------------------------------------------

void CKJVSearchSpec::enableCopySearchPhraseSummary(bool bEnable)
{
	ui->widgetSearchCriteria->enableCopySearchPhraseSummary(bEnable);
}

void CKJVSearchSpec::setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode)
{
	ui->widgetSearchCriteria->setSearchScopeMode(mode);
}

// ------------------------------------------------------------------

void CKJVSearchSpec::reset()
{
	closeAllSearchPhrases();
	addSearchPhrase();
	ui->widgetSearchCriteria->setSearchScopeMode(CSearchCriteria::SSME_WHOLE_BIBLE);
}

void CKJVSearchSpec::readKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup)
{
	CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScope = CSearchCriteria::SSME_WHOLE_BIBLE;

	closeAllSearchPhrases();

	kjsFile.beginGroup(groupCombine(strSubgroup, "SearchCriteria"));
	nSearchScope = static_cast<CSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(kjsFile.value("SearchScope", CSearchCriteria::SSME_WHOLE_BIBLE).toInt());
	if ((nSearchScope < CSearchCriteria::SSME_WHOLE_BIBLE) ||
		(nSearchScope > CSearchCriteria::SSME_VERSE))
		nSearchScope = CSearchCriteria::SSME_WHOLE_BIBLE;
	kjsFile.endGroup();

	ui->widgetSearchCriteria->setSearchScopeMode(nSearchScope);

	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = NULL;
	int nPhrases = kjsFile.beginReadArray(groupCombine(strSubgroup, "SearchPhrases"));
	if (nPhrases != 0) {
		for (int ndx = 0; ndx < nPhrases; ++ndx) {
			CKJVSearchPhraseEdit *pPhraseEditor = addSearchPhrase();
			assert(pPhraseEditor != NULL);
			if (ndx == 0) pFirstSearchPhraseEditor = pPhraseEditor;
			kjsFile.setArrayIndex(ndx);
			pPhraseEditor->phraseEditor()->setCaseSensitive(kjsFile.value("CaseSensitive", false).toBool());
			pPhraseEditor->phraseEditor()->setText(kjsFile.value("Phrase").toString());
		}
	} else {
		// If the search had no phrases (like default loading from registry), start
		//		with a single empty search phrase:
		pFirstSearchPhraseEditor = addSearchPhrase();
	}
	kjsFile.endArray();

	// Set focus to our first editor.  Note that calling of focusEditor
	//	doesn't work when running from the constructor during a restore
	//	operation.  So we'll set it to trigger later:
	assert(pFirstSearchPhraseEditor != NULL);
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));
}

void CKJVSearchSpec::writeKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup) const
{
	kjsFile.beginGroup(groupCombine(strSubgroup, "SearchCriteria"));
	kjsFile.setValue("SearchScope", ui->widgetSearchCriteria->searchCriteria().searchScopeMode());
	kjsFile.endGroup();

	int ndxCurrent = 0;
	kjsFile.beginWriteArray(groupCombine(strSubgroup, "SearchPhrases"));
	kjsFile.remove("");
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		if (m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase().isEmpty()) continue;
		kjsFile.setArrayIndex(ndxCurrent);
		kjsFile.setValue("Phrase", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase());
		kjsFile.setValue("CaseSensitive", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isCaseSensitive());
		ndxCurrent++;
	}
	kjsFile.endArray();
}

// ------------------------------------------------------------------

void CKJVSearchSpec::setFocusSearchPhrase(int nIndex)
{
	if ((nIndex >= 0) && (nIndex < m_lstSearchPhraseEditors.size()))
		setFocusSearchPhrase(m_lstSearchPhraseEditors.at(nIndex));
}

void CKJVSearchSpec::setFocusSearchPhrase(const CKJVSearchPhraseEdit *pSearchPhrase)
{
	assert(pSearchPhrase != NULL);
	pSearchPhrase->focusEditor();
	ensureSearchPhraseVisible(pSearchPhrase);
}

// ------------------------------------------------------------------

void CKJVSearchSpec::closeAllSearchPhrases()
{
	for (int ndx = m_lstSearchPhraseEditors.size()-1; ndx>=0; --ndx) {
		m_lstSearchPhraseEditors.at(ndx)->closeSearchPhrase();
	}
}

CKJVSearchPhraseEdit *CKJVSearchSpec::addSearchPhrase()
{
	assert(m_pBibleDatabase.data() != NULL);

	CKJVSearchPhraseEdit *pPhraseWidget = new CKJVSearchPhraseEdit(m_pBibleDatabase, haveUserDatabase(), this);
	connect(pPhraseWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SLOT(on_closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(on_phraseChanged(CKJVSearchPhraseEdit *)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit*)), this, SLOT(on_activatedPhraseEditor(const CPhraseLineEdit*)));

	// Set pass-throughs:
	connect(pPhraseWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit*)), this, SIGNAL(phraseChanged(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)));

	m_lstSearchPhraseEditors.append(pPhraseWidget);
	pPhraseWidget->showSeperatorLine(m_lstSearchPhraseEditors.size() > 1);
	pPhraseWidget->resize(pPhraseWidget->minimumSizeHint());
	m_pLayoutPhrases->addWidget(pPhraseWidget);
	// Calculate height, since it varies depending on whether or not the widget is showing a separator:
	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	ui->scrollAreaWidgetContents->setMinimumSize(pPhraseWidget->sizeHint().width(), nHeight);
	ensureSearchPhraseVisible(pPhraseWidget);
	pPhraseWidget->phraseStatisticsChanged();
	pPhraseWidget->focusEditor();

//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);

	return pPhraseWidget;
}

void CKJVSearchSpec::ensureSearchPhraseVisible(int nIndex)
{
	if ((nIndex >= 0) && (nIndex < m_lstSearchPhraseEditors.size())) {
		ensureSearchPhraseVisible(m_lstSearchPhraseEditors.at(nIndex));
	}
}

void CKJVSearchSpec::ensureSearchPhraseVisible(const CKJVSearchPhraseEdit *pSearchPhrase)
{
	// Calculate height, since it varies depending on whether or not the widget is showing a separator:
	int nHeight = 0;
	bool bFound = false;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
		if (m_lstSearchPhraseEditors.at(ndx) == pSearchPhrase) {
			bFound = true;
			break;
		}
	}
	if (bFound) ui->scrollAreaSearchPhrases->ensureVisible((pSearchPhrase->sizeHint().width()/2),
															nHeight - (pSearchPhrase->sizeHint().height()/2));
}

void CKJVSearchSpec::on_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase)
{
	assert(pSearchPhrase != NULL);

	if (pSearchPhrase->phraseEditor() == m_pLastEditorActive) m_pLastEditorActive = NULL;

	bool bPhraseChanged = ((!pSearchPhrase->parsedPhrase()->IsDuplicate()) &&
							(pSearchPhrase->parsedPhrase()->GetNumberOfMatches() != 0) &&
							(pSearchPhrase->parsedPhrase()->isCompleteMatch()));

	int ndx = m_lstSearchPhraseEditors.indexOf(pSearchPhrase);
	assert(ndx != -1);
	if (ndx != -1) {
		m_lstSearchPhraseEditors.removeAt(ndx);
	}
	if ((ndx == 0) && (m_lstSearchPhraseEditors.size() != 0))
		m_lstSearchPhraseEditors.at(0)->showSeperatorLine(false);
	int ndxActivate = ((ndx < m_lstSearchPhraseEditors.size()) ? ndx : ndx-1);

	int nHeight = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	ui->scrollAreaWidgetContents->setMinimumSize(ui->scrollAreaWidgetContents->minimumSize().width(), nHeight);
	if (bPhraseChanged) on_phraseChanged(NULL);

	setFocusSearchPhrase(ndxActivate);
}

void CKJVSearchSpec::on_changedSearchCriteria()
{
	on_phraseChanged(NULL);
}

typedef struct {
	unsigned int m_nNumMatches;
	unsigned int m_nNumContributingMatches;
} TPhraseOccurrenceInfo;
Q_DECLARE_METATYPE(TPhraseOccurrenceInfo)

QString CKJVSearchSpec::searchPhraseSummaryText() const
{
	int nNumPhrases = 0;
	bool bCaseSensitive = false;

	CPhraseList phrases;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		if ((pPhrase->GetNumberOfMatches()) &&
			(!pPhrase->IsDuplicate())) {
			nNumPhrases++;
			CPhraseEntry entry;
			entry.m_bCaseSensitive = pPhrase->isCaseSensitive();
			entry.m_strPhrase = pPhrase->phrase();
			entry.m_nNumWrd = pPhrase->phraseSize();
			TPhraseOccurrenceInfo poiUsage;
			poiUsage.m_nNumMatches = pPhrase->GetNumberOfMatches();
			poiUsage.m_nNumContributingMatches = pPhrase->GetContributingNumberOfMatches();
			entry.m_varExtraInfo = QVariant::fromValue(poiUsage);
			phrases.append(entry);
			if (entry.m_bCaseSensitive) bCaseSensitive = true;
		}
	}

	CPhraseListModel mdlPhrases(phrases);
	mdlPhrases.sort(0);

	QString strScope;
	CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nScope = ui->widgetSearchCriteria->searchCriteria().searchScopeMode();
	switch (nScope) {
		case (CSearchCriteria::SSME_WHOLE_BIBLE):
			strScope = tr("in the Entire Bible");
			break;
		case (CSearchCriteria::SSME_TESTAMENT):
			strScope = tr("in the same Testament");
			break;
		case (CSearchCriteria::SSME_BOOK):
			strScope = tr("in the same Book");
			break;
		case (CSearchCriteria::SSME_CHAPTER):
			strScope = tr("in the same Chapter");
			break;
		case (CSearchCriteria::SSME_VERSE):
			strScope = tr("in the same Verse");
			break;
		default:
			break;
	}

	QString strSummary;
	if (nNumPhrases != 1) {
		strSummary += tr("Search of %n Phrase(s) %1:\n", NULL, nNumPhrases).arg(strScope);
	} else {
		strSummary += tr("Search of:") + " ";
	}
	if (nNumPhrases > 1) strSummary += "\n";
	for (int ndx=0; ndx<mdlPhrases.rowCount(); ++ndx) {
		const CPhraseEntry &aPhrase = mdlPhrases.index(ndx).data(CPhraseListModel::PHRASE_ENTRY_ROLE).value<CPhraseEntry>();
		if (nNumPhrases > 1) {
			if (nScope != CSearchCriteria::SSME_WHOLE_BIBLE) {
				strSummary += QString("    \"%1\" ").arg(mdlPhrases.index(ndx).data().toString()) +
								tr("(Found %n Time(s) in the Entire Bible, %1 in Scope)", NULL, aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches)
									.arg(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches) + "\n";
			} else {
				strSummary += QString("    \"%1\" ").arg(mdlPhrases.index(ndx).data().toString()) +
								tr("(Found %n Time(s) in the Entire Bible)", NULL, aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches) + "\n";
				assert(aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumMatches == aPhrase.m_varExtraInfo.value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
			}
		} else {
			strSummary += QString("\"%1\"\n").arg(mdlPhrases.index(ndx).data().toString());
		}
	}
	if (bCaseSensitive) {
		if (nNumPhrases > 1) strSummary += "\n";
		strSummary += "    " + tr("(%1 = Case Sensitive)").arg(QChar(0xA7)) + "\n";
	}
	if (nNumPhrases) strSummary += "\n";

	return strSummary;
}

void CKJVSearchSpec::on_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase)
{
	Q_UNUSED(pSearchPhrase);

	CBusyCursor iAmBusy(NULL);

	TParsedPhrasesList lstPhrases;
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		pPhrase->SetContributingNumberOfMatches(0);
		pPhrase->SetIsDuplicate(false);
		pPhrase->ClearScopedPhraseTagSearchResults();
		if ((!pPhrase->isCompleteMatch()) || (pPhrase->GetNumberOfMatches() == 0)) {
			continue;		// Don't include phrases that had no matches of themselves
		}
		// Check for phrases with the same text and ignore them:
		bool bDuplicate = false;
		for (int ndx2 = 0; ndx2 < ndx; ++ndx2) {
			if ((*pPhrase) == (*m_lstSearchPhraseEditors.at(ndx2)->parsedPhrase())) {
				bDuplicate = true;
				break;
			}
		}
		if (bDuplicate) {
			pPhrase->SetIsDuplicate(true);
			continue;
		}

		// Do sorted insertion so the results with the greatest matches comes last.  That
		//		way it will have more things we'll discard rather than things we'll uselessly
		//		look at:
		int ndxInsert = 0;
		for ( ; ndxInsert < lstPhrases.size(); ++ndxInsert) {
			if (pPhrase->GetNumberOfMatches() < lstPhrases.at(ndxInsert)->GetNumberOfMatches()) break;
		}
		lstPhrases.insert(ndxInsert, pPhrase);
	}

	// ----------------------------

	// Emit that we have new search phrases and/or search criteria:
	emit changedSearchSpec(ui->widgetSearchCriteria->searchCriteria(), lstPhrases);

	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
	}
}

void CKJVSearchSpec::on_activatedPhraseEditor(const CPhraseLineEdit *pEditor)
{
	if (pEditor) {
		m_pLastEditorActive = pEditor;
	} else {
		m_bDoneActivation = false;
	}
}

bool CKJVSearchSpec::eventFilter(QObject *obj, QEvent *ev)
{
	if ((obj == ui->scrollAreaSearchPhrases) && (ev->type() == QEvent::FocusIn)) {
		if (m_pLastEditorActive) {
			for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
				if (m_lstSearchPhraseEditors.at(ndx)->phraseEditor() == m_pLastEditorActive) {
					if (!m_bDoneActivation) {
						setFocusSearchPhrase(m_lstSearchPhraseEditors.at(ndx));
					} else {
						m_lstSearchPhraseEditors.at(ndx)->focusEditor();
					}
				}
			}
		} else {
			if (m_lstSearchPhraseEditors.size()) {
				if (!m_bDoneActivation) {
					setFocusSearchPhrase(0);
				} else {
					m_lstSearchPhraseEditors.at(0)->focusEditor();
				}
			}
		}
		m_bDoneActivation = true;
		return true;
	}

	return QWidget::eventFilter(obj, ev);
}
