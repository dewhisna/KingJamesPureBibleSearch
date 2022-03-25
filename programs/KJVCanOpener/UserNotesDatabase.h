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

#ifndef USER_NOTES_DATABASE_H
#define USER_NOTES_DATABASE_H

#include "dbstruct.h"
#include "PersistentSettings.h"

#include <map>
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QSharedPointer>
#include <QIODevice>
#include <QStringList>
#include <QMetaType>

// ============================================================================

#define KJN_FILE_VERSION 1				// Current KJN File Version (King James Notes file)

// ============================================================================

//
// User notes data is stored as:
//		Map of CRelIndex -> CUserNoteEntry
//

class CUserNoteEntry
{
protected:
	friend class CUserNotesDatabase;
	CUserNoteEntry(const CRelIndex &ndxRel, unsigned int nVerseCount = 0);

public:
	CUserNoteEntry();

	CUserNoteEntry(const CUserNoteEntry &other) = default;

	~CUserNoteEntry() { }

	QString htmlText() const;		// Formatted HTML to insert into Scripture Browser (with background colored)
	QString plainText() const;		// Formatted PlainText rendering

	QString text() const
	{
		return m_strText;
	}
	void setText(const QString &strText)
	{
		m_strText = strText;
	}

	const QStringList &keywordList() const { return m_lstKeywords; }
	int keywordCount() const { return m_lstKeywords.size(); }
	QString keyword(int ndx) const { Q_ASSERT((ndx >= 0) && (ndx < m_lstKeywords.size())); return m_lstKeywords.at(ndx); }
	void setKeywordList(const QStringList &lstKeywords) { m_lstKeywords = lstKeywords; }
	void addKeyword(const QString &strKeyword) { if (!m_lstKeywords.contains(strKeyword, Qt::CaseInsensitive)) m_lstKeywords.append(strKeyword); }
	void clearKeywords() { m_lstKeywords.clear(); }

	TPassageTag passageTag() const { return m_PassageTag; }
	CRelIndex index() const { return m_PassageTag.relIndex(); }

	unsigned int verseCount() const
	{
		return m_PassageTag.verseCount();
	}
	void setVerseCount(unsigned int nCount)
	{
		m_PassageTag = TPassageTag(m_PassageTag.relIndex(), nCount);
	}

	QColor backgroundColor() const { return m_clrBackground; }
	void setBackgroundColor(const QColor &color) { m_clrBackground = color; }

	bool isVisible() const { return m_bIsVisible; }
	void setIsVisible(bool bIsVisible) { m_bIsVisible = bIsVisible; }

protected:
	void setPassageTag(const CRelIndex &ndxRel, unsigned int nVerseCount = 0)
	{
		m_PassageTag = TPassageTag(ndxRel, nVerseCount);
	}

private:
	QString m_strText;			// Rich text
	QStringList m_lstKeywords;	// List of keywords for this note
	TPassageTag m_PassageTag;	// RelIndex of tag and Verse count
	QColor m_clrBackground;		// Tag Background Color
	bool m_bIsVisible;			// Visible in Scripture Browser
};

typedef std::map<CRelIndex, CUserNoteEntry, RelativeIndexSortPredicate> CUserNoteEntryMap;		// Index by [nBk|nChp|nVrs|nWrd]


// ============================================================================

//
// User highlighter data is stored as:
//			Map of BibleDatabaseUUID (Highlighter) -> Map of HighlighterName -> TPhraseTagList
//
//	This way we can find all of the highlightations that are for the specific
//		database being rendered, and then find the highlighter name for which
//		to apply it...
//

struct HighlighterNameSortPredicate {
	bool operator() (const QString &v1, const QString &v2) const;
};

// PhraseTag Highlighter Mapping Types:
typedef std::map<QString, TPhraseTagList, HighlighterNameSortPredicate> THighlighterTagMap;		// Map of HighlighterName to TPhraseTagList (Highlighters are kept in sorted decomposed alphabetical order for overlay order)
typedef std::map<QString, THighlighterTagMap> TBibleDBHighlighterTagMap;						// Map of Bible Database UUID to THighlighterTagMap

// ----------------------------------------------------------------------------

