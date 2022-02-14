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

#ifndef SEARCH_CRITERIA_WIDGET_H
#define SEARCH_CRITERIA_WIDGET_H

#include "dbstruct.h"

#include "SearchCriteria.h"

#include <QString>
#include <QStringList>
#include <QObject>
#include <QModelIndex>
#include <QList>
#include <QMap>

#include <QWidget>

// ============================================================================

// Forward Declarations:
class CSearchWithinModel;

// ============================================================================

#include "ui_SearchCriteriaWidget.h"

class CSearchCriteriaWidget : public QWidget
{
	Q_OBJECT

public:
	explicit CSearchCriteriaWidget(QWidget *parent = nullptr);
	~CSearchCriteriaWidget();

	void initialize(CBibleDatabasePtr pBibleDatabase);

	const CSearchCriteria &searchCriteria() const { return m_SearchCriteria; }

signals:
	void changedSearchCriteria();
	void gotoIndex(const CRelIndex &relIndex);

public slots:
	void setSearchScopeMode(CSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);

	void setSearchWithin(const TRelativeIndexSet &aSetSearchWithin);
	void setSearchWithin(const QString &strSearchWithin);

	void setTextBrightness(bool bInvert, int nBrightness);
	void setAdjustDialogElementBrightness(bool bAdjust);

private slots:
	void en_changedSearchScopeMode(int ndx);
	void en_changedSearchWithin();
	void en_SearchWithinItemActivated(const QModelIndex &index);		// Triggered on Activate or DoubleClick to handle enter or double-click of search-within item (emits gotoIndex() signal)

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	CSearchCriteria m_SearchCriteria;
	CSearchWithinModel *m_pSearchWithinModel;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			CBusyCursor iAmBusy(nullptr);		\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;

	Ui::CSearchCriteriaWidget ui;
};

// ============================================================================

#endif	// SEARCH_CRITERIA_WIDGET_H

