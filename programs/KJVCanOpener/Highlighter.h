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

#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include "dbstruct.h"

#include <QObject>
#include <QTextCharFormat>
#include <QList>
#ifdef QT_WIDGETS_LIB
#include <QKeySequence>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QActionGroup>
#include <QWidgetAction>
#include <QMouseEvent>
#include <QShortcutEvent>
#include <QIcon>
#include <QPointer>
#include <QMenu>
#endif

// ============================================================================

// Forward Declarations:
class i_TVerseListModelResults;									// Nasty intermediate class type defintion for CVerseListModel::TVerseListModelResults, but avoids very nasty header interdependency
class CVerseListModel;
class CVerseListItem;
typedef QMap<CRelIndex, CVerseListItem> CVerseMap;				// Redundant with definition in VerseListModel.h, but avoids very nasty header interdependency

// ============================================================================

class CHighlighterPhraseTagFwdItr
{
protected:
	CHighlighterPhraseTagFwdItr(const i_TVerseListModelResults *pvlmResults);			// Takes ownership of i_TVerseListModelResults object
	CHighlighterPhraseTagFwdItr(const TPhraseTagList &lstTags);

public:
	~CHighlighterPhraseTagFwdItr();

public:
	TPhraseTag nextTag();
	bool isEnd() const;

private:
	const i_TVerseListModelResults *m_pvlmResults;
	const TPhraseTagList &m_lstPhraseTags;
	TPhraseTagList m_lstDummyPhraseTags;				// Dummy list used for m_lstPhraseTags when iterating verses

	CVerseMap::const_iterator m_itrVerses;
	TPhraseTagList::const_iterator m_itrTags;

	friend class CBasicHighlighter;
	friend class CSearchResultHighlighter;
	friend class CCursorFollowHighlighter;
	friend class CUserDefinedHighlighter;
};

// ============================================================================

class CBasicHighlighter : public QObject {
	Q_OBJECT
public:
	explicit CBasicHighlighter(QObject *parent = NULL)
		:	QObject(parent),
			m_bEnabled(true)
	{
	}

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const = 0;
	virtual bool enabled() const { return m_bEnabled; }
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const = 0;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const = 0;
	virtual bool isEmpty() const = 0;

	virtual bool isContinuous() const { return false; }			// Continuous = no word breaks in highlighting.  Default is false

public slots:
	virtual void setEnabled(bool bEnabled = true) { m_bEnabled = bEnabled; }

signals:
	void phraseTagsChanged();
	void charFormatsChanged();

protected:
	bool m_bEnabled;
	Q_DISABLE_COPY(CBasicHighlighter)
};

// ============================================================================

class CSearchResultHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CSearchResultHighlighter(const CVerseListModel *pVerseListModel, bool bExcludedResults = false, QObject *parent = NULL);
	CSearchResultHighlighter(const TPhraseTagList &lstPhraseTags, bool bExcludedResults = false, QObject *parent = NULL);
	CSearchResultHighlighter(const TPhraseTag &aTag, bool bExcludedResults = false, QObject *parent = NULL);
	virtual ~CSearchResultHighlighter();

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const;
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const;
	virtual bool isEmpty() const;

	bool isExcludedResults() const { return m_bExcludedResults; }

private slots:
	void verseListChanged();
	void verseListModelDestroyed();

private:
	const CVerseListModel *m_pVerseListModel;
	bool m_bExcludedResults;						// True if this highlighter is displaying excluded search results, false if displaying normal search results

	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		inline const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		inline void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;
};

// ============================================================================

class CCursorFollowHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CCursorFollowHighlighter(const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	CBasicHighlighter(parent)
	{
		m_myPhraseTags.setPhraseTags(lstPhraseTags);
	}
	CCursorFollowHighlighter(const TPhraseTag &aTag, QObject *parent = NULL)
		:	CBasicHighlighter(parent)
	{
		TPhraseTagList lstTags;
		lstTags.append(aTag);
		m_myPhraseTags.setPhraseTags(lstTags);
	}
	CCursorFollowHighlighter(const CCursorFollowHighlighter &aCursorFollowHighlighter)
		:	CBasicHighlighter(aCursorFollowHighlighter.parent())
	{
		setEnabled(aCursorFollowHighlighter.enabled());
		m_myPhraseTags.setPhraseTags(aCursorFollowHighlighter.m_myPhraseTags.phraseTags());
	}

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const;
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const;
	virtual bool isEmpty() const;

	const TPhraseTagList &phraseTags() const;
	void setPhraseTags(const TPhraseTagList &lstPhraseTags);

public slots:
	void clearPhraseTags();

private:
	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		inline const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		inline void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;
};

