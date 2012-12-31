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
#include <QMimeData>

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
		return (m_selectedPhrase.second.haveSelection());
	}
	TPhraseTag selection() const {
		if (!m_selectedPhrase.second.haveSelection()) return TPhraseTag(m_tagLast.first, 0);
		return (m_selectedPhrase.second);
	}

//signals:
//	void gotoIndex(const TPhraseTag &tag);
//	void activatedScriptureText();
//	void copyRawAvailable(bool bAvailable);

protected:
	virtual bool event(QEvent *ev);
	virtual bool eventFilter(QObject *obj, QEvent *ev);
	virtual void mouseDoubleClickEvent(QMouseEvent *ev);
	virtual void contextMenuEvent(QContextMenuEvent *ev);
	virtual QMimeData *createMimeDataFromSelection () const;

protected:
	virtual void updateSelection();
	virtual void copyVersesCommon(bool bPlainOnly);

//private slots:
protected:
	virtual void on_cursorPositionChanged();
	virtual void on_selectionChanged();
	virtual void clearHighlighting();

//public slots:
public:
	virtual void on_passageNavigator();
	virtual void on_copy();
	virtual void on_copyPlain();
	virtual void on_copyRaw();
	virtual void on_copyVeryRaw();
	virtual void on_copyVerses();
	virtual void on_copyVersesPlain();
	virtual void on_copyReferenceDetails();
	virtual void on_copyPassageStatistics();
	virtual void on_copyEntirePassageDetails();

private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog and we don't want the highlight to disable
	CPhraseEditNavigator m_navigator;
	CCursorFollowHighlighter m_Highlighter;
	QTimer m_HighlightTimer;
	TPhraseTag m_tagLast;			// Last mouse/keyboard reference tag for tool tips, etc (used for copying, etc)
	QPair<CParsedPhrase, TPhraseTag> m_selectedPhrase;		// Selected phrase and cursor selection reference
	bool m_bDoPlainCopyOnly;		// Flag for the createMimeDataFromSelection function to use only plain text

	QMenu *m_pEditMenu;				// Edit menu for main screen when this editor is active
	// ----
	QAction *m_pActionCopy;			// Edit menu copy (text as shown)
	QAction *m_pActionCopyPlain;	// Edit menu copy, as shown, but plaintext
	QAction *m_pActionCopyRaw;		// Edit menu copy raw phrase text
	QAction *m_pActionCopyVeryRaw;	// Edit menu copy very (no punctuation) raw phrase text
	// ----
	QAction *m_pActionCopyVerses;	// Edit menu copy as formatted verses
	QAction *m_pActionCopyVersesPlain;	// Edit menu copy as formatted verses, but plaintext
	// ----
	QAction *m_pActionCopyReferenceDetails;			// Reference ToolTip Copy
	QAction *m_pActionCopyPassageStatistics;		// Statistics ToolTip Copy
	QAction *m_pActionCopyEntirePassageDetails;		// Entire ToolTip Copy
	// ----
	QAction *m_pActionSelectAll;	// Edit menu select all
	// ----
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

	static bool useToolTipEdit() { return false; }

signals:
	void gotoIndex(const TPhraseTag &tag);
	void activatedScriptureText();
	void copyRawAvailable(bool bAvailable);

protected slots:
	virtual void on_cursorPositionChanged() = 0;
	virtual void on_selectionChanged() = 0;
	virtual void clearHighlighting() = 0;

public slots:
//	virtual void on_passageNavigator() = 0;			-- Don't implement this because we don't want the navigator launching the navigator
	virtual void on_copy() = 0;
	virtual void on_copyPlain() = 0;
	virtual void on_copyRaw() = 0;
	virtual void on_copyVeryRaw() = 0;
	virtual void on_copyVerses() = 0;
	virtual void on_copyVersesPlain() = 0;
	virtual void on_copyReferenceDetails() = 0;
	virtual void on_copyPassageStatistics() = 0;
	virtual void on_copyEntirePassageDetails() = 0;
};

class i_CScriptureBrowser : public QTextBrowser
{
	Q_OBJECT
public:
	explicit i_CScriptureBrowser(QWidget *parent = 0)
		:	QTextBrowser(parent)
	{ }

	static bool useToolTipEdit() { return true; }

signals:
	void gotoIndex(const TPhraseTag &tag);
	void activatedScriptureText();
	void copyRawAvailable(bool bAvailable);

protected slots:
	virtual void on_cursorPositionChanged() = 0;
	virtual void on_selectionChanged() = 0;
	virtual void clearHighlighting() = 0;

public slots:
	virtual void on_passageNavigator() = 0;
	virtual void on_copy() = 0;
	virtual void on_copyPlain() = 0;
	virtual void on_copyRaw() = 0;
	virtual void on_copyVeryRaw() = 0;
	virtual void on_copyVerses() = 0;
	virtual void on_copyVersesPlain() = 0;
	virtual void on_copyReferenceDetails() = 0;
	virtual void on_copyPassageStatistics() = 0;
	virtual void on_copyEntirePassageDetails() = 0;
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
