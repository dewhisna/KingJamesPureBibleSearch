/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#include "SubControls.h"

// ============================================================================

CComboBox::CComboBox(QWidget *pParent)
	:	QComboBox(pParent)
{

}

CComboBox::~CComboBox()
{

}

void CComboBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QComboBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CFontComboBox::CFontComboBox(QWidget *pParent)
	:	QFontComboBox(pParent)
{

}

CFontComboBox::~CFontComboBox()
{

}

void CFontComboBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QFontComboBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CSpinBox::CSpinBox(QWidget *pParent)
	:	QSpinBox(pParent)
{

}

CSpinBox::~CSpinBox()
{

}

void CSpinBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QSpinBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================

CDoubleSpinBox::CDoubleSpinBox(QWidget *pParent)
	:	QDoubleSpinBox(pParent)
{

}

CDoubleSpinBox::~CDoubleSpinBox()
{

}

void CDoubleSpinBox::keyPressEvent(QKeyEvent *pEvent)
{
	// let base class handle the event
	QDoubleSpinBox::keyPressEvent(pEvent);

	if ((pEvent->key() == Qt::Key_Enter) || (pEvent->key() == Qt::Key_Return)) {
		// Process enter/return so it won't propagate and "accept" the parent dialog:
		pEvent->accept();
		emit enterPressed();
	}
}

// ============================================================================
