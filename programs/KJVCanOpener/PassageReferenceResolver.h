/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef PASSAGE_REFERENCE_RESOLVER_H
#define PASSAGE_REFERENCE_RESOLVER_H

#include "dbstruct.h"

#include <QList>
#include <QString>
#include <QObject>

// ============================================================================

class CPassageReferenceResolver : public QObject
{
	Q_OBJECT

public:
	CPassageReferenceResolver(CBibleDatabasePtr pBibleDatabase, QObject *pParent = nullptr);

	TPhraseTag resolve(const QString &strPassageReference) const;

private:
	void buildSoundExTables();
	uint32_t resolveBook(const QString &strPreBook, const QString &strBook) const;

// Data Private:
private:
	CBibleDatabasePtr m_pBibleDatabase;
	QList<QStringList> m_lstBookSoundEx;			// Index of [nBk-1], List of Book SoundEx Value lists.  Each sublist has the SoundEx for the book name as well as all abbreviations
};

// ============================================================================

#endif	// PASSAGE_REFERENCE_RESOLVER_H
