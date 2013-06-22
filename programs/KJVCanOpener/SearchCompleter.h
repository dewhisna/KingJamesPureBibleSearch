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

#ifndef SEARCH_COMPLETER_H
#define SEARCH_COMPLETER_H

#include "dbstruct.h"

#include <QWidget>
#include <QString>
#include <QVariant>
#include <QModelIndex>
#include <QCompleter>
#include <QAbstractListModel>

// ============================================================================

// Forward Declarations:
class CParsedPhrase;

// ============================================================================

class CSearchStringListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CSearchStringListModel(const CParsedPhrase &parsedPhrase, QObject *parent = NULL);
	virtual ~CSearchStringListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	static QString decompose(const QString &strWord);			// Word decompose() function to breakdown and remove accents from words for searching purposes

public slots:
	void setWordsFromPhrase();

private:
	Q_DISABLE_COPY(CSearchStringListModel)
	const CParsedPhrase &m_parsedPhrase;
};

// ============================================================================

class CSearchCompleter : public QCompleter
{
	Q_OBJECT

public:
	CSearchCompleter(CSearchStringListModel *model, QWidget *parentWidget);
	virtual ~CSearchCompleter();


};



// ============================================================================

#endif	// SEARCH_COMPLETER_H