// ============================================================================

class CUserDefinedHighlighter : public CBasicHighlighter
{
	Q_OBJECT
public:
	explicit CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstPhraseTags = TPhraseTagList(), QObject *parent = NULL)
		:	CBasicHighlighter(parent),
			m_strUserDefinedHighlighterName(strUserDefinedHighlighterName)
	{
		m_myPhraseTags.setPhraseTags(lstPhraseTags);
	}
	CUserDefinedHighlighter(const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag, QObject *parent = NULL)
		:	CBasicHighlighter(parent),
			m_strUserDefinedHighlighterName(strUserDefinedHighlighterName)
	{
		TPhraseTagList lstTags;
		lstTags.append(aTag);
		m_myPhraseTags.setPhraseTags(lstTags);
	}
	CUserDefinedHighlighter(const CUserDefinedHighlighter &aUserDefinedHighlighter)
		:	CBasicHighlighter(aUserDefinedHighlighter.parent())
	{
		setEnabled(aUserDefinedHighlighter.enabled());
		m_myPhraseTags.setPhraseTags(aUserDefinedHighlighter.m_myPhraseTags.phraseTags());
		m_strUserDefinedHighlighterName = aUserDefinedHighlighter.m_strUserDefinedHighlighterName;
	}

	virtual QTextCharFormat doHighlighting(const QTextCharFormat &aFormat, bool bClear) const;
	virtual bool intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const;

	virtual CHighlighterPhraseTagFwdItr getForwardIterator() const;
	virtual bool isEmpty() const;

	virtual bool isContinuous() const { return true; }

	const TPhraseTagList &phraseTags() const;
	void setPhraseTags(const TPhraseTagList &lstPhraseTags);

public slots:
	void clearPhraseTags();

private:
	// Guard class to keep me from accidentally accessing non-const functions and
	//		causing unintentional copying, as that can be expensive in large searches:
	class CMyPhraseTags {
	public:
		inline const TPhraseTagList &phraseTags() const { return m_lstPhraseTags; }
		inline void setPhraseTags(const TPhraseTagList &lstPhraseTags) { m_lstPhraseTags = lstPhraseTags; }

	private:
		TPhraseTagList m_lstPhraseTags;				// Tags to highlight
	} m_myPhraseTags;

	QString m_strUserDefinedHighlighterName;		// Name of User Defined Highlighter to use
};

// ============================================================================
// ============================================================================

#ifdef QT_WIDGETS_LIB

#define MAX_HIGHLIGHTER_NAME_SIZE 40				// Maximum number of characters in Highlighter Names

class CHighlighterToolButton : public QToolButton
{
public:
	CHighlighterToolButton(QWidget *pParent = NULL)
		:	QToolButton(pParent),
			m_bControlActivation(false)
	{
	}

	virtual ~CHighlighterToolButton()
	{
	}

	virtual void mousePressEvent(QMouseEvent *pEvent)
	{
		assert(pEvent != NULL);
#ifndef Q_OS_MAC
		m_bControlActivation = ((pEvent->modifiers() & Qt::ControlModifier) ? true : false);
#else
		m_bControlActivation = ((pEvent->modifiers() & Qt::MetaModifier) ? true : false);
#endif
		QToolButton::mousePressEvent(pEvent);
	}

	virtual void mouseReleaseEvent(QMouseEvent *pEvent)
	{
		QToolButton::mouseReleaseEvent(pEvent);
	}

	bool controlActivation() const { return m_bControlActivation; }

private:
	bool m_bControlActivation;				// Set to true when the control-shortcut has been activated for this action (i.e. pressing Control or Command while clicking the action)
};

class CHighlighterAction : public QAction
{
	Q_OBJECT

public:
	CHighlighterAction(const QList<QKeySequence> &lstShortcuts, QObject *pParent)
		:	QAction(pParent),
			m_lstShortcuts(lstShortcuts)
	{
		if (!m_lstShortcuts.isEmpty()) setShortcuts(m_lstShortcuts);
	}

	CHighlighterAction(const QList<QKeySequence> &lstShortcuts, const QString &strText, QObject *pParent)
		:	QAction(strText, pParent),
			m_lstShortcuts(lstShortcuts)
	{
		if (!m_lstShortcuts.isEmpty()) setShortcuts(m_lstShortcuts);
	}

	CHighlighterAction(const QList<QKeySequence> &lstShortcuts, const QIcon &anIcon, const QString &strText, QObject *pParent)
		:	QAction(anIcon, strText, pParent),
			m_lstShortcuts(lstShortcuts)
	{
		if (!m_lstShortcuts.isEmpty()) setShortcuts(m_lstShortcuts);
	}

