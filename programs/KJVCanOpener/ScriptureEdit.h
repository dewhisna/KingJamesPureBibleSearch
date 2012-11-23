#ifndef SCRIPTUREEDIT_H
#define SCRIPTUREEDIT_H

#include "PhraseEdit.h"
#include "Highlighter.h"

#include <QTextEdit>
#include <QTimer>

// ============================================================================

class CScriptureEdit : public QTextEdit
{
	Q_OBJECT

public:
	explicit CScriptureEdit(QWidget *parent = 0);
	virtual ~CScriptureEdit();

protected:
	virtual bool event(QEvent *e);
	virtual bool eventFilter(QObject *obj, QEvent *ev);

private slots:
	void clearHighlighting();

private:
	CPhraseEditNavigator m_navigator;
	CCursorFollowHighlighter m_Highlighter;
	QTimer m_HighlightTimer;
};

// ============================================================================

#endif // SCRIPTUREEDIT_H
