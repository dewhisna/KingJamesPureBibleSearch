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
#include "ScriptureDocument.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QColor>
#include <QtIOCompressor>
#include <QTextDocument>			// Also needed for Qt::escape, which is in this header, not <Qt> as is assistant says
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>

#include <algorithm>

// ============================================================================

// Global Variables:

// Our User Notes Databases:
CUserNotesDatabasePtr g_pUserNotesDatabase;		// Main User Notes Database (database currently active for user use)
// Currently we only allow a single notes database.  Uncomment this to enable multiple:
//TUserNotesDatabaseList g_lstUserNotesDatabases;

// ============================================================================

//#define DEBUG_KJN_XML_READ

namespace {
	const QString constrKJNPrefix("kjn");
	const QString constrKJNNameSpaceURI("http://www.dewtronics.com/KingJamesPureBibleSearch/namespace");
	// ----
	const QString constrKJNDocumentTag("KJNDocument");
	const QString constrKJNDocumentTextTag("KJNDocumentText");
	const QString constrNotesTag("Notes");
	const QString constrNoteTag("Note");
	const QString constrHighlightingTag("Highlighting");
	const QString constrHighlighterDBTag("HighlighterDB");
	const QString constrHighlighterTagsTag("HighlighterTags");
	const QString constrPhraseTagTag("PhraseTag");
	const QString constrRelIndexTag("RelIndex");
	const QString constrCrossReferencesTag("CrossReferences");
	const QString constrCrossRefTag("CrossRef");
	const QString constrHighlighterDefinitionsTag("HighlighterDefinitions");
	const QString constrHighlighterDefTag("HighlighterDef");
	// ----
	const QString constrVersionAttr("Version");
	const QString constrRelIndexAttr("RelIndex");
	const QString constrCountAttr("Count");
	const QString constrValueAttr("Value");
	const QString constrSizeAttr("Size");
	const QString constrUUIDAttr("DatabaseUUID");
	const QString constrHighlighterNameAttr("HighlighterName");
	const QString constrColorAttr("Color");
	const QString constrBackgroundColorAttr("BackgroundColor");
	const QString constrEnabledAttr("Enabled");
	const QString constrVisibleAttr("Visible");
	const QString constrKeywordsAttr("Keywords");
}

// ============================================================================

CUserNotesDatabase::TUserNotesDatabaseData::TUserNotesDatabaseData()
	:	m_bIsDirty(false)
{
	// Set Default Highlighters:
	m_mapHighlighterDefinitions[tr("Basic Highlighter #1")] = TUserDefinedColor(QColor(255, 255, 170));			// "yellow" highlighter
	m_mapHighlighterDefinitions[tr("Basic Highlighter #2")] = TUserDefinedColor(QColor(170, 255, 255));			// "blue" highlighter
	m_mapHighlighterDefinitions[tr("Basic Highlighter #3")] = TUserDefinedColor(QColor(170, 255, 170));			// "green" highligher
	m_mapHighlighterDefinitions[tr("Basic Highlighter #4")] = TUserDefinedColor(QColor(255, 170, 255));			// "pink" highlighter
	m_mapHighlighterDefinitions[tr("Basic Highlighter #5")] = TUserDefinedColor(QColor(255, 200, 0));			// "orange" highlighter
	m_mapHighlighterDefinitions[tr("Basic Highlighter #6")] = TUserDefinedColor(QColor(230, 200, 255));			// "purple" highlighter
	m_mapHighlighterDefinitions[tr("Basic Highlighter #7")] = TUserDefinedColor(QColor(255, 230, 230));			// "apricot" highlighter
	m_mapHighlighterDefinitions[tr("Basic Highlighter #8")] = TUserDefinedColor(QColor(255, 70, 200));			// "hot pink" highlighter
}

// ============================================================================

QString CUserNoteEntry::htmlText() const
{
	QTextDocument docUserNote;
	docUserNote.setHtml(text());
	QTextCursor cursorDocUserNote(&docUserNote);

	cursorDocUserNote.select(QTextCursor::Document);
	QTextCharFormat fmtCharUserNote;
	fmtCharUserNote.setBackground(backgroundColor());
	QTextBlockFormat fmtBlockUserNote;
	fmtBlockUserNote.setBackground(backgroundColor());
	cursorDocUserNote.mergeBlockFormat(fmtBlockUserNote);
	cursorDocUserNote.mergeBlockCharFormat(fmtCharUserNote);

// TODO : Figure out how to make this do background color correctly:
//
//	CScriptureTextHtmlBuilder scriptureHTML;
//	CScriptureTextDocumentDirector scriptureDirector(&scriptureHTML);		// Note: No Bible Database needed since user notes contains no database generated scripture
//
//	scriptureDirector.processDocument(&docUserNote);
//
//	return scriptureHTML.getResult();

	return docUserNote.toHtml();
}

QString CUserNoteEntry::plainText() const
{
	QTextDocument docUserNote;
	docUserNote.setHtml(text());
	QTextCursor cursorDocUserNote(&docUserNote);

	cursorDocUserNote.select(QTextCursor::Document);
	QTextCharFormat fmtCharUserNote;
	fmtCharUserNote.setBackground(backgroundColor());
	QTextBlockFormat fmtBlockUserNote;
	fmtBlockUserNote.setBackground(backgroundColor());
	cursorDocUserNote.mergeBlockFormat(fmtBlockUserNote);
	cursorDocUserNote.mergeBlockCharFormat(fmtCharUserNote);

	CScripturePlainTextBuilder scripturePlainText;
	CScriptureTextDocumentDirector scriptureDirector(&scripturePlainText);		// Note: No Bible Database needed since user notes contains no database generated scripture

	scriptureDirector.processDocument(&docUserNote);

	return scripturePlainText.getResult();
}

// ============================================================================

CUserNotesDatabase::CUserNotesDatabase(QObject *pParent)
	:	QObject(pParent),
		m_pUserNotesDatabaseData(&m_UserNotesDatabaseData1),
		m_bKeepBackup(true),
		m_strBackupFilenamePostfix(QString(".bak")),
		m_bIsDirty(false),
		m_nVersion(KJN_FILE_VERSION)
{
	clearXMLVars();
}

CUserNotesDatabase::~CUserNotesDatabase()
{

}

