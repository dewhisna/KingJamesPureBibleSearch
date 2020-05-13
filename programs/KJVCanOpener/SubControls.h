/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SUB_CONTROLS_H
#define SUB_CONTROLS_H

#include "dbstruct.h"
#include "SearchCompleter.h"

#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QKeyEvent>
#include <QTextEdit>
#include <QWheelEvent>
#include <QFocusEvent>
#include <QInputMethodEvent>
#include <QSize>
#include <QMimeData>

// ============================================================================

class CComboBox : public QComboBox
{
	Q_OBJECT

public:
	CComboBox(QWidget *pParent = nullptr);
	virtual ~CComboBox();

	virtual void keyPressEvent(QKeyEvent *pEvent);

signals:
	void enterPressed();
};

// ============================================================================

class CFontComboBox : public QFontComboBox
{
	Q_OBJECT

public:
	CFontComboBox(QWidget *pParent = nullptr);
	virtual ~CFontComboBox();

	virtual void keyPressEvent(QKeyEvent *pEvent);

signals:
	void enterPressed();
};

// ============================================================================

class CSpinBox : public QSpinBox
{
	Q_OBJECT

public:
	CSpinBox(QWidget *pParent = nullptr);
	virtual ~CSpinBox();

	virtual void keyPressEvent(QKeyEvent *pEvent);

signals:
	void enterPressed();
};

// ============================================================================

class CDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT

public:
	CDoubleSpinBox(QWidget *pParent = nullptr);
	virtual ~CDoubleSpinBox();

	virtual void keyPressEvent(QKeyEvent *pEvent);

signals:
	void enterPressed();
};

// ============================================================================

class CSingleLineTextEdit : public QTextEdit
{
	Q_OBJECT

public:
	CSingleLineTextEdit(int nMinHeight = -1, QWidget *pParent = nullptr);
	virtual ~CSingleLineTextEdit();

	virtual QSize sizeHint() const;

protected:
	virtual void insertFromMimeData(const QMimeData * source);
	virtual bool canInsertFromMimeData(const QMimeData *source) const;

	bool updateInProgress() const { return m_bUpdateInProgress; }
	class CDoUpdate {
	public:
		CDoUpdate(CSingleLineTextEdit *pLineEdit)
			:	m_pLineEdit(pLineEdit)
		{
			assert(m_pLineEdit != nullptr);
			m_bUpdateSave = m_pLineEdit->m_bUpdateInProgress;
			m_pLineEdit->m_bUpdateInProgress = true;
		}

		~CDoUpdate()
		{
			m_pLineEdit->m_bUpdateInProgress = m_bUpdateSave;
		}

	private:
		bool m_bUpdateSave;
		CSingleLineTextEdit *m_pLineEdit;
	};
	friend class CDoUpdate;

signals:
	void enterTriggered();						// Signaled when the user presses enter/return

protected slots:
	virtual void en_textChanged();
	virtual void en_cursorPositionChanged();

protected:
	virtual void wheelEvent(QWheelEvent *event);
	virtual void focusInEvent(QFocusEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void inputMethodEvent(QInputMethodEvent *event);

	virtual QString textUnderCursor() const;

	virtual void setupCompleter(const QString &strText, bool bForce = false) = 0;
	virtual void UpdateCompleter() = 0;

private:
	int m_nMinHeight;
	bool m_bUpdateInProgress;	// Completer/Case-Sensivitity update in progress (to guard against re-entrance)
};

// ============================================================================

#endif	// SUB_CONTROLS_H
