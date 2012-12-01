#ifndef SCRIPTUREEDIT_H
#define SCRIPTUREEDIT_H

#include "dbstruct.h"
#include "Highlighter.h"
#include "PhraseEdit.h"

#include <QWidget>
#include <QTextEdit>
#include <QTimer>
#include <QMenu>
#include <QContextMenuEvent>

// ============================================================================

class CScriptureEdit : public QTextEdit
{
	Q_OBJECT

public:
	explicit CScriptureEdit(QWidget *parent = 0);
	virtual ~CScriptureEdit();

	CPhraseEditNavigator &navigator()
	{
		return m_navigator;
	}

	QMenu *getEditMenu() { return m_pEditMenu; }

signals:
	void gotoIndex(TPhraseTag tag);
	void activatedScriptureEdit();

protected:
	virtual bool event(QEvent *ev);
	virtual bool eventFilter(QObject *obj, QEvent *ev);
	virtual void mouseDoubleClickEvent(QMouseEvent *ev);
	virtual void contextMenuEvent(QContextMenuEvent *ev);

private slots:
	void on_cursorPositionChanged();
	void clearHighlighting();
	void on_copyReferenceDetails();
	void on_copyPassageStatistics();
	void on_copyEntirePassageDetails();

private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog and we don't want the highlight to disable
	CPhraseEditNavigator m_navigator;
	CCursorFollowHighlighter m_Highlighter;
	QTimer m_HighlightTimer;
	TPhraseTag m_tagLast;			// Last reference tag for tool tips, etc (used for copying, etc)

	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QAction *m_pActionCopy;			// Edit menu copy
	QAction *m_pActionSelectAll;	// Edit menu select all
	QAction *m_pActionCopyReferenceDetails;			// Reference ToolTip Copy
	QAction *m_pActionCopyPassageStatistics;		// Statistics ToolTip Copy
	QAction *m_pActionCopyEntirePassageDetails;		// Entire ToolTip Copy
};

// ============================================================================

#endif // SCRIPTUREEDIT_H
