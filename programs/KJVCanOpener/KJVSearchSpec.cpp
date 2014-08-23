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

#include "PhraseEdit.h"
#include "PhraseListModel.h"
#include "Highlighter.h"
#include "PersistentSettings.h"
#include "BusyCursor.h"

#include <QScrollBar>
#include <QItemSelection>
#include <QTimer>
#include <QApplication>

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
		m_bDoingResizing(false),
		m_pLastEditorActive(NULL),
		m_bDoneActivation(false),
		m_bCloseAllSearchPhrasesInProgress(false),
		m_bReadingSearchFile(false)
{
	ui.setupUi(this);

	// -------------------- Search Phrase Widgets:

	m_pLayoutPhrases = new QVBoxLayout(ui.scrollAreaWidgetContents);
	m_pLayoutPhrases->setSpacing(0);
	m_pLayoutPhrases->setContentsMargins(0, 0, 0, 0);

	ui.scrollAreaWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	// Create pushButton for inserting a new Search Phrase:
	QKeySequence ksAddSearchPhrase(QKeySequence(Qt::CTRL + Qt::Key_P));
	QString strAddButtonText = tr("Add Phrase to Search Criteria", "MainMenu");
	strAddButtonText += QString(" (%1)").arg(ksAddSearchPhrase.toString(QKeySequence::NativeText));
	m_buttonAddSearchPhrase.setText(strAddButtonText);
	m_buttonAddSearchPhrase.setShortcut(ksAddSearchPhrase);
	m_buttonAddSearchPhrase.setStatusTip(tr("Add another Phrase to the current Search Criteria", "MainMenu"));
	m_buttonAddSearchPhrase.adjustSize();			// Must adjust size here so its height() will be correct or else addSearchPhrase() won't set correct height for the scroll area
	connect(&m_buttonAddSearchPhrase, SIGNAL(clicked()), this, SLOT(addSearchPhrase()));
	m_pLayoutPhrases->addWidget(&m_buttonAddSearchPhrase);

	// Create separator between the two buttons:
	m_frameAddCopySeparator.setFrameShape(QFrame::HLine);
	m_frameAddCopySeparator.setFrameShadow(QFrame::Raised);
	m_frameAddCopySeparator.setLineWidth(2);
	m_frameAddCopySeparator.adjustSize();			// Must adjust size here so its height() will be correct or else addSearchPhrase() won't set correct height for the scroll area
	m_pLayoutPhrases->addWidget(&m_frameAddCopySeparator);

	// Create pushButton for Copying Search Phrase Summary:
	m_buttonCopySummary.setText(tr("&Copy Search Phrase Summary to Clipboard", "MainMenu"));
	m_buttonCopySummary.adjustSize();				// Must adjust size here so its height() will be correct or else addSearchPhrase() won't set correct height for the scroll area
	connect(&m_buttonCopySummary, SIGNAL(clicked()), this, SIGNAL(copySearchPhraseSummary()));
	m_pLayoutPhrases->addWidget(&m_buttonCopySummary);
	m_buttonCopySummary.setEnabled(false);

	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = addSearchPhrase();
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));

	ui.scrollAreaSearchPhrases->setMinimumSize(m_pLayoutPhrases->sizeHint().width() +
							ui.scrollAreaSearchPhrases->verticalScrollBar()->sizeHint().width() +
							ui.scrollAreaSearchPhrases->frameWidth() * 2,
							m_pLayoutPhrases->sizeHint().height() /* pFirstSearchPhraseEditor->sizeHint() */);

	ui.scrollAreaSearchPhrases->installEventFilter(this);

//m_modelSearchPhraseEditors.setPhraseEditorsList(m_lstSearchPhraseEditors);


	// -------------------- Search Criteria Widget:

	ui.widgetSearchCriteria->initialize(m_pBibleDatabase);

	connect(ui.widgetSearchCriteria, SIGNAL(changedSearchCriteria()), this, SLOT(en_changedSearchCriteria()));

	// Connect Pass-through:
	connect(ui.widgetSearchCriteria, SIGNAL(gotoIndex(const CRelIndex &)), this, SIGNAL(triggeredSearchWithinGotoIndex(const CRelIndex &)));

	// -------------------- Bible Database Settings:
	//		Used Queued Connection so that Bible Database can catch the signal first and update the rendered word list before we get called:
	connect(CPersistentSettings::instance(), SIGNAL(changedBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &)), this, SLOT(en_changedBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &)), Qt::QueuedConnection);

	// -------------------- Persistent Settings:
	connect(CPersistentSettings::instance(), SIGNAL(changedHideMatchingPhrasesLists(bool)), this, SLOT(en_changedHideMatchingPhrasesLists(bool)), Qt::QueuedConnection);
}

