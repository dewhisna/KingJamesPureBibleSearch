/****************************************************************************
**
** Copyright (C) 2024 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef ELS_SEARCH_MAIN_WINDOW_H
#define ELS_SEARCH_MAIN_WINDOW_H

#include <QMainWindow>
#include <QPointer>

#include "LetterMatrix.h"

// Forward Declarations
class CLetterMatrixTableModel;
class CELSResultListModel;

// ============================================================================

namespace Ui {
class CELSSearchMainWindow;
}

class CELSSearchMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CELSSearchMainWindow(CBibleDatabasePtr pBibleDatabase,
							bool bSkipColophons, bool bSkipSuperscriptions,
							QWidget *parent = nullptr);
	~CELSSearchMainWindow();

protected slots:
	void en_letterMatrixLayoutChanged();
	void en_searchResultClicked(const QModelIndex &index);
	// ----
	void insertSearchLogText(const QString &strText);
	void clearSearchLogText();
	// ----
	void search();
	void clear();

private:
	CLetterMatrix m_letterMatrix;
	// ----
	QPointer<CLetterMatrixTableModel> m_pLetterMatrixTableModel;
	QPointer<CELSResultListModel> m_pELSResultListModel;
	// ----
	Ui::CELSSearchMainWindow *ui;
};

// ============================================================================

#endif // ELS_SEARCH_MAIN_WINDOW_H