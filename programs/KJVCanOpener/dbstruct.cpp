/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

// dbstruct.cpp -- Defines structures used for database info
//

#include "dbstruct.h"
#include "VerseRichifier.h"
#include "SearchCompleter.h"
#include "PhraseEdit.h"
#include "ScriptureDocument.h"
#include "ReadDB.h"
#include "ReportError.h"
#include "PersistentSettings.h"
#include "BusyCursor.h"

#include <QtAlgorithms>
#include <QSet>
#include <QObject>
#include <iterator>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>

#ifdef USING_WEBCHANNEL
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#endif

#include <assert.h>

// ============================================================================

TBibleDatabaseList::TBibleDatabaseList(QObject *pParent)
	:	QObject(pParent),
		m_bHaveSearchedAvailableDatabases(false)
{
	// This one should be a Direct connection so that we update the database words immediately before users get updated:
	connect(CPersistentSettings::instance(), SIGNAL(changedBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &)), this, SLOT(en_changedBibleDatabaseSettings(const QString &, const TBibleDatabaseSettings &)), Qt::DirectConnection);
}

TBibleDatabaseList::~TBibleDatabaseList()
{

}

TBibleDatabaseList *TBibleDatabaseList::instance()
{
	static TBibleDatabaseList theBibleDatabaseList;
	return &theBibleDatabaseList;
}

#ifdef USING_WEBCHANNEL
QString TBibleDatabaseList::availableBibleDatabasesAsJson()
{
	QJsonArray arrBibleList;
	QStringList lstAvailableDatabases = TBibleDatabaseList::instance()->availableBibleDatabasesUUIDs();
	QString strUUIDDefault;
	CBibleDatabasePtr pDefaultBible = TBibleDatabaseList::instance()->atUUID(QString());
	if (!pDefaultBible.isNull()) strUUIDDefault = pDefaultBible->compatibilityUUID();
	for (int ndx = 0; ndx < lstAvailableDatabases.size(); ++ndx) {
		QJsonObject objBible;
		objBible["id"] = lstAvailableDatabases.at(ndx);
		objBible["isDefault"] = ((strUUIDDefault.compare(lstAvailableDatabases.at(ndx), Qt::CaseInsensitive) == 0) ? true : false);
		CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->atUUID(lstAvailableDatabases.at(ndx));
		if (!pBibleDatabase.isNull()) {
			objBible["name"] = pBibleDatabase->description();
		} else {
			BIBLE_DESCRIPTOR_ENUM nBDE = bibleDescriptorFromUUID(lstAvailableDatabases.at(ndx));
			assert(nBDE != BDE_UNKNOWN);
			const TBibleDescriptor &bblDesc = bibleDescriptor(nBDE);
			objBible["name"] = bblDesc.m_strDBDesc;
		}
		arrBibleList.append(objBible);
	}

	return QJsonDocument(arrBibleList).toJson(QJsonDocument::Compact);
}
#endif

bool TBibleDatabaseList::loadBibleDatabase(BIBLE_DESCRIPTOR_ENUM nBibleDB, bool bAutoSetAsMain, QWidget *pParent)
{
	if (nBibleDB == BDE_UNKNOWN) return false;
	const TBibleDescriptor &bblDesc = bibleDescriptor(nBibleDB);
	CBusyCursor iAmBusy(NULL);
	CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath, pParent);
	if ((!rdbMain.haveBibleDatabaseFiles(bblDesc)) || (!rdbMain.ReadBibleDatabase(bblDesc, (bAutoSetAsMain && !TBibleDatabaseList::instance()->haveMainBibleDatabase())))) {
		iAmBusy.earlyRestore();
		displayWarning(pParent, tr("Load Bible Database", "Errors"), tr("Failed to Read and Validate Bible Database!\n%1\nCheck Installation!", "Errors").arg(bblDesc.m_strDBDesc));
		return false;
	}
	return true;
}

bool TBibleDatabaseList::loadBibleDatabase(const QString &strUUID, bool bAutoSetAsMain, QWidget *pParent)
{
	return loadBibleDatabase(bibleDescriptorFromUUID(strUUID), bAutoSetAsMain, pParent);
}

void TBibleDatabaseList::setMainBibleDatabase(const QString &strUUID)
{
	QString strOldUUID = ((!m_pMainBibleDatabase.isNull()) ? m_pMainBibleDatabase->compatibilityUUID() : QString());
	CBibleDatabasePtr pBibleDatabase = atUUID(strUUID);
	if (!pBibleDatabase.isNull()) {
		m_pMainBibleDatabase = pBibleDatabase;
		if (strOldUUID.compare(strUUID, Qt::CaseInsensitive) != 0) emit changedMainBibleDatabase(pBibleDatabase);
	}
}

void TBibleDatabaseList::removeBibleDatabase(const QString &strUUID)
{
	for (int ndx = 0; ndx < size(); ++ndx) {
		CBibleDatabasePtr pBibleDatabase = at(ndx);
		if (pBibleDatabase->compatibilityUUID().compare(strUUID, Qt::CaseInsensitive) == 0) {
			removeAt(ndx);
			if (m_pMainBibleDatabase == pBibleDatabase) {
				assert(false);				// Shouldn't allow removing of MainBibleDatabase -- call setMainBibleDatabase first
			}
			emit removeBibleDatabase(pBibleDatabase->compatibilityUUID());
			emit changedBibleDatabaseList();
		}
	}
}

void TBibleDatabaseList::clear()
{
	for (int ndx = size()-1; ndx >= 0; --ndx) {
		emit removingBibleDatabase(at(ndx));
	}
	QList<CBibleDatabasePtr>::clear();
	m_pMainBibleDatabase.clear();
	emit changedMainBibleDatabase(CBibleDatabasePtr());
	emit changedBibleDatabaseList();
}

CBibleDatabasePtr TBibleDatabaseList::atUUID(const QString &strUUID) const
{
	QString strTargetUUID = strUUID;

	if (strTargetUUID.isEmpty()) {
		// Default database is KJV
		strTargetUUID = bibleDescriptor(BDE_KJV).m_strUUID;
	}

	for (int ndx = 0; ndx < size(); ++ndx) {
		if (at(ndx)->compatibilityUUID().compare(strTargetUUID, Qt::CaseInsensitive) == 0)
			return at(ndx);
	}

	return CBibleDatabasePtr();
}

QList<BIBLE_DESCRIPTOR_ENUM> TBibleDatabaseList::availableBibleDatabases()
{
	if (!m_bHaveSearchedAvailableDatabases) findBibleDatabases();
	return m_lstAvailableDatabases;
}

QStringList TBibleDatabaseList::availableBibleDatabasesUUIDs()
{
	QStringList lstUUIDs;

	if (!m_bHaveSearchedAvailableDatabases) findBibleDatabases();

	lstUUIDs.reserve(m_lstAvailableDatabases.size());
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		lstUUIDs.append(bibleDescriptor(m_lstAvailableDatabases.at(ndx)).m_strUUID);
	}

	return lstUUIDs;
}

static QStringList languageList()
{
	QStringList lstLanguages;
	lstLanguages.append("en");
	lstLanguages.append("es");
	lstLanguages.append("fr");
	lstLanguages.append("de");
	return lstLanguages;
}

static int languageIndex(const QString &strLanguage)
{
	static QStringList lstLanguages = languageList();

	int nIndex = lstLanguages.indexOf(strLanguage, Qt::CaseInsensitive);
	return ((nIndex != -1) ? nIndex : lstLanguages.size());
}

static QList<BIBLE_DESCRIPTOR_ENUM> BDElist()
{
	QList<BIBLE_DESCRIPTOR_ENUM> lstBDE;
	lstBDE.append(BDE_KJV);
	lstBDE.append(BDE_KJVA);
	lstBDE.append(BDE_KJVPCE);
	lstBDE.append(BDE_KJV1611);
	lstBDE.append(BDE_KJV1611A);
	lstBDE.append(BDE_UKJV);
	return lstBDE;
}

static int BDEIndex(BIBLE_DESCRIPTOR_ENUM nBDE)
{
	static QList<BIBLE_DESCRIPTOR_ENUM> lstBDE = BDElist();

	int nIndex = lstBDE.indexOf(nBDE);
	return ((nIndex != -1) ? nIndex : lstBDE.size());
}

void TBibleDatabaseList::findBibleDatabases()
{
	m_lstAvailableDatabases.clear();
	for (unsigned int dbNdx = 0; dbNdx < bibleDescriptorCount(); ++dbNdx) {
		const TBibleDescriptor &bblDesc = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
		CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath);
		if (!rdbMain.haveBibleDatabaseFiles(bblDesc)) continue;
		// Sort the list as we insert them:
		int nInsertPoint = 0;
		while (nInsertPoint < m_lstAvailableDatabases.size()) {
			BIBLE_DESCRIPTOR_ENUM nBDE = static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx);
			// Sort by Specific decriptro ID, language, then by description, then by general descriptor ID:
			int nBIndex1 = BDEIndex(nBDE);
			int nBIndex2 = BDEIndex(m_lstAvailableDatabases.at(nInsertPoint));
			int nLIndex1 = languageIndex(bblDesc.m_strLanguage);
			int nLIndex2 = languageIndex(bibleDescriptor(m_lstAvailableDatabases.at(nInsertPoint)).m_strLanguage);
			int nBDEComp = ((nBIndex1 < nBIndex2) ? -1 : ((nBIndex2 < nBIndex1) ? 1 : 0));
			int nLangComp =  ((nLIndex1 < nLIndex2) ? -1 : ((nLIndex2 < nLIndex1) ? 1 : 0));
			int nDescComp = CSearchStringListModel::decompose(bblDesc.m_strDBDesc, true).compare(CSearchStringListModel::decompose(bibleDescriptor(m_lstAvailableDatabases.at(nInsertPoint)).m_strDBDesc, true), Qt::CaseInsensitive);
			if ((nBDEComp < 0) ||
				((nBDEComp == 0) && (nLangComp < 0)) ||
				((nBDEComp == 0) && (nLangComp == 0) && (nDescComp < 0)) ||
				((nBDEComp == 0) && (nLangComp == 0) && (nDescComp == 0) && (nBDE < m_lstAvailableDatabases.at(nInsertPoint)))) break;
			++nInsertPoint;
		}
		m_lstAvailableDatabases.insert(nInsertPoint, static_cast<BIBLE_DESCRIPTOR_ENUM>(dbNdx));
	}
	m_bHaveSearchedAvailableDatabases = true;
	emit changedAvailableBibleDatabaseList();
}

void TBibleDatabaseList::addBibleDatabase(CBibleDatabasePtr pBibleDatabase, bool bSetAsMain)
{
	assert(!pBibleDatabase.isNull());
	push_back(pBibleDatabase);
	if (bSetAsMain) {
		CBibleDatabasePtr pOldMain = m_pMainBibleDatabase;
		m_pMainBibleDatabase = pBibleDatabase;
		if (pOldMain != m_pMainBibleDatabase) emit changedMainBibleDatabase(pBibleDatabase);
	}
	emit loadedBibleDatabase(pBibleDatabase);
	emit changedBibleDatabaseList();
}

void TBibleDatabaseList::en_changedBibleDatabaseSettings(const QString &strUUID, const TBibleDatabaseSettings &aSettings)
{
	Q_UNUSED(aSettings);

	CBibleDatabasePtr pBibleDatabase = atUUID(strUUID);
	if (!pBibleDatabase.isNull()) {
		pBibleDatabase->setRenderedWords();
	}
}

// ============================================================================

TDictionaryDatabaseList::TDictionaryDatabaseList(QObject *pParent)
	:	QObject(pParent),
		m_bHaveSearchedAvailableDatabases(false)
{

}

TDictionaryDatabaseList::~TDictionaryDatabaseList()
{

}

TDictionaryDatabaseList *TDictionaryDatabaseList::instance()
{
	static TDictionaryDatabaseList theDictionaryDatabaseList;
	return &theDictionaryDatabaseList;
}

CDictionaryDatabasePtr TDictionaryDatabaseList::locateAndLoadDictionary(const QString &strLanguage, QWidget *pParentWidget)
{
	// Try Loaded Main Dictionary first:
	CDictionaryDatabasePtr pMainDictDatabase = TDictionaryDatabaseList::instance()->mainDictionaryDatabase();
	if (!pMainDictDatabase.isNull()) {
		if ((strLanguage.isEmpty()) || (pMainDictDatabase->language().compare(strLanguage, Qt::CaseInsensitive) == 0) ||
			(pMainDictDatabase->flags() & DTO_IgnoreLang)) return pMainDictDatabase;
	}

	// Try Selected Main Dictionary second (if it isn't the same as main):
	CDictionaryDatabasePtr pDictDatabase;
	QString strUUIDSelMain = CPersistentSettings::instance()->mainDictDatabaseUUID();
	if (!strUUIDSelMain.isEmpty()) {
		if ((pMainDictDatabase.isNull()) ||
			((!pMainDictDatabase.isNull()) && (pMainDictDatabase->compatibilityUUID().compare(strUUIDSelMain, Qt::CaseInsensitive) != 0))) {
			if ((strLanguage.isEmpty()) ||
				(dictionaryDescriptor(dictionaryDescriptorFromUUID(strUUIDSelMain)).m_strLanguage.compare(strLanguage, Qt::CaseInsensitive) == 0) ||
				(dictionaryDescriptor(dictionaryDescriptorFromUUID(strUUIDSelMain)).m_dtoFlags & DTO_IgnoreLang)) {
				pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(strUUIDSelMain);
				if (!pDictDatabase.isNull()) {
					return pDictDatabase;
				} else {
#ifndef ENABLE_ONLY_LOADED_DICTIONARY_DATABASES
					if (TDictionaryDatabaseList::loadDictionaryDatabase(strUUIDSelMain, false, pParentWidget)) {
						pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(strUUIDSelMain);
						assert(!pDictDatabase.isNull());
						if (!pDictDatabase.isNull()) return pDictDatabase;
					}
				}
#endif
			}
		}
	}

	QStringList lstAvailableUUIDs = TDictionaryDatabaseList::instance()->availableDictionaryDatabasesUUIDs();

	// Loaded dictionaries have precedence:
	for (int ndx = 0; ndx < lstAvailableUUIDs.size(); ++ndx) {
		pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(lstAvailableUUIDs.at(ndx));
		if (!pDictDatabase.isNull()) {
			if ((strLanguage.isEmpty()) || (pDictDatabase->language().compare(strLanguage, Qt::CaseInsensitive) == 0) ||
				(pDictDatabase->flags() & DTO_IgnoreLang)) return pDictDatabase;
		}
	}

	// Try to find one that isn't loaded if we're allowed to:
#ifndef ENABLE_ONLY_LOADED_DICTIONARY_DATABASES
	for (int ndx = 0; ndx < lstAvailableUUIDs.size(); ++ndx) {
		if (!TDictionaryDatabaseList::instance()->atUUID(lstAvailableUUIDs.at(ndx)).isNull()) continue;
		if ((strLanguage.isEmpty()) ||
			(dictionaryDescriptor(dictionaryDescriptorFromUUID(lstAvailableUUIDs.at(ndx))).m_strLanguage.compare(strLanguage, Qt::CaseInsensitive) == 0) ||
			(dictionaryDescriptor(dictionaryDescriptorFromUUID(lstAvailableUUIDs.at(ndx))).m_dtoFlags & DTO_IgnoreLang)) {
			if (TDictionaryDatabaseList::loadDictionaryDatabase(lstAvailableUUIDs.at(ndx), false, pParentWidget)) {
				pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(lstAvailableUUIDs.at(ndx));
				assert(!pDictDatabase.isNull());
				if (!pDictDatabase.isNull()) return pDictDatabase;
			}
		}
	}
#endif

	return CDictionaryDatabasePtr();
}