CKJVSearchSpec::~CKJVSearchSpec()
{
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		delete m_lstSearchPhraseEditors[ndx];
	}
	m_lstSearchPhraseEditors.clear();

	m_pLastEditorActive = NULL;
}

// ------------------------------------------------------------------

QString CKJVSearchSpec::searchWindowDescription() const
{
	QString strDescription;

	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		if (!strDescription.isEmpty()) strDescription += "; ";
		strDescription += m_lstSearchPhraseEditors.at(ndx)->phraseEntry().textEncoded();		/* parsedPhrase()->phrase(); */
	}

	if (strDescription.isEmpty()) {
		strDescription = tr("<Empty Search Window>", "MainMenu");
	} else {
		strDescription = QString("\"%1\"").arg(strDescription);
	}

	return strDescription;
}

// ------------------------------------------------------------------

void CKJVSearchSpec::enableCopySearchPhraseSummary(bool bEnable)
{
	m_buttonCopySummary.setEnabled(bEnable);
}

void CKJVSearchSpec::setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode)
{
	ui.widgetSearchCriteria->setSearchScopeMode(mode);
}

// ------------------------------------------------------------------

void CKJVSearchSpec::reset()
{
	closeAllSearchPhrases();
	for (int nCount = 0; nCount < CPersistentSettings::instance()->initialNumberOfSearchPhrases(); ++nCount)
		addSearchPhrase();
	ui.widgetSearchCriteria->setSearchScopeMode(CSearchCriteria::SSME_UNSCOPED);
	ui.widgetSearchCriteria->setSearchWithin(QString());
}

void CKJVSearchSpec::clearAllSearchPhrases()
{
	for (int ndx = m_lstSearchPhraseEditors.size()-1; ndx >= 0; --ndx) {		// Clear in reverse order to minimize flicker due to sizeHint changes during the textChange notifications
		m_lstSearchPhraseEditors.at(ndx)->clearSearchPhrase();
	}
}

void CKJVSearchSpec::readKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup)
{
	CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nSearchScope = CSearchCriteria::SSME_UNSCOPED;
	QString strSearchWithin;

	m_bReadingSearchFile = true;

	closeAllSearchPhrases();

	kjsFile.beginGroup(groupCombine(strSubgroup, "SearchCriteria"));
	nSearchScope = static_cast<CSearchCriteria::SEARCH_SCOPE_MODE_ENUM>(kjsFile.value("SearchScope", CSearchCriteria::SSME_UNSCOPED).toInt());
	strSearchWithin = kjsFile.value("SearchWithin", QString()).toString();
	if ((nSearchScope < SSME_MINIMUM) ||
		(nSearchScope > SSME_MAXIMUM))
		nSearchScope = CSearchCriteria::SSME_UNSCOPED;
	kjsFile.endGroup();

	ui.widgetSearchCriteria->setSearchScopeMode(nSearchScope);
	ui.widgetSearchCriteria->setSearchWithin(strSearchWithin);

	CKJVSearchPhraseEdit *pFirstSearchPhraseEditor = NULL;
	int nPhrases = kjsFile.beginReadArray(groupCombine(strSubgroup, "SearchPhrases"));
	if (nPhrases != 0) {
		for (int ndx = 0; ndx < nPhrases; ++ndx) {
			CKJVSearchPhraseEdit *pPhraseEditor = addSearchPhrase();
			assert(pPhraseEditor != NULL);
			if (ndx == 0) pFirstSearchPhraseEditor = pPhraseEditor;
			kjsFile.setArrayIndex(ndx);
			TPhraseSettings aPhrase;
			aPhrase.m_bCaseSensitive = kjsFile.value("CaseSensitive", false).toBool();
			aPhrase.m_bAccentSensitive = kjsFile.value("AccentSensitive", false).toBool();
			aPhrase.m_bExclude = kjsFile.value("Exclude", false).toBool();
			aPhrase.m_strPhrase = kjsFile.value("Phrase").toString();
			aPhrase.m_bDisabled = kjsFile.value("Disabled", false).toBool();
			pPhraseEditor->setupPhrase(aPhrase);
		}
	} else {
		// If the search had no phrases (like default loading from registry), start
		//		with empty search phrase(s):
		pFirstSearchPhraseEditor = addSearchPhrase();
		for (int nCount = 1; nCount < CPersistentSettings::instance()->initialNumberOfSearchPhrases(); ++nCount)
			addSearchPhrase();
	}
	kjsFile.endArray();

	m_bReadingSearchFile = false;
	en_phraseChanged(NULL);			// Update all results at once

	// Set focus to our first editor.  Note that calling of focusEditor
	//	doesn't work when running from the constructor during a restore
	//	operation.  So we'll set it to trigger later:
	assert(pFirstSearchPhraseEditor != NULL);
	QTimer::singleShot(0, pFirstSearchPhraseEditor, SLOT(focusEditor()));
}