void CUserNotesDatabase::setDataFrom(const CUserNotesDatabase &other)
{
	// Copy Highlighter Defintions:
	m_UserNotesDatabaseData1 = other.m_UserNotesDatabaseData1;
	m_UserNotesDatabaseData1.m_bIsDirty = false;
	m_UserNotesDatabaseData2 = other.m_UserNotesDatabaseData2;
	m_UserNotesDatabaseData2.m_bIsDirty = false;
	m_pUserNotesDatabaseData = ((other.m_pUserNotesDatabaseData == &other.m_UserNotesDatabaseData1) ? &m_UserNotesDatabaseData1 : &m_UserNotesDatabaseData2);

	m_mapNotes = other.m_mapNotes;							// Notes
	m_mapHighlighterTags = other.m_mapHighlighterTags;		// Highlighter Tags
	m_mapCrossReference = other.m_mapCrossReference;		// Cross-References

	m_bIsDirty = false;
	emit changedUserNotesDatabase();
	emit changedHighlighters();
	emit addedUserNote(CRelIndex());
	emit changedUserNotesKeywords();
	emit changedAllCrossRefs();
}

void CUserNotesDatabase::clear()
{
	removeAllNotes();
	removeAllHighlighterTags();
	removeAllCrossReferences();
	removeAllHighlighters();
	m_bIsDirty = true;
	emit changedUserNotesDatabase();				// Note: Changed highlighters already emitted above in removeAllHighligters
}

// ============================================================================

void CUserNotesDatabase::clearXMLVars()
{
	m_strLastError.clear();
	// ----
	m_bInCDATA = false;
	m_strXMLBuffer.clear();
	m_ndxRelIndex.clear();
	m_ndxRelIndexTag.clear();
	m_nCount = 0;
	m_strDatabaseUUID.clear();
	m_strHighlighterName.clear();
	m_strColor.clear();
	m_strBackgroundColor.clear();
	m_bEnabled = true;
	m_bVisible = true;
	m_strKeywords.clear();
	m_bInKJNDocument = false;
	m_bInKJNDocumentText = false;
	m_bInNotes = false;
	m_bInNote = false;
	m_bInHighlighting = false;
	m_bInHighlighterDB = false;
	m_bInHighlighterTags = false;
	m_bInPhraseTag = false;
	m_bInCrossReferences = false;
	m_bInCrossRef = false;
	m_bInRelIndex = false;
	m_bInHighlighterDefinitions = false;
	m_bInHighlighterDef = false;
}

bool CUserNotesDatabase::startCDATA()
{
	m_bInCDATA = true;
	return true;
}

bool CUserNotesDatabase::endCDATA()
{
	m_bInCDATA = false;
	return true;
}

bool CUserNotesDatabase::characters(const QString &strChars)
{
	if (m_bInCDATA) m_strXMLBuffer += strChars;
	return true;
}

bool CUserNotesDatabase::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attr)
{
	Q_UNUSED(qName);

#ifdef DEBUG_KJN_XML_READ
		qDebug("%s", localName.toUtf8().data());
#endif

	if ((!m_bInKJNDocument) && (localName.compare(constrKJNDocumentTag, Qt::CaseInsensitive) == 0) &&
		(!m_bInKJNDocumentText)) {
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : NamespaceURI:  \"%s\"", localName.toUtf8().data(), namespaceURI.toUtf8().data());
#endif
		// TODO : Consider not throwing error here if the namespace doesn't match, and
		//			instead just not setting m_bInKJNDocument, then we could daisychain
		//			XML documents:
		if (namespaceURI.compare(constrKJNNameSpaceURI, Qt::CaseInsensitive) != 0) {
			m_strLastError = tr("Unexpected Namespace URI: \"%1\"").arg(namespaceURI);
			return false;
		}
		m_bInKJNDocument = true;
	} else if ((m_bInKJNDocument) && (!m_bInKJNDocumentText) && (localName.compare(constrKJNDocumentTextTag, Qt::CaseInsensitive) == 0)) {
		int ndxVersion = findAttribute(attr, constrVersionAttr);
		if (ndxVersion == -1) {
			m_strLastError = tr("Missing Version Identifier");
			return false;
		}
		m_nVersion = attr.value(ndxVersion).toInt();
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : Version %d", localName.toUtf8().data(), m_nVersion);
#endif
		m_bInKJNDocumentText = true;
	} else if ((m_bInKJNDocumentText) && (!m_bInNotes) && (localName.compare(constrNotesTag, Qt::CaseInsensitive) == 0) &&
			   (!m_bInCrossReferences && !m_bInHighlighting && !m_bInHighlighterDefinitions)) {
#ifdef DEBUG_KJN_XML_READ
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
		}
#endif
		m_bInNotes = true;
	} else if ((m_bInKJNDocumentText) && (m_bInNotes) && (!m_bInNote) && (localName.compare(constrNoteTag, Qt::CaseInsensitive) == 0)) {
		int ndxRelIndex = findAttribute(attr, constrRelIndexAttr);
		if (ndxRelIndex == -1) {
			m_strLastError = tr("Missing RelIndex on Note Declaration");
			return false;
		}
		m_ndxRelIndex = CRelIndex(attr.value(ndxRelIndex));
		if (!m_ndxRelIndex.isSet()) {
			m_strLastError = tr("RelIndex for Note Declaration specifies a Null Destination");
			return false;
		}
		int ndxCountIndex = findAttribute(attr, constrCountAttr);
		m_nCount = ((ndxCountIndex != -1) ? attr.value(ndxCountIndex).toUInt() : 0);		// Count is optional
		int ndxBackgroundColor = findAttribute(attr, constrBackgroundColorAttr);
		if (ndxBackgroundColor != -1) m_strBackgroundColor = attr.value(ndxBackgroundColor);
		int ndxVisible = findAttribute(attr, constrVisibleAttr);
		if (ndxVisible == -1) {
			m_bVisible = true;
		} else {
			m_bVisible = (attr.value(ndxVisible).compare("True", Qt::CaseInsensitive) == 0);
			if ((!m_bVisible) && (attr.value(ndxVisible).compare("False", Qt::CaseInsensitive) != 0)) {
				m_strLastError = tr("Invalid Visible Attribute Value in Notes Declaration");
				return false;
			}
		}
		int ndxKeywords = findAttribute(attr, constrKeywordsAttr);			// Keywords are optional
		if (ndxKeywords != -1) {
			m_strKeywords = attr.value(ndxKeywords);
		} else {
			m_strKeywords.clear();
		}
		m_strXMLBuffer.clear();		// Clear buffer to get ready to capture the Note Text
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : RelIndex: %d  Count: %d  BackgroundColor: \"%s\"  Visible: %s  Keywords: \"%s\"",
					localName.toUtf8().data(),
					m_ndxRelIndex.index(),
					m_nCount,
					m_strBackgroundColor.toUtf8().data(),
					(m_bVisible ? "True" : "False"),
					m_strKeywords.toUtf8().data());
