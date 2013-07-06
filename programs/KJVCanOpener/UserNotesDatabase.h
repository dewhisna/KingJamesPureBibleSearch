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

#ifndef USER_NOTES_DATABASE_H
#define USER_NOTES_DATABASE_H

#include "dbstruct.h"

#include <map>


// ============================================================================

//
// User highlighter data is stored as:
//			Map of BibleDatabaseUUID -> Map of HighlighterName -> TPhraseTagList
//
//	This way we can find all of the highlightations that are for the specific
//		database being rendered, and then find the highlighter name for which
//		to apply it...
//

class CUserNotesDatabase : public QObject
{
	Q_OBJECT

public:
	CUserNotesDatabase(QObject *pParent = NULL);
	virtual ~CUserNotesDatabase();

	inline bool isDirty() const { return m_bIsDirty; }
	void clear();

	// --------------------

	inline QString noteFor(const CRelIndex &ndx) const {
		TFootnoteEntryMap::const_iterator itr = m_mapNotes.find(ndx);
		if (itr == m_mapNotes.end()) return QString();
		return (itr->second).text();
	}
	inline bool existsNoteFor(const CRelIndex &ndx) const { return (m_mapNotes.find(ndx) != m_mapNotes.end()); }
	void setNoteFor(const CRelIndex &ndx, const QString &strNote);
	void removeNoteFor(const CRelIndex &ndx);
	void removeAllNotes();

	// --------------------

	inline const THighlighterTagMap *highlighterTagsFor(const QString &strUUID) const
	{
		TBibleDBHighlighterTagMap::const_iterator itr = m_mapHighlighterTags.find(strUUID);
		if (itr == m_mapHighlighterTags.end()) return NULL;
		return &(itr->second);
	}
	inline const TPhraseTagList *highlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName) const
	{
		const THighlighterTagMap *pmapHighlightTags = highlighterTagsFor(strUUID);
		if (pmapHighlightTags == NULL) return NULL;
		THighlighterTagMap::const_iterator itr = pmapHighlightTags->find(strUserDefinedHighlighterName);
		if (itr == pmapHighlightTags->end()) return NULL;
		return &(itr->second);
	}
	inline bool existsHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName) const
	{
		return (highlighterTagsFor(strUUID, strUserDefinedHighlighterName) != NULL);
	}
	void setHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags);
	void appendHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags);
	void removeHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName = QString());
	void removeAllHighlighterTags();

	// --------------------

	inline bool haveCrossReferencesFor(const CRelIndex &ndx) const
	{
		return (m_mapCrossReference.find(ndx) != m_mapCrossReference.end());
	}
	inline const TRelativeIndexSet crossReferencesFor(const CRelIndex &ndx) const
	{
		TCrossReferenceMap::const_iterator itr = m_mapCrossReference.find(ndx);
		if (itr == m_mapCrossReference.end()) return TRelativeIndexSet();
		return (itr->second);
	}
	void setCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond);
	void removeCrossReferencesFor(const CRelIndex &ndx);
	void removeAllCrossReferences();

	// --------------------

signals:
	void userNotesDatabaseHasChanged();

private:
	TFootnoteEntryMap m_mapNotes;						// User notes, kept in the same format as our footnotes database
	TBibleDBHighlighterTagMap m_mapHighlighterTags;		// Tags to highlight by Bible Database compatibility and Highlighter name
	TCrossReferenceMap m_mapCrossReference;				// Cross reference of passage to other passages
	bool m_bIsDirty;
};

// ============================================================================

#endif	//  USER_NOTES_DATABASE_H