bool TDictionaryDatabaseList::loadDictionaryDatabase(DICTIONARY_DESCRIPTOR_ENUM nDictDB, bool bAutoSetAsMain, QWidget *pParent)
{
	if (nDictDB == DDE_UNKNOWN) return false;
	const TDictionaryDescriptor &dctDesc = dictionaryDescriptor(nDictDB);
	CBusyCursor iAmBusy(NULL);
	CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath, pParent);
	if ((!rdbMain.haveDictionaryDatabaseFiles(dctDesc)) || (!rdbMain.ReadDictionaryDatabase(dctDesc, (bAutoSetAsMain && !TDictionaryDatabaseList::instance()->haveMainDictionaryDatabase())))) {
		iAmBusy.earlyRestore();
		displayWarning(pParent, tr("Load Dictionary Database", "Errors"), tr("Failed to Read and Validate Dictionary Database!\n%1\nCheck Installation!", "Errors").arg(dctDesc.m_strDBDesc));
		return false;
	}
	return true;
}

bool TDictionaryDatabaseList::loadDictionaryDatabase(const QString &strUUID, bool bAutoSetAsMain, QWidget *pParent)
{
	return loadDictionaryDatabase(dictionaryDescriptorFromUUID(strUUID), bAutoSetAsMain, pParent);
}

void TDictionaryDatabaseList::setMainDictionaryDatabase(const QString &strUUID)
{
	QString strOldUUID = ((!m_pMainDictionaryDatabase.isNull()) ? m_pMainDictionaryDatabase->compatibilityUUID() : QString());
	CDictionaryDatabasePtr pDictionaryDatabase = atUUID(strUUID);
	if (!pDictionaryDatabase.isNull()) {
		m_pMainDictionaryDatabase = pDictionaryDatabase;
		if (strOldUUID.compare(strUUID, Qt::CaseInsensitive) != 0) emit changedMainDictionaryDatabase(pDictionaryDatabase);
	}
}

void TDictionaryDatabaseList::removeDictionaryDatabase(const QString &strUUID)
{
	for (int ndx = 0; ndx < size(); ++ndx) {
		CDictionaryDatabasePtr pDictionaryDatabase = at(ndx);
		if (pDictionaryDatabase->compatibilityUUID().compare(strUUID, Qt::CaseInsensitive) == 0) {
			removeAt(ndx);
			if (m_pMainDictionaryDatabase == pDictionaryDatabase) {
				assert(false);				// Shouldn't allow removing of MainDictionaryDatabase -- call setMainDictionaryDatabase first
			}
			emit removeDictionaryDatabase(pDictionaryDatabase->compatibilityUUID());
			emit changedDictionaryDatabaseList();
		}
	}
}

void TDictionaryDatabaseList::clear()
{
	for (int ndx = size()-1; ndx >= 0; --ndx) {
		emit removingDictionaryDatabase(at(ndx));
	}
	QList<CDictionaryDatabasePtr>::clear();
	m_pMainDictionaryDatabase.clear();
	emit changedMainDictionaryDatabase(CDictionaryDatabasePtr());
	emit changedDictionaryDatabaseList();
}

CDictionaryDatabasePtr TDictionaryDatabaseList::atUUID(const QString &strUUID) const
{
	QString strTargetUUID = strUUID;

	if (strTargetUUID.isEmpty()) {
		// Default database is WEB1828
		strTargetUUID = dictionaryDescriptor(DDE_WEB1828).m_strUUID;
	}

	for (int ndx = 0; ndx < size(); ++ndx) {
		if (at(ndx)->compatibilityUUID().compare(strTargetUUID, Qt::CaseInsensitive) == 0)
			return at(ndx);
	}

	return CDictionaryDatabasePtr();
}

QList<DICTIONARY_DESCRIPTOR_ENUM> TDictionaryDatabaseList::availableDictionaryDatabases()
{
	if (!m_bHaveSearchedAvailableDatabases) findDictionaryDatabases();
	return m_lstAvailableDatabases;
}

QStringList TDictionaryDatabaseList::availableDictionaryDatabasesUUIDs()
{
	QStringList lstUUIDs;

	if (!m_bHaveSearchedAvailableDatabases) findDictionaryDatabases();

	lstUUIDs.reserve(m_lstAvailableDatabases.size());
	for (int ndx = 0; ndx < m_lstAvailableDatabases.size(); ++ndx) {
		lstUUIDs.append(dictionaryDescriptor(m_lstAvailableDatabases.at(ndx)).m_strUUID);
	}

	return lstUUIDs;
}

void TDictionaryDatabaseList::findDictionaryDatabases()
{
	m_lstAvailableDatabases.clear();
	for (unsigned int dbNdx = 0; dbNdx < dictionaryDescriptorCount(); ++dbNdx) {
		const TDictionaryDescriptor &dictDesc = dictionaryDescriptor(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(dbNdx));
		CReadDatabase rdbMain(g_strBibleDatabasePath, g_strDictionaryDatabasePath);
		if (!rdbMain.haveDictionaryDatabaseFiles(dictDesc)) continue;
		m_lstAvailableDatabases.append(static_cast<DICTIONARY_DESCRIPTOR_ENUM>(dbNdx));
	}
	m_bHaveSearchedAvailableDatabases = true;
	emit changedAvailableDictionaryDatabaseList();
}

void TDictionaryDatabaseList::addDictionaryDatabase(CDictionaryDatabasePtr pDictionaryDatabase, bool bSetAsMain)
{
	assert(!pDictionaryDatabase.isNull());
	push_back(pDictionaryDatabase);
	if (bSetAsMain) {
		CDictionaryDatabasePtr pOldMain = m_pMainDictionaryDatabase;
		m_pMainDictionaryDatabase = pDictionaryDatabase;
		if (pOldMain != m_pMainDictionaryDatabase) emit changedMainDictionaryDatabase(pDictionaryDatabase);
	}
	emit loadedDictionaryDatabase(pDictionaryDatabase);
	emit changedDictionaryDatabaseList();
}

// ============================================================================

CPhraseEntry::CPhraseEntry(const QString &strEncodedText, const QVariant &varExtraInfo)
	:	m_bCaseSensitive(false),
		m_bAccentSensitive(false),
		m_bExclude(false),
		m_bDisabled(false),
		m_varExtraInfo(varExtraInfo)
{
	setTextEncoded(strEncodedText);
}

CPhraseEntry::~CPhraseEntry()
{

}

void CPhraseEntry::clear()
{
	m_strPhrase.clear();
	m_bCaseSensitive = false;
	m_bAccentSensitive = false;
	m_bExclude = false;
	m_bDisabled = false;
	m_varExtraInfo.clear();
}

void CPhraseEntry::setFromPhrase(const CParsedPhrase *pPhrase)
{
	assert(pPhrase != NULL);
	if (pPhrase == NULL) return;

	clear();
	m_strPhrase = pPhrase->phrase();
	m_bCaseSensitive = pPhrase->isCaseSensitive();
	m_bAccentSensitive = pPhrase->isAccentSensitive();
	m_bExclude = pPhrase->isExcluded();
	m_bDisabled = pPhrase->isDisabled();
}

QString CPhraseEntry::textEncoded() const
{
	QString strText;

	// The order here matters as we will always read/write the special flags in order
	//		so we don't need a complete parser to allow any arbitrary order:
	if (isDisabled()) strText += encCharDisabled();
	if (isExcluded()) strText += encCharExclude();
	if (accentSensitive()) strText += encCharAccentSensitive();
	if (caseSensitive()) strText += encCharCaseSensitive();
	strText += m_strPhrase;

	return strText;
}

void CPhraseEntry::setText(const QString &strText)
{
	CParsedPhrase parsedPhrase(CBibleDatabasePtr(), caseSensitive(), accentSensitive(), isExcluded());			// Note: the ParsePhrase() function doesn't need the database.  If that ever changes, this must change (TODO)
	parsedPhrase.ParsePhrase(strText);
	m_strPhrase = strText;
}

void CPhraseEntry::setTextEncoded(const QString &strText)
{
	QString strTextToSet = strText;

	// The order here matters as we will always read/write the special flags in order
	//		so we don't need a complete parser to allow any arbitrary order:

	if (strTextToSet.startsWith(encCharDisabled())) {
		strTextToSet = strTextToSet.mid(1);				// Remove the special disable flag
		setDisabled(true);
	} else {
		setDisabled(false);
	}

	if (strTextToSet.startsWith(encCharExclude())) {
		strTextToSet = strTextToSet.mid(1);
		setExclude(true);
	} else {
		setExclude(false);
	}

	if (strTextToSet.startsWith(encCharAccentSensitive())) {
		strTextToSet = strTextToSet.mid(1);
		setAccentSensitive(true);
	} else {
		setAccentSensitive(false);
	}

	if (strTextToSet.startsWith(encCharCaseSensitive())) {
		strTextToSet = strTextToSet.mid(1);				// Remove the special case-sensitive flag
		setCaseSensitive(true);
	} else {
		setCaseSensitive(false);
	}

	setText(strTextToSet);
}

// ============================================================================

int CPhraseList::removeDuplicates()
{
	int n = size();
	int j = 0;
	QSet<CPhraseEntry> seen;
	seen.reserve(n);
	for (int i = 0; i < n; ++i) {
		const CPhraseEntry &s = at(i);
		if (seen.contains(s))
			continue;
		seen.insert(s);
		if (j != i)
			(*this)[j] = s;
		++j;
	}
	if (n != j)
		erase(begin() + j, end());
	return n - j;
}

// ============================================================================

QString CFootnoteEntry::htmlText(const CBibleDatabase *pBibleDatabase) const
{
	QTextDocument docFootote;
	docFootote.setHtml(text());

	CScriptureTextHtmlBuilder scriptureHTML;
	CScriptureTextDocumentDirector scriptureDirector(&scriptureHTML, pBibleDatabase);		// Only need BibleDatabase for embedded scripture and cross-refs, etc

	scriptureDirector.processDocument(&docFootote);

	return scriptureHTML.getResult();
}

QString CFootnoteEntry::plainText(const CBibleDatabase *pBibleDatabase) const
{
	QTextDocument docFootote;
	docFootote.setHtml(text());

	CScripturePlainTextBuilder scripturePlainText;
	CScriptureTextDocumentDirector scriptureDirector(&scripturePlainText, pBibleDatabase);		// Only need BibleDatabase for embedded scripture and cross-refs, etc

	scriptureDirector.processDocument(&docFootote);

	return scripturePlainText.getResult();
}

// ============================================================================

#ifdef OSIS_PARSER_BUILD

uint32_t CBibleDatabase::NormalizeIndexNoAccum(const CRelIndex &ndxRelIndex) const
{
	uint32_t nNormalIndex = 0;
	unsigned int nBk = ndxRelIndex.book();
	unsigned int nChp = ndxRelIndex.chapter();
	unsigned int nVrs = ndxRelIndex.verse();
	unsigned int nWrd = ndxRelIndex.word();

	if (!ndxRelIndex.isSet()) return 0;

	// Add the number of words for all books prior to the target book:
	if (nBk == 0) return 0;
	if (nBk > m_lstBooks.size()) return 0;
	for (unsigned int ndxBk = 1; ndxBk < nBk; ++ndxBk) {
		nNormalIndex += m_lstBooks.at(ndxBk-1).m_nNumWrd;
	}
	// Add the number of words for all chapters in this book prior to the target chapter:
	if ((nChp == 0) && (!m_lstBooks.at(nBk-1).m_bHaveColophon)) nChp = 1;

	if ((nChp != 0) && (m_lstBooks.at(nBk-1).m_bHaveColophon)) {
		nNormalIndex += (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,0,0,0)).m_nNumWrd;
	}

	if (nChp > m_lstBooks.at(nBk-1).m_nNumChp) return 0;
	for (unsigned int ndxChp = 1; ndxChp < nChp; ++ndxChp) {
		nNormalIndex += m_mapChapters.at(CRelIndex(nBk,ndxChp,0,0)).m_nNumWrd;
	}
	// Add the number of words for all verses in this book prior to the target verse:
	if ((nVrs == 0) && (nChp != 0) && (!m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_bHaveSuperscription)) nVrs = 1;
	if (nChp > 0) {
		if ((nVrs != 0) && (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_bHaveSuperscription)) {
			nNormalIndex += (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,0,0)).m_nNumWrd;
		}
		if (nVrs > m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) return 0;
		for (unsigned int ndxVrs = 1; ndxVrs < nVrs; ++ndxVrs) {
			nNormalIndex += (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,ndxVrs,0)).m_nNumWrd;
		}
	} else {
		if (nVrs != 0) return 0;
	}
	// Add the target word:
	if (nWrd == 0) nWrd = 1;
	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nNumWrd) return 0;
	nNormalIndex += nWrd;

	return nNormalIndex;
}

CRelIndex CBibleDatabase::DenormalizeIndexNoAccum(uint32_t nNormalIndex) const
{
	unsigned int nBk = 0;
	unsigned int nWrd = nNormalIndex;

	if (nNormalIndex == 0) return 0;

	while (nBk < m_lstBooks.size()) {
		if (m_lstBooks[nBk].m_nNumWrd >= nWrd) break;
		nWrd -= m_lstBooks[nBk].m_nNumWrd;
		nBk++;
	}
	if (nBk >= m_lstBooks.size()) return 0;
	nBk++;

	unsigned int nChp = (m_lstBooks.at(nBk-1).m_bHaveColophon ? 0 : 1);

	while (nChp <= m_lstBooks.at(nBk-1).m_nNumChp) {
		if (nChp > 0) {
			if (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumWrd >= nWrd) break;
			nWrd -= m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumWrd;
		} else {
			// Handle Colophon:
			if ((m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,0,0,0)).m_nNumWrd >= nWrd) break;
			nWrd -= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,0,0,0)).m_nNumWrd;
		}
		nChp++;
	}
	if (nChp > m_lstBooks[nBk-1].m_nNumChp) return 0;

	unsigned int nVrs = (((nChp == 0) || (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_bHaveSuperscription)) ? 0 : 1);
	if (nChp > 0) {
		while (nVrs <= m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) {
			// Note: Superscription is handled implicitly:
			if ((m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nNumWrd >= nWrd) break;
			nWrd -= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nNumWrd;
			nVrs++;
		}
		if (nVrs > m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) return 0;
	}

	// Note: Allow "first word" to be equivalent to the "zeroth word" to correctly handle verses that are empty:
	if ((nWrd != 1) && (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nNumWrd)) return 0;

	return CRelIndex(nBk, nChp, nVrs, nWrd);
}