class TUserDefinedColor
{
public:
	explicit TUserDefinedColor(const QColor &color = QColor(), bool bEnabled = true)
		:	m_color(color),
			m_bEnabled(bEnabled)
	{ }

	bool isValid() const { return m_color.isValid(); }

	inline bool operator==(const TUserDefinedColor &other) const {
		return ((m_color == other.m_color) && (m_bEnabled == other.m_bEnabled));
	}
	inline bool operator!=(const TUserDefinedColor &other) const {
		return (!operator==(other));
	}

	QColor m_color;
	bool m_bEnabled;
};

class TQStringHighlighterName : public QString
{
public:
	TQStringHighlighterName(const TQStringHighlighterName &other)
		:	QString(other)
	{ }
	TQStringHighlighterName(const QString &other)
		:	QString(other)
	{ }

	inline bool operator <(const TQStringHighlighterName &other) const {
		HighlighterNameSortPredicate pred;
		return (pred(*this, other));
	}
};

typedef QMap<TQStringHighlighterName, TUserDefinedColor> TUserDefinedColorMap;

// ============================================================================

//
// User cross-ref data is stored as:
//		Map of CRelIndex -> TRelativeIndexSet
//

class TCrossReferenceMap : public std::map<CRelIndex, TRelativeIndexSet, RelativeIndexSortPredicate>		// Map of Relative Index to Relative Index Set, used for cross-references (such as User Notes Database cross-reference, etc)
{
public:
	TCrossReferenceMap()
		:	std::map<CRelIndex, TRelativeIndexSet, RelativeIndexSortPredicate>(),
			m_bNoWordRefs(true)
	{

	}

	TCrossReferenceMap(const TCrossReferenceMap &aMap)
		:	std::map<CRelIndex, TRelativeIndexSet, RelativeIndexSortPredicate>(aMap),
			m_bNoWordRefs(true)
	{

	}

	TCrossReferenceMap & operator=(const TCrossReferenceMap &aMap) = default;

	inline bool haveCrossReferencesFor(const CRelIndex &ndx) const
	{
		return (find(relIndexMaskWord(ndx)) != end());
	}
	inline bool haveCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond) const
	{
		const TRelativeIndexSet refs = crossReferencesFor(ndxFirst);
		TRelativeIndexSet::const_iterator itr = refs.find(relIndexMaskWord(ndxSecond));
		return (itr != refs.end());
	}
	inline const TRelativeIndexSet crossReferencesFor(const CRelIndex &ndx) const
	{
		TCrossReferenceMap::const_iterator itr = find(relIndexMaskWord(ndx));
		if (itr == end()) return TRelativeIndexSet();
		return (itr->second);
	}
	TCrossReferenceMap createScopedMap(const CBibleDatabase *pBibleDatabase) const;

private:
	// TODO : If we ever change this to allow WordRefs, we must update calls in
	//		VerseListModel to properly mask the word for the special "word 1"
	//		indexes used for verses to distinguish other types
	inline CRelIndex relIndexMaskWord(const CRelIndex &ndx) const
	{
		if (m_bNoWordRefs) {
			return CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0);
		}
		return ndx;
	}

private:
	bool m_bNoWordRefs;
};

// ============================================================================

class CUserNotesDatabase : public QObject, protected QXmlDefaultHandler
{
	Q_OBJECT

public:
	CUserNotesDatabase(QObject *pParent = nullptr);
	virtual ~CUserNotesDatabase();

	void setDataFrom(const CUserNotesDatabase &other);		// Works like a load() from the other object

	inline bool isDirty() const { return m_bIsDirty || m_pUserNotesDatabaseData->m_bIsDirty; }
	void clear();
	inline int version() const { return m_nVersion; }

	QString filePathName() const { return m_strFilePathName; }
	void setFilePathName(const QString &strFilePathName) { m_strFilePathName = strFilePathName; }
	QString errorFilePathName() const { return m_strErrorFilePathName; }
	void setErrorFilePathName(const QString &strFilePathName) { m_strErrorFilePathName = strFilePathName; }

	// --------------------

	bool load();
	bool load(QIODevice *pIODevice);
	bool save();
	bool save(QIODevice *pIODevice);
	QString lastLoadSaveError() const { return m_strLastError; }

	// --------------------

