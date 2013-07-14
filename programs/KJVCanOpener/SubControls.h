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

#ifndef SUB_CONTROLS_H
#define SUB_CONTROLS_H

#include <QComboBox>
#include <QFontComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QKeyEvent>

// ============================================================================

class CComboBox : public QComboBox
{
	Q_OBJECT

public:
	CComboBox(QWidget *pParent = NULL);
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
	CFontComboBox(QWidget *pParent = NULL);
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
	CSpinBox(QWidget *pParent = NULL);
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
	CDoubleSpinBox(QWidget *pParent = NULL);
	virtual ~CDoubleSpinBox();

	virtual void keyPressEvent(QKeyEvent *pEvent);

signals:
	void enterPressed();
};

// ============================================================================

#endif	// SUB_CONTROLS_H