void CKJVSearchSpec::writeKJVSearchFile(QSettings &kjsFile, const QString &strSubgroup) const
{
	kjsFile.beginGroup(groupCombine(strSubgroup, "SearchCriteria"));
	kjsFile.setValue("SearchScope", ui.widgetSearchCriteria->searchCriteria().searchScopeMode());
	kjsFile.setValue("SearchWithin", ui.widgetSearchCriteria->searchCriteria().searchWithinToString());
	kjsFile.endGroup();

	int ndxCurrent = 0;
	kjsFile.beginWriteArray(groupCombine(strSubgroup, "SearchPhrases"));
	kjsFile.remove("");
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		if (m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase().isEmpty()) continue;
		kjsFile.setArrayIndex(ndxCurrent);
		kjsFile.setValue("Phrase", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->phrase());
		kjsFile.setValue("CaseSensitive", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isCaseSensitive());
		kjsFile.setValue("AccentSensitive", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isAccentSensitive());
		kjsFile.setValue("Exclude", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isExcluded());
		kjsFile.setValue("Disabled", m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isDisabled());
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
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		m_lstSearchPhraseEditors.at(ndx)->processPendingTextChanges();
	}

	assert(pSearchPhrase != NULL);
	pSearchPhrase->focusEditor();
	ensureSearchPhraseVisible(pSearchPhrase);
}

// ------------------------------------------------------------------

void CKJVSearchSpec::closeAllSearchPhrases()
{
	// Set flag so we don't emit extra en_phraseChanged() signals:
	m_bCloseAllSearchPhrasesInProgress = true;

	for (int ndx = m_lstSearchPhraseEditors.size()-1; ndx>=0; --ndx) {
		m_lstSearchPhraseEditors.at(ndx)->disconnect(this);
		m_lstSearchPhraseEditors.at(ndx)->disconnect(&m_dlySearchResultsUpdate);
		m_lstSearchPhraseEditors.at(ndx)->closeSearchPhrase();
	}

	m_lstSearchPhraseEditors.clear();
	m_pLastEditorActive = NULL;
	m_bCloseAllSearchPhrasesInProgress = false;

	resizeScrollAreaLayout();
	en_phraseChanged(NULL);							// Still need to emit one change
}

