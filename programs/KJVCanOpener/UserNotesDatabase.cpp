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

#include "UserNotesDatabase.h"

// ============================================================================

CUserNotesDatabase::CUserNotesDatabase(QObject *pParent)
	:	QObject(pParent),
		m_bIsDirty(false)
{

}

CUserNotesDatabase::~CUserNotesDatabase()
{

}

void CUserNotesDatabase::clear()
{
	removeAllNotes();
	removeAllHighlighterTags();
	removeAllCrossReferences();
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

// ============================================================================

void CUserNotesDatabase::setNoteFor(const CRelIndex &ndx, const QString &strNote)
{
	m_mapNotes[ndx].setText(strNote);
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::removeNoteFor(const CRelIndex &ndx)
{
	m_mapNotes.erase(ndx);
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::removeAllNotes()
{
	m_mapNotes.clear();
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

// ============================================================================

void CUserNotesDatabase::setHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags)
{
	assert(!strUUID.isEmpty());
	assert(!strUserDefinedHighlighterName.isEmpty());
	if ((strUUID.isEmpty()) || (strUserDefinedHighlighterName.isEmpty())) return;

	(m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName] = lstTags;
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::appendHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags)
{
	assert(!strUUID.isEmpty());
	assert(!strUserDefinedHighlighterName.isEmpty());
	if ((strUUID.isEmpty()) || (strUserDefinedHighlighterName.isEmpty())) return;

	(m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].append(lstTags);
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::removeHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName)
{
	assert(!strUUID.isEmpty());
	if (strUUID.isEmpty()) return;

	if (strUserDefinedHighlighterName.isEmpty()) {
		if (highlighterTagsFor(strUUID) == NULL) return;				// Return if it doesn't exist so we don't set dirty flag
		m_mapHighlighterTags.erase(strUUID);
	} else {
		TBibleDBHighlighterTagMap::iterator itr = m_mapHighlighterTags.find(strUUID);
		if (itr == m_mapHighlighterTags.end()) return;
		if (highlighterTagsFor(strUUID, strUserDefinedHighlighterName) == NULL) return;		// Return if it doesn't exist so we don't set dirty flag
		(itr->second).erase(strUserDefinedHighlighterName);
	}
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::removeAllHighlighterTags()
{
	m_mapHighlighterTags.clear();
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

// ============================================================================

void CUserNotesDatabase::setCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond)
{
	if (ndxFirst == ndxSecond) return;							// Don't allow cross references to ourselves (that's just stupid)
	m_mapCrossReference[ndxFirst].insert(ndxSecond);
	m_mapCrossReference[ndxSecond].insert(ndxFirst);
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::removeCrossReferencesFor(const CRelIndex &ndx)
{
	TCrossReferenceMap::iterator itrMap = m_mapCrossReference.find(ndx);
	if (itrMap == m_mapCrossReference.end()) return;

	for (TRelativeIndexSet::iterator itrSet = (itrMap->second).begin(); itrSet != (itrMap->second).end(); ++itrSet) {
		assert(*itrSet != ndx);		// Shouldn't have any cross references to our same index, as we didn't allow them to be added
		if (*itrSet == ndx) continue;
		m_mapCrossReference[*itrSet].erase(ndx);			// Remove all cross references of other indexes to this index
		if (m_mapCrossReference[*itrSet].empty()) m_mapCrossReference.erase(*itrSet);			// Remove any mappings that become empty
	}
	m_mapCrossReference.erase(ndx);		// Now, remove this index mapping to other indexes

	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

void CUserNotesDatabase::removeAllCrossReferences()
{
	m_mapCrossReference.clear();
	m_bIsDirty = true;
	emit userNotesDatabaseHasChanged();
}

// ============================================================================