#endif

uint32_t CBibleDatabase::NormalizeIndex(const CRelIndex &ndxRelIndex) const
{
	unsigned int nBk = ndxRelIndex.book();
	unsigned int nChp = ndxRelIndex.chapter();
	unsigned int nVrs = ndxRelIndex.verse();;
	unsigned int nWrd = ndxRelIndex.word();

	if (!ndxRelIndex.isSet()) return 0;

	if (nBk == 0) return 0;
	if (nBk > m_lstBooks.size()) return 0;
	if ((nChp == 0) && (!m_lstBooks.at(nBk-1).m_bHaveColophon)) nChp = 1;
	if (nChp > m_lstBooks[nBk-1].m_nNumChp) return 0;
	if ((nVrs == 0) && (nChp != 0) && (!m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_bHaveSuperscription)) nVrs = 1;
	if (nWrd == 0) nWrd = 1;
	if (nChp > 0) {
		// Note: Allow "first verse" to be equivalent to the "zeroth verse" to correctly handle chapters that are empty:
		// Note: Allow "first word" to be equivalent to the "zeroth word" to correctly handle verses that are empty:
		if ((nVrs == 1) && (nWrd == 1)) {
			if (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_bHaveSuperscription) {
				return (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nWrdAccum + nWrd +
						(m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,0,0)).m_nNumWrd);
			}
			return (m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nWrdAccum + nWrd);
		}
		if (nVrs > m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs) return 0;
	} else {
		if (nVrs != 0) return 0;
	}
	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nNumWrd) return 0;

	return ((m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nWrdAccum + nWrd);
}

CRelIndex CBibleDatabase::DenormalizeIndex(uint32_t nNormalIndex) const
{
	unsigned int nWrd = nNormalIndex;

	if (nWrd == 0) return 0;

	unsigned int nBk = m_lstBooks.size();
	while ((nBk > 0) && (nWrd <= m_lstBooks.at(nBk-1).m_nWrdAccum)) {
		nBk--;
	}
	if (nBk == 0) {
		assert(false);
		return 0;
	}

	unsigned int nChp = m_lstBooks.at(nBk-1).m_nNumChp;
	while ((nChp > 0) && (nWrd <= m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nWrdAccum)) {
		nChp--;
	}
	if ((nChp == 0) && (!m_lstBooks.at(nBk-1).m_bHaveColophon)) {
		assert(false);
		return 0;
	}

	unsigned int nVrs = ((nChp != 0) ? m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_nNumVrs : 0);
	while ((nVrs > 0) && (nWrd <= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nWrdAccum)) {
		nVrs--;
	}
	if ((nVrs == 0) && (nChp != 0) && (!m_mapChapters.at(CRelIndex(nBk,nChp,0,0)).m_bHaveSuperscription)) {
		assert(false);
		return 0;
	}

	nWrd -= (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nWrdAccum;
	if (nWrd > (m_lstBookVerses.at(nBk-1)).at(CRelIndex(nBk,nChp,nVrs,0)).m_nNumWrd) {
		// We can get here if the caller is addressing one word beyond the end-of-the-text, for example,
		//		and this has always been defined as "0" (out-of-bounds or not-set), just like the "0"
		//		at the beginning of the text.
		return 0;
	}

	return CRelIndex(nBk, nChp, nVrs, nWrd);
}

// ============================================================================

CConcordanceEntry::CConcordanceEntry(TWordListMap::const_iterator itrEntryWord, int nAltWordIndex, int nIndex)
	:	m_itrEntryWord(itrEntryWord),
		m_nAltWordIndex(nAltWordIndex),
		m_nIndex(nIndex)
{

}

// ============================================================================

#ifdef USING_WEBCHANNEL
QString CBibleDatabase::toJsonBkChpStruct() const
{
	QJsonObject objBible;
	objBible["testamentCount"] = static_cast<int>(bibleEntry().m_nNumTst);
	objBible["bookCount"] = static_cast<int>(bibleEntry().m_nNumBk);
	objBible["chapterCount"] = static_cast<int>(bibleEntry().m_nNumChp);
	QJsonArray arrTestaments;
	for (unsigned int nTst = 1; nTst <= bibleEntry().m_nNumTst; ++nTst) {
		QJsonObject objTestament;
		objTestament["name"] = testamentEntry(nTst)->m_strTstName;
		objTestament["bookCount"] = static_cast<int>(testamentEntry(nTst)->m_nNumBk);
		objTestament["chapterCount"] = static_cast<int>(testamentEntry(nTst)->m_nNumChp);
		arrTestaments.append(objTestament);
	}
	objBible["testaments"] = arrTestaments;
	QJsonArray arrBooks;
	for (unsigned int nBk = 1; nBk <= bibleEntry().m_nNumBk; ++nBk) {
		QJsonObject objBook;
		objBook["name"] = bookEntry(nBk)->m_strBkName;
		objBook["chapterCount"] = static_cast<int>(bookEntry(nBk)->m_nNumChp);
		arrBooks.append(objBook);
	}
	objBible["books"] = arrBooks;

	return QJsonDocument(objBible).toJson(QJsonDocument::Compact);
}
#endif

// ============================================================================

QString CBibleDatabase::testamentName(const CRelIndex &nRelIndex) const
{
	uint32_t nTst = testament(nRelIndex);
	if ((nTst < 1) || (nTst > m_lstTestaments.size())) return QString();
	return m_lstTestaments.at(nTst-1).m_strTstName;
}

uint32_t CBibleDatabase::testament(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return 0;
	return m_lstBooks.at(nBk-1).m_nTstNdx;
}

QString CBibleDatabase::bookCategoryName(const CRelIndex &nRelIndex) const
{
	uint32_t nCat = bookCategory(nRelIndex);
	if ((nCat < 1) || (nCat > m_lstBookCategories.size())) return QString();
	return m_lstBookCategories.at(nCat-1).m_strCategoryName;
}

uint32_t CBibleDatabase::bookCategory(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return 0;
	return m_lstBooks.at(nBk-1).m_nCatNdx;
}

QString CBibleDatabase::bookName(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return QString();
	const CBookEntry &book = m_lstBooks[nBk-1];
	return book.m_strBkName;
}

QString CBibleDatabase::bookNameAbbr(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return QString();
	const CBookEntry &book = m_lstBooks[nBk-1];
	if (book.m_lstBkAbbr.size() >= 2) return book.m_lstBkAbbr.at(1);		// Return preferred Common Abbreviation if it exists
	if (book.m_lstBkAbbr.size() == 1) return book.m_lstBkAbbr.at(0);		// Return OSIS Abbreviation of there are no Common Abbreviations
	assert(false);
	return QString();
}

QString CBibleDatabase::bookOSISAbbr(const CRelIndex &nRelIndex) const
{
	uint32_t nBk = nRelIndex.book();
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return QString();
	const CBookEntry &book = m_lstBooks[nBk-1];
	if (book.m_lstBkAbbr.size() >= 1) return book.m_lstBkAbbr.at(0);		// Return OSIS Abbreviation
	assert(false);
	return QString();
}

QString CBibleDatabase::SearchResultToolTip(const CRelIndex &nRelIndex, unsigned int nRIMask, unsigned int nSelectionSize) const
{
	CRefCountCalc Bk(this, CRefCountCalc::RTE_BOOK, nRelIndex);
	CRefCountCalc Chp(this, CRefCountCalc::RTE_CHAPTER, nRelIndex);
	CRefCountCalc Vrs(this, CRefCountCalc::RTE_VERSE, nRelIndex);
	CRefCountCalc Wrd(this, CRefCountCalc::RTE_WORD, nRelIndex);

	QString strTemp;

	if (nRIMask & RIMASK_HEADING) {
		if (nSelectionSize > 1) {
			strTemp += PassageReferenceText(nRelIndex);
			strTemp += " - ";
			strTemp += PassageReferenceText(DenormalizeIndex(NormalizeIndex(nRelIndex) + nSelectionSize - 1));
			strTemp += " " + QObject::tr("(%1 Words)", "Statistics").arg(nSelectionSize);
			strTemp += "\n\n";
		} else {
			strTemp += PassageReferenceText(nRelIndex);
			strTemp += "\n\n";
		}
	}

	if ((nRIMask & RIMASK_BOOK) &&
		((Bk.ofBible().first != 0) ||
		 (Bk.ofTestament().first != 0))) {
		strTemp += QObject::tr("Book:", "Statistics") + "\n";
		if (Bk.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible", "Statistics").arg(Bk.ofBible().first).arg(Bk.ofBible().second) + "\n";
		}
		if (Bk.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Bk.ofTestament().first).arg(Bk.ofTestament().second).arg(
							/* testamentName(nRelIndex) */
							xc_dbDescriptors::translatedBibleTestamentName(compatibilityUUID(), testament(nRelIndex))
						) + "\n";
		}
	}

	if ((nRIMask & RIMASK_CHAPTER) &&
		(nRelIndex.chapter() != 0) &&
		((Chp.ofBible().first != 0) ||
		 (Chp.ofTestament().first != 0) ||
		 (Chp.ofBook().first != 0))) {
		strTemp += QObject::tr("Chapter:", "Statistics") + "\n";
		if (Chp.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible", "Statistics").arg(Chp.ofBible().first).arg(Chp.ofBible().second) + "\n";
		}
		if (Chp.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Chp.ofTestament().first).arg(Chp.ofTestament().second).arg(
							/* testamentName(nRelIndex) */
							xc_dbDescriptors::translatedBibleTestamentName(compatibilityUUID(), testament(nRelIndex))
						) + "\n";
		}
		if (Chp.ofBook().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Chp.ofBook().first).arg(Chp.ofBook().second).arg(PassageReferenceText(CRelIndex(nRelIndex.book(),0,0,0))) + "\n";
		}
	}

	if ((nRIMask & RIMASK_VERSE) &&
		(nRelIndex.verse() != 0) &&
		((Vrs.ofBible().first != 0) ||
		 (Vrs.ofTestament().first != 0) ||
		 (Vrs.ofBook().first != 0) ||
		 (Vrs.ofChapter().first != 0))) {
		strTemp += QObject::tr("Verse:", "Statistics") + "\n";
		if (Vrs.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible", "Statistics").arg(Vrs.ofBible().first).arg(Vrs.ofBible().second) + "\n";
		}
		if (Vrs.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Vrs.ofTestament().first).arg(Vrs.ofTestament().second).arg(
							/* testamentName(nRelIndex) */
							xc_dbDescriptors::translatedBibleTestamentName(compatibilityUUID(), testament(nRelIndex))
						) + "\n";
		}
		if (Vrs.ofBook().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Vrs.ofBook().first).arg(Vrs.ofBook().second).arg(PassageReferenceText(CRelIndex(nRelIndex.book(),0,0,0))) + "\n";
		}
		if (Vrs.ofChapter().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Vrs.ofChapter().first).arg(Vrs.ofChapter().second).arg(PassageReferenceText(CRelIndex(nRelIndex.book(),nRelIndex.chapter(),0,0))) + "\n";
		}
	}

	if ((nRIMask & RIMASK_WORD) &&
		(nRelIndex.word() != 0) &&
		((Wrd.ofBible().first != 0) ||
		 (Wrd.ofTestament().first != 0) ||
		 (Wrd.ofBook().first != 0) ||
		 (Wrd.ofChapter().first != 0) ||
		 (Wrd.ofVerse().first != 0))) {
		strTemp += QObject::tr("Word/Phrase:", "Statistics") + "\n";
		if (Wrd.ofBible().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of Bible", "Statistics").arg(Wrd.ofBible().first).arg(Wrd.ofBible().second) + "\n";
		}
		if (Wrd.ofTestament().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Wrd.ofTestament().first).arg(Wrd.ofTestament().second).arg(
							/* testamentName(nRelIndex) */
							xc_dbDescriptors::translatedBibleTestamentName(compatibilityUUID(), testament(nRelIndex))
						) + "\n";
		}
		if (Wrd.ofBook().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Wrd.ofBook().first).arg(Wrd.ofBook().second).arg(PassageReferenceText(CRelIndex(nRelIndex.book(),0,0,0))) + "\n";
		}
		if ((Wrd.ofChapter().first != 0) && (nRelIndex.chapter() != 0)) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Wrd.ofChapter().first).arg(Wrd.ofChapter().second).arg(PassageReferenceText(CRelIndex(nRelIndex.book(),nRelIndex.chapter(),0,0))) + "\n";
		}
		if (Wrd.ofVerse().first != 0) {
			strTemp += "    " + QObject::tr("%1 of %2 of %3", "Statistics").arg(Wrd.ofVerse().first).arg(Wrd.ofVerse().second).arg(PassageReferenceText(nRelIndex, true)) + "\n";
		}
	}

	return strTemp;
}

QString CBibleDatabase::PassageReferenceText(const CRelIndex &nRelIndex, bool bSuppressWordOnPseudoVerse) const
{
	if (!nRelIndex.isSet()) return QObject::tr("<Invalid Reference>", "Statistics");
	QString strBookName = bookName(nRelIndex);
	if (nRelIndex.chapter() == 0) {
		if (!bSuppressWordOnPseudoVerse) {
			return QString("%1%2").arg(strBookName).arg((nRelIndex.word() != 0) ? QString("%1%2 [%3]").arg(!strBookName.isEmpty() ? " " : QString()).arg(QObject::tr("Colophon", "Statistics")).arg(nRelIndex.word()) : QString());
		} else {
			return QString("%1%2").arg(strBookName).arg((nRelIndex.word() != 0) ? QString("%1%2").arg(!strBookName.isEmpty() ? " " : QString()).arg(QObject::tr("Colophon", "Statistics")) : QString());
		}
	}
	if (!strBookName.isEmpty()) strBookName += " ";
	if (nRelIndex.verse() == 0) {
		if (!bSuppressWordOnPseudoVerse) {
			return QString("%1%2%3").arg(strBookName).arg(nRelIndex.chapter()).arg((nRelIndex.word() != 0) ? QString(" %1 [%2]").arg(QObject::tr("Superscription", "Statistics")).arg(nRelIndex.word()) : QString());
		} else {
			return QString("%1%2%3").arg(strBookName).arg(nRelIndex.chapter()).arg((nRelIndex.word() != 0) ? QString(" %1").arg(QObject::tr("Superscription", "Statistics")) : QString());
		}
	}
	if (nRelIndex.word() == 0) {
		return QString("%1%2:%3").arg(strBookName).arg(nRelIndex.chapter()).arg(nRelIndex.verse());
	}
	return QString("%1%2:%3 [%4]").arg(strBookName).arg(nRelIndex.chapter()).arg(nRelIndex.verse()).arg(nRelIndex.word());
}

