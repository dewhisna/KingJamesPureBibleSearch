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

}

CScriptureEdit::~CScriptureEdit()
{

}

bool CScriptureEdit::event(QEvent *e)
{
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *pHelpEvent = static_cast<QHelpEvent*>(e);
		if (!m_navigator.handleToolTipEvent(pHelpEvent)) pHelpEvent->ignore();
		return true;
	}

	return QTextEdit::event(e);
}

// ============================================================================


