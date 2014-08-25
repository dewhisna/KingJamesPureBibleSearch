/****************************************************************************
**
** Copyright (C) 2014 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef THREADED_SEARCH_RESULTS_H
#define THREADED_SEARCH_RESULTS_H

#include "dbstruct.h"
#include "VerseListModel.h"

#include <QThread>
#include <QObject>
#include <QList>

// ============================================================================

#ifdef USE_MULTITHREADED_SEARCH_RESULTS

class CThreadedSearchResultWorker : public QObject
{
	Q_OBJECT
	QThread workerThreadedSearchResult;

public slots:
	void doWork(CSearchResultsProcess *theSearchResultsProcess)
	{
		theSearchResultsProcess->buildScopedResultsFromParsedPhrases();
		emit resultsReady();
	}

signals:
	void resultsReady();
};

class CThreadedSearchResultCtrl : public QObject
{
	Q_OBJECT
	QThread workerThreadedSearchResult;

public:
	CThreadedSearchResultCtrl(CBibleDatabasePtr pBibleDatabase, const CSearchResultsData &theData, QObject *pParent = NULL)
		:	QObject(pParent),
			m_bActive(true),
			m_searchResultsProcess(pBibleDatabase, theData)
	{
		CThreadedSearchResultWorker *pWorker = new CThreadedSearchResultWorker;
		pWorker->moveToThread(&workerThreadedSearchResult);
		connect(&workerThreadedSearchResult, SIGNAL(finished()), pWorker, SLOT(deleteLater()));
		connect(this, SIGNAL(internalStartWorking(CSearchResultsProcess *)), pWorker, SLOT(doWork(CSearchResultsProcess *)));
		connect(pWorker, SIGNAL(resultsReady()), this, SLOT(en_resultsReady()));
		workerThreadedSearchResult.start();
	}

	~CThreadedSearchResultCtrl()
	{
		workerThreadedSearchResult.quit();
		workerThreadedSearchResult.wait();
	}

	void startWorking()
	{
		emit internalStartWorking(&m_searchResultsProcess);
	}

	void deactivate()
	{
		m_bActive = false;
	}

	bool isActive() const { return m_bActive; }
	const CSearchResultsProcess *searchResultsProcess() const { return &m_searchResultsProcess; }

signals:
	void internalStartWorking(CSearchResultsProcess *theSearchResultsProcess);
	void resultsReady(const CThreadedSearchResultCtrl *theThreadedSearchResult);

protected slots:
	void en_resultsReady()
	{
		if (m_bActive) {
			emit resultsReady(this);
		}
		deleteLater();
	}

private:
	bool m_bActive;									// Set to True if the results of this thread is to be used to drive results.  Set to False if this result has been superceded by later results and we need to ignore this
	CSearchResultsProcess m_searchResultsProcess;	// Search results data processing
};

#endif	// USE_MULTITHREADED_SEARCH_RESULTS

// ============================================================================

#endif	// THREADED_SEARCH_RESULTS_H