QString CBibleDatabase::PassageReferenceAbbrText(const CRelIndex &nRelIndex, bool bSuppressWordOnPseudoVerse) const
{
	if (!nRelIndex.isSet()) return QObject::tr("<Invalid Reference>", "Statistics");
	QString strBookName = bookNameAbbr(nRelIndex);
	if (nRelIndex.chapter() == 0) {
		if (!bSuppressWordOnPseudoVerse) {
			return QString("%1%2").arg(strBookName).arg((nRelIndex.word() != 0) ? QString("%1%2 [%3]").arg(!strBookName.isEmpty() ? " " : QString()).arg(QObject::tr("Colophon", "Statistics")).arg(nRelIndex.word()) : QString());
		} else {
			return QString("%1%2").arg(strBookName).arg((nRelIndex.word() != 0) ? QString("%1%2").arg(!strBookName.isEmpty() ? " " : QString()).arg(QObject::tr("Colophon", "Statistics")) : QString());
		}
	}
	if (!strBookName.isEmpty()) strBookName += " ";
	if (nRelIndex.verse() == 0) {
		if (!bSuppressWordOnPseudoVerse) {
			return QString("%1%2%3").arg(strBookName).arg(nRelIndex.chapter()).arg((nRelIndex.word() != 0) ? QString(" %1 [%2]").arg(QObject::tr("Superscription", "Statistics")).arg(nRelIndex.word()) : QString());
		} else {
			return QString("%1%2%3").arg(strBookName).arg(nRelIndex.chapter()).arg((nRelIndex.word() != 0) ? QString(" %1").arg(QObject::tr("Superscription", "Statistics")) : QString());
		}
	}
	if (nRelIndex.word() == 0) {
		return QString("%1%2:%3").arg(strBookName).arg(nRelIndex.chapter()).arg(nRelIndex.verse());
	}
	return QString("%1%2:%3 [%4]").arg(strBookName).arg(nRelIndex.chapter()).arg(nRelIndex.verse()).arg(nRelIndex.word());
}


// ============================================================================