	CUserNoteEntry noteFor(const CRelIndex &ndx) const {
		CUserNoteEntryMap::const_iterator itr = m_mapNotes.find(ndx);
		if (itr == m_mapNotes.end()) return CUserNoteEntry();
		return (itr->second);
	}
	bool existsNoteFor(const CRelIndex &ndx) const { return (m_mapNotes.find(ndx) != m_mapNotes.end()); }
	void setNoteFor(const CRelIndex &ndx, const CUserNoteEntry &strNote);
	void removeNoteFor(const CRelIndex &ndx);
	void removeAllNotes();
	const CUserNoteEntryMap &notesMap() const { return m_mapNotes; }
	QStringList compositeKeywordList() const;

	// --------------------

	const THighlighterTagMap *highlighterTagsFor(CBibleDatabasePtr pBibleDatabase) const
	{
		Q_ASSERT(!pBibleDatabase.isNull());
		return highlighterTagsFor(pBibleDatabase->highlighterUUID());
	}
	const THighlighterTagMap *highlighterTagsFor(const QString &strUUID) const
	{
		TBibleDBHighlighterTagMap::const_iterator itr = m_mapHighlighterTags.find(strUUID);
		if (itr == m_mapHighlighterTags.end()) return nullptr;
		return &(itr->second);
	}
	const TPhraseTagList *highlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName) const
	{
		Q_ASSERT(!pBibleDatabase.isNull());
		return highlighterTagsFor(pBibleDatabase->highlighterUUID(), strUserDefinedHighlighterName);
	}
	const TPhraseTagList *highlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName) const
	{
		const THighlighterTagMap *pmapHighlightTags = highlighterTagsFor(strUUID);
		if (pmapHighlightTags == nullptr) return nullptr;
		THighlighterTagMap::const_iterator itr = pmapHighlightTags->find(strUserDefinedHighlighterName);
		if (itr == pmapHighlightTags->end()) return nullptr;
		return &(itr->second);
	}
	bool existsHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName) const
	{
		return (highlighterTagsFor(pBibleDatabase, strUserDefinedHighlighterName) != nullptr);
	}
	bool existsHighlighterTagsFor(const QString &strUUID, const QString &strUserDefinedHighlighterName) const
	{
		return (highlighterTagsFor(strUUID, strUserDefinedHighlighterName) != nullptr);
	}
	bool existsHighlighterTagsFor(const QString &strUserDefinedHighlighterName) const
	{
		for (TBibleDBHighlighterTagMap::const_iterator itrDB = m_mapHighlighterTags.begin(); itrDB != m_mapHighlighterTags.end(); ++itrDB) {
			if (highlighterTagsFor(itrDB->first, strUserDefinedHighlighterName)) return true;
		}
		return false;
	}
	void setHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags);
	void appendHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags);
	void appendHighlighterTagFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag);
	void removeHighlighterTagFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag);
	void removeHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags);
	void removeHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName = QString());
	void removeAllHighlighterTags();

	// --------------------

	bool setCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond);			// Returns true if it was set or false if it was already set or can't be set
	bool removeCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond);		// Returns true if it was removed or false if it wasn't there or can't be removed
	bool removeCrossReferencesFor(const CRelIndex &ndx);									// Returns true if it was removed or false if it wasn't there or can't be removed
	void removeAllCrossReferences();
	const TCrossReferenceMap &crossRefsMap() const { return m_mapCrossReference; }
	void setCrossRefsMap(const TCrossReferenceMap &mapCrossRefs);

	// --------------------

	QColor highlighterColor(const QString &strUserDefinedHighlighterName) const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strUserDefinedHighlighterName, TUserDefinedColor()).m_color; }
	bool highlighterEnabled(const QString &strUserDefinedHighlighterName) const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strUserDefinedHighlighterName, TUserDefinedColor()).m_bEnabled; }
	bool existsHighlighter(const QString &strUserDefinedHighlighterName) const { return (m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.find(strUserDefinedHighlighterName) != m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.constEnd()); }
	const TUserDefinedColor highlighterDefinition(const QString &strUserDefinedHighlighterName) const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strUserDefinedHighlighterName, TUserDefinedColor()); }
	const TUserDefinedColorMap highlighterDefinitionsMap() const { return m_pUserNotesDatabaseData->m_mapHighlighterDefinitions; }
	bool renameHighlighter(const QString &strOldUserDefinedHighlighterName, const QString &strNewUserDefinedHighlighterName);

	void toggleUserNotesDatabaseData(bool bCopy);
	void initUserNotesDatabaseData();								// Reinitializes the current working data set to the default constructed version (used in things like fail KJN load attempts)