CKJVSearchPhraseEdit *CKJVSearchSpec::addSearchPhrase()
{
	assert(m_pBibleDatabase.data() != NULL);

	CKJVSearchPhraseEdit *pPhraseWidget = new CKJVSearchPhraseEdit(m_pBibleDatabase, haveUserDatabase(), this);
	connect(pPhraseWidget, SIGNAL(resizing(CKJVSearchPhraseEdit*)), this, SLOT(en_phraseResizing(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SLOT(en_closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(changingShowMatchingPhrases(CKJVSearchPhraseEdit*)), this, SLOT(en_changingShowMatchingPhrases(CKJVSearchPhraseEdit*)));
//	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit *)), this, SLOT(en_phraseChanged(CKJVSearchPhraseEdit *)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit*)), this, SLOT(en_activatedPhraseEditor(const CPhraseLineEdit*)));

	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit*)), &m_dlySearchResultsUpdate, SLOT(trigger()));
#ifdef USE_SEARCH_RESULTS_UPDATE_DELAY
	setSearchResultsUpdateDelay(CPersistentSettings::instance()->searchActivationDelay());
	connect(CPersistentSettings::instance(), SIGNAL(changedSearchPhraseActivationDelay(int)), this, SLOT(setSearchResultsUpdateDelay(int)));
#else
	setSearchResultsUpdateDelay(0);
#endif
	connect(&m_dlySearchResultsUpdate, SIGNAL(triggered()), this, SLOT(en_phraseChanged()));
	connect(pPhraseWidget, SIGNAL(enterTriggered()), this, SLOT(en_phraseChanged()), Qt::QueuedConnection);

	// Set pass-throughs:
	connect(pPhraseWidget, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)), this, SIGNAL(closingSearchPhrase(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(phraseChanged(CKJVSearchPhraseEdit*)), this, SIGNAL(phraseChanged(CKJVSearchPhraseEdit*)));
	connect(pPhraseWidget, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)), this, SIGNAL(activatedPhraseEditor(const CPhraseLineEdit *)));

	m_lstSearchPhraseEditors.append(pPhraseWidget);
	pPhraseWidget->showSeperatorLine(true);		// Always show the separator since we have the "AddSearchPhrase" button
	pPhraseWidget->resize(pPhraseWidget->minimumSizeHint());
	m_pLayoutPhrases->insertWidget(m_pLayoutPhrases->indexOf(&m_buttonAddSearchPhrase), pPhraseWidget);		// m_pLayoutPhrases->addWidget(pPhraseWidget);
	resizeScrollAreaLayout();
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
	if (bFound) ui.scrollAreaSearchPhrases->ensureVisible((pSearchPhrase->sizeHint().width()/2),
															nHeight - (pSearchPhrase->sizeHint().height()/2));
}

void CKJVSearchSpec::en_phraseResizing(CKJVSearchPhraseEdit *pSearchPhrase)
{
	Q_UNUSED(pSearchPhrase);

	if (m_bDoingResizing) return;
	resizeScrollAreaLayout();
}

void CKJVSearchSpec::en_changingShowMatchingPhrases(CKJVSearchPhraseEdit *pSearchPhrase)
{
	resizeScrollAreaLayout();
	ensureSearchPhraseVisible(pSearchPhrase);
	setFocusSearchPhrase(pSearchPhrase);
}

void CKJVSearchSpec::resizeScrollAreaLayout()
{
	m_bDoingResizing = true;

	// Calculate height, since it varies depending on whether or not the widget is showing a separator:
	int nHeight = 0;
	int nWidth = 0;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		if (ndx == 0) nWidth = m_lstSearchPhraseEditors.at(ndx)->sizeHint().width();
		nHeight += m_lstSearchPhraseEditors.at(ndx)->sizeHint().height();
	}
	nHeight += m_buttonAddSearchPhrase.height();
	nHeight += m_frameAddCopySeparator.height();
	nHeight += m_buttonCopySummary.height();
	if (nWidth == 0) nWidth = ui.scrollAreaWidgetContents->minimumSize().width();
	ui.scrollAreaWidgetContents->setMinimumSize(nWidth, nHeight);

	m_bDoingResizing = false;
}

void CKJVSearchSpec::en_closingSearchPhrase(CKJVSearchPhraseEdit *pSearchPhrase)
{
	if (m_bCloseAllSearchPhrasesInProgress) return;

	assert(pSearchPhrase != NULL);

	if (pSearchPhrase->phraseEditor() == m_pLastEditorActive) m_pLastEditorActive = NULL;

	bool bPhraseChanged = ((!pSearchPhrase->parsedPhrase()->isDuplicate()) &&
							(!pSearchPhrase->parsedPhrase()->isDisabled()) &&
							(pSearchPhrase->parsedPhrase()->GetNumberOfMatches() != 0) &&
							(pSearchPhrase->parsedPhrase()->isCompleteMatch()));

	int ndx = m_lstSearchPhraseEditors.indexOf(pSearchPhrase);
	assert(ndx != -1);
	if (ndx != -1) {
		m_lstSearchPhraseEditors.removeAt(ndx);
	}
	int ndxActivate = ((ndx < m_lstSearchPhraseEditors.size()) ? ndx : ndx-1);

	resizeScrollAreaLayout();

	if (bPhraseChanged) en_phraseChanged(NULL);

	setFocusSearchPhrase(ndxActivate);
}