#endif
		m_bInNote = true;
	} else if ((m_bInKJNDocumentText) && (!m_bInHighlighting) && (localName.compare(constrHighlightingTag, Qt::CaseInsensitive) == 0) &&
			   (!m_bInCrossReferences && !m_bInNotes && !m_bInHighlighterDefinitions)) {
#ifdef DEBUG_KJN_XML_READ
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
		}
#endif
		m_bInHighlighting = true;
	} else if ((m_bInKJNDocumentText) && (m_bInHighlighting) && (!m_bInHighlighterDB) && (localName.compare(constrHighlighterDBTag, Qt::CaseInsensitive) == 0)) {
		int ndxUUID = findAttribute(attr, constrUUIDAttr);
		if (ndxUUID == -1) {
			m_strLastError = tr("Missing DatabaseUUID on HighlighterDB Declaration");
			return false;
		}
		m_strDatabaseUUID = attr.value(ndxUUID);
		if (m_strDatabaseUUID.isEmpty()) {
			m_strLastError = tr("DatabaseUUID on HighlighterDB is Empty");
			return false;
		}
#ifdef DEBUG_KJN_XML_READ
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
		}
		qDebug("%s : DatabaseUUID: \"%s\"", localName.toUtf8().data(), m_strDatabaseUUID.toUtf8().data());
#endif
		m_bInHighlighterDB = true;
	} else if ((m_bInKJNDocumentText) && (m_bInHighlighting) && (m_bInHighlighterDB) && (!m_bInHighlighterTags) && (localName.compare(constrHighlighterTagsTag, Qt::CaseInsensitive) == 0)) {
		int ndxHighlighterName = findAttribute(attr, constrHighlighterNameAttr);
		if (ndxHighlighterName == -1) {
			m_strLastError = tr("Missing HighlighterName on HighlighterTags Declaration");
			return false;
		}
		m_strHighlighterName = attr.value(ndxHighlighterName);
		if (m_strHighlighterName.isEmpty()) {
			m_strLastError = tr("HighligherName on HighlighterTags Declaration is Empty");
			return false;
		}
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
#ifdef DEBUG_KJN_XML_READ
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
#endif
			// We can pre-allocate our HighlighterTags:
			int nSize = attr.value(ndxSize).toInt();
			if (nSize) (m_mapHighlighterTags[m_strDatabaseUUID])[m_strHighlighterName].reserve(nSize);
		}
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : HighlighterName: \"%s\"", localName.toUtf8().data(), m_strHighlighterName.toUtf8().data());
#endif
		m_bInHighlighterTags = true;
	} else if ((m_bInKJNDocumentText) && (m_bInHighlighting) && (m_bInHighlighterDB) && (m_bInHighlighterTags) && (!m_bInPhraseTag) && (localName.compare(constrPhraseTagTag, Qt::CaseInsensitive) == 0)) {
		int ndxRelIndex = findAttribute(attr, constrRelIndexAttr);
		if (ndxRelIndex == -1) {
			m_strLastError = tr("Missing RelIndex on PhraseTag Declaration in HighlighterTag Declaration");
			return false;
		}
		m_ndxRelIndex = CRelIndex(attr.value(ndxRelIndex));
		if (!m_ndxRelIndex.isSet()) {
			m_strLastError = tr("RelIndex for PhraseTag Declaration in HighlighterTag Declaration specifies a Null Destination");
			return false;
		}
		int ndxCountIndex = findAttribute(attr, constrCountAttr);
		m_nCount = ((ndxCountIndex != -1) ? attr.value(ndxCountIndex).toUInt() : 0);		// Count is optional
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : RelIndex: %d  Count: %d", localName.toUtf8().data(), m_ndxRelIndex.index(), m_nCount);
#endif
		m_bInPhraseTag = true;
	} else if ((m_bInKJNDocumentText) && (!m_bInCrossReferences) && (localName.compare(constrCrossReferencesTag, Qt::CaseInsensitive) == 0) &&
			   (!m_bInNotes && !m_bInHighlighting && !m_bInHighlighterDefinitions)) {
#ifdef DEBUG_KJN_XML_READ
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
		}
#endif
		m_bInCrossReferences = true;
	} else if ((m_bInKJNDocumentText) && (m_bInCrossReferences) && (!m_bInCrossRef) && (localName.compare(constrCrossRefTag, Qt::CaseInsensitive) == 0)) {
		int ndxRelIndex = findAttribute(attr, constrRelIndexAttr);
		if (ndxRelIndex == -1) {
			m_strLastError = tr("Missing RelIndex on CrossRef Declaration");
			return false;
		}
		m_ndxRelIndex = CRelIndex(attr.value(ndxRelIndex));
		if (!m_ndxRelIndex.isSet()) {
			m_strLastError = tr("RelIndex for CrossRef Declaration specifies a Null Destination");
			return false;
		}
#ifdef DEBUG_KJN_XML_READ
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
		}
		qDebug("%s : RelIndex: %d", localName.toUtf8().data(), m_ndxRelIndex.index());
