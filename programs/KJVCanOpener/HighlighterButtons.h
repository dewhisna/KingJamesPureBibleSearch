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

#ifndef HIGHLIGHTER_BUTTONS_H
#define HIGHLIGHTER_BUTTONS_H

#include <QKeySequence>
#include <QToolButton>
#include <QAction>
#include <QActionGroup>
#include <QWidgetAction>
#include <QMouseEvent>
#include <QShortcutEvent>
#include <QIcon>
#include <QPointer>
#include <QMenu>

// Forward Declarations:
class QToolBar;

// ============================================================================

class CHighlighterToolButton : public QToolButton
{
public:
	CHighlighterToolButton(QWidget *pParent = nullptr)
		:	QToolButton(pParent),
			m_bControlActivation(false)
	{
	}

	virtual ~CHighlighterToolButton()
	{
	}

	virtual void mousePressEvent(QMouseEvent *pEvent) override
	{
		Q_ASSERT(pEvent != nullptr);
#ifndef Q_OS_MAC
		m_bControlActivation = ((pEvent->modifiers() & Qt::ControlModifier) ? true : false);
#else
		m_bControlActivation = ((pEvent->modifiers() & Qt::MetaModifier) ? true : false);
#endif
		QToolButton::mousePressEvent(pEvent);
	}

	virtual void mouseReleaseEvent(QMouseEvent *pEvent) override
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
	virtual bool event(QEvent *pEvent) override
	{
		if (pEvent->type() == QEvent::Shortcut) {
			QShortcutEvent *pSE = static_cast<QShortcutEvent *>(pEvent);
			Q_ASSERT(m_lstShortcuts.contains(pSE->key()));
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
	CHighlighterWidgetAction(CHighlighterAction *pButtonAction, QObject *pParent = nullptr)
		:	QWidgetAction(pParent),
			m_pButtonAction(pButtonAction),
			m_pHighlighterToolButton(nullptr)
	{
		setMenu(new QMenu);					// The action will take ownership via setOverrideMenuAction()
	}
	virtual ~CHighlighterWidgetAction()
	{ }

	virtual QWidget *createWidget(QWidget *parent) override
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
		Q_ASSERT(m_pHighlighterToolButton != nullptr);
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

// ============================================================================

#endif	// HIGHLIGHTER_BUTTONS_H
