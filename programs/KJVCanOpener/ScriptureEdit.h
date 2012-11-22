#ifndef SCRIPTUREEDIT_H
#define SCRIPTUREEDIT_H

#include "PhraseEdit.h"

#include <QTextEdit>

// ============================================================================

class CScriptureEdit : public QTextEdit
{
	Q_OBJECT

public:
	explicit CScriptureEdit(QWidget *parent = 0);
	virtual ~CScriptureEdit();

protected:
	virtual bool event(QEvent *e);

private:
	CPhraseNavigator m_navigator;
};

// ============================================================================

#endif // SCRIPTUREEDIT_H