#endif
		m_bInCrossRef = true;
	} else if ((m_bInKJNDocumentText) && (m_bInCrossReferences) && (m_bInCrossRef) && (!m_bInRelIndex) && (localName.compare(constrRelIndexTag, Qt::CaseInsensitive) == 0)) {
		int ndxValue = findAttribute(attr, constrValueAttr);
		if (ndxValue == -1) {
			m_strLastError = tr("Missing Value on RelIndex Declaration in CrossRef Declaration");
			return false;
		}
		m_ndxRelIndexTag = CRelIndex(attr.value(ndxValue));
		if (!m_ndxRelIndexTag.isSet()) {
			m_strLastError = tr("Value for RelIndex Declaration Declaration in CrossRef Declaration specifies a Null Destination");
			return false;
		}
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : RelIndex: %d", localName.toUtf8().data(), m_ndxRelIndexTag.index());
#endif
		m_bInRelIndex = true;
	} else if ((m_bInKJNDocumentText) && (!m_bInHighlighterDefinitions) && (localName.compare(constrHighlighterDefinitionsTag, Qt::CaseInsensitive) == 0) &&
			   (!m_bInNotes && !m_bInCrossReferences && !m_bInHighlighting)) {
#ifdef DEBUG_KJN_XML_READ
		int ndxSize = findAttribute(attr, constrSizeAttr);
		if (ndxSize != -1) {
			qDebug("%s : Size: %s", localName.toUtf8().data(), attr.value(ndxSize).toUtf8().data());
		}
#endif
		m_bInHighlighterDefinitions = true;
	} else if ((m_bInKJNDocumentText) && (m_bInHighlighterDefinitions) && (!m_bInHighlighterDef) && (localName.compare(constrHighlighterDefTag, Qt::CaseInsensitive) == 0)) {
		int ndxHighlighterName = findAttribute(attr, constrHighlighterNameAttr);
		if (ndxHighlighterName == -1) {
			m_strLastError = tr("Missing HighlighterName on HighlighterDef Declaration");
			return false;
		}
		m_strHighlighterName = attr.value(ndxHighlighterName);
		if (m_strHighlighterName.isEmpty()) {
			m_strLastError = tr("HighligherName on HighlighterDef Declaration is Empty");
			return false;
		}
		int ndxColor = findAttribute(attr, constrColorAttr);
		if (ndxColor == -1) {
			m_strLastError = tr("Missing Color on HighligherDef Declaration");
			return false;
		}
		m_strColor = attr.value(ndxColor);
		int ndxEnabled = findAttribute(attr, constrEnabledAttr);
		if (ndxEnabled == -1) {
			m_bEnabled = true;
		} else {
			m_bEnabled = (attr.value(ndxEnabled).compare("True", Qt::CaseInsensitive) == 0);
			if ((!m_bEnabled) && (attr.value(ndxEnabled).compare("False", Qt::CaseInsensitive) != 0)) {
				m_strLastError = tr("Invalid Enable Attribute Value in HighlighterDef Declaration");
				return false;
			}
		}
#ifdef DEBUG_KJN_XML_READ
		qDebug("%s : HighlighterName: \"%s\"  Color: \"%s\"  Enabled: %s", localName.toUtf8().data(), m_strHighlighterName.toUtf8().data(), m_strColor.toUtf8().data(), (m_bEnabled ? "True" : "False"));
#endif
		m_bInHighlighterDef = true;
	}

	return true;
}

bool CUserNotesDatabase::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

	if ((m_bInNotes) && (m_bInNote) && (localName.compare(constrNoteTag, Qt::CaseInsensitive) == 0)) {
		m_strXMLBuffer.replace("]]&gt;", "]]>");			// safe-guard to make sure we don't have any embedded CDATA terminators
#ifdef DEBUG_KJN_XML_READ
		qDebug("Text: \"%s\"", m_strXMLBuffer.toUtf8().data());
#endif
		CUserNoteEntry &userNote = m_mapNotes[m_ndxRelIndex];
		userNote.setText(m_strXMLBuffer);
		userNote.m_lstKeywords = m_strKeywords.split(QChar(','), QString::SkipEmptyParts);
		userNote.setVerseCount(m_nCount);
		if (!m_strBackgroundColor.isEmpty()) userNote.setBackgroundColor(QColor(m_strBackgroundColor));
		userNote.setIsVisible(m_bVisible);
		m_strXMLBuffer.clear();
		m_strKeywords.clear();
		m_ndxRelIndex.clear();
		m_nCount = 0;
		m_strBackgroundColor.clear();
		m_bVisible = true;
		m_bInNote = false;
	} else if ((m_bInNotes) && (localName.compare(constrNotesTag, Qt::CaseInsensitive) == 0)) {
		m_bInNotes = false;
	} else if ((m_bInHighlighting) && (m_bInHighlighterDB) && (m_bInHighlighterTags) && (m_bInPhraseTag) && (localName.compare(constrPhraseTagTag, Qt::CaseInsensitive) == 0)) {
		(m_mapHighlighterTags[m_strDatabaseUUID])[m_strHighlighterName].append(TPhraseTag(m_ndxRelIndex, m_nCount));
		m_ndxRelIndex.clear();
		m_nCount = 0;
		m_bInPhraseTag = false;
	} else if ((m_bInHighlighting) && (m_bInHighlighterDB) && (m_bInHighlighterTags) && (localName.compare(constrHighlighterTagsTag, Qt::CaseInsensitive) == 0)) {
		m_strHighlighterName.clear();
		m_bInHighlighterTags = false;
	} else if ((m_bInHighlighting) && (m_bInHighlighterDB) && (localName.compare(constrHighlighterDBTag, Qt::CaseInsensitive) == 0)) {
		m_strDatabaseUUID.clear();
		m_bInHighlighterDB = false;
	} else if ((m_bInHighlighting) && (localName.compare(constrHighlightingTag, Qt::CaseInsensitive) == 0)) {
		m_bInHighlighting = false;
	} else if ((m_bInCrossReferences) && (m_bInCrossRef) && (m_bInRelIndex) && (localName.compare(constrRelIndexTag, Qt::CaseInsensitive) == 0)) {
		if (m_ndxRelIndex != m_ndxRelIndexTag) {				// Add it only if the cross-reference doesn't reference itself
			m_mapCrossReference[m_ndxRelIndex].insert(m_ndxRelIndexTag);
			m_mapCrossReference[m_ndxRelIndexTag].insert(m_ndxRelIndex);
		}
		m_ndxRelIndexTag.clear();
		m_bInRelIndex = false;
	} else if ((m_bInCrossReferences) && (m_bInCrossRef) && (localName.compare(constrCrossRefTag, Qt::CaseInsensitive) == 0)) {
		m_ndxRelIndex.clear();
		m_bInCrossRef = false;
	} else if ((m_bInCrossReferences) && (localName.compare(constrCrossReferencesTag, Qt::CaseInsensitive) == 0)) {
		m_bInCrossReferences = false;
	} else if ((m_bInHighlighterDefinitions) && (m_bInHighlighterDef) && (localName.compare(constrHighlighterDefTag, Qt::CaseInsensitive) == 0)) {
		m_pUserNotesDatabaseData->m_mapHighlighterDefinitions[m_strHighlighterName].m_color.setNamedColor(m_strColor);
		m_pUserNotesDatabaseData->m_mapHighlighterDefinitions[m_strHighlighterName].m_bEnabled = m_bEnabled;
		m_strHighlighterName.clear();
		m_strColor.clear();
		m_bEnabled = true;
		m_bInHighlighterDef = false;
	} else if ((m_bInHighlighterDefinitions) && (localName.compare(constrHighlighterDefinitionsTag, Qt::CaseInsensitive) == 0)) {
		m_bInHighlighterDefinitions = false;
	} else if ((m_bInKJNDocument) && (m_bInKJNDocumentText) && (localName.compare(constrKJNDocumentTextTag, Qt::CaseInsensitive) == 0)) {
		m_bInKJNDocumentText = false;
	} else if ((m_bInKJNDocument) && (localName.compare(constrKJNDocumentTag, Qt::CaseInsensitive) == 0)) {
		m_bInKJNDocument = false;
	}

#ifdef DEBUG_KJN_XML_READ
		qDebug("/%s", localName.toUtf8().data());
