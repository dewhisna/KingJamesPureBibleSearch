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

#ifndef DICTIONARY_WIDGET_H
#define DICTIONARY_WIDGET_H

#include "dbstruct.h"

#include "SubControls.h"

#include <QWidget>
#include <QModelIndex>

// ============================================================================

class CDictionaryLineEdit : public CSingleLineTextEdit
{
	Q_OBJECT

public:
	explicit CDictionaryLineEdit(QWidget *pParent = 0);
	virtual ~CDictionaryLineEdit();

	void initialize(CDictionaryDatabasePtr pDictionary);

protected slots:
	void en_textChanged();
	void insertCompletion(const QModelIndex &index);

protected:
	virtual void setupCompleter(const QString &strText, bool bForce = false);

// Data Private:
private:
	CDictionaryDatabasePtr m_pDictionaryDatabase;
	SearchCompleter_t *m_pCompleter;			// Word completer
};

// ============================================================================

#include "ui_DictionaryWidget.h"

class CDictionaryWidget : public QWidget
{
	Q_OBJECT
	
public:
	explicit CDictionaryWidget(CDictionaryDatabasePtr pDictionary, QWidget *parent = 0);
	~CDictionaryWidget();

// Data Private:
private:
	CDictionaryDatabasePtr m_pDictionaryDatabase;

// UI Private:
private:
	bool m_bDoingPopup;				// True if popping up a menu or dialog (useful for things like not disabling highlight, etc)
	Ui::CDictionaryWidget ui;
};

// ============================================================================

#endif // DICTIONARY_WIDGET_H