	virtual ~CHighlighterAction()
	{

	}

signals:
	void highlightTriggered(QAction *pAction, bool bSecondaryActive);		// bSecondaryActive if secondary function was selected

protected:
	virtual bool event(QEvent *pEvent)
	{
		if (pEvent->type() == QEvent::Shortcut) {
			QShortcutEvent *pSE = static_cast<QShortcutEvent *>(pEvent);
			assert(m_lstShortcuts.contains(pSE->key()));
			if (pSE->isAmbiguous()) {
				qWarning("QAction::eventFilter: Ambiguous shortcut overload: %s", pSE->key().toString().toLatin1().constData());
			} else {
				emit highlightTriggered(this, m_lstShortcuts.indexOf(pSE->key()) == 1);
			}
			return true;
		}
		return QAction::event(pEvent);
	}

private:
	const QList<QKeySequence> m_lstShortcuts;
};

class CHighlighterWidgetAction : public QWidgetAction
{
public:
	CHighlighterWidgetAction(CHighlighterAction *pButtonAction, QObject *pParent = 0)
		:	QWidgetAction(pParent),
			m_pButtonAction(pButtonAction),
			m_pHighlighterToolButton(NULL)
	{
		setMenu(new QMenu);					// The action will take ownership via setOverrideMenuAction()
	}
	virtual ~CHighlighterWidgetAction()
	{ }

	virtual QWidget *createWidget(QWidget *parent)
	{
		m_pHighlighterToolButton = new CHighlighterToolButton(parent);
		m_pHighlighterToolButton->setDefaultAction(m_pButtonAction);
		m_pHighlighterToolButton->setMenu(this->menu());
		m_pHighlighterToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
		m_pHighlighterToolButton->setPopupMode(QToolButton::MenuButtonPopup);
		return m_pHighlighterToolButton;
	}

	bool controlActivation() const
	{
		assert(m_pHighlighterToolButton != NULL);
		return m_pHighlighterToolButton->controlActivation();
	}

	CHighlighterAction *buttonAction() const { return m_pButtonAction; }

private:
	CHighlighterAction *m_pButtonAction;
	CHighlighterToolButton *m_pHighlighterToolButton;
};

class CHighlighterButtons : public QObject
{
	Q_OBJECT

public:
	CHighlighterButtons(QObject *pParent);
	virtual ~CHighlighterButtons();

	void addHighlighterButtonsToToolBar(QToolBar *pToolBar);

	int count() const { return m_lstButtons.size(); }
	QList<CHighlighterWidgetAction *> widgetActions() const
	{
		QList<CHighlighterWidgetAction *> lstWidgetActions;
		for (int ndx = 0; ndx < m_lstButtons.size(); ++ndx) {
			if (!m_lstButtons.at(ndx).isNull())
				lstWidgetActions.append(m_lstButtons.at(ndx));
		}
		return lstWidgetActions;
	}
	QList<QAction *> actions() const
	{
		return m_pActionGroupHighlighterTools->actions();
	}

	QString highlighter(int ndx) const;
	void setHighlighterLists();
	void setHighlighterList(int ndx, const QString &strUserDefinedHighlighterName = QString());

	void setHighlighterTips(bool bSearchResultsActive);

signals:
	void highlighterToolTriggered(int ndxHighlighterTool, bool bSecondaryActive);	// Triggered whenever one of the Highlighter Tools (or menu equivalents) is clicked

public slots:
	void enterConfigurationMode();
	void leaveConfigurationMode();

protected slots:
	void en_changedHighlighters();
	void en_highlighterSelectionChanged(QAction *pAction);
	void en_highlighterToolTriggered(QAction *pAction, bool bSecondary = false);			// Internal trigger of Highlighter Tool -- reads control status and fires highlighterToolTriggered()

protected:
	void setHighlighterPreview(int ndx, const QString &strUserDefinedHighlighterName);
	QIcon iconHighlighterPreview(const QString &strUserDefinedHighlighterName);

private:
	typedef QPointer<CHighlighterWidgetAction> TToolButtonPtr;
	QList<TToolButtonPtr> m_lstButtons;					// List of highlighter buttons
	QActionGroup *m_pActionGroupHighlighterTools;		// Group of highlighter tool actions
	QList<QActionGroup *> m_lstActionGroups;			// Groups of "actions" that is the list of available highlighters in each button
};

#endif

// ============================================================================

#endif // HIGHLIGHTER_H