void CKJVSearchSpec::en_changedSearchCriteria()
{
	en_phraseChanged(NULL);
}

typedef struct {
	unsigned int m_nNumMatches;					// Num Matches in Whole Bible
	unsigned int m_nNumMatchesWithin;			// Num Matches within Selected Search Text
	unsigned int m_nNumContributingMatches;		// Num Contributing Matches
	bool m_bExclude;							// True if Num Contributing Matches are number of matches removed
} TPhraseOccurrenceInfo;
Q_DECLARE_METATYPE(TPhraseOccurrenceInfo)

QString CKJVSearchSpec::searchPhraseSummaryText() const
{
	int nNumPhrases = 0;
	bool bCaseSensitive = false;
	bool bAccentSensitive = false;
	bool bExclude = false;

	CPhraseList phrases;
	for (int ndx=0; ndx<m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		if ((pPhrase->GetNumberOfMatches()) &&
			(!pPhrase->isDuplicate()) &&
			(!pPhrase->isDisabled())) {
			nNumPhrases++;
			CPhraseEntry entry;
			entry.setFromPhrase(pPhrase);
			TPhraseOccurrenceInfo poiUsage;
			poiUsage.m_nNumMatches = pPhrase->GetNumberOfMatches();
			poiUsage.m_nNumMatchesWithin = pPhrase->GetNumberOfMatchesWithin();
			poiUsage.m_nNumContributingMatches = pPhrase->GetContributingNumberOfMatches();
			poiUsage.m_bExclude = pPhrase->isExcluded();
			entry.setExtraInfo(QVariant::fromValue(poiUsage));
			phrases.append(entry);
			if (entry.caseSensitive()) bCaseSensitive = true;
			if (entry.accentSensitive()) bAccentSensitive = true;
			if (entry.isExcluded()) bExclude = true;
		}
	}

	CPhraseListModel mdlPhrases(phrases);
	mdlPhrases.sort(0);

	QString strScope = ui.widgetSearchCriteria->searchCriteria().searchScopeDescription();
	CSearchCriteria::SEARCH_SCOPE_MODE_ENUM nScope = ui.widgetSearchCriteria->searchCriteria().searchScopeMode();

	QString strSummary;
	QString strSearchWithinDescription = ui.widgetSearchCriteria->searchCriteria().searchWithinDescription(m_pBibleDatabase);
	if (!strSearchWithinDescription.isEmpty()) {
		if (nNumPhrases != 1) {
			strSummary += tr("Search of %n Phrase(s) %1 within %2", "Statistics", nNumPhrases).arg(strScope).arg(strSearchWithinDescription) + QString("\n");
		} else {
			strSummary += tr("Search within %1 of:", "Statistics").arg(strSearchWithinDescription) + " ";
		}
	} else {
		if (nNumPhrases != 1) {
			strSummary += tr("Search of %n Phrase(s) %1", "Statistics", nNumPhrases).arg(strScope) + QString("\n");
		} else {
			strSummary += tr("Search of:", "Statistics") + " ";
		}
	}
	if (nNumPhrases > 1) strSummary += "\n";
	for (int ndx=0; ndx<mdlPhrases.rowCount(); ++ndx) {
		const CPhraseEntry &aPhrase = mdlPhrases.index(ndx).data(CPhraseListModel::PHRASE_ENTRY_ROLE).value<CPhraseEntry>();
		if (nNumPhrases > 1) {
			if (!aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_bExclude) {
				if (!bExclude) {
					if (nScope != CSearchCriteria::SSME_UNSCOPED) {
						strSummary += QString("    \"%1\" ").arg(mdlPhrases.index(ndx).data().toString()) +
										tr("(Found %n Time(s), %1 in Scope)", "Statistics", aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumMatchesWithin)
											.arg(aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumContributingMatches) + "\n";
					} else {
						strSummary += QString("    \"%1\" ").arg(mdlPhrases.index(ndx).data().toString()) +
										tr("(Found %n Time(s))", "Statistics", aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumMatchesWithin) + "\n";
						assert(aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumMatchesWithin == aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumContributingMatches);
					}
				} else {
					strSummary += QString("    \"%1\" ").arg(mdlPhrases.index(ndx).data().toString()) +
									tr("(Found %n Time(s), %1 in Scope and not removed by exclusions)", "Statistics", aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumMatchesWithin)
										.arg(aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumContributingMatches) + "\n";
				}
			} else {
				strSummary += QString("    \"%1\" ").arg(mdlPhrases.index(ndx).data().toString()) +
								"(" +
								tr("Found %n Time(s)", "Statistics", aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumMatchesWithin) +
								", " +
								tr("Removed %n matching exclusion(s) from Scope", "Statistics", aPhrase.extraInfo().value<TPhraseOccurrenceInfo>().m_nNumContributingMatches) +
								")\n";
			}
		} else {
			strSummary += QString("\"%1\"\n").arg(mdlPhrases.index(ndx).data().toString());
		}
	}
	if (bCaseSensitive || bAccentSensitive || bExclude) {
		if (nNumPhrases > 1) strSummary += "\n";
		if (bCaseSensitive) {
			strSummary += "    " + tr("(%1 = Case Sensitive)", "Statistics").arg(CPhraseEntry::encCharCaseSensitive()) + "\n";
		}
		if (bAccentSensitive) {
			strSummary += "    " + tr("(%1 = Accent Sensitive)", "Statistics").arg(CPhraseEntry::encCharAccentSensitive()) + "\n";
		}
		if (bExclude) {
			strSummary += "    " + tr("(%1 = Excluding Results From)", "Statistics").arg(CPhraseEntry::encCharExclude()) + "\n";
		}

	}
	if (nNumPhrases) strSummary += "\n";

	return strSummary;
}

