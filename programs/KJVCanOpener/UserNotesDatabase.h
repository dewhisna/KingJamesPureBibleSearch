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
#include "PersistentSettings.h"

#include <map>
#include <QtXml>
#include <QSharedPointer>

// ============================================================================

#define KJN_FILE_VERSION 1				// Current KJN File Version (King James Notes file)

// ============================================================================

//
// User highlighter data is stored as:
//			Map of BibleDatabaseUUID -> Map of HighlighterName -> TPhraseTagList
//
//	This way we can find all of the highlightations that are for the specific
//		database being rendered, and then find the highlighter name for which
//		to apply it...
//

class CUserNotesDatabase : public QObject, protected QXmlDefaultHandler
{
	Q_OBJECT

public:
	CUserNotesDatabase(QObject *pParent = NULL);
	virtual ~CUserNotesDatabase();

	inline bool isDirty() const { return m_bIsDirty; }
	void clear();
	inline int version() const { return m_nVersion; }

	// --------------------

	bool loadFromFile(const QString &strFilePathName);
	bool saveToFile(const QString &strFilePathName);
	QString lastLoadSaveError() const { return m_strLastError; }

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
	void appendHighlighterTagFor(const QString &strUUID, const QString &strUserDefinedHighlighterName, const TPhraseTag &lstTag);
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

	inline const TUserDefinedColorMap &loadedHighlighterDefinitions() const { return m_mapHighlighterDefinitions; }			// Highlighter Definitions loaded from KJN File or to save in KJN
	void setHighlighterDefinitions(const TUserDefinedColorMap &mapHighlighterColors) { m_mapHighlighterDefinitions = mapHighlighterColors; }	// See note below (No "changed" signal!)

	// --------------------

signals:
	void userNotesDatabaseHasChanged();


protected:
	// XML Parsing overrides:
	virtual bool characters(const QString &strChars);
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attr);
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
	virtual QString errorString() const;
	virtual bool startCDATA();
	virtual bool endCDATA();

private:
	void clearXMLVars();
	static int findAttribute(const QXmlAttributes &attr, const QString &strName)
	{
		for (int i = 0; i < attr.count(); ++i) {
			if (attr.localName(i).compare(strName, Qt::CaseInsensitive) == 0) return i;
		}
		return -1;
	}

private:
	TFootnoteEntryMap m_mapNotes;						// User notes, kept in the same format as our footnotes database
	TBibleDBHighlighterTagMap m_mapHighlighterTags;		// Tags to highlight by Bible Database compatibility and Highlighter name
	TCrossReferenceMap m_mapCrossReference;				// Cross reference of passage to other passages
	//		Note: m_mapHighlighterDefinitions is not considered part of the KJN data proper, used in load/save to read/set highligher definitions from the main app settings:
	TUserDefinedColorMap m_mapHighlighterDefinitions;	// Highlighter Definitions Read from the KJN File, used for merging with the Program Main Configuration
	bool m_bIsDirty;									// True when the document has been modified
	int m_nVersion;										// Version of the file read
	QString m_strLastError;								// Last error during load/save

	// ---- XML Temp Parsing varaibles:
	bool m_bInCDATA;									// True if we're in a CDATA section
	QString m_strXMLBuffer;								// XML CDATA Character input buffer during parsing
	CRelIndex m_ndxRelIndex;							// RelIndex attribute when present
	CRelIndex m_ndxRelIndexTag;							// RelIndex Tag Value when processing <CRelIndex> tag
	unsigned int m_nCount;								// Count attribute when present
	QString m_strDatabaseUUID;							// Bible Database UUID attribute when present
	QString m_strHighlighterName;						// HighlighterName attribute when present
	QString m_strColor;									// Color attribute when present
	bool m_bInKJNDocument;								// Inside <KJNDocument> tag
	bool m_bInKJNDocumentText;							// Inside <KJNDocumentText> tag
	bool m_bInNotes;									// Inside <Notes> tag
	bool m_bInNote;										// Processing <Note> tag
	bool m_bInHighlighting;								// Inside <Highlighting> tag
	bool m_bInHighlighterDB;							// Processing <HighlighterDB> tag
	bool m_bInHighlighterTags;							// Processing <HighlighterTags> tag
	bool m_bInPhraseTag;								// Processing <PhraseTag> tag
	bool m_bInCrossReferences;							// Inside <CrossReferences> tag
	bool m_bInCrossRef;									// Processing <CrossRef> tag
	bool m_bInRelIndex;									// Processing <RelIndex> tag
	bool m_bInHighlighterDefinitions;					// Inside <HighlighterDefinitions> tag
	bool m_bInHighlighterDef;							// Processing <HighlighterDef> tag
};


typedef QSharedPointer<CUserNotesDatabase> CUserNotesDatabasePtr;

typedef QList<CUserNotesDatabasePtr> TUserNotesDatabaseList;

// ============================================================================

// Global Variables:

extern CUserNotesDatabasePtr g_pMainUserNotesDatabase;		// Main User Notes Database (database currently active for user use)
extern TUserNotesDatabaseList g_lstUserNotesDatabases;

// ============================================================================

#endif	//  USER_NOTES_DATABASE_H
