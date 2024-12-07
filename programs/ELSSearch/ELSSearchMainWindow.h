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
#include <QScopedPointer>
#include <QFile>
#include <QTableView>

#include "LetterMatrix.h"

// Forward Declarations
class CLetterMatrixTableModel;
class CELSResultListModel;
class QAction;
class QMenu;
class QEvent;
class QPaintEvent;
class QtIOCompressor;
class CCSVStream;

// ============================================================================

class CLetterMatrixTableView : public QTableView
{
	Q_OBJECT
public:
	CLetterMatrixTableView(QWidget *pParent = nullptr);

	virtual void scrollContentsBy(int dx, int dy) override;

private:
	class CELSSearchMainWindow *m_pMainWindow = nullptr;
};

// ============================================================================

// Special widget for drawing lines in the LetterMatrix for ELSResults:
class CLetterMatrixLineWidget : public QWidget
{
	Q_OBJECT
public:
	CLetterMatrixLineWidget(CLetterMatrixTableModel *pModel, QTableView *pView, QWidget *pParent = nullptr)
		:	QWidget(pParent),
		m_pModel(pModel),
		m_pView(pView)
	{ }

	virtual void paintEvent(QPaintEvent *event) override;

private:
	CLetterMatrixTableModel *m_pModel = nullptr;
	QTableView *m_pView = nullptr;
};

// ============================================================================

namespace Ui {
class CELSSearchMainWindow;
}

class CELSSearchMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit CELSSearchMainWindow(CBibleDatabasePtr pBibleDatabase,
								  LetterMatrixTextModifierOptionFlags flagsLMTMO,
								  QWidget *parent = nullptr);
	~CELSSearchMainWindow();

	CBibleDatabasePtr bibleDatabase() const { return m_letterMatrix.bibleDatabase(); }

protected:
	virtual bool eventFilter(QObject *obj, QEvent *ev) override;
	// ----
	QMenu *createELSResultsContextMenu();
	QMenu *createLetterMatrixContextMenu();

protected slots:
#ifdef USING_ELSSEARCH
	void newELSSearchWindow();
#endif

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	void en_openSearchTranscript(const QString &strFilePath = QString());
	void en_createSearchTranscript();
	// ----
	void closeSearchTranscript();
#endif
	// ----
	void en_searchResultClicked(const QModelIndex &index);
	void en_changedSortOrder(int nIndex);
	// ----
	void en_changedSearchType(int nIndex);
	// ----
	void en_letterMatrixLayoutAboutToChange();
	void en_widthChanged(int nWidth);					// Width from ItemModel
	void en_offsetChanged(int nOffset);
	void en_widthSpinValueChanged(int nWidth);			// SpinBox value changed
	// ----
	void en_letterMatrixCellClicked(const QModelIndex &index);
	void en_letterMatrixCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
	// ----
	void insertSearchLogText(const QString &strText);
	void clearSearchLogText();
	// ----
	bool search();				// Returns False on cancel or failed
	void clear();
	// ----
	void en_copySearchResults();
	void en_copyLetterMatrix();

private:
	friend class CLetterMatrixTableView;

#if !defined(EMSCRIPTEN) && !defined(VNCSERVER)
	static QString g_strLastELSFilePath;
	// ----
	QFile m_fileSearchTranscript;
	QScopedPointer<QtIOCompressor> m_pSearchTranscriptCompressor;		// Stream Compressor for ELS Search Transcript
	QScopedPointer<CCSVStream> m_pSearchTranscriptCSVStream;			// CSV Stream for Search Transcript
	// ----
	bool m_bRecordingTranscript = false;
	// ----
	QPointer<QAction> m_pLoadTranscriptionAction;
	QPointer<QAction> m_pCreateTranscriptionAction;
	QPointer<QAction> m_pCloseTranscriptionAction;
#endif
	// ----
	CLetterMatrix m_letterMatrix;
	// ----
	QPointer<CLetterMatrixTableModel> m_pLetterMatrixTableModel;
	QPointer<CELSResultListModel> m_pELSResultListModel;
	// ----
	uint32_t m_nMatrixIndexToCenter = 0;		// Matrix Index to Center during layout change
	// ----
	QPointer<QAction> m_pStatusAction;			// Used to update the status bar without an enter/leave sequence
	// ----
	CLetterMatrixLineWidget *m_pLetterMatrixLineWidget = nullptr;		// Widget to draw lines in the LetterMatrix for ELSResults
	Ui::CELSSearchMainWindow *ui;
};

// ============================================================================

#endif // ELS_SEARCH_MAIN_WINDOW_H