void CKJVSearchSpec::processAllPendingUpdateCompleter()
{
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		CPhraseLineEdit *pPhraseEditor = m_lstSearchPhraseEditors.at(ndx)->phraseEditor();
		assert(pPhraseEditor != NULL);
		if (m_lstSearchPhraseEditors.at(ndx)->parsedPhrase()->isDisabled()) continue;
		// No need to update the active one as it will handle itself:
		if (pPhraseEditor != m_pLastEditorActive)
			pPhraseEditor->processPendingUpdateCompleter();
	}
}

void CKJVSearchSpec::en_phraseChanged(CKJVSearchPhraseEdit *pSearchPhrase)
{
	m_dlySearchResultsUpdate.untrigger();

	Q_UNUSED(pSearchPhrase);

	if (m_bReadingSearchFile) return;

	CBusyCursor iAmBusy(NULL);

	processAllPendingUpdateCompleter();

	TParsedPhrasesList lstPhrases;
	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		const CParsedPhrase *pPhrase = m_lstSearchPhraseEditors.at(ndx)->parsedPhrase();
		assert(pPhrase != NULL);
		if (pPhrase->isDisabled()) continue;
		pPhrase->setIsDuplicate(false);
		pPhrase->ClearWithinPhraseTagSearchResults();
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
			pPhrase->setIsDuplicate(true);
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
	emit changedSearchSpec(ui.widgetSearchCriteria->searchCriteria(), lstPhrases);

	for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
		m_lstSearchPhraseEditors.at(ndx)->phraseStatisticsChanged();
	}
}

void CKJVSearchSpec::en_activatedPhraseEditor(const CPhraseLineEdit *pEditor)
{
	if (pEditor) {
		m_pLastEditorActive = pEditor;
	} else {
		m_pLastEditorActive = NULL;
		m_bDoneActivation = false;
	}
}

void CKJVSearchSpec::en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings)
{
	Q_UNUSED(aSettings);
	if (m_pBibleDatabase->compatibilityUUID().compare(strUUID, Qt::CaseInsensitive) == 0) {
		for (int ndx = 0; ndx < m_lstSearchPhraseEditors.size(); ++ndx) {
			m_lstSearchPhraseEditors.at(ndx)->phraseEditor()->en_textChanged();
		}
	}
}

void CKJVSearchSpec::en_changedHideMatchingPhrasesLists(bool bHideMatchingPhrasesLists)
{
	Q_UNUSED(bHideMatchingPhrasesLists);
	setFocusSearchPhrase(0);
}

bool CKJVSearchSpec::eventFilter(QObject *obj, QEvent *ev)
{
	if ((!m_bDoingResizing) && (obj == ui.scrollAreaSearchPhrases) && (ev->type() == QEvent::FocusIn)) {
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
