#ifndef SCRIPTUREEDIT_H
#define SCRIPTUREEDIT_H

#include "dbstruct.h"
#include "Highlighter.h"
#include "PhraseEdit.h"

#include <QWidget>
#include <QTextBrowser>
#include <QTextEdit>
#include <QTimer>
#include <QMenu>
#include <QContextMenuEvent>

// ============================================================================

//
// CScriptureText - Base template class functionality for CScriptureEdit and CScriptureBrowser
//
//
//	It really bugged me that CScriptureEdit and CScriptureBrowser were absolutely
//	identical except that one inherited from QTextEdit and the other QTextBrowser.
//	But Q_OBJECT doesn't work in templates, so I couldn't just make it a template
//	and inherit CScriptureText and CScriptureBrowser from that.  That would have
//	been too easy.
//
//	So, I turned the problem on its head and derived intermediate classes
//	(i_CScriptureEdit and i_CScriptureBrowser) inherited from QTextEdit and
//	QTextBrowser that do nothing but the Qt Signal/Slot mechanism with Q_OBJECT.
//	Then, inherit the final CScriptureEdit and CScriptureBrowser classes from this
//	CScriptureText template that gives the actual final desired functionality.
//	All because Q_OBJECT can't exist in templated classes!!
//

template <class T, class U>
class CScriptureText : public T
{
public:
	explicit CScriptureText(QWidget *parent = 0);
	virtual ~CScriptureText();

	CPhraseEditNavigator &navigator()
	{
		return m_navigator;
	}

	QMenu *getEditMenu() { return m_pEditMenu; }

	bool haveSelection() const {
		return ((m_tagSelection.first.isSet()) && (m_tagSelection.second != 0));
	}

//signals:
//	void gotoIndex(const TPhraseTag &tag);
//	void activatedScriptureText();

protected:
	virtual bool event(QEvent *ev);
	virtual bool eventFilter(QObject *obj, QEvent *ev);
	virtual void mouseDoubleClickEvent(QMouseEvent *ev);
	virtual void contextMenuEvent(QContextMenuEvent *ev);

//private slots:
protected:
	virtual void on_cursorPositionChanged();
	virtual void on_selectionChanged();
	virtual void clearHighlighting();
	virtual void on_copyReferenceDetails();
	virtual void on_copyPassageStatistics();
	virtual void on_copyEntirePassageDetails();

//public slots:
public:
	virtual void on_passageNavigator();

private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog and we don't want the highlight to disable
	CPhraseEditNavigator m_navigator;
	CCursorFollowHighlighter m_Highlighter;
	QTimer m_HighlightTimer;
	TPhraseTag m_tagLast;			// Last mouse/keyboard reference tag for tool tips, etc (used for copying, etc)
	TPhraseTag m_tagSelection;		// Current cursor selection reference for tool tips, etc (used for copying, etc)

	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	QAction *m_pActionCopy;			// Edit menu copy
	QAction *m_pActionSelectAll;	// Edit menu select all
	QAction *m_pActionCopyReferenceDetails;			// Reference ToolTip Copy
	QAction *m_pActionCopyPassageStatistics;		// Statistics ToolTip Copy
	QAction *m_pActionCopyEntirePassageDetails;		// Entire ToolTip Copy
	QAction *m_pStatusAction;		// Used to update the status bar without an enter/leave sequence

#define begin_popup()							\
			bool bPopupSave = m_bDoingPopup;	\
			m_bDoingPopup = true;
#define end_popup()								\
			m_bDoingPopup = bPopupSave;

};


// ============================================================================

// Intermediate classes to marshall signals/slots:

class i_CScriptureEdit : public QTextEdit
{
	Q_OBJECT
public:
	explicit i_CScriptureEdit(QWidget *parent = 0)
		:	QTextEdit(parent)
	{ }

signals:
	void gotoIndex(const TPhraseTag &tag);
	void activatedScriptureText();

protected slots:
	virtual void on_cursorPositionChanged() = 0;
	virtual void on_selectionChanged() = 0;
	virtual void clearHighlighting() = 0;
	virtual void on_copyReferenceDetails() = 0;
	virtual void on_copyPassageStatistics() = 0;
	virtual void on_copyEntirePassageDetails() = 0;

public slots:
//	virtual void on_passageNavigator() = 0;			-- Don't implement this because we don't want the navigator launching the navigator
};

class i_CScriptureBrowser : public QTextBrowser
{
	Q_OBJECT
public:
	explicit i_CScriptureBrowser(QWidget *parent = 0)
		:	QTextBrowser(parent)
	{ }

signals:
	void gotoIndex(const TPhraseTag &tag);
	void activatedScriptureText();

protected slots:
	virtual void on_cursorPositionChanged() = 0;
	virtual void on_selectionChanged() = 0;
	virtual void clearHighlighting() = 0;
	virtual void on_copyReferenceDetails() = 0;
	virtual void on_copyPassageStatistics() = 0;
	virtual void on_copyEntirePassageDetails() = 0;

public slots:
	virtual void on_passageNavigator() = 0;
};

// ============================================================================

// Real Exported Classes to serve as our subclassed controls:

class CScriptureEdit : public CScriptureText<i_CScriptureEdit, QTextEdit>
{
public:
	explicit CScriptureEdit(QWidget *parent = 0)
		:	CScriptureText<i_CScriptureEdit, QTextEdit>(parent)
	{ }
};

class CScriptureBrowser : public CScriptureText<i_CScriptureBrowser, QTextBrowser>
{
public:
	explicit CScriptureBrowser(QWidget *parent = 0)
		:	CScriptureText<i_CScriptureBrowser, QTextBrowser>(parent)
	{ }
};

// ============================================================================

#endif // SCRIPTUREEDIT_H