public slots:
	void setHighlighterColor(const QString &strUserDefinedHighlighterName, const QColor &color);
	void setHighlighterEnabled(const QString &strUserDefinedHighlighterName, bool bEnabled);
	void removeHighlighter(const QString &strUserDefinedHighlighterName);
	void removeAllHighlighters();

signals:
	void highlighterTagsAboutToChange(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName);
	void highlighterTagsChanged(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName);

	void changedHighlighter(const QString &strUserDefinedHighlighterName);		// Note: If entire map is swapped or cleared, this signal isn't fired!
	void removedHighlighter(const QString &strUserDefinedHighlighterName);		// Note: If entire map is swapped or cleared, this signal isn't fired!
	void aboutToChangeHighlighters();											// Fired before either individual or entire UserDefinedColor map change
	void changedHighlighters();													// Fired on both individual and entire UserDefinedColor map change

	void changedUserNote(const CRelIndex &ndx);			// Fired on Set with data changed only -- not used on add/remove all
	void addedUserNote(const CRelIndex &ndx);			// Fired on Set (as new).  If !ndx.isSet() then the entire list changed (such as file load)
	void removedUserNote(const CRelIndex &ndx);			// Fired on Remove.  If !ndx.isSet() then the entire list changed (such as remove all)
	void changedUserNotesKeywords();					// Fired if a note changes its keyword list

	void addedCrossRef(const CRelIndex &ndxRef1, const CRelIndex &ndxRef2);
	void removedCrossRef(const CRelIndex &ndxRef1, const CRelIndex &ndxRef2);
	void changedAllCrossRefs();

	void changedUserNotesDatabase();

	// --------------------

protected:
	// XML Parsing overrides:
	virtual bool characters(const QString &strChars) override;
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attr) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual QString errorString() const override;
	virtual bool startCDATA() override;
	virtual bool endCDATA() override;

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
	//		like Configuration can request that the settings be
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

	CUserNoteEntryMap m_mapNotes;						// User notes
	TBibleDBHighlighterTagMap m_mapHighlighterTags;		// Tags to highlight by Bible Database compatibility and Highlighter name
	TCrossReferenceMap m_mapCrossReference;				// Cross reference of passage to other passages

	QString m_strFilePathName;							// FilePathName of KJN used on load/save and available for saving in persistent settings for this KJN when setting as the default file
	QString m_strErrorFilePathName;						// FilePathName previously used if there was an error reading an existing file.  This allows us to force a save-prompt when exiting without accidentally overwriting it, prompting them with the old filename and path. (Cleared on successful save or load on hard file)
	bool m_bIsDirty;									// True when the document has been modified
	bool m_bKeepDirtyAfterLoad;							// Used for bug workarounds to make sure we rewrite our corrected file
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
	QString m_strBackgroundColor;						// BackgroundColor attribute when present
	bool m_bEnabled;									// Enabled attribute when present
	bool m_bVisible;									// Visible attribute when present
	QString m_strKeywords;								// Keywords attribute when present
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

Q_DECLARE_METATYPE(CUserNotesDatabase *)
typedef QSharedPointer<CUserNotesDatabase> CUserNotesDatabasePtr;
Q_DECLARE_METATYPE(CUserNotesDatabasePtr)

// Currently we only allow a single notes database.  Uncomment this to enable multiple:
//typedef QList<CUserNotesDatabasePtr> TUserNotesDatabaseList;

// ============================================================================

// Global Variables:

extern CUserNotesDatabasePtr g_pUserNotesDatabase;		// Main User Notes Database (database currently active for user use)
// Currently we only allow a single notes database.  Uncomment this to enable multiple:
//extern TUserNotesDatabaseList g_lstUserNotesDatabases;

// ============================================================================

#endif	//  USER_NOTES_DATABASE_H