CRefCountCalc::CRefCountCalc(const CBibleDatabase *pBibleDatabase, REF_TYPE_ENUM nRefType, const CRelIndex &refIndex)
:	m_ndxRef(refIndex),
	m_nRefType(nRefType),
	m_nOfBible(0,0),
	m_nOfTst(0,0),
	m_nOfBk(0,0),
	m_nOfChp(0,0),
	m_nOfVrs(0,0)
{
	assert(pBibleDatabase != NULL);
	switch (nRefType) {
		case RTE_TESTAMENT:				// Calculate the Testament of the Bible
			m_nOfBible.first = pBibleDatabase->testament(m_ndxRef);
			m_nOfBible.second = pBibleDatabase->bibleEntry().m_nNumTst;
			break;

		case RTE_BOOK:					// Calculate the Book of the Testament and Bible
			m_nOfBible.first = m_ndxRef.book();
			m_nOfBible.second = pBibleDatabase->bibleEntry().m_nNumBk;
			if (m_ndxRef.book() != 0) {
				const CBookEntry &book = *pBibleDatabase->bookEntry(m_ndxRef.book());
				m_nOfTst.first = book.m_nTstBkNdx;
				m_nOfTst.second = pBibleDatabase->testamentEntry(book.m_nTstNdx)->m_nNumBk;
			}
			break;

		case RTE_CHAPTER:				// Calculate the Chapter of the Book, Testament, and Bible
			m_nOfBk.first = m_ndxRef.chapter();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= pBibleDatabase->bibleEntry().m_nNumBk)) {
				m_nOfBk.second = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumChp;
				// Number of Chapters in books prior to target:
				for (unsigned int ndxBk=1; ndxBk<m_ndxRef.book(); ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.first += pBook->m_nNumChp;
					m_nOfBible.first += pBook->m_nNumChp;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=m_ndxRef.book(); ndxBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.second += pBook->m_nNumChp;
					m_nOfBible.second += pBook->m_nNumChp;
				}
				// Number of Chapter in target:
				m_nOfTst.first += m_ndxRef.chapter();
				m_nOfBible.first += m_ndxRef.chapter();
			}
			break;

		case RTE_VERSE:					// Calculate the Verse of the Chapter, Book, Testament, and Bible
			m_nOfChp.first = m_ndxRef.verse();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= pBibleDatabase->bibleEntry().m_nNumBk) &&
				(m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumChp)) {
				m_nOfChp.second = pBibleDatabase->chapterEntry(m_ndxRef)->m_nNumVrs;
				m_nOfBk.second = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumVrs;
				// Number of Verses in books prior to target:
				for (unsigned int ndxBk=1; ndxBk<m_ndxRef.book(); ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nVerses = pBook->m_nNumVrs;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.first += nVerses;
					m_nOfBible.first += nVerses;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=m_ndxRef.book(); ndxBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nVerses = pBook->m_nNumVrs;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.second += nVerses;
					m_nOfBible.second += nVerses;
				}
				// Number of Verses in Chapters prior to target in target book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nVerses = pBibleDatabase->chapterEntry(CRelIndex(m_ndxRef.book(),ndxChp,0,0))->m_nNumVrs;
					m_nOfBk.first += nVerses;
					m_nOfTst.first += nVerses;
					m_nOfBible.first += nVerses;
				}
				// Number of Verses in target:
				m_nOfBk.first += m_ndxRef.verse();
				m_nOfTst.first += m_ndxRef.verse();
				m_nOfBible.first += m_ndxRef.verse();
			}
			break;

		case RTE_WORD:					// Calculate the Word of the Verse, Book, Testament, and Bible
			m_nOfVrs.first = m_ndxRef.word();
			if ((m_ndxRef.book() > 0) && (m_ndxRef.book() <= pBibleDatabase->bibleEntry().m_nNumBk) &&
				(((m_ndxRef.chapter() == 0) && (m_ndxRef.word() != 0)) || ((m_ndxRef.chapter() > 0) && (m_ndxRef.chapter() <= pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumChp))) &&
				(((m_ndxRef.verse() == 0) && (m_ndxRef.word() != 0)) || ((m_ndxRef.verse() > 0) && (m_ndxRef.verse() <= pBibleDatabase->chapterEntry(m_ndxRef)->m_nNumVrs)))) {
				m_nOfVrs.second = pBibleDatabase->verseEntry(m_ndxRef)->m_nNumWrd;
				if (m_ndxRef.chapter() != 0) m_nOfChp.second = pBibleDatabase->chapterEntry(m_ndxRef)->m_nNumWrd;
				m_nOfBk.second = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumWrd;
				// Number of Words in books prior to target:
				for (unsigned int ndxBk=1; ndxBk<m_ndxRef.book(); ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nWords = pBook->m_nNumWrd;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				m_nOfTst.second = m_nOfTst.first;
				m_nOfBible.second = m_nOfBible.first;
				for (unsigned int ndxBk=m_ndxRef.book(); ndxBk<= pBibleDatabase->bibleEntry().m_nNumBk; ++ndxBk) {
					const CBookEntry *pBook = pBibleDatabase->bookEntry(ndxBk);
					unsigned int nWords = pBook->m_nNumWrd;
					if (pBook->m_nTstNdx == pBibleDatabase->testament(m_ndxRef))
						m_nOfTst.second += nWords;
					m_nOfBible.second += nWords;
				}
				// Number of Words in Chapters prior to target in target Book:
				for (unsigned int ndxChp=1; ndxChp<m_ndxRef.chapter(); ++ndxChp) {
					unsigned int nWords = pBibleDatabase->chapterEntry(CRelIndex(m_ndxRef.book(),ndxChp,0,0))->m_nNumWrd;
					m_nOfBk.first += nWords;
					m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				bool bHaveColophon = (pBibleDatabase->bookEntry(m_ndxRef.book())->m_bHaveColophon);
				// Even though the colophons logistically are indexed at the start of the book, treat them as if
				//		they come at the end of the book (since that's the definition of a colophon) -- if this is a colophon,
				//		add the words of the book minus the colophon to our target and below we'll index
				//		into the colophon itself.
				if ((bHaveColophon) && (m_ndxRef.chapter() == 0) && (m_ndxRef.verse() == 0)) {
					unsigned int nWords = pBibleDatabase->bookEntry(m_ndxRef.book())->m_nNumWrd - pBibleDatabase->verseEntry(CRelIndex(m_ndxRef.book(),0,0,0))->m_nNumWrd;
					m_nOfBk.first += nWords;
					m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				// Number of Words in Verses prior to target in target Chapter:
				bool bHaveSuperscription = ((m_ndxRef.chapter() != 0) ? (pBibleDatabase->chapterEntry(m_ndxRef)->m_bHaveSuperscription) : false);
				for (unsigned int ndxVrs=(bHaveSuperscription ? 0 : 1); ndxVrs<m_ndxRef.verse(); ++ndxVrs) {
					unsigned int nWords = pBibleDatabase->verseEntry(CRelIndex(m_ndxRef.book(),m_ndxRef.chapter(),ndxVrs,0))->m_nNumWrd;
					m_nOfChp.first += nWords;
					m_nOfBk.first += nWords;
					m_nOfTst.first += nWords;
					m_nOfBible.first += nWords;
				}
				// Number of Words in target:
				m_nOfChp.first += m_ndxRef.word();
				m_nOfBk.first += m_ndxRef.word();
				m_nOfTst.first += m_ndxRef.word();
				m_nOfBible.first += m_ndxRef.word();
			}
			break;
	}
}

// ============================================================================

unsigned int CBibleDatabase::bookWordCountProper(unsigned int nBook) const
{
	assert((nBook > 0) && (nBook <= m_lstBooks.size()));
	const CBookEntry &bookEntry = m_lstBooks[nBook-1];
	const TVerseEntryMap &mapVerses = m_lstBookVerses[nBook-1];
	unsigned int nWords = bookEntry.m_nNumWrd;
	for (unsigned int ndxChp = 1; ndxChp < bookEntry.m_nNumChp; ++ndxChp) {
		if (m_mapChapters.at(CRelIndex(nBook, ndxChp, 0,0)).m_bHaveSuperscription) {
			nWords -= mapVerses.at(CRelIndex(nBook, ndxChp, 0, 0)).m_nNumWrd;
		}
	}
	if (bookEntry.m_bHaveColophon) nWords -= mapVerses.at(CRelIndex(nBook, 0, 0, 0)).m_nNumWrd;

	return nWords;
}

unsigned int CBibleDatabase::chapterWordCountProper(unsigned int nBook, unsigned int nChapter) const
{
	assert((nBook > 0) && (nBook <= m_lstBooks.size()));
	assert((nChapter > 0) && (nChapter <= m_lstBooks[nBook-1].m_nNumChp));
	CRelIndex ndxChapter(nBook, nChapter, 0, 0);
	const TVerseEntryMap &mapVerses = m_lstBookVerses.at(nBook-1);
	const CChapterEntry &chapterEntry = m_mapChapters.at(ndxChapter);
	unsigned int nWords = chapterEntry.m_nNumWrd;
	if (chapterEntry.m_bHaveSuperscription) nWords -= mapVerses.at(ndxChapter).m_nNumWrd;
	return nWords;
}

CRelIndex CBibleDatabase::calcRelIndex(
					unsigned int nWord, unsigned int nVerse, unsigned int nChapter,
					unsigned int nBook, unsigned int nTestament,
					const CRelIndex &ndxStart,
					bool bReverse) const
{
	uint32_t ndxWord = 0;			// We will calculate target via word, which we can then call Denormalize on
	CRelIndex ndxResult;

	if (!bReverse) {
		// FORWARD:

		// Start with our relative location:
		nWord += ndxStart.word();
		nVerse += ndxStart.verse();
		nChapter += ndxStart.chapter();
		nBook += ndxStart.book();

		// Assume 1st word/verse/chapter/book of target:
		if (nWord == 0) nWord = 1;
		if (nVerse == 0) nVerse = 1;
		if (nChapter == 0) nChapter = 1;
		if (nBook == 0) nBook = 1;

		// ===================
		// Testament of Bible:
		if (nTestament) {
			if (nTestament > m_lstTestaments.size()) return CRelIndex();		// Testament too large, past end of Bible
			for (unsigned int ndx=1; ndx<nTestament; ++ndx) {
				// Ripple down to children:
				nBook += m_lstTestaments[ndx-1].m_nNumBk;
			}
		}	// At this point, top specified index will be relative to the Bible, nTestament isn't needed beyond this point

		// ===================
		// Book of Bible/Testament:
		if (nBook > m_lstBooks.size()) return CRelIndex();
		for (unsigned int ndx=1; ndx<nBook; ++ndx)
			ndxWord += m_lstBooks[ndx-1].m_nNumWrd;					// Add words for Books prior to target

		// ===================
		// Chapter of Bible/Testament:
		while (nChapter > m_lstBooks[nBook-1].m_nNumChp) {		// Resolve nBook
			ndxWord += m_lstBooks[nBook-1].m_nNumWrd;			// Add words for books prior to target book
			nChapter -= m_lstBooks[nBook-1].m_nNumChp;
			nBook++;
			if (nBook > m_lstBooks.size()) return CRelIndex();	// Chapter too large (past end of last Book of Bible/Testament)
		}
		// Chapter of Book:
		//	Note:  Here we'll push the verses of the chapter down to nVerse and
		//			relocate Chapter back to 1.  We do that so that we will be
		//			relative to the start of the book for the word search
		//			so that it can properly push to a following chapter or
		//			book if that's necessary.  Otherwise, we can run off the
		//			end of this book looking for more chapters.  We could,
		//			of course, update verse, chapter and/or book in the verse in
		//			book resolution loop, but that really complicates that,
		//			especially since we have to do that anyway.  We won't
		//			update ndxWord here since that will get done in the Verse
		//			loop below once we push this down to that:
		if (nChapter>m_lstBooks[nBook-1].m_nNumChp) return CRelIndex();		// Chapter too large (past end of book)
		for (unsigned int ndx=1; ndx<nChapter; ++ndx) {
			nVerse += m_mapChapters.at(CRelIndex(nBook, ndx, 0, 0)).m_nNumVrs;	// Push all chapters prior to target down to nVerse level
		}
		nChapter = 1;	// Reset to beginning of book so nVerse can count from there

		// ===================
		// Verse of Bible/Testament:
		while (nVerse > m_lstBooks[nBook-1].m_nNumVrs) {	// Resolve nBook
			ndxWord += m_lstBooks[nBook-1].m_nNumWrd;		// Add words for books prior to target book
			nVerse -= m_lstBooks[nBook-1].m_nNumVrs;
			nBook++;
			if (nBook > m_lstBooks.size()) return CRelIndex();	// Verse too large (past end of last Book of Bible/Testament)
		}
		// Verse of Book:
		if (nVerse>m_lstBooks[nBook-1].m_nNumVrs) return CRelIndex();		// Verse too large (past end of book)
		while (nVerse > m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs) {		// Resolve nChapter
			nVerse -= m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs;
			nWord += chapterWordCountProper(nBook, nChapter);	// Push all chapters prior to target down to nWord level
			nChapter++;
			if (nChapter > m_lstBooks[nBook-1].m_nNumChp) return CRelIndex();	// Verse too large (past end of last Chapter of Book)
		}

		// Verse of Chapter:
		//	Note:  Here we'll push the words of the verses down to nWord and
		//			relocate Verse back to 1.  We do that so that we will be
		//			relative to the start of the chapter for the word search
		//			so that it can properly push to a following chapter or
		//			book if that's necessary.  Otherwise, we can run off the
		//			end of this chapter looking for more verses.  We could,
		//			of course, update chapter and/or book in the word in
		//			chapter resolution loop, but that really complicates that,
		//			especially since we have to do that anyway.  We won't
		//			update ndxWord here since that will get done in the Word
		//			loop below once we push this down to that:
		if (nVerse>m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs) return CRelIndex();		// Verse too large (past end of Chapter of Book)
		for (unsigned int ndx=1; ndx<nVerse; ++ndx) {
			nWord += m_lstBookVerses[nBook-1].at(CRelIndex(nBook, nChapter, ndx, 0)).m_nNumWrd;				// Push all verses prior to target down to nWord level
		}
		nVerse = 1;		// Reset to beginning of chapter so nWord can count from there

		nChapter = 1;	// Reset to beginning of book so nWord can count from there

		// ===================
		// Word of Bible/Testament:
		while (nWord > bookWordCountProper(nBook)) {		// Resolve nBook
			ndxWord += m_lstBooks[nBook-1].m_nNumWrd;
			nWord -= bookWordCountProper(nBook);
			nBook++;
			if (nBook > m_lstBooks.size()) return CRelIndex();		// Word too large (past end of last Book of Bible/Testament)
		}
		// Word of Book:
		if (nWord>m_lstBooks[nBook-1].m_nNumWrd) return CRelIndex();	// Word too large (past end of Book/Chapter)

		// If the current book has a colophon, add words from the colophon since
		//		they logistically come ahead of the chapters:
		if (m_lstBooks[nBook-1].m_bHaveColophon) {
			ndxWord += m_lstBookVerses[nBook-1].at(CRelIndex(nBook, 0, 0, 0)).m_nNumWrd;
		}

		while (nWord > chapterWordCountProper(nBook, nChapter)) {	// Resolve nChapter
			ndxWord += m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd;
			nWord -= chapterWordCountProper(nBook, nChapter);
			nChapter++;
			if (nChapter > m_lstBooks[nBook-1].m_nNumChp) return CRelIndex();		// Word too large (past end of last Verse of last Book/Chapter)
		}

		// Word of Chapter:
		if (nWord>m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd) return CRelIndex();		// Word too large (past end of Book/Chapter)

		// If the current chapter has a superscription, add words from the superscription since
		//		they logistically come ahead of the vereses:
		if (m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_bHaveSuperscription) {
			ndxWord += m_lstBookVerses[nBook-1].at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumWrd;
		}

		const TVerseEntryMap &bookVerses = m_lstBookVerses[nBook-1];
		while (nWord > bookVerses.at(CRelIndex(nBook, nChapter, nVerse, 0)).m_nNumWrd) {	// Resolve nVerse
			ndxWord += bookVerses.at(CRelIndex(nBook, nChapter, nVerse, 0)).m_nNumWrd;
			nWord -= bookVerses.at(CRelIndex(nBook, nChapter, nVerse, 0)).m_nNumWrd;
			nVerse++;
			if (nVerse > m_mapChapters.at(CRelIndex(nBook, nChapter, 0, 0)).m_nNumVrs) return CRelIndex();	// Word too large (past end of last Verse of last Book/Chapter)
		}

		// Word of Verse:
		if (nWord>m_lstBookVerses[nBook-1].at(CRelIndex(nBook, nChapter, nVerse, 0)).m_nNumWrd) return CRelIndex();		// Word too large (past end of Verse of Chapter of Book)
		ndxWord += nWord;		// Add up to include target word

		// ===================
		// At this point, nBook/nChapter/nVerse/nWord is completely resolved.
		//		As a cross-check, ndxWord should be the Normalized index:
		ndxResult = CRelIndex(nBook, nChapter, nVerse, nWord);
		uint32_t ndxNormal = NormalizeIndex(ndxResult);
		assert(ndxWord == ndxNormal);
	} else {
		// REVERSE:

		// Start with starting location or last word of Bible:
		ndxWord = NormalizeIndex(ndxStart);
		if (ndxWord == 0) {
			// Set ndxWord to the total number of words in Bible:
			for (unsigned int ndx = 0; ndx<m_lstTestaments.size(); ++ndx) {
				ndxWord += m_lstTestaments[ndx].m_nNumWrd;
			}
		}
		// ndxWord is now pointing to the last word of the last verse of
		//	the last chapter of the last book... or is the word of the
		//	specified starting point...
		assert(ndxWord != 0);

		CRelIndex ndxTarget(DenormalizeIndex(ndxWord));
		assert(ndxTarget.index() != 0);		// Must be either the starting location or the last entry in the Bible

		// Force to the first whole chapter/verse (i.e. not colophon or superscription):
		if (ndxTarget.verse() == 0) ndxTarget.setVerse(1);
		if (ndxTarget.chapter() == 0) ndxTarget.setChapter(1);
		ndxWord = NormalizeIndex(ndxTarget);
		ndxTarget = DenormalizeIndex(ndxWord);
		assert((ndxWord != 0) && (ndxTarget != 0));

		// In Reverse mode, we ignore the nTestament entry

		// Word back:
		if (ndxWord <= nWord) return CRelIndex();
		while (nWord) {
			--nWord;
			--ndxWord;
			ndxTarget = DenormalizeIndex(ndxWord);
			if ((ndxTarget.verse() == 0) ||
				(ndxTarget.chapter() == 0)) {
				// If we hit a colophon or superscription, move to the previous verse:
				ndxTarget.setWord(0);
				ndxTarget.setVerse(0);
				if (ndxTarget.chapter() == 1) ndxTarget.setChapter(0);
				ndxWord = NormalizeIndex(ndxTarget) - 1;
			}
		}
		ndxTarget = DenormalizeIndex(ndxWord);
		nWord = ndxTarget.word();					// nWord = Offset of word into verse so we can traverse from start of verse to start of verse
		ndxTarget.setWord(1);						// Move to first word of this verse
		ndxWord = NormalizeIndex(ndxTarget);

		// Verse back:
		while (nVerse) {
			ndxWord--;				// This will move us to previous verse since we are at word 1 of current verse (see above and below)
			if (ndxWord == 0) return CRelIndex();
			ndxTarget = DenormalizeIndex(ndxWord);
			if ((ndxTarget.verse() == 0) ||
				(ndxTarget.chapter() == 0)) {
				// If we hit a colophon or superscription, move to the previous verse:
				ndxTarget.setWord(0);
				ndxTarget.setVerse(0);
				if (ndxTarget.chapter() == 1) ndxTarget.setChapter(0);
				ndxWord = NormalizeIndex(ndxTarget) - 1;
				ndxTarget = DenormalizeIndex(ndxWord);
			}
			ndxTarget.setWord(1);	// Move to first word of this verse
			ndxWord = NormalizeIndex(ndxTarget);
			if (ndxWord == 0) return CRelIndex();
			nVerse--;
		}
		nVerse = ndxTarget.verse();					// nVerse = Offset of verse into chapter so we can traverse from start of chapter to start of chapter
		ndxTarget.setVerse(1);						// Move to first verse of this chapter
		ndxWord = NormalizeIndex(ndxTarget);

		// Chapter back:
		while (nChapter) {
			ndxWord--;				// This will move us to previous chapter since we are at word 1 of verse 1 (see above and below)
			if (ndxWord == 0) return CRelIndex();
			ndxTarget = DenormalizeIndex(ndxWord);
			if ((ndxTarget.verse() == 0) ||
				(ndxTarget.chapter() == 0)) {
				// If we hit a colophon or superscription, move to the previous chapter:
				ndxTarget.setWord(0);
				ndxTarget.setVerse(0);
				if (ndxTarget.chapter() == 1) ndxTarget.setChapter(0);
				ndxWord = NormalizeIndex(ndxTarget) - 1;
				ndxTarget = DenormalizeIndex(ndxWord);
			}
			ndxTarget.setVerse(1);	// Move to first word of first verse of this chapter
			ndxTarget.setWord(1);
			ndxWord = NormalizeIndex(ndxTarget.index());
			if (ndxWord == 0) return CRelIndex();
			nChapter--;
		}
		nChapter = ndxTarget.chapter();				// nChapter = Offset of chapter into book so we can traverse from start of book to start of book
		ndxTarget.setChapter(1);					// Move to first chapter of this book
		ndxWord = NormalizeIndex(ndxTarget);

		// Book back:
		while (nBook) {
			ndxWord--;				// This will move us to previous book since we are at word 1 of verse 1 of chapter 1 (see above and below)
			if (ndxWord == 0) return CRelIndex();
			ndxTarget = DenormalizeIndex(ndxWord);
			if ((ndxTarget.verse() == 0) ||
				(ndxTarget.chapter() == 0)) {
				// If we hit a colophon or superscription, move to the previous book:
				ndxTarget.setWord(0);
				ndxTarget.setVerse(0);
				if (ndxTarget.chapter() == 1) ndxTarget.setChapter(0);
				ndxWord = NormalizeIndex(ndxTarget) -1;
				ndxTarget = DenormalizeIndex(ndxWord);
			}
			ndxTarget.setChapter(1);	// Move to first word of first verse of first chapter of this book
			ndxTarget.setVerse(1);
			ndxTarget.setWord(1);
			ndxWord = NormalizeIndex(ndxTarget);
			if (ndxWord == 0) return CRelIndex();
			nBook--;
		}
		nBook = ndxTarget.book();					// nBook = Offset of book into Bible for final location
		ndxTarget.setBook(1);						// Move to first book of the Bible
		ndxWord = NormalizeIndex(ndxTarget);
		assert(ndxWord == NormalizeIndex(CRelIndex(1,1,1,1)));		// We should be at the beginning of the Bible now

		// Call ourselves to calculate index from beginning of Bible:
		ndxResult = calcRelIndex(nWord, nVerse, nChapter, nBook, 0);
	}

	return ndxResult;
}

CRelIndex CBibleDatabase::calcRelIndex(const CRelIndex &ndxStart, RELATIVE_INDEX_MOVE_ENUM nMoveMode) const
{
	CRelIndex ndx;

	switch (nMoveMode) {
		case RIME_Absolute:
			// Normalize and denormalize to make the location actually resolve to the first
			//		existing passage.  For example, if we are called with CRelIndex of [1, 1, 1, 1]
			//		for Genesis 1:1 [1] and we are in a New Testament only database, this
			//		round-trip will cause us to instead return Matthew 1:1 [1], since it's our
			//		first existing passage at or after the specified index.  Similarly, it should
			//		work if we are passed a reference for an empty verse or chapter, etc...
			// This actually gives this mode a meaning other than simply returning ndxStart unaltered.
			ndx = DenormalizeIndex(NormalizeIndex(ndxStart));
			break;

		case RIME_Start:
			ndx = DenormalizeIndex(NormalizeIndex(CRelIndex(1, 1, 1, 1)));
			break;

		case RIME_StartOfBook:
			ndx = CRelIndex(ndxStart.book(), 1, 1, 1);
			break;

		case RIME_StartOfChapter:
			ndx = CRelIndex(ndxStart.book(), ndxStart.chapter(), 1, 1);
			break;

		case RIME_StartOfVerse:
			ndx = CRelIndex(ndxStart.book(), ndxStart.chapter(), ndxStart.verse(), 1);
			break;

		case RIME_End:
			ndx.setBook(bibleEntry().m_nNumBk);
			ndx.setChapter(bookEntry(ndx)->m_nNumChp);
			ndx.setVerse(chapterEntry(ndx)->m_nNumVrs);
			ndx.setWord(verseEntry(ndx)->m_nNumWrd);
			break;

		case RIME_EndOfBook:
			ndx.setBook(ndxStart.book());
			if (bookEntry(ndx)) { ndx.setChapter(bookEntry(ndx)->m_nNumChp); } else { ndx.clear(); }
			if (chapterEntry(ndx)) { ndx.setVerse(chapterEntry(ndx)->m_nNumVrs); } else { ndx.clear(); }
			if (verseEntry(ndx)) { ndx.setWord(verseEntry(ndx)->m_nNumWrd); } else { ndx.clear(); }
			break;

		case RIME_EndOfChapter:
			ndx.setBook(ndxStart.book());
			ndx.setChapter(ndxStart.chapter());
			if (chapterEntry(ndx)) { ndx.setVerse(chapterEntry(ndx)->m_nNumVrs); } else { ndx.clear(); }
			if (verseEntry(ndx)) { ndx.setWord(verseEntry(ndx)->m_nNumWrd); } else { ndx.clear(); }
			break;

		case RIME_EndOfVerse:
			ndx.setBook(ndxStart.book());
			ndx.setChapter(ndxStart.chapter());
			ndx.setVerse(ndxStart.verse());
			if (verseEntry(ndx)) { ndx.setWord(verseEntry(ndx)->m_nNumWrd); } else { ndx.clear(); }
			break;

		case RIME_PreviousBook:
			if (ndxStart.book() < 2) break;
			ndx = CRelIndex(ndxStart.book()-1, 1, 1, 1);
			break;

		case RIME_PreviousChapter:
			ndx = calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndxStart.book(), ndxStart.chapter(), 1, 1), true);
			if (ndx.isSet()) {
				// The following sets are needed to handle the case of scrolling backward from a missing chapter/verse entry -- for example
				//		the Additions to Esther in the Apocrypha.  The above calculation will normalize the current location to 10:4 in that
				//		passage, causing us to goto the 4th verse of the preceding chapter:
				ndx.setVerse(1);
				ndx.setWord(1);
			}
			break;

		case RIME_PreviousVerse:
			ndx = calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndxStart.book(), ndxStart.chapter(), ndxStart.verse(), 1), true);
			break;

		case RIME_PreviousWord:
			ndx = calcRelIndex(1, 0, 0, 0, 0, ndxStart, true);
			break;

		case RIME_NextBook:
			if (ndxStart.book() >= bibleEntry().m_nNumBk) break;
			ndx = CRelIndex(ndxStart.book()+1, 1, 1, 1);
			break;

		case RIME_NextChapter:
			ndx = calcRelIndex(0, 0, 1, 0, 0, CRelIndex(ndxStart.book(), ndxStart.chapter(), 1, 1), false);
			break;

		case RIME_NextVerse:
			ndx = calcRelIndex(0, 1, 0, 0, 0, CRelIndex(ndxStart.book(), ndxStart.chapter(), ndxStart.verse(), 1), false);
			break;

		case RIME_NextWord:
			ndx = calcRelIndex(1, 0, 0, 0, 0, ndxStart, false);
			break;

		default:
			break;
	}

	return ndx;
}