#endif

	return true;
}

QString CUserNotesDatabase::errorString() const
{
	QString strErrorText = m_strLastError;
	QString strReaderError = QXmlDefaultHandler::errorString();
	if ((!strErrorText.isEmpty()) && (!strReaderError.isEmpty())) strErrorText += QChar('\n');
	strErrorText += strReaderError;
	return strErrorText;
}

// ============================================================================

bool CUserNotesDatabase::load()
{
	m_bIsDirty = true;				// Leave isDirty set until we've finished loading it
	if (m_strFilePathName.isEmpty()) {
		m_strLastError = tr("King James Notes File Path Name not set");
		return false;
	}

	QFile fileUND;

	fileUND.setFileName(m_strFilePathName);
	if (!fileUND.open(QIODevice::ReadOnly)) {
		m_strLastError = tr("Failed to open King James Notes File \"%1\" for reading.").arg(m_strFilePathName);
		return false;
	}

	if (!load(&fileUND)) {
		m_strLastError = tr("Failed to read King James Notes File \"%1\".\n\n").arg(m_strFilePathName) + m_strLastError;
		fileUND.close();
		return false;
	}

	m_strErrorFilePathName.clear();

	fileUND.close();
	return true;
}

bool CUserNotesDatabase::load(QIODevice *pIODevice)
{
	emit aboutToChangeHighlighters();
	clear();				// This will set "isDirty", which we'll leave set until we've finished loading it
	emit removedUserNote(CRelIndex());
	m_strLastError.clear();

	QtIOCompressor inUND(pIODevice);
	inUND.setStreamFormat(QtIOCompressor::ZlibFormat);
	if  (!inUND.open(QIODevice::ReadOnly)) {
		m_strLastError = tr("Failed to open the I/O compressor");
		return false;
	}

	QXmlInputSource xmlInput(&inUND);
	QXmlSimpleReader xmlReader;

	xmlReader.setContentHandler(this);
	xmlReader.setErrorHandler(this);
	xmlReader.setLexicalHandler(this);

	clearXMLVars();

	if (!xmlReader.parse(xmlInput)) {
		m_strLastError = tr("Failed to read and parse King James User Notes Database File\n\n%1").arg(errorString());
		inUND.close();
		return false;
	}

	clearXMLVars();				// Might as well clear it when we're done -- it isn't much extra memory, but...

	inUND.close();
	m_bIsDirty = false;
	m_pUserNotesDatabaseData->m_bIsDirty = false;
	emit changedUserNotesDatabase();
	emit changedHighlighters();
	emit addedUserNote(CRelIndex());
	emit changedUserNotesKeywords();
	emit changedAllCrossRefs();

	return true;
}

bool CUserNotesDatabase::save()
{
	if (m_strFilePathName.isEmpty()) {
		m_strLastError = tr("User Notes File Path Name not set");
		return false;
	}

	QFile fileUND;

	fileUND.setFileName(m_strFilePathName);

	QFileInfo fiKJN(fileUND);

	// Make backup if it's enabled:
	if ((m_bKeepBackup) && (fiKJN.exists())) {
		QFileInfo fiBackup(fiKJN.dir(), fiKJN.fileName() + m_strBackupFilenamePostfix);
		if (fiBackup.exists()) QFile::remove(fiBackup.absoluteFilePath());
		if (!QFile::copy(fiKJN.absoluteFilePath(), fiBackup.absoluteFilePath())) {
			m_strLastError = tr("Failed to create Backup File.");
			return false;
		}
	}

	if (!fileUND.open(QIODevice::WriteOnly)) {
		m_strLastError = tr("Failed to open King James Notes File \"%1\" for writing.").arg(m_strFilePathName);
		return false;
	}

	if (!save(&fileUND)) {
		m_strLastError = tr("Failed to write King James Notes File \"%1\".\n\n").arg(m_strFilePathName) + m_strLastError;
		fileUND.close();
		return false;
	}

	m_strErrorFilePathName.clear();

	fileUND.close();
	return true;
}

