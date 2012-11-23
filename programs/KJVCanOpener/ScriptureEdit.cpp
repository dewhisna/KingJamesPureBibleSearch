#include "ScriptureEdit.h"

#include "dbstruct.h"

#include <assert.h>

#include <QString>
#include <QEvent>
#include <QHelpEvent>

// ============================================================================

CScriptureEdit::CScriptureEdit(QWidget *parent)
	:	QTextEdit(parent),
		m_navigator(*this)
{
	setMouseTracking(true);
	installEventFilter(this);

	m_HighlightTimer.stop();

	connect(&m_navigator, SIGNAL(changedDocumentText()), &m_Highlighter, SLOT(clearPhraseTags()));
	connect(&m_HighlightTimer, SIGNAL(timeout()), this, SLOT(clearHighlighting()));
}

CScriptureEdit::~CScriptureEdit()
{

}

void CScriptureEdit::clearHighlighting()
{
	m_navigator.doHighlighting(m_Highlighter, true);
	m_Highlighter.clearPhraseTags();
	m_HighlightTimer.stop();
}

bool CScriptureEdit::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == this) {
		switch (ev->type()) {
			case QEvent::Wheel:
			case QEvent::ActivationChange:
			case QEvent::KeyPress:
			case QEvent::KeyRelease:
			case QEvent::FocusOut:
			case QEvent::FocusIn:
			case QEvent::MouseButtonPress:
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonDblClick:
			case QEvent::Leave:
				return false;
			default:
				break;
		}
	}

	return QTextEdit::eventFilter(obj, ev);
}

bool CScriptureEdit::event(QEvent *e)
{
	switch (e->type()) {
		case QEvent::ToolTip:
			{
				QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(e);
				if (m_navigator.handleToolTipEvent(pHelpEvent, m_Highlighter)) {
					m_HighlightTimer.stop();
				} else {
					pHelpEvent->ignore();
				}
				return true;
			}
			break;

		// User input and window activation makes tooltips sleep
		case QEvent::Wheel:
		case QEvent::ActivationChange:
		case QEvent::KeyPress:
		case QEvent::KeyRelease:
		case QEvent::FocusOut:
		case QEvent::FocusIn:
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
			// Unfortunately, there doesn't seem to be any event we can hook to to determine
			//		when the ToolTip disappears.  Looking at the Qt code, it looks to be on
			//		a 2 second timeout.  So, we'll do a similar timeout here for the highlight:
			if ((!m_Highlighter.getHighlightTags().isEmpty()) && (!m_HighlightTimer.isActive()))
				m_HighlightTimer.start(2000);
			break;
		case QEvent::Leave:
			if (!m_Highlighter.getHighlightTags().isEmpty()) {
				m_HighlightTimer.start(20);
			}
			break;
		default:
			break;
	}

	return QTextEdit::event(e);
}

// ============================================================================