// ============================================================================

TCrossReferenceMap TCrossReferenceMap::createScopedMap(const CBibleDatabase *pBibleDatabase) const
{
	assert(pBibleDatabase != NULL);
	if (pBibleDatabase == NULL) return TCrossReferenceMap(*this);

	TCrossReferenceMap mapScoped;

	for (const_iterator itrMap = begin(); itrMap != end(); ++itrMap) {
		if (pBibleDatabase->NormalizeIndex(itrMap->first) == 0) continue;

		TRelativeIndexSet setRefs;
		for (TRelativeIndexSet::const_iterator itrSet = (itrMap->second).begin(); itrSet != (itrMap->second).end(); ++itrSet) {
			if (pBibleDatabase->NormalizeIndex(*itrSet) != 0) setRefs.insert(*itrSet);
		}
		if (!setRefs.empty()) mapScoped[itrMap->first] = setRefs;
	}

	return mapScoped;
}

// ============================================================================


CBibleDatabase::CBibleDatabase(const TBibleDescriptor &bblDesc)
	:	m_pKJPBSWordScriptureObject(new CKJPBSWordScriptureObject(this))
{
	// Note: For ReadSpecialBibleDatabase() to work correctly (command-line tools), this function must be
	//	able to work with the BDE_SPECIAL_TEST descriptor:
	if (bblDesc.m_strUUID.compare(bibleDescriptor(BDE_SPECIAL_TEST).m_strUUID, Qt::CaseInsensitive) != 0) {
		// If this database is setup for auto-loading, preload the corresponding autoLoad flag in the persistent settings to match (i.e. force on):
		//	Note: This has to use the TBibleDescriptor object because the other data hasn't been set yet!
		if (bblDesc.m_btoFlags & BTO_AutoLoad) {
			TBibleDatabaseSettings bblDBaseSettings = CPersistentSettings::instance()->bibleDatabaseSettings(bblDesc.m_strUUID);
			bblDBaseSettings.setLoadOnStart(true);
			CPersistentSettings::instance()->setBibleDatabaseSettings(bblDesc.m_strUUID, bblDBaseSettings);
		}
	}

	// Even though the main compatibility UUID is always derived from the Bible Database in ReadDB,
	//		we are the current the one to determine cross-database compatibility.  Perhaps this would
	//		best be kept in the database as well, but it's almost a toss-up.
	m_strHighlighterUUID = bblDesc.m_strHighlighterUUID;

	m_btoFlags = bblDesc.m_btoFlags;
}

CBibleDatabase::~CBibleDatabase()
{
	if (m_pKJPBSWordScriptureObject) {
		delete m_pKJPBSWordScriptureObject;
		m_pKJPBSWordScriptureObject = NULL;
	}
}

TBibleDatabaseSettings CBibleDatabase::settings() const
{
	return CPersistentSettings::instance()->bibleDatabaseSettings(m_strCompatibilityUUID);
}

void CBibleDatabase::setSettings(const TBibleDatabaseSettings &aSettings)
{
	CPersistentSettings::instance()->setBibleDatabaseSettings(m_strCompatibilityUUID, aSettings);
}

bool CBibleDatabase::completelyContains(const TPhraseTag &aPhraseTag) const
{
	return bounds().completelyContains(aPhraseTag.bounds(this));
}

TTagBoundsPair CBibleDatabase::bounds() const
{
	return TTagBoundsPair(1, m_EntireBible.m_nNumWrd);
}

TPhraseTag CBibleDatabase::bookPhraseTag(const CRelIndex &nRelIndex) const
{
	TPhraseTag aTag(CRelIndex(nRelIndex.book(), 0, 0, 0));
	const CBookEntry *pBook = bookEntry(aTag.relIndex());
	if (pBook) aTag.setCount(pBook->m_nNumWrd);
	return aTag;
}

TPhraseTag CBibleDatabase::chapterPhraseTag(const CRelIndex &nRelIndex) const
{
	TPhraseTag aTag(CRelIndex(nRelIndex.book(), nRelIndex.chapter(), 0, 0));
	const CChapterEntry *pChapter = chapterEntry(aTag.relIndex());
	if (pChapter) aTag.setCount(pChapter->m_nNumWrd);
	return aTag;
}

TPhraseTag CBibleDatabase::versePhraseTag(const CRelIndex &nRelIndex) const
{
	TPhraseTag aTag(CRelIndex(nRelIndex.book(), nRelIndex.chapter(), nRelIndex.verse(), 0));
	const CVerseEntry *pVerse = verseEntry(aTag.relIndex());
	if (pVerse) aTag.setCount(pVerse->m_nNumWrd);
	return aTag;
}

void CBibleDatabase::registerTextLayoutHandlers(QAbstractTextDocumentLayout *pDocLayout)
{
	assert(m_pKJPBSWordScriptureObject != NULL);
	m_pKJPBSWordScriptureObject->registerTextLayoutHandlers(pDocLayout);
}

const CTestamentEntry *CBibleDatabase::testamentEntry(uint32_t nTst) const
{
	assert((nTst >= 1) && (nTst <= m_lstTestaments.size()));
	if ((nTst < 1) || (nTst > m_lstTestaments.size())) return NULL;
	return &m_lstTestaments.at(nTst-1);
}

const CBookCategoryEntry *CBibleDatabase::bookCategoryEntry(uint32_t nCat) const
{
	assert((nCat >= 1) && (nCat <= m_lstBookCategories.size()));
	if ((nCat < 1) || (nCat > m_lstBookCategories.size())) return NULL;
	return &m_lstBookCategories.at(nCat-1);
}

const CBookEntry *CBibleDatabase::bookEntry(uint32_t nBk) const
{
	if ((nBk < 1) || (nBk > m_lstBooks.size())) return NULL;
	return &m_lstBooks.at(nBk-1);
}

const CBookEntry *CBibleDatabase::bookEntry(const CRelIndex &ndx) const
{
	return bookEntry(ndx.book());
}

#ifdef OSIS_PARSER_BUILD
const CChapterEntry *CBibleDatabase::chapterEntry(const CRelIndex &ndx, bool bForceCreate) const
{
	if (bForceCreate) (const_cast<TChapterMap &>(m_mapChapters))[CRelIndex(ndx.book(),ndx.chapter(),0,0)];		// Force the creation of this entry
	TChapterMap::const_iterator chapter = m_mapChapters.find(CRelIndex(ndx.book(),ndx.chapter(),0,0));
	if (chapter == m_mapChapters.end()) return NULL;
	return &(chapter->second);
}

const CVerseEntry *CBibleDatabase::verseEntry(const CRelIndex &ndx, bool bForceCreate) const
{
	if ((ndx.book() < 1) || (ndx.book() > m_lstBookVerses.size())) return NULL;
	const TVerseEntryMap &book = m_lstBookVerses[ndx.book()-1];
	if (bForceCreate) (const_cast<TVerseEntryMap &>(book))[CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0)];			// Force the creation of this entry
	const TVerseEntryMap::const_iterator mapVerse = book.find(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0));
	if (mapVerse == book.end()) return NULL;
	return &(mapVerse->second);
}
#else
const CChapterEntry *CBibleDatabase::chapterEntry(const CRelIndex &ndx) const
{
	TChapterMap::const_iterator chapter = m_mapChapters.find(CRelIndex(ndx.book(),ndx.chapter(),0,0));
	if (chapter == m_mapChapters.end()) return NULL;
	return &(chapter->second);
}

const CVerseEntry *CBibleDatabase::verseEntry(const CRelIndex &ndx) const
{
	if ((ndx.book() < 1) || (ndx.book() > m_lstBookVerses.size())) return NULL;
	const TVerseEntryMap &book = m_lstBookVerses[ndx.book()-1];
	const TVerseEntryMap::const_iterator mapVerse = book.find(CRelIndex(ndx.book(), ndx.chapter(), ndx.verse(), 0));
	if (mapVerse == book.end()) return NULL;
	return &(mapVerse->second);
}
#endif

const CWordEntry *CBibleDatabase::wordlistEntry(const QString &strWord) const
{
	TWordListMap::const_iterator word = m_mapWordList.find(strWord);
	if (word == m_mapWordList.end()) return NULL;
	return &(word->second);
}

void CBibleDatabase::setRenderedWords()
{
	for (TWordListMap::iterator itrWord = m_mapWordList.begin(); itrWord != m_mapWordList.end(); ++itrWord) {
		setRenderedWords(itrWord->second);
	}
}

void CBibleDatabase::setRenderedWords(CWordEntry &aWordEntry) const
{
	bool bHideHyphens = false;

	if ((settings().hideHyphens() & TBibleDatabaseSettings::HHO_ProperWords) && (aWordEntry.m_bIsProperWord)) {
		bHideHyphens = true;
	} else if ((settings().hideHyphens() & TBibleDatabaseSettings::HHO_OrdinaryWords) && (!aWordEntry.m_bIsProperWord)) {
		bHideHyphens = true;
	}

	assert(aWordEntry.m_lstAltWords.size() == aWordEntry.m_lstRenderedAltWords.size());
	for (int ndx = 0; ndx < aWordEntry.m_lstAltWords.size(); ++ndx) {
		aWordEntry.m_lstRenderedAltWords[ndx] = (bHideHyphens ? CSearchStringListModel::deHyphen(aWordEntry.m_lstAltWords.at(ndx), true) : aWordEntry.m_lstAltWords.at(ndx));
	}
}

int CBibleDatabase::concordanceIndexForWordAtIndex(uint32_t ndxNormal) const
{
	if ((ndxNormal < 1) || (ndxNormal >= m_lstConcordanceMapping.size()))
		return -1;
	return m_lstConcordanceMapping.at(ndxNormal);
}

int CBibleDatabase::concordanceIndexForWordAtIndex(const CRelIndex &relIndex) const
{
	if (!relIndex.isSet()) return -1;
	return concordanceIndexForWordAtIndex(NormalizeIndex(relIndex));
}

const CConcordanceEntry *CBibleDatabase::concordanceEntryForWordAtIndex(uint32_t ndxNormal) const
{
	int nConcordanceIndex = concordanceIndexForWordAtIndex(ndxNormal);
	const CConcordanceEntry *pConcordanceEntry = ((nConcordanceIndex != -1) ? &m_lstConcordanceWords.at(nConcordanceIndex) : NULL);
	return pConcordanceEntry;
}

const CConcordanceEntry *CBibleDatabase::concordanceEntryForWordAtIndex(const CRelIndex &relIndex) const
{
	int nConcordanceIndex = concordanceIndexForWordAtIndex(relIndex);
	const CConcordanceEntry *pConcordanceEntry = ((nConcordanceIndex != -1) ? &m_lstConcordanceWords.at(nConcordanceIndex) : NULL);
	return pConcordanceEntry;
}

QString CBibleDatabase::wordAtIndex(uint32_t ndxNormal, bool bAsRendered) const
{
	if ((ndxNormal < 1) || (ndxNormal >= m_lstConcordanceMapping.size()))
		return QString();

	if (bAsRendered) {
		return m_lstConcordanceWords.at(m_lstConcordanceMapping.at(ndxNormal)).renderedWord();
	} else {
		return m_lstConcordanceWords.at(m_lstConcordanceMapping.at(ndxNormal)).word();
	}
}

QString CBibleDatabase::wordAtIndex(const CRelIndex &relIndex, bool bAsRendered) const
{
	if (!relIndex.isSet()) return QString();
	return wordAtIndex(NormalizeIndex(relIndex), bAsRendered);
}

QString CBibleDatabase::decomposedWordAtIndex(uint32_t ndxNormal) const
{
	assert((ndxNormal >= 1) && (ndxNormal < m_lstConcordanceMapping.size()));
	if ((ndxNormal < 1) || (ndxNormal >= m_lstConcordanceMapping.size()))
		return QString();

	return m_lstConcordanceWords.at(m_lstConcordanceMapping.at(ndxNormal)).decomposedWord();
}

const CFootnoteEntry *CBibleDatabase::footnoteEntry(const CRelIndex &ndx) const
{
	TFootnoteEntryMap::const_iterator footnote = m_mapFootnotes.find(ndx);
	if (footnote == m_mapFootnotes.end()) return NULL;
	return &(footnote->second);
}

QString CBibleDatabase::richVerseText(const CRelIndex &ndxRel, const CVerseTextRichifierTags &tags, bool bAddAnchors, const CBasicHighlighter *aHighlighter) const
{
	CRelIndex ndx = ndxRel;
	ndx.setWord(0);							// We always return the whole verse, not specific words
	const CVerseEntry *pVerse = verseEntry(ndx);
	assert(pVerse != NULL);

#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
	TVerseCacheMap &cache = (bAddAnchors ? m_mapVerseCacheWithAnchors[tags.hash()] : m_mapVerseCacheNoAnchors[tags.hash()]);
	TVerseCacheMap::iterator itr = cache.find(ndx);
	if (itr != cache.end()) return (itr->second);
	cache[ndx] = CVerseTextRichifier::parse(ndx, this, pVerse, tags, bAddAnchors, NULL, aHighlighter);
	return cache[ndx];
#else
	return CVerseTextRichifier::parse(ndx, this, pVerse, tags, bAddAnchors, NULL, aHighlighter);
#endif
}