bool CUserNotesDatabase::save(QIODevice *pIODevice)
{
	m_strLastError.clear();

	QtIOCompressor outUND(pIODevice);
	outUND.setStreamFormat(QtIOCompressor::ZlibFormat);
	if  (!outUND.open(QIODevice::WriteOnly)) {
		m_strLastError = tr("Failed to open the I/O compressor.");
		return false;
	}

	// Write our data in XML format:
	outUND.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n\n").toUtf8());
	outUND.write(QString("<%1:%2 xmlns:%1=\"%3\" "
						 "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
						 "xsi:schemaLocation=\"%3 kjnDocument.1.0.0.xsd\">\n")
						 .arg(constrKJNPrefix)
						 .arg(constrKJNDocumentTag)
						 .arg(constrKJNNameSpaceURI)
						 .toUtf8());

	outUND.write(QString("\t<%1:%2 %3=\"%4\">\n").arg(constrKJNPrefix).arg(constrKJNDocumentTextTag)
						.arg(constrVersionAttr).arg(KJN_FILE_VERSION).toUtf8());
	m_nVersion = KJN_FILE_VERSION;

	outUND.write(QString("\t\t<%1:%2 %3=\"%4\">\n").arg(constrKJNPrefix).arg(constrNotesTag)
							.arg(constrSizeAttr).arg(m_mapNotes.size())
							.toUtf8());
	for (CUserNoteEntryMap::const_iterator itrNotes = m_mapNotes.begin(); itrNotes != m_mapNotes.end(); ++itrNotes) {
		outUND.write(QString("\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\" %7=\"%8\" %9=\"%10\"%11>\n<![CDATA[").arg(constrKJNPrefix).arg(constrNoteTag)
								.arg(constrRelIndexAttr).arg((itrNotes->first).asAnchor())
								.arg(constrCountAttr).arg((itrNotes->second).verseCount())
								.arg(constrBackgroundColorAttr).arg(Qt::escape((itrNotes->second).backgroundColor().name()))
								.arg(constrVisibleAttr).arg((itrNotes->second).isVisible() ? "True" : "False")
								.arg(((itrNotes->second).m_lstKeywords.size() != 0) ? QString(" %1=\"%2\"").arg(constrKeywordsAttr).arg(Qt::escape((itrNotes->second).m_lstKeywords.join(","))) : QString())
								.toUtf8());
		QString strNote = (itrNotes->second).text();
		strNote.replace("]]>", "]]&gt;");			// safe-guard to make sure we don't have any embedded CDATA terminators
		outUND.write(strNote.toUtf8());
		outUND.write(QString("]]>\n").toUtf8());
		outUND.write(QString("\t\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrNoteTag).toUtf8());
	}
	outUND.write(QString("\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrNotesTag).toUtf8());

	outUND.write(QString("\t\t<%1:%2 %3=\"%4\">\n").arg(constrKJNPrefix).arg(constrHighlightingTag)
							.arg(constrSizeAttr).arg(m_mapHighlighterTags.size())
							.toUtf8());
	for (TBibleDBHighlighterTagMap::const_iterator itrHighlightDB = m_mapHighlighterTags.begin(); itrHighlightDB != m_mapHighlighterTags.end(); ++itrHighlightDB) {
		const THighlighterTagMap &highlightMap(itrHighlightDB->second);
		outUND.write(QString("\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\">\n").arg(constrKJNPrefix).arg(constrHighlighterDBTag)
											.arg(constrUUIDAttr).arg(Qt::escape(itrHighlightDB->first))
											.arg(constrSizeAttr).arg(highlightMap.size())
											.toUtf8());
		for (THighlighterTagMap::const_iterator itrHighlightHL = highlightMap.begin(); itrHighlightHL != highlightMap.end(); ++itrHighlightHL) {
			const TPhraseTagList &tagList(itrHighlightHL->second);
			outUND.write(QString("\t\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\">\n").arg(constrKJNPrefix).arg(constrHighlighterTagsTag)
													.arg(constrHighlighterNameAttr).arg(Qt::escape(itrHighlightHL->first))
													.arg(constrSizeAttr).arg(tagList.size())
													.toUtf8());
			for (TPhraseTagList::const_iterator itrTags = tagList.begin(); itrTags != tagList.end(); ++itrTags) {
				outUND.write(QString("\t\t\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\" />\n").arg(constrKJNPrefix).arg(constrPhraseTagTag)
														.arg(constrRelIndexAttr).arg(itrTags->relIndex().asAnchor())
														.arg(constrCountAttr).arg(itrTags->count())
														.toUtf8());
			}
			outUND.write(QString("\t\t\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrHighlighterTagsTag).toUtf8());
		}
		outUND.write(QString("\t\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrHighlighterDBTag).toUtf8());
	}
	outUND.write(QString("\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrHighlightingTag).toUtf8());

	outUND.write(QString("\t\t<%1:%2 %3=\"%4\">\n").arg(constrKJNPrefix).arg(constrCrossReferencesTag)
							.arg(constrSizeAttr).arg(m_mapCrossReference.size())
							.toUtf8());
	for (TCrossReferenceMap::const_iterator itrCrossRef = m_mapCrossReference.begin(); itrCrossRef != m_mapCrossReference.end(); ++itrCrossRef) {
		outUND.write(QString("\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\">\n").arg(constrKJNPrefix).arg(constrCrossRefTag)
								.arg(constrRelIndexAttr).arg((itrCrossRef->first).asAnchor())
								.arg(constrSizeAttr).arg((itrCrossRef->second).size())
								.toUtf8());
		for (TRelativeIndexSet::const_iterator itrTargetRefs = (itrCrossRef->second).begin(); itrTargetRefs != (itrCrossRef->second).end(); ++itrTargetRefs) {
			outUND.write(QString("\t\t\t\t<%1:%2 %3=\"%4\" />\n").arg(constrKJNPrefix).arg(constrRelIndexTag)
									.arg(constrValueAttr).arg((*itrTargetRefs).asAnchor())
									.toUtf8());
		}
		outUND.write(QString("\t\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrCrossRefTag).toUtf8());
	}
	outUND.write(QString("\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrCrossReferencesTag).toUtf8());

	outUND.write(QString("\t\t<%1:%2 %3=\"%4\">\n").arg(constrKJNPrefix).arg(constrHighlighterDefinitionsTag)
							.arg(constrSizeAttr).arg(m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.size())
							.toUtf8());
	for (TUserDefinedColorMap::const_iterator itrHLDefs = m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.constBegin(); itrHLDefs != m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.constEnd(); ++itrHLDefs) {
		outUND.write(QString("\t\t\t<%1:%2 %3=\"%4\" %5=\"%6\" %7=\"%8\" />\n").arg(constrKJNPrefix).arg(constrHighlighterDefTag)
								.arg(constrHighlighterNameAttr).arg(Qt::escape(itrHLDefs.key()))
								.arg(constrColorAttr).arg(Qt::escape(itrHLDefs.value().m_color.name()))
								.arg(constrEnabledAttr).arg(itrHLDefs.value().m_bEnabled ? "True" : "False")
								.toUtf8());
	}
	outUND.write(QString("\t\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrHighlighterDefinitionsTag).toUtf8());

	outUND.write(QString("\t</%1:%2>\n").arg(constrKJNPrefix).arg(constrKJNDocumentTextTag).toUtf8());

	outUND.write(QString("</%1:%2>\n").arg(constrKJNPrefix).arg(constrKJNDocumentTag).toUtf8());

	outUND.close();
	m_bIsDirty = false;
	m_pUserNotesDatabaseData->m_bIsDirty = false;

	return true;
}

// ============================================================================

void CUserNotesDatabase::setNoteFor(const CRelIndex &ndx, const CUserNoteEntry &strNote)
{
	bool bExists = existsNoteFor(ndx);
	QStringList lstKeywords;
	if (bExists) {
		lstKeywords = m_mapNotes.at(ndx).keywordList();
	}
	m_mapNotes[ndx] = strNote;
	m_mapNotes[ndx].setPassageTag(ndx, strNote.verseCount());
	m_bIsDirty = true;
	if (!bExists) {
		emit addedUserNote(ndx);
		emit changedUserNotesKeywords();
	} else {
		emit changedUserNote(ndx);
		if (lstKeywords != strNote.keywordList()) emit changedUserNotesKeywords();
	}
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::removeNoteFor(const CRelIndex &ndx)
{
	bool bExists = existsNoteFor(ndx);
	if (!bExists) return;
	m_mapNotes.erase(ndx);
	m_bIsDirty = true;
	emit removedUserNote(ndx);
	emit changedUserNotesKeywords();
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::removeAllNotes()
{
	m_mapNotes.clear();
	m_bIsDirty = true;
	emit removedUserNote(CRelIndex());
	emit changedUserNotesKeywords();
	emit changedUserNotesDatabase();
}

QStringList CUserNotesDatabase::compositeKeywordList() const
{
	QStringList lstKeywords;

	for (CUserNoteEntryMap::const_iterator itrNotes = m_mapNotes.begin(); itrNotes != m_mapNotes.end(); ++itrNotes) {
		lstKeywords.append((itrNotes->second).m_lstKeywords);
	}

	lstKeywords.removeDuplicates();
	lstKeywords.removeOne(QString());
	return lstKeywords;
}

// ============================================================================

void CUserNotesDatabase::setHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags)
{
	assert(pBibleDatabase != NULL);
	const QString strUUID = pBibleDatabase->compatibilityUUID();
	assert(!strUUID.isEmpty());
	assert(!strUserDefinedHighlighterName.isEmpty());
	if ((strUUID.isEmpty()) || (strUserDefinedHighlighterName.isEmpty())) return;

	emit highlighterTagsAboutToChange(pBibleDatabase, strUserDefinedHighlighterName);
	(m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName] = lstTags;
	qSort((m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].begin(), (m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].end(), TPhraseTagListSortPredicate::ascendingLessThan);
	m_bIsDirty = true;
	emit highlighterTagsChanged(pBibleDatabase, strUserDefinedHighlighterName);
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::appendHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTagList &lstTags)
{
	assert(pBibleDatabase != NULL);
	const QString strUUID = pBibleDatabase->compatibilityUUID();
	assert(!strUUID.isEmpty());
	assert(!strUserDefinedHighlighterName.isEmpty());
	if ((strUUID.isEmpty()) || (strUserDefinedHighlighterName.isEmpty())) return;

	emit highlighterTagsAboutToChange(pBibleDatabase, strUserDefinedHighlighterName);
	for (TPhraseTagList::const_iterator itrTags = lstTags.constBegin(); itrTags != lstTags.constEnd(); ++itrTags) {
		(m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].intersectingInsert(pBibleDatabase, *itrTags);
	}
	qSort((m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].begin(), (m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].end(), TPhraseTagListSortPredicate::ascendingLessThan);
	m_bIsDirty = true;
	emit highlighterTagsChanged(pBibleDatabase, strUserDefinedHighlighterName);
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::appendHighlighterTagFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag)
{
	assert(pBibleDatabase != NULL);
	const QString strUUID = pBibleDatabase->compatibilityUUID();
	assert(!strUUID.isEmpty());
	assert(!strUserDefinedHighlighterName.isEmpty());
	if ((strUUID.isEmpty()) || (strUserDefinedHighlighterName.isEmpty())) return;

	emit highlighterTagsAboutToChange(pBibleDatabase, strUserDefinedHighlighterName);
	(m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].intersectingInsert(pBibleDatabase, aTag);
	qSort((m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].begin(), (m_mapHighlighterTags[strUUID])[strUserDefinedHighlighterName].end(), TPhraseTagListSortPredicate::ascendingLessThan);
	m_bIsDirty = true;
	emit highlighterTagsChanged(pBibleDatabase, strUserDefinedHighlighterName);
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::removeHighlighterTagFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName, const TPhraseTag &aTag)
{
	assert(pBibleDatabase != NULL);
	const QString strUUID = pBibleDatabase->compatibilityUUID();

	TBibleDBHighlighterTagMap::iterator itrBibleDB = m_mapHighlighterTags.find(strUUID);
	if (itrBibleDB == m_mapHighlighterTags.end()) return;
	THighlighterTagMap::iterator itrTags = (itrBibleDB->second).find(strUserDefinedHighlighterName);
	if (itrTags == (itrBibleDB->second).end()) return;
	emit highlighterTagsAboutToChange(pBibleDatabase, strUserDefinedHighlighterName);
	(itrTags->second).removeIntersection(pBibleDatabase, aTag);
	if ((itrTags->second).empty()) (itrBibleDB->second).erase(itrTags);
	if ((itrBibleDB->second).empty()) m_mapHighlighterTags.erase(itrBibleDB);
	m_bIsDirty = true;
	emit highlighterTagsChanged(pBibleDatabase, strUserDefinedHighlighterName);
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::removeHighlighterTagsFor(CBibleDatabasePtr pBibleDatabase, const QString &strUserDefinedHighlighterName)
{
	assert(pBibleDatabase != NULL);
	const QString strUUID = pBibleDatabase->compatibilityUUID();
	assert(!strUUID.isEmpty());
	if (strUUID.isEmpty()) return;

	if (strUserDefinedHighlighterName.isEmpty()) {
		if (highlighterTagsFor(pBibleDatabase) == NULL) return;				// Return if it doesn't exist so we don't set dirty flag
		emit highlighterTagsAboutToChange(pBibleDatabase, strUserDefinedHighlighterName);
		m_mapHighlighterTags.erase(strUUID);
	} else {
		TBibleDBHighlighterTagMap::iterator itr = m_mapHighlighterTags.find(strUUID);
		if (itr == m_mapHighlighterTags.end()) return;
		if (highlighterTagsFor(pBibleDatabase, strUserDefinedHighlighterName) == NULL) return;		// Return if it doesn't exist so we don't set dirty flag
		emit highlighterTagsAboutToChange(pBibleDatabase, strUserDefinedHighlighterName);
		(itr->second).erase(strUserDefinedHighlighterName);
	}
	m_bIsDirty = true;
	emit highlighterTagsChanged(pBibleDatabase, strUserDefinedHighlighterName);
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::removeAllHighlighterTags()
{
	emit highlighterTagsAboutToChange(CBibleDatabasePtr(), QString());
	m_mapHighlighterTags.clear();
	m_bIsDirty = true;
	emit highlighterTagsChanged(CBibleDatabasePtr(), QString());
	emit changedUserNotesDatabase();
}

// ============================================================================

bool CUserNotesDatabase::setCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond)
{
	if (ndxFirst == ndxSecond) return false;							// Don't allow cross references to ourselves (that's just stupid, and can lead to weird consequences)

	// Since they are cross-linked, it doesn't matter which way we do this check:
	TRelativeIndexSet sCrossRef = crossReferencesFor(ndxFirst);
	if (sCrossRef.find(ndxSecond) != sCrossRef.end()) return false;		// Don't add it if it already exists

	m_mapCrossReference[ndxFirst].insert(ndxSecond);
	m_mapCrossReference[ndxSecond].insert(ndxFirst);
	m_bIsDirty = true;
	emit addedCrossRef(ndxFirst, ndxSecond);
	emit changedUserNotesDatabase();
	return true;
}

bool CUserNotesDatabase::removeCrossReference(const CRelIndex &ndxFirst, const CRelIndex &ndxSecond)
{
	assert(ndxFirst != ndxSecond);
	if (ndxFirst == ndxSecond) return false;
	TCrossReferenceMap::iterator itrMapFirst = m_mapCrossReference.find(ndxFirst);
	TCrossReferenceMap::iterator itrMapSecond = m_mapCrossReference.find(ndxSecond);
	if ((itrMapFirst == m_mapCrossReference.end()) || (itrMapSecond == m_mapCrossReference.end())) return false;

	// Remove the cross-reference entries from each other:
	(itrMapFirst->second).erase(ndxSecond);
	(itrMapSecond->second).erase(ndxFirst);

	// Remove mappings that become empty:
	if ((itrMapFirst->second).empty()) m_mapCrossReference.erase(ndxFirst);
	if ((itrMapSecond->second).empty()) m_mapCrossReference.erase(ndxSecond);

	m_bIsDirty = true;
	emit removedCrossRef(ndxFirst, ndxSecond);
	emit changedUserNotesDatabase();
	return true;
}

bool CUserNotesDatabase::removeCrossReferencesFor(const CRelIndex &ndx)
{
	TCrossReferenceMap::iterator itrMap = m_mapCrossReference.find(ndx);
	if (itrMap == m_mapCrossReference.end()) return false;

	for (TRelativeIndexSet::iterator itrSet = (itrMap->second).begin(); itrSet != (itrMap->second).end(); ++itrSet) {
		assert(*itrSet != ndx);		// Shouldn't have any cross references to our same index, as we didn't allow them to be added
		if (*itrSet == ndx) continue;
		m_mapCrossReference[*itrSet].erase(ndx);			// Remove all cross references of other indexes to this index
		if (m_mapCrossReference[*itrSet].empty()) m_mapCrossReference.erase(*itrSet);			// Remove any mappings that become empty
	}
	m_mapCrossReference.erase(ndx);		// Now, remove this index mapping to other indexes

	m_bIsDirty = true;
	emit changedAllCrossRefs();				// TODO : Once we get better support for individual changes in the model, replace this with individual remove calls during erases above
	emit changedUserNotesDatabase();
	return true;
}

void CUserNotesDatabase::removeAllCrossReferences()
{
	m_mapCrossReference.clear();
	m_bIsDirty = true;
	emit changedAllCrossRefs();
	emit changedUserNotesDatabase();
}

void CUserNotesDatabase::setCrossRefsMap(const TCrossReferenceMap &mapCrossRefs)
{
	m_mapCrossReference = mapCrossRefs;
	m_bIsDirty = true;
	emit changedAllCrossRefs();
	emit changedUserNotesDatabase();
}

// ============================================================================

void CUserNotesDatabase::setHighlighterColor(const QString &strUserDefinedHighlighterName, const QColor &color)
{
	QColor colorOriginal = m_pUserNotesDatabaseData->m_mapHighlighterDefinitions[strUserDefinedHighlighterName].m_color;
	if (colorOriginal != color) {
		emit aboutToChangeHighlighters();
		m_pUserNotesDatabaseData->m_mapHighlighterDefinitions[strUserDefinedHighlighterName].m_color = color;
		emit changedHighlighter(strUserDefinedHighlighterName);
		emit changedHighlighters();
		m_pUserNotesDatabaseData->m_bIsDirty = true;
	}
}

void CUserNotesDatabase::setHighlighterEnabled(const QString &strUserDefinedHighlighterName, bool bEnabled)
{
	bool bEnabledOriginal = m_pUserNotesDatabaseData->m_mapHighlighterDefinitions[strUserDefinedHighlighterName].m_bEnabled;
	if (bEnabledOriginal != bEnabled) {
		emit aboutToChangeHighlighters();
		m_pUserNotesDatabaseData->m_mapHighlighterDefinitions[strUserDefinedHighlighterName].m_bEnabled = bEnabled;
		emit changedHighlighter(strUserDefinedHighlighterName);
		emit changedHighlighters();
		m_pUserNotesDatabaseData->m_bIsDirty = true;
	}
}

bool CUserNotesDatabase::renameHighlighter(const QString &strOldUserDefinedHighlighterName, const QString &strNewUserDefinedHighlighterName)
{
	if (!existsHighlighter(strOldUserDefinedHighlighterName)) return false;
	if (existsHighlighter(strNewUserDefinedHighlighterName)) return false;

	for (TBibleDBHighlighterTagMap::const_iterator itrDB = m_mapHighlighterTags.begin(); itrDB != m_mapHighlighterTags.end(); ++itrDB) {
		if (highlighterTagsFor(itrDB->first, strOldUserDefinedHighlighterName)) return false;
	}

	emit aboutToChangeHighlighters();
	m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.insert(strNewUserDefinedHighlighterName, m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.value(strOldUserDefinedHighlighterName));
	m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.remove(strOldUserDefinedHighlighterName);
	emit changedHighlighter(strNewUserDefinedHighlighterName);
	emit removedHighlighter(strOldUserDefinedHighlighterName);
	emit changedHighlighters();
	m_pUserNotesDatabaseData->m_bIsDirty = true;

	return true;
}

void CUserNotesDatabase::removeHighlighter(const QString &strUserDefinedHighlighterName)
{
	if (existsHighlighter(strUserDefinedHighlighterName)) {
		emit aboutToChangeHighlighters();
		m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.remove(strUserDefinedHighlighterName);
		emit removedHighlighter(strUserDefinedHighlighterName);
		emit changedHighlighters();
		m_pUserNotesDatabaseData->m_bIsDirty = true;
	}
}

void CUserNotesDatabase::removeAllHighlighters()
{
	if (m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.size()) {
		emit aboutToChangeHighlighters();
		m_pUserNotesDatabaseData->m_mapHighlighterDefinitions.clear();
		emit changedHighlighters();
		m_pUserNotesDatabaseData->m_bIsDirty = true;
	}
}

void CUserNotesDatabase::toggleUserNotesDatabaseData(bool bCopy)
{
	TUserNotesDatabaseData *pSource = ((m_pUserNotesDatabaseData == &m_UserNotesDatabaseData1) ? &m_UserNotesDatabaseData1 : &m_UserNotesDatabaseData2);
	TUserNotesDatabaseData *pTarget = ((m_pUserNotesDatabaseData == &m_UserNotesDatabaseData1) ? &m_UserNotesDatabaseData2 : &m_UserNotesDatabaseData1);

	// Signal changes if we aren't copying and something changed:
	if (!bCopy) {
		if (pSource->m_mapHighlighterDefinitions != pTarget->m_mapHighlighterDefinitions) emit aboutToChangeHighlighters();
	}

	if (bCopy) *pTarget = *pSource;

	m_pUserNotesDatabaseData = pTarget;

	// Signal changes if we aren't copying and something changed:
	if (!bCopy) {
		if (pSource->m_mapHighlighterDefinitions != pTarget->m_mapHighlighterDefinitions) emit changedHighlighters();
	}
}
