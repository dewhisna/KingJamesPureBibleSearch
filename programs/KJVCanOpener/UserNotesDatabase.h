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
#include <QIODevice>

// ============================================================================

#define KJN_FILE_VERSION 1				// Current KJN File Version (King James Notes file)

// ============================================================================

class TUserDefinedColor
{
public:
	explicit TUserDefinedColor(const QColor &color = QColor(), bool bEnabled = true)
		:	m_color(color),
			m_bEnabled(bEnabled)
	{ }

	bool isValid() const { return m_color.isValid(); }

	inline bool operator==(const TUserDefinedColor &other) const {
		return ((m_color == other.m_color) && (m_bEnabled == m_bEnabled));
	}
	inline bool operator!=(const TUserDefinedColor &other) const {
		return (!operator==(other));
	}

	QColor m_color;
	bool m_bEnabled;
};

typedef QMap<QString, TUserDefinedColor> TUserDefinedColorMap;


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

	inline bool isDirty() const { return m_bIsDirty || m_pUserNotesDatabaseData->m_bIsDirty; }
	void clear();
	inline int version() const { return m_nVersion; }

	QString filePathName() const { return m_strFilePathName; }
	void setFilePathName(const QString &strFilePathName) { m_strFilePathName = strFilePathName; }

	// --------------------

	bool load();
	bool load(QIODevice *pIODevice);
	bool save();
	bool save(QIODevice *pIODevice);
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
	void removeCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond);
	void removeCrossReferencesFor(const CRelIndex &ndx);
	void removeAllCrossReferences();

	// --------------------

	QColor highlighterColor(const QString &strUserDefinedHighlighterName) const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strUserDefinedHighlighterName, TUserDefinedColor()).m_color; }
	bool highlighterEnabled(const QString &strUserDefinedHighlighterName) const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strUserDefinedHighlighterName, TUserDefinedColor()).m_bEnabled; }
	bool existsHighlighter(const QString &strUserDefinedHighlighterName) const { return (m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.find(strUserDefinedHighlighterName) != m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.constEnd()); }
	const TUserDefinedColor highlighterDefinition(const QString &strUserDefinedHighlighterName) const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strUserDefinedHighlighterName, TUserDefinedColor()); }
	inline const TUserDefinedColorMap &highlighterDefinitionsMap() const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions; }

	void toggleUserNotesDatabaseData(bool bCopy);

public slots:
	void setHighlighterColor(const QString &strUserDefinedHighlighterName, const QColor &color);
	void setHighlighterEnabled(const QString &strUserDefinedHighlighterName, bool bEnabled);
	void removeHighlighter(const QString &strUserDefinedHighlighterName);
	void removeAllHighlighters();

signals:
	void changedHighlighter(const QString &strUserDefinedHighlighterName);		// Note: If entire map is swapped or cleared, this signal isn't fired!
	void removedHighlighter(const QString &strUserDefinedHighlighterName);		// Note: If entire map is swapped or cleared, this signal isn't fired!
	void changedHighlighters();													// Fired on both individual and entire UserDefinedColor map change

	void changedUserNotesDatabase();

	// --------------------

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
	// m_UserNotesDatabaseData1 and m_UserNotesDatabaseData2 are
	//		two complete copies of our user notes database data.  Only
	//		one will be active and used at any given time.  Classes
	//		like KJVConfiguration can request that the settings be
	//		copied to the other copy and that other copy made to be
	//		the main copy for preview purposes so that controls will
	//		appear with the new set of settings.  When it's done with
	//		it, it can either revert back to the original copy without
	//		copying it back or leave the new settings to be the new
	//		settings.
	class TUserNotesDatabaseData {
	public:
		TUserNotesDatabaseData();

		TUserDefinedColorMap m_mapHighlighterDefinitions;	// Highlighter Definitions Read from the KJN File, used for merging with the Program Main Configuration
		bool m_bIsDirty;
	} m_UserNotesDatabaseData1, m_UserNotesDatabaseData2, *m_pUserNotesDatabaseData;

	TFootnoteEntryMap m_mapNotes;						// User notes, kept in the same format as our footnotes database
	TBibleDBHighlighterTagMap m_mapHighlighterTags;		// Tags to highlight by Bible Database compatibility and Highlighter name
	TCrossReferenceMap m_mapCrossReference;				// Cross reference of passage to other passages

	QString m_strFilePathName;							// FilePathName of KJN used on load/save and available for saving in persistent settings for this KJN when setting as the default file
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
	bool m_bEnabled;									// Enabled attribute when present
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

// Currently we only allow a single notes database.  Uncomment this to enable multiple:
//typedef QList<CUserNotesDatabasePtr> TUserNotesDatabaseList;

// ============================================================================

// Global Variables:

extern CUserNotesDatabasePtr g_pUserNotesDatabase;		// Main User Notes Database (database currently active for user use)
// Currently we only allow a single notes database.  Uncomment this to enable multiple:
//extern TUserNotesDatabaseList g_lstUserNotesDatabases;

// ============================================================================

#endif	//  USER_NOTES_DATABASE_H