#ifdef BIBLE_DATABASE_RICH_TEXT_CACHE
void CBibleDatabase::dumpRichVerseTextCache(uint nTextRichifierTagHash)
{
	if (nTextRichifierTagHash == 0) {
		m_mapVerseCacheWithAnchors.clear();
		m_mapVerseCacheNoAnchors.clear();
		return;
	}

	TSpecVerseCacheMap::iterator itr;

	itr = m_mapVerseCacheWithAnchors.find(nTextRichifierTagHash);
	if (itr != m_mapVerseCacheWithAnchors.end())
		(itr->second).clear();

	itr = m_mapVerseCacheNoAnchors.find(nTextRichifierTagHash);
	if (itr != m_mapVerseCacheNoAnchors.end())
		(itr->second).clear();
}
#endif

QString CBibleDatabase::soundEx(const QString &strDecomposedConcordanceWord, bool bCache) const
{
	if (bCache) {
		QString &strSoundEx = m_mapSoundEx[strDecomposedConcordanceWord];
		assert(!language().isEmpty());
		if (strSoundEx.isEmpty()) strSoundEx = CSoundExSearchCompleterFilter::soundEx(strDecomposedConcordanceWord, CSoundExSearchCompleterFilter::languageValue(language()));
		return strSoundEx;
	}

	TSoundExMap::const_iterator itrSoundEx = m_mapSoundEx.find(strDecomposedConcordanceWord);
	if (itrSoundEx != m_mapSoundEx.end()) return (itrSoundEx->second);
	assert(!language().isEmpty());
	return CSoundExSearchCompleterFilter::soundEx(strDecomposedConcordanceWord, CSoundExSearchCompleterFilter::languageValue(language()));
}

// ============================================================================

CDictionaryWordEntry::CDictionaryWordEntry()
{

}

CDictionaryWordEntry::CDictionaryWordEntry(const QString &strWord)
	:	m_strWord(strWord),
		m_strDecomposedWord(CSearchStringListModel::decompose(strWord, false).toLower())
{

}

// ============================================================================

CDictionaryDatabase::CDictionaryDatabase(const TDictionaryDescriptor &dctDesc)
	:	m_dtoFlags(dctDesc.m_dtoFlags),
		m_strName(dctDesc.m_strDBName),
		m_strDescription(dctDesc.m_strDBDesc),
		m_strCompatibilityUUID(dctDesc.m_strUUID)
{

}

CDictionaryDatabase::~CDictionaryDatabase()
{
#ifndef NOT_USING_SQL
	if (isLiveDatabase()) {
		assert(m_myDatabase.contains(m_strCompatibilityUUID));
		m_myDatabase.close();
		m_myDatabase = QSqlDatabase();
		QSqlDatabase::removeDatabase(m_strCompatibilityUUID);
	} else {
		assert(!m_myDatabase.contains(m_strCompatibilityUUID));
	}
#endif
}

QString CDictionaryDatabase::soundEx(const QString &strDecomposedDictionaryWord, bool bCache) const
{
	if (bCache) {
		QString &strSoundEx = m_mapSoundEx[strDecomposedDictionaryWord];
		assert(!language().isEmpty());
		if (strSoundEx.isEmpty()) strSoundEx = CSoundExSearchCompleterFilter::soundEx(strDecomposedDictionaryWord, CSoundExSearchCompleterFilter::languageValue(language()));
		return strSoundEx;
	}

	TSoundExMap::const_iterator itrSoundEx = m_mapSoundEx.find(strDecomposedDictionaryWord);
	if (itrSoundEx != m_mapSoundEx.end()) return (itrSoundEx->second);
	assert(!language().isEmpty());
	return CSoundExSearchCompleterFilter::soundEx(strDecomposedDictionaryWord, CSoundExSearchCompleterFilter::languageValue(language()));
}

QString CDictionaryDatabase::definition(const QString &strWord) const
{
	QString strDecomposedWord = CSearchStringListModel::decompose(strWord, false).toLower();
	TDictionaryWordListMap::const_iterator itrWord = m_mapWordDefinitions.find(strDecomposedWord);
	if (itrWord == m_mapWordDefinitions.end()) return QString();
	return CReadDatabase::dictionaryDefinition(this, itrWord->second);
}

bool CDictionaryDatabase::wordExists(const QString &strWord) const
{
	QString strDecomposedWord = CSearchStringListModel::decompose(strWord, false).toLower();
	TDictionaryWordListMap::const_iterator itrWord = m_mapWordDefinitions.find(strDecomposedWord);
	return (itrWord != m_mapWordDefinitions.end());
}

// ============================================================================

TTagBoundsPair::TTagBoundsPair(uint32_t nNormalLo, uint32_t nNormalHi, bool bHadCount)
	:	m_pairNormals(nNormalLo, nNormalHi),
		m_bHadCount(bHadCount)
{

}

TTagBoundsPair::TTagBoundsPair(const TTagBoundsPair &tbpSrc)
	:	m_pairNormals(tbpSrc.m_pairNormals),
		m_bHadCount(tbpSrc.m_bHadCount)
{

}

TTagBoundsPair::TTagBoundsPair(const TPhraseTag &aTag, const CBibleDatabase *pBibleDatabase)
{
	uint32_t nNormalRefLo = pBibleDatabase->NormalizeIndex(aTag.relIndex());
	uint32_t nNormalRefHi = nNormalRefLo + aTag.count() - ((aTag.count() != 0) ? 1 : 0);
	m_pairNormals = TNormalPair(nNormalRefLo, nNormalRefHi);
	m_bHadCount = (aTag.count() != 0);
}

bool TTagBoundsPair::completelyContains(const TTagBoundsPair &tbpSrc) const
{
	if (!isValid() || !tbpSrc.isValid()) return false;
	return ((tbpSrc.lo() >= lo()) && (tbpSrc.hi() <= hi()));
}

bool TTagBoundsPair::intersects(const TTagBoundsPair &tbpSrc) const
{
	if (!isValid() || !tbpSrc.isValid()) return false;
	return (((tbpSrc.lo() >= lo()) && (tbpSrc.lo() <= hi())) ||			// Front end of Src starts somewhere in This range
			((tbpSrc.hi() >= lo()) && (tbpSrc.hi() <= hi())) ||			// Tail end of Src ends somewhere in This range
			((lo() >= tbpSrc.lo()) && (lo() <= tbpSrc.hi())) ||			// Front end of This starts somewhere in Src range
			((hi() >= tbpSrc.lo()) && (hi() <= tbpSrc.hi())));			// Tail end of This ends somewhere in Src range
}

bool TTagBoundsPair::intersectingInsert(const TTagBoundsPair &tbpSrc)
{
	if (intersects(tbpSrc)) {
		m_pairNormals = TNormalPair(qMin(lo(), tbpSrc.lo()), qMax(hi(), tbpSrc.hi()));
		m_bHadCount = m_bHadCount || tbpSrc.hadCount();
		return true;
	}
	return false;
}

bool TTagBoundsPair::intersectingTrim(const TTagBoundsPair &tbpSrc)
{
	if (intersects(tbpSrc)) {
		m_pairNormals = TNormalPair(qMax(lo(), tbpSrc.lo()), qMin(hi(), tbpSrc.hi()));
		m_bHadCount = m_bHadCount || tbpSrc.hadCount();
		return true;
	}
	return false;
}

// ============================================================================

TPhraseTag::TPhraseTag(const CBibleDatabase *pBibleDatabase, const TTagBoundsPair &tbpSrc)
{
	m_RelIndex = pBibleDatabase->DenormalizeIndex(tbpSrc.lo());
	m_nCount = (tbpSrc.hi() - tbpSrc.lo() + 1);
	if ((m_nCount == 1) && (!tbpSrc.hadCount())) m_nCount = 0;
}

void TPhraseTag::setFromPassageTag(const CBibleDatabase *pBibleDatabase, const TPassageTag &tagPassage)
{
	if (!tagPassage.isSet()) {
		m_RelIndex = CRelIndex();
		m_nCount = 0;
	} else if ((tagPassage.relIndex().isColophon()) || (tagPassage.relIndex().isSuperscription())) {
		assert(pBibleDatabase != NULL);
		m_RelIndex = tagPassage.relIndex();
		assert(m_RelIndex.word() == 1);			// All passages should start at first word of verse (see its constructors)
		assert(tagPassage.verseCount() == 1);	// Colophons and superscriptions each constitute one pseudo-verse
		const CVerseEntry *pEntry = pBibleDatabase->verseEntry(m_RelIndex);
		m_nCount = ((pEntry != NULL) ? pEntry->m_nNumWrd : 0);
	} else {
		assert(pBibleDatabase != NULL);
		m_RelIndex = tagPassage.relIndex();
		assert(m_RelIndex.word() == 1);			// All passages should start at first word of verse (see its constructors)
		CRelIndex ndxStart = tagPassage.relIndex();
		CRelIndex ndxTarget = tagPassage.relIndex();
		if (tagPassage.verseCount() > 1) {
			// If more than one verse is specified, find the beginning of the last verse:
			ndxTarget = pBibleDatabase->calcRelIndex(0, tagPassage.verseCount()-1, 0, 0, 0, ndxStart);
		}
		if (pBibleDatabase->verseEntry(ndxTarget) != NULL) {
			ndxTarget.setWord(pBibleDatabase->verseEntry(ndxTarget)->m_nNumWrd);		// Select all words of last verse
		}
		m_nCount = pBibleDatabase->NormalizeIndex(ndxTarget) - pBibleDatabase->NormalizeIndex(ndxStart) + 1;
	}
}

QString TPhraseTag::PassageReferenceRangeText(const CBibleDatabase *pBibleDatabase) const {
	assert(pBibleDatabase != NULL);

	if (pBibleDatabase == NULL) return QString();
	QString strReferenceRangeText = pBibleDatabase->PassageReferenceText(m_RelIndex);
	if (m_nCount > 1) {
		uint32_t nNormal = pBibleDatabase->NormalizeIndex(m_RelIndex);
		strReferenceRangeText += " - " + pBibleDatabase->PassageReferenceText(pBibleDatabase->DenormalizeIndex(nNormal + m_nCount - 1));
	}
	return strReferenceRangeText;
}

TTagBoundsPair TPhraseTag::bounds(const CBibleDatabase *pBibleDatabase) const
{
	return TTagBoundsPair(*this, pBibleDatabase);
}

bool TPhraseTag::completelyContains(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	if ((!relIndex().isSet()) || (!aTag.relIndex().isSet())) return false;
	assert(pBibleDatabase != NULL);
	return bounds(pBibleDatabase).completelyContains(aTag.bounds(pBibleDatabase));
}

bool TPhraseTag::intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	if ((!relIndex().isSet()) || (!aTag.relIndex().isSet())) return false;
	assert(pBibleDatabase != NULL);
	return bounds(pBibleDatabase).intersects(aTag.bounds(pBibleDatabase));
}

bool TPhraseTag::intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag)
{
	if ((!relIndex().isSet()) || (!aTag.relIndex().isSet())) return false;

	assert(pBibleDatabase != NULL);

	TTagBoundsPair tbpRef = bounds(pBibleDatabase);

	if (tbpRef.intersectingInsert(aTag.bounds(pBibleDatabase))) {
		*this = TPhraseTag(pBibleDatabase, tbpRef);
		return true;
	}

	return false;
}

TPhraseTag TPhraseTag::mask(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	if (!isSet() || !aTag.isSet()) return TPhraseTag();

	TTagBoundsPair tbpRef = bounds(pBibleDatabase);
	TTagBoundsPair tbpTag = aTag.bounds(pBibleDatabase);

	if (!tbpRef.intersects(tbpTag)) return TPhraseTag();

	uint32_t ndxNormal = qMax(tbpRef.lo(), tbpTag.lo());
	unsigned int nCount = qMin(tbpRef.hi(), tbpTag.hi()) - ndxNormal + 1;
	if ((nCount == 1) && !tbpRef.hadCount() && !tbpTag.hadCount()) nCount = 0;

	return TPhraseTag(pBibleDatabase->DenormalizeIndex(ndxNormal), nCount);
}

// ============================================================================

TPhraseTagList::TPhraseTagList()
	:	QList<TPhraseTag>()
{

}

TPhraseTagList::TPhraseTagList(const TPhraseTag &aTag)
	:	QList<TPhraseTag>()
{
	append(aTag);
}

TPhraseTagList::TPhraseTagList(const TPhraseTagList &src)
	:	QList<TPhraseTag>(src)
{

}

TPhraseTagList::TPhraseTagList(const CBibleDatabase *pBibleDatabase, const TPassageTagList &lstPassageTags)
	:	QList<TPhraseTag>()
{
	setFromPassageTagList(pBibleDatabase, lstPassageTags);
}

bool TPhraseTagList::isSet() const
{
	if (isEmpty()) return false;

	bool bIsSet = true;
	for (const_iterator itrTags = begin(); itrTags != end(); ++itrTags) {
		if (!itrTags->isSet()) {
			bIsSet = false;
			break;
		}
	}

	return bIsSet;
}

void TPhraseTagList::setFromPassageTagList(const CBibleDatabase *pBibleDatabase, const TPassageTagList &lstPassageTags)
{
	clear();
	reserve(lstPassageTags.size());
	for (int ndx = 0; ndx < lstPassageTags.size(); ++ndx) {
		append(TPhraseTag(pBibleDatabase, lstPassageTags.at(ndx)));
	}
}

bool TPhraseTagList::completelyContains(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	assert(pBibleDatabase != NULL);

	if (!aTag.relIndex().isSet()) return false;

	// To avoid having to flatten our entire list of tags and check for complete intersection, it
	//	would be easier to check every word of the specified tag and see if we have an intersection.
	//	Since we are limiting it to one-word tags, if we have an intersection for all of them, we
	//	know it's completely contained:
	TTagBoundsPair tbpSrc = aTag.bounds(pBibleDatabase);

	bool bContained = true;

	uint32_t nNormalLo = tbpSrc.lo();
	while ((bContained) && (nNormalLo <= tbpSrc.hi())) {
		bool bFound = false;
		for (const_iterator itrTags = constBegin(); ((!bFound) && (itrTags != constEnd())); ++itrTags) {
			TTagBoundsPair tbpRef = itrTags->bounds(pBibleDatabase);
			if ((nNormalLo >= tbpRef.lo()) && (nNormalLo <= tbpRef.hi())) bFound = true;
		}
		if (!bFound) bContained = false;
		++nNormalLo;
	}

	return bContained;
}

bool TPhraseTagList::completelyContains(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &aTagList) const
{
	for (int ndx = 0; ndx < aTagList.size(); ++ndx) {
		if (!completelyContains(pBibleDatabase, aTagList.at(ndx))) return false;
	}
	return true;
}

bool TPhraseTagList::intersects(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag) const
{
	assert(pBibleDatabase != NULL);

	if (!aTag.relIndex().isSet()) return false;

	for (const_iterator itrTags = begin(); itrTags != end(); ++itrTags) {
		if (itrTags->intersects(pBibleDatabase, aTag)) return true;
	}

	return false;
}

void TPhraseTagList::intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag)
{
	assert(pBibleDatabase != NULL);

	if (!aTag.relIndex().isSet()) return;

	iterator itrLast = end();			// Tag we are comparing once we find an intersection, just initialize it to something (I don't like uninitialized vars!)
	bool bFoundFirst = false;			// True when we find our first intersection

	//	When we find an intersection, combine them, move

	for (iterator itrTags = begin(); itrTags != end(); /* iterator inside loop */) {
		if (!bFoundFirst) {
			if (itrTags->intersectingInsert(pBibleDatabase, aTag)) {
				// If we find an intersection, update it and set our iterator to it.  But keep
				//		processing in case we find others:
				itrLast = itrTags;
				bFoundFirst = true;
			}
		} else {
			// If we've already found one, compare the next one with the previous.
			if (itrLast->intersectingInsert(pBibleDatabase, *itrTags)) {
				// If we were able to insert it back into the other, calc our iterator distances
				//		so we can nuke the list with a remove, and then continue:
				int nDistLast = std::distance(begin(), itrLast);
				itrTags = erase(itrTags);
				itrLast = begin() + nDistLast;		// Fix our iterator to our combining tag
				continue;			// No need to increment our iterator, since we just did in the erase
			}
		}

		++itrTags;
	}
	if (!bFoundFirst) append(aTag);			// If we didn't find it anywhere, add the new one on the end
}

void TPhraseTagList::intersectingInsert(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &aTagList)
{
	assert(pBibleDatabase != NULL);
	if (aTagList.isEmpty()) return;

	const_iterator itrNewTags = aTagList.constBegin();

	for (iterator itrOrgTags = begin(); itrOrgTags != end(); /* iterator inside loop */) {
		// Find next new tag that's set:
		while ((itrNewTags != aTagList.constEnd()) && (!itrNewTags->isSet())) {
			++itrNewTags;
		}
		if (itrNewTags == aTagList.constEnd()) break;				// When we've exhausted our input, we are done

		// Find first org tag that's set:
		if (!itrOrgTags->isSet()) {
			++itrOrgTags;
			continue;
		}

		TTagBoundsPair tbpOrg = itrOrgTags->bounds(pBibleDatabase);
		TTagBoundsPair tbpNew = itrNewTags->bounds(pBibleDatabase);

		// If this Org set is completely less than the next new tag, we
		//		have no intersection, continue on and find the first that
		//		does intersect or the end of the list:
		if (tbpOrg.hi() < tbpNew.lo()) {
			++itrOrgTags;
			continue;
		}

		// While the new tag is completely lower than the current tag within this
		//		Org set, then they don't intersect and should just be inserted:
		while ((itrNewTags != aTagList.constEnd()) && (tbpNew.hi() < tbpOrg.lo())) {
			itrOrgTags = insert(itrOrgTags, *itrNewTags);
			++itrOrgTags;				// Returned iterator points to newly inserted item.  Go back to what is now the original location
			++itrNewTags;
			if (itrNewTags != aTagList.constEnd()) tbpNew = itrNewTags->bounds(pBibleDatabase);
		}
		if (itrNewTags == aTagList.constEnd()) break;			// If we've exhausted our input, we are done

		// Recheck if our Org set is completely less than the next new tag, if
		//		so, they just passed each other above and we just have regular insertions,
		//		and not an intersection, to do next:
		if (tbpOrg.hi() < tbpNew.lo()) {
			++itrOrgTags;
			continue;
		}

		// At this point, we should have an intersection because if our original tags
		//		were completely less than our new ones, they didn't intersect and
		//		we've already continued above.  If the new tags were completely below
		//		the original tags, we've inserted them.
		//		So handle the intersection:
		bool bAtLeastOneIntersection = false;
		while ((itrNewTags != aTagList.constEnd()) && (tbpOrg.intersectingInsert(tbpNew))) {
			bAtLeastOneIntersection = true;
			++itrNewTags;
			if (itrNewTags != aTagList.constEnd()) tbpNew = itrNewTags->bounds(pBibleDatabase);
		}
		assert(bAtLeastOneIntersection);
		// See if any remaining indexes spilled over into the new intersection and
		//		if so, combine them:
		iterator itrOrgNextTag = itrOrgTags + 1;
		while ((itrOrgNextTag != end()) && (tbpOrg.intersectingInsert(itrOrgNextTag->bounds(pBibleDatabase)))) {
			++itrOrgNextTag;
		}
		*itrOrgTags = TPhraseTag(pBibleDatabase, tbpOrg);		// Write-back the newly updated value
		int nPosSave = std::distance(begin(), itrOrgTags);		// Save our position for when we nuke the iterator (One last than first position to nuke)
		int nPosLast = std::distance(begin(), itrOrgNextTag-1);	// Last position to nuke
		while (nPosLast > nPosSave) {
			removeAt(nPosLast);
			--nPosLast;
		}
		itrOrgTags = begin() + nPosSave;						// Restore our iterator
		// Continue without incrementing itrOrgTags, as the newly combined tag could intersect the next incoming tag
	}
	// If we exhausted our original tags, but still have new incoming tags we haven't
	//		exhausted, append them on the end:
	while (itrNewTags != aTagList.constEnd()) {
		append(*itrNewTags);
		++itrNewTags;
	}
}

bool TPhraseTagList::removeIntersection(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag)
{
	assert(pBibleDatabase != NULL);

	bool bRemovedIntersection = false;

	if (!aTag.relIndex().isSet()) return false;

	for (iterator itrTags = begin(); itrTags != end(); /* increment in loop */) {
		if ((!aTag.haveSelection()) && (pBibleDatabase->completelyContains(*itrTags))) {
			// If there is no selection and the tag in the list contains the specified
			//		reference and the tag is completely inside this Bible Database, remove it:
			if (itrTags->completelyContains(pBibleDatabase, aTag)) {
				itrTags = erase(itrTags);
				bRemovedIntersection = true;
				continue;
			}
		} else if (aTag.completelyContains(pBibleDatabase, *itrTags)) {
			assert(aTag.haveSelection());
			// If the passed tag completely contains the one in the list, remove the one in the list:
			itrTags = erase(itrTags);
			bRemovedIntersection = true;
			continue;
		} else if (itrTags->intersects(pBibleDatabase, aTag)) {
			// This is the difficult one...  There's an intersection between the two,
			//		but we don't know if it's the first part, last part, or some part in the middle.
			//		If it's some part in the middle, we have to split it in half (yuck):

			// Note: Check special case for no-selection (i.e. a "remove") but where the tag isn't entirely
			//		inside our Bible Database, in which case we should treat it like a trim for the
			//		part that's actually inside our database rather than remove it all:

			TTagBoundsPair tbpRef = itrTags->bounds(pBibleDatabase);
			TTagBoundsPair tbpSrc = (aTag.haveSelection() ? aTag.bounds(pBibleDatabase) : pBibleDatabase->bounds());
			if (!aTag.haveSelection()) tbpSrc.intersectingTrim(tbpRef);
			TTagBoundsPair tbpNew = tbpRef;
			tbpNew.setHadCount(tbpRef.hadCount() || tbpSrc.hadCount());			// If we end up with a single word, this will tell us if we have that word selected or not
			bool bSingle = false;

			if ((tbpSrc.lo() > tbpRef.lo()) && (tbpSrc.hi() >= tbpRef.hi())) {
				// Trim on the bottom part of the range case:
				tbpNew.setHi(tbpSrc.lo() - 1);
				bSingle = true;
			} else if ((tbpSrc.lo() <= tbpRef.lo()) && (tbpSrc.hi() < tbpRef.hi())) {
				// Trim on the top part of the range case:
				tbpNew.setLo(tbpSrc.hi() + 1);
				bSingle = true;
			}

			// if bSingle, trim to nNormalNew, else nNormalSrc is in the middle of nNormalRef and needs a split:
			if (bSingle) {
				*itrTags = TPhraseTag(pBibleDatabase, tbpNew);
				if (!(*itrTags).isSet()) {
					// Special case for dealing with Highlighters that run off the end of the Bible Database text.
					//		This can happen in the case of the KJVA Apocrypha text if you highlight the end of
					//		Revelation and extend into 1 Esdras of the Apocrypha.  Then, you edit the highlighters
					//		using the non-Apocrypha database and delete the portion (or part of the portion) of
					//		the highlighter in Revelation.  While the other database is compatible, you are
					//		editing a highlighter that extends outside the text of the current database and so
					//		the DenormalizeIndex() function will return 0, rather than the first word of the book
					//		just after this database.  That breaks the highlighter by setting the tag to "0" rather
					//		than the first word of the next book.  This "work-around" checks for that and manually
					//		sets it to the next word.  It does make the assumption (and asserts if it fails) that
					//		the last word of the tbpSrc boundary is still in this database:
					assert(tbpNew.lo() == (tbpSrc.hi() + 1));		// If we "ran off the database", make sure it's the upper half we are keeping
					if (tbpNew.lo() == (tbpSrc.hi() + 1)) {
						CRelIndex ndxUpper = (*itrTags).relIndex();
						ndxUpper = pBibleDatabase->DenormalizeIndex(tbpSrc.hi());
						assert(ndxUpper.isSet());
						ndxUpper = CRelIndex(ndxUpper.book()+1, 1, 1, 1);
						*itrTags = TPhraseTag(ndxUpper, (*itrTags).count());
					}
				}
			} else {
				// For the split, make the current be the low half:
				itrTags->m_RelIndex = pBibleDatabase->DenormalizeIndex(tbpRef.lo());
				itrTags->m_nCount = (tbpSrc.lo() - tbpRef.lo());		// Note, don't include first of cut section
				int nDistTags = std::distance(begin(), itrTags);		// Save where our iterator is at, since we're about to nuke it
				// And insert the upper half:
				CRelIndex ndxUpper = pBibleDatabase->DenormalizeIndex(tbpSrc.hi() + 1);
				if (!ndxUpper.isSet()) {
					// Special case for dealing with Highlighters that run off the end of the Bible Database text.
					//		This can happen in the case of the KJVA Apocrypha text if you highlight the end of
					//		Revelation and extend into 1 Esdras of the Apocrypha.  Then, you edit the highlighters
					//		using the non-Apocrypha database and delete the portion (or part of the portion) of
					//		the highlighter in Revelation.  While the other database is compatible, you are
					//		editing a highlighter that extends outside the text of the current database and so
					//		the DenormalizeIndex() function will return 0, rather than the first word of the book
					//		just after this database.  That breaks the highlighter by setting the tag to "0" rather
					//		than the first word of the next book.  This "work-around" checks for that and manually
					//		sets it to the next word.  It does make the assumption (and asserts if it fails) that
					//		the last word of the tbpSrc boundary is still in this database:
					ndxUpper = pBibleDatabase->DenormalizeIndex(tbpSrc.hi());
					assert(ndxUpper.isSet());
					ndxUpper = CRelIndex(ndxUpper.book()+1, 1, 1, 1);
				}
				append(TPhraseTag(ndxUpper, tbpRef.hi() - tbpSrc.hi()));
				// Fix the iterator we just nuked:
				itrTags = begin() + nDistTags;
			}
		}
		++itrTags;
	}

	return bRemovedIntersection;
}

int TPhraseTagList::findIntersectingIndex(const CBibleDatabase *pBibleDatabase, const TPhraseTag &aTag, int nStartIndex) const
{
	assert(pBibleDatabase != NULL);
	if (nStartIndex < 0) return -1;
	if (!aTag.isSet()) return -1;

	TTagBoundsPair tbpTag = aTag.bounds(pBibleDatabase);
	for (int ndx = nStartIndex; ndx < size(); ++ndx) {
		if (at(ndx).bounds(pBibleDatabase).intersects(tbpTag)) return ndx;
	}

	return -1;
}

// ============================================================================

void TPassageTag::setFromPhraseTag(const CBibleDatabase *pBibleDatabase, const TPhraseTag &tagPhrase)
{
	if (!tagPhrase.isSet()) {
		m_RelIndex = CRelIndex();
		m_nVerseCount = 0;
	} else if ((tagPhrase.relIndex().isColophon()) || (tagPhrase.relIndex().isSuperscription())) {
		m_RelIndex = tagPhrase.relIndex();
		m_RelIndex.setWord(1);
		m_nVerseCount = 1;
	} else {
		assert(pBibleDatabase != NULL);
		m_RelIndex = tagPhrase.relIndex();
		m_RelIndex.setWord(1);
		CRelIndex ndxTarget = tagPhrase.relIndex();
		if (tagPhrase.count() > 1) {
			ndxTarget = pBibleDatabase->calcRelIndex(tagPhrase.count()-1, 0, 0, 0, 0, tagPhrase.relIndex());
		}
		m_nVerseCount = (CRefCountCalc(pBibleDatabase, CRefCountCalc::RTE_VERSE, ndxTarget).ofBible().first -
						CRefCountCalc(pBibleDatabase, CRefCountCalc::RTE_VERSE, tagPhrase.relIndex()).ofBible().first) + 1;
	}
}

QString TPassageTag::PassageReferenceRangeText(const CBibleDatabase *pBibleDatabase) const {
	assert(pBibleDatabase != NULL);

	if (pBibleDatabase == NULL) return QString();
	CRelIndex ndxFirst(m_RelIndex);
	ndxFirst.setWord(0);
	QString strReferenceRangeText = pBibleDatabase->PassageReferenceText(ndxFirst);
	if (m_nVerseCount > 1) {
		CRelIndex ndxLast(pBibleDatabase->calcRelIndex(0, m_nVerseCount-1, 0, 0, 0, ndxFirst));
		ndxLast.setWord(0);
		strReferenceRangeText += " - " + pBibleDatabase->PassageReferenceText(ndxLast);
	}
	return strReferenceRangeText;
}

// ============================================================================

void TPassageTagList::setFromPhraseTagList(const CBibleDatabase *pBibleDatabase, const TPhraseTagList &lstPhraseTags)
{
	clear();
	reserve(lstPhraseTags.size());
	for (int ndx = 0; ndx < lstPhraseTags.size(); ++ndx) {
		append(TPassageTag(pBibleDatabase, lstPhraseTags.at(ndx)));
	}
}

unsigned int TPassageTagList::verseCount() const
{
	unsigned int nVerseCount = 0;
	for (int ndx = 0; ndx < size(); ++ndx) {
		nVerseCount += at(ndx).verseCount();
	}
	return nVerseCount;
}

// ============================================================================

bool HighlighterNameSortPredicate::operator() (const QString &v1, const QString &v2) const
{
	return (CSearchStringListModel::decompose(v1, false).compare(CSearchStringListModel::decompose(v2, false), Qt::CaseInsensitive) < 0);
}

// ============================================================================

