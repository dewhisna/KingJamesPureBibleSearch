/****************************************************************************
**
** Copyright (C) 2019 Donna Whisnant, a.k.a. Dewtronics.
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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/ReadDB.h"
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/VerseRichifier.h"
#include "../KJVCanOpener/SearchCompleter.h"
#include "../KJVCanOpener/PhraseEdit.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/KJVSearchCriteria.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QSharedPointer>
#include <QList>
#include <QMap>
#include <QHash>
#include <QVariant>

#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif

#include <iostream>
#include <set>
#include <algorithm>
#include <vector>

#define NUM_BK 80u				// Total Books Defined
#define NUM_BK_OT 39u			// Total Books in Old Testament
#define NUM_BK_NT 27u			// Total Books in New Testament
#define NUM_BK_APOC 14u			// Total Books in Apocrypha (KJVA)
#define NUM_TST 3u				// Total Number of Testaments (or pseudo-testaments, in the case of Apocrypha)

constexpr int MIN_SEARCH_WITHIN_CACHE_LIMIT = 100;		// Minimum number of results needed before caching a search phrase

#define DEBUG_MODE 0			// Set to 1 to enable debug output

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	const unsigned int VERSION = 10000;		// Version 1.0.0

	const char *g_constrBibleDatabasePath = "../../KJVCanOpener/db/";

	// Use translations from the main app:
	const char *g_constrTranslationsPath = "../../KJVCanOpener/translations/";
	const char *g_constrTranslationFilenamePrefix = "kjpbs";

}	// namespace


// ============================================================================

class CMyPhraseSearch;
typedef QHash<CPhraseEntry, CMyPhraseSearch> CSearchPhraseCacheHash;
CSearchPhraseCacheHash g_hashSearchPhraseCache;			// Keep searches already performed so we don't repeat them

class CMyPhraseSearch : public CParsedPhrase
{
public:
	CMyPhraseSearch(CBibleDatabasePtr pBibleDatabase = CBibleDatabasePtr(), uint32_t nNormalIndex = 0, bool bCaseSensitive = false, bool bAccentSensitive = false)
		:	CParsedPhrase(pBibleDatabase, bCaseSensitive, bAccentSensitive),
			m_nNormalIndex(nNormalIndex),
			m_nTargetLength(0),
			m_bSensitivityOptionsChanged(false)
	{
	}

	CMyPhraseSearch(const CMyPhraseSearch &aPhraseSearch)
		:	CParsedPhrase(aPhraseSearch.m_pBibleDatabase, aPhraseSearch.m_bCaseSensitive, aPhraseSearch.m_bAccentSensitive)
	{
		*this = aPhraseSearch;
	}

	CMyPhraseSearch &operator =(const CMyPhraseSearch &aPhraseSearch)
	{
		m_pBibleDatabase = aPhraseSearch.m_pBibleDatabase;
		m_nNormalIndex = aPhraseSearch.m_nNormalIndex;
		m_nTargetLength = aPhraseSearch.m_nTargetLength;
		m_bSensitivityOptionsChanged = aPhraseSearch.m_bSensitivityOptionsChanged;
		CParsedPhrase::operator =(aPhraseSearch);

		// Sneak the cache results across the copy too:
		m_cache_lstPhraseTagResults = aPhraseSearch.m_cache_lstPhraseTagResults;
		m_lstWithinPhraseTagResults = aPhraseSearch.m_lstWithinPhraseTagResults;
		return *this;
	}

	virtual ~CMyPhraseSearch()
	{ }

	virtual void setCaseSensitive(bool bCaseSensitive) override
	{
		m_bSensitivityOptionsChanged = (isCaseSensitive() != bCaseSensitive);
		CParsedPhrase::setCaseSensitive(bCaseSensitive);
	}

	virtual void setAccentSensitive(bool bAccentSensitive) override
	{
		m_bSensitivityOptionsChanged = (isAccentSensitive() != bAccentSensitive);
		CParsedPhrase::setAccentSensitive(bAccentSensitive);
	}

	virtual void FindWords() override
	{
		ClearWithinPhraseTagSearchResults();
		CParsedPhrase::FindWords();
	}

	virtual void ResumeFindWords() override
	{
		ClearWithinPhraseTagSearchResults();
		CParsedPhrase::ResumeFindWords();
	}

	bool nextPhraseLength()
	{
		assert(!m_pBibleDatabase.isNull());

		if (m_bSensitivityOptionsChanged) {
			QStringList lstPhrase;
			lstPhrase.reserve(m_nTargetLength+1);
			for (int ndx = 0; ndx < m_nTargetLength+1; ++ndx) {
				const CConcordanceEntry *pConcordEntry = m_pBibleDatabase->concordanceEntryForWordAtIndex(m_nNormalIndex+ndx);
				if (pConcordEntry == NULL) {
					m_bSensitivityOptionsChanged = false;
					return false;
				}
				// If this isn't a case-sensitive search, go ahead and convert
				//		to lower-case here so that the hash-cache gets more hits
				lstPhrase.append(isCaseSensitive() ? pConcordEntry->renderedWord() : pConcordEntry->renderedWord().toLower());
			}
			primarySubPhrase()->ParsePhrase(lstPhrase);
		} else {
			const CConcordanceEntry *pConcordEntry = m_pBibleDatabase->concordanceEntryForWordAtIndex(m_nNormalIndex+m_nTargetLength);
			if (pConcordEntry == NULL) return false;
			primarySubPhrase()->AppendPhrase(isCaseSensitive() ? pConcordEntry->renderedWord() : pConcordEntry->renderedWord().toLower());
		}

		CPhraseEntry phraseEntry(*this);
		CSearchPhraseCacheHash::const_iterator itrCache = g_hashSearchPhraseCache.find(phraseEntry);
		if (itrCache != g_hashSearchPhraseCache.constEnd()) {
			uint32_t nSaveNormalIndex = m_nNormalIndex;		// Must save our index so we don't relocate to the cached location
			int nSaveTargetLength = m_nTargetLength;		// Use for comparison.  The cache had better be at the same level!
			*this = *itrCache;
			m_nNormalIndex = nSaveNormalIndex;
			assert(m_nTargetLength == (nSaveTargetLength+1));	// Compare to +1 since we will be incrementing it below
			m_nTargetLength = nSaveTargetLength;			// Restore so we don't over increment below
			// Note: Since the object was previously stored in the cache
			//	after calling this function, m_bSensitivityOptionsChanged
			//	will already be false in the cached copy
		} else {
			if ((m_nTargetLength == 0) || m_bSensitivityOptionsChanged) {
				FindWords();
			} else {
				ResumeFindWords();
			}
		}
		m_bSensitivityOptionsChanged = false;

		++m_nTargetLength;
		return true;
	}

	int targetLength() const { return m_nTargetLength; }

	bool hasConverged(const CSearchCriteria &searchCriteria, bool bSearchWithinIsEntireBible) const
			// Note: bSearchWithinIsEntireBible is only an optimization to keep from
			//		having to calculate it from the searchCriteria itself:
	{
		assert(!m_pBibleDatabase.isNull());
		// If we've only found one possible match OR if this phrase
		//	isn't within the searchCriteria, then consider it converged:
		TPhraseTag tagPhrase(m_pBibleDatabase->DenormalizeIndex(m_nNormalIndex), m_nTargetLength);
		bool bIsContained = ((bSearchWithinIsEntireBible) ?
								(m_pBibleDatabase->completelyContains(tagPhrase)) :
								(searchCriteria.phraseIsCompletelyWithin(m_pBibleDatabase, tagPhrase)));
		if ((m_nNormalIndex + m_nTargetLength) > m_pBibleDatabase->bibleEntry().m_nNumWrd) bIsContained = false;
		return ((bIsContained && (!isCaseSensitive() && !isAccentSensitive() && (GetNumberOfMatches() <= 1)))
				|| !bIsContained);
	}

	void buildWithinResultsInParsedPhrase(const CSearchCriteria &searchCriteria, bool bSearchWithinIsEntireBible)
			// Note: bSearchWithinIsEntireBible is only an optimization to keep from
			//		having to calculate it from the searchCriteria itself:
	{
		assert(!m_pBibleDatabase.isNull());
		const TPhraseTagList &lstPhraseTags = GetPhraseTagSearchResults();
		TPhraseTagList &lstWithinPhraseTags = GetWithinPhraseTagSearchResultsNonConst();
		if (!lstWithinPhraseTags.isEmpty()) return;

		if (bSearchWithinIsEntireBible) {
			lstWithinPhraseTags = lstPhraseTags;
			return;
		}

		lstWithinPhraseTags.reserve(lstPhraseTags.size());
		for (TPhraseTagList::const_iterator itrTags = lstPhraseTags.constBegin(); itrTags != lstPhraseTags.constEnd(); ++itrTags) {
			if (searchCriteria.phraseIsCompletelyWithin(m_pBibleDatabase, *itrTags)) lstWithinPhraseTags.append(*itrTags);
		}
	}

protected:
	uint32_t m_nNormalIndex;
	int m_nTargetLength = 0;
	bool m_bSensitivityOptionsChanged = false;
};

typedef QList<CMyPhraseSearch> CMyPhraseSearchList;

// ============================================================================

class CRenderFormat
{
public:
	void setIncludePerPhraseOccurrenceNumbers(bool bValue) { m_bIncludePerPhraseOccurrenceNumbers = bValue; }
	bool includePerPhraseOccurrenceNumbers() const { return m_bIncludePerPhraseOccurrenceNumbers; }

private:
	bool m_bIncludePerPhraseOccurrenceNumbers = false;
};

static QString renderResult(const CRenderFormat &aFormat, const CPhraseEntry &phraseEntry)
{
	return QString("%1%2(%3)%4")
			.arg(phraseEntry.caseSensitive() ? "" : "*")
			.arg(phraseEntry.accentSensitive() ? "^" : "")
			.arg(phraseEntry.caseSensitive() ? phraseEntry.text() : phraseEntry.text().toLower())
			.arg(aFormat.includePerPhraseOccurrenceNumbers() ?
					 QString(" [%1]").arg(phraseEntry.extraInfo().toInt()) : QString());
}

static QString renderResult(const CRenderFormat &aFormat, const CPhraseList &lstResult, int nModulus)
{
	QString strResult;
	int nTotalMatches = 0;

	for (int ndx = 0; ndx < lstResult.size(); ++ndx) {
		if (ndx) strResult += " / ";
		strResult += renderResult(aFormat, lstResult.at(ndx));
		nTotalMatches += lstResult.at(ndx).extraInfo().toInt();
	}

	strResult += QString(" [%1 * %2]").arg(nTotalMatches/nModulus).arg(nModulus);

	return strResult;
}

static bool ascendingLessThanTextFirst(const CPhraseList &lst1, const CPhraseList &lst2)
{
	assert(lst1.size() == lst2.size());
	for (int ndx = 0; ndx < lst1.size(); ++ndx) {
		// Sort Accent ahead of Case:
		if (lst1.at(ndx).accentSensitive() != lst2.at(ndx).accentSensitive()) {
			return lst1.at(ndx).accentSensitive();
		}
		if (lst1.at(ndx).caseSensitive() != lst2.at(ndx).caseSensitive()) {
			return lst1.at(ndx).caseSensitive();
		}

		QString strPhrase1 = lst1.at(ndx).caseSensitive() ? lst1.at(ndx).text() : lst1.at(ndx).text().toLower();
		QString strPhrase2 = lst2.at(ndx).caseSensitive() ? lst2.at(ndx).text() : lst2.at(ndx).text().toLower();
		QStringList lstPhrase1 = strPhrase1.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);
		QStringList lstPhrase2 = strPhrase2.normalized(QString::NormalizationForm_C).split(QRegExp("\\s+"), QString::SkipEmptyParts);

		if (lstPhrase1.size() < lstPhrase2.size()) return true;
		if (lstPhrase1.size() > lstPhrase2.size()) return false;

		for (int nWord = 0; nWord < lstPhrase1.size(); ++nWord) {
			int nTextComp = lstPhrase1.at(nWord).compare(lstPhrase2.at(nWord), Qt::CaseSensitive);
			if (nTextComp < 0) return true;
			if (nTextComp > 0) return false;
		}

		// If they are identical, it means we found a duplicate entry in our
		//	list (which can happen by finding the same phrase again later).
		//	But, the occurrence count had better be the same since we should
		//	have the same results for the same phrase:
		assert(lst1.at(ndx).extraInfo().toInt() == lst2.at(ndx).extraInfo().toInt());
	}
	return false;
}

static bool ascendingLessThanModulusFirst(const CPhraseList &lst1, const CPhraseList &lst2)
{
	assert(lst1.size() == lst2.size());
	int nCounts1 = 0;
	int nCounts2 = 0;
	for (int ndx = 0; ndx < lst1.size(); ++ndx) {
		nCounts1 += lst1.at(ndx).extraInfo().toInt();
		nCounts2 += lst2.at(ndx).extraInfo().toInt();
	}
	if (nCounts1 < nCounts2) return true;
	if (nCounts1 > nCounts2) return false;
	return ascendingLessThanTextFirst(lst1, lst2);		// Sort by text if the modulus result is the same
}

// ============================================================================

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	a.setApplicationVersion(QString("%1.%2.%3").arg(VERSION/10000).arg((VERSION/100)%100).arg(VERSION%100));

#if QT_VERSION < 0x050000
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	g_strTranslationsPath = QFileInfo(QCoreApplication::applicationDirPath(), g_constrTranslationsPath).absoluteFilePath();
	g_strTranslationFilenamePrefix = QString::fromUtf8(g_constrTranslationFilenamePrefix);

	// Load translations and set main application based on our locale:
	CTranslatorList::instance()->setApplicationLanguage();

	// ------------------------------------------------------------------------

	int nBibleDescriptor = -1;
	int nPhraseCount = 1;
	int nModulus = 2;
	int nArgsFound = 0;
	TBibleDescriptor bblDescriptor;
	bool bUnknownOption = false;
	bool bPreserveHyphenSensitive = false;
	bool bPreserveCaseSensitive = false;
	bool bPreserveAccentSensitive = false;
	bool bToggleCaseSensitive = false;
	bool bToggleAccentSensitive = false;
//	bool bConstrainBooks = false;
//	bool bConstrainChapters = false;
//	bool bConstrainVerses = false;
	bool bInvertCriteria = false;
	bool bOrderByModulus = false;
	CRenderFormat fmtRender;
	bool bVerbose = false;
	CSearchCriteria searchCriteria;
	TRelativeIndexSet setSearchWithin;
	bool bSearchWithinIsEntireBible = true;

	// Default to searching Entire Bible:
	for (unsigned int nBk = 1; nBk <= NUM_BK; ++nBk) {
		setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
	}
	setSearchWithin.insert(CSearchCriteria::SSI_COLOPHON);
	setSearchWithin.insert(CSearchCriteria::SSI_SUPERSCRIPTION);

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				nBibleDescriptor = strArg.toInt();
			} else if (nArgsFound == 2) {
				nPhraseCount = strArg.toInt();
				if (nPhraseCount < 1) {
					std::cerr << QString("*** Limiting Phrase Count to a minimum of 1\n").toUtf8().data();
					nPhraseCount = 1;
				}
			} else if (nArgsFound == 3) {
				nModulus = strArg.toInt();
				if (nModulus < 2) {
					std::cerr << QString("*** Limiting Modulus to a minimum of 2\n").toUtf8().data();
					nModulus = 2;
				}
			} else {
				bUnknownOption = true;
			}
		} else if (strArg.compare("-pc") == 0) {
			bPreserveCaseSensitive = true;
		} else if (strArg.compare("-pa") == 0) {
			bPreserveAccentSensitive = true;
		} else if (strArg.compare("-ph") == 0) {
			bPreserveHyphenSensitive = true;
		} else if (strArg.compare("-tc") == 0) {
			bToggleCaseSensitive = true;
		} else if (strArg.compare("-ta") == 0) {
			bToggleAccentSensitive = true;
		} else if (strArg.compare("-sc") == 0) {
			if (bInvertCriteria) {
				setSearchWithin.insert(CSearchCriteria::SSI_COLOPHON);
			} else {
				setSearchWithin.erase(CSearchCriteria::SSI_COLOPHON);
			}
		} else if (strArg.compare("-ss") == 0) {
			if (bInvertCriteria) {
				setSearchWithin.insert(CSearchCriteria::SSI_SUPERSCRIPTION);
			} else {
				setSearchWithin.erase(CSearchCriteria::SSI_SUPERSCRIPTION);
			}
		} else if (strArg.compare("-so") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_OT; ++nBk) {
				if (bInvertCriteria) {
					setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
				} else {
					setSearchWithin.erase(CRelIndex(nBk, 0, 0, 0));
				}
			}
		} else if (strArg.compare("-sn") == 0) {
			for (unsigned int nBk = 1; nBk <= NUM_BK_NT; ++nBk) {
				if (bInvertCriteria) {
					setSearchWithin.insert(CRelIndex(nBk+NUM_BK_OT, 0, 0, 0));
				} else {
					setSearchWithin.erase(CRelIndex(nBk+NUM_BK_OT, 0, 0, 0));
				}
			}
		} else if (strArg.startsWith("-s")) {
			unsigned int nBk = strArg.mid(2).toUInt();
			if (bInvertCriteria) {
				setSearchWithin.insert(CRelIndex(nBk, 0, 0, 0));
			} else {
				setSearchWithin.erase(CRelIndex(nBk, 0, 0, 0));
			}
		} else if (strArg.compare("-i") == 0) {
			bInvertCriteria = true;

			// Invert the current selection:
			TRelativeIndexSet setInvert;
			for (unsigned int nBk = 1; nBk <= NUM_BK; ++nBk) {
				if (setSearchWithin.find(CRelIndex(nBk, 0, 0, 0)) == setSearchWithin.end()) {
					setInvert.insert(CRelIndex(nBk, 0, 0, 0));
				}
			}
			if (setSearchWithin.find(CSearchCriteria::SSI_COLOPHON) == setSearchWithin.end())
				setInvert.insert(CSearchCriteria::SSI_COLOPHON);
			if (setSearchWithin.find(CSearchCriteria::SSI_SUPERSCRIPTION) == setSearchWithin.end())
				setInvert.insert(CSearchCriteria::SSI_SUPERSCRIPTION);

			setSearchWithin = setInvert;

		} else if (strArg.compare("-om") == 0) {
			bOrderByModulus = true;
		} else if (strArg.compare("-fn") == 0) {
			fmtRender.setIncludePerPhraseOccurrenceNumbers(true);
		} else if (strArg.compare("-v") == 0) {
			bVerbose = true;
		} else {
			bUnknownOption = true;
		}
	}

	if (bToggleCaseSensitive) bPreserveCaseSensitive = false;
	if (bToggleAccentSensitive) bPreserveAccentSensitive = false;

	if ((nArgsFound != 3) || (bUnknownOption)) {
		std::cerr << QString("KJVSumThing Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <Bible-UUID-Index> <Phrase-Count> <Modulus-Value>\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Reads the specified Bible Database and Searches for n-consecutive\n").toUtf8().data();
		std::cerr << QString("    phrases of varying length whose combined occurrence-count is\n").toUtf8().data();
		std::cerr << QString("    an even modulus of the specified Modulus-Value.\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Options are:\n").toUtf8().data();
		std::cerr << QString("  -pc =  Preserve Phrase Case-Sensitivity when comparing (default=false)\n").toUtf8().data();
		std::cerr << QString("  -pa =  Preserve Phrase Accent-Sensitivity when comparing (default=false)\n").toUtf8().data();
		std::cerr << QString("  -ph =  Preserve Hyphens when comparing (default=false)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("  -tc =  Toggle Phrase Case-Sensitivity when comparing (default=false, overrides -pc)\n").toUtf8().data();
		std::cerr << QString("  -ta =  Toggle Phrase Accent-Sensitivity when comparing (default=false, overrides -pa)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Search Criteria:\n").toUtf8().data();
		std::cerr << QString("  Default is to search the Entire Bible\n").toUtf8().data();
		std::cerr << QString("  -sc =  Skip Colophons (or Search Colophons if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -ss =  Skip Superscriptions (or Search Superscriptions if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -so =  Skip Old Testament (or Search Old Testament if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -sn =  Skip New Testament (or Search New Testament if -i is used)\n").toUtf8().data();
		std::cerr << QString("  -sN =  Skip Book 'N', where 'N' is Book Number in Bible\n").toUtf8().data();
		std::cerr << QString("           (or Search Book 'N' if -i is used)\n").toUtf8().data();
		std::cerr << QString("   -i =  Invert search criteria so that the default is to search\n").toUtf8().data();
		std::cerr << QString("           none of the Bible except when -sX options are used to\n").toUtf8().data();
		std::cerr << QString("           select a specific Book or Testament, etc.\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Output:\n").toUtf8().data();
		std::cerr << QString("  -om =  Output ordered by modulus multiplicand first\n").toUtf8().data();
		std::cerr << QString("           (Default is to order by text value first)\n").toUtf8().data();
		std::cerr << QString("  -fn =  Format: Include Per-Phrase Occurrence Count Numbers\n").toUtf8().data();
		std::cerr << QString("   -v =  Verbose display while searching (to stderr)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("Bible-UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx)).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Phrase-Count : Number of consecutive phrases searched (>=1)\n").toUtf8().data();
		std::cerr << QString("Modulus : Occurrence Count Modulus to find (>=2)\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		return -1;
	}

	if ((nBibleDescriptor >= 0) && (static_cast<unsigned int>(nBibleDescriptor) < bibleDescriptorCount())) {
		bblDescriptor = bibleDescriptor(static_cast<BIBLE_DESCRIPTOR_ENUM>(nBibleDescriptor));
	} else {
		std::cerr << "Unknown Bible-UUID-Index\n";
		return -1;
	}

	// ------------------------------------------------------------------------

	QFileInfo fiBblDBPath(QDir(QCoreApplication::applicationDirPath()), g_constrBibleDatabasePath);

	CReadDatabase rdbMain(fiBblDBPath.absoluteFilePath(), QString(), NULL);

	std::cerr << QString("Reading Bible Database: %1\n").arg(bblDescriptor.m_strDBName).toUtf8().data();
	if (!rdbMain.haveBibleDatabaseFiles(bblDescriptor)) {
		std::cerr << QString("\n*** ERROR: Unable to locate Bible Database Files!\n").toUtf8().data();
		return -2;
	}
	if (!rdbMain.ReadBibleDatabase(bblDescriptor, true)) {
		std::cerr << QString("\n*** ERROR: Failed to Read the Bible Database!\n").toUtf8().data();
		return -3;
	}

	// ------------------------------------------------------------------------

	CBibleDatabasePtr pBibleDatabase = TBibleDatabaseList::instance()->mainBibleDatabase();

	TBibleDatabaseSettings bdbSettings = pBibleDatabase->settings();
	bdbSettings.setHyphenSensitive(bPreserveHyphenSensitive);
	pBibleDatabase->setSettings(bdbSettings);

	searchCriteria.setSearchWithin(setSearchWithin);
	bSearchWithinIsEntireBible = searchCriteria.withinIsEntireBible(pBibleDatabase, false);

	std::cerr << "\n";
	std::cerr << QString("Searching within %1\n").arg(searchCriteria.searchWithinDescription(pBibleDatabase)).toUtf8().data();
	std::cerr << QString("for %1 Consecutive-Phrase(s) which have an Occurrence-Modulus of %2\n").arg(nPhraseCount).arg(nModulus).toUtf8().data();
	if (bToggleCaseSensitive) {
		std::cerr << QString("while toggling case-sensitivity\n").toUtf8().data();
	} else if (bPreserveCaseSensitive) {
		std::cerr << QString("while preserving case-sensitivity\n").toUtf8().data();
	}
	if (bToggleAccentSensitive) {
		std::cerr << QString("while toggling accent-sensitivity\n").toUtf8().data();
	} else if (bPreserveAccentSensitive) {
		std::cerr << QString("while preserving accent-sensitivity\n").toUtf8().data();
	}
	if (bPreserveHyphenSensitive) {
		std::cerr << QString("while preserving hyphenated words\n").toUtf8().data();
	}

	// ------------------------------------------------------------------------

	// Go through the entire Bible selecting n-consecutive known phrases from
	//	the given text, increasing the size of each phrase until its contributing
	//	occurrence count decreases to '1'.  Add the occurrence counts of the
	//	consecutive phrases together, and save ones that are an even modulus of
	//	the specified Modulus-Value:

	// lstOverallResults is a list of results lists.  There will be
	//		"nPhraseCount" entries in each CPhraseList in this list and an
	//		entry in the outer QList for each result that matches the modulus.
	//		The extraInfo member of each CPhraseEntry in the CPhraseList
	//		will contain the NumberOfMataches for for that phrase and is
	//		the count of results within the search criteria only:
	QList<CPhraseList> lstOverallResults;
	bool bNeedNewline = false;			// For output beautification

	uint32_t nNormalIndex = 1;
	uint32_t nBk = 0;
	uint32_t nChp = 0;
	while (nNormalIndex <= pBibleDatabase->bibleEntry().m_nNumWrd) {
		CRelIndex ndxPhrase(pBibleDatabase->DenormalizeIndex(nNormalIndex));
		if (!searchCriteria.indexIsWithin(CRelIndex(ndxPhrase.book(), 0, 0, 0))) {
			// Skip entire books we aren't searching:
			uint32_t nNewNormal = pBibleDatabase->NormalizeIndex(CRelIndex(ndxPhrase.book()+1, 0, 0, 0));
			if (nNewNormal > nNormalIndex) {
				nNormalIndex = nNewNormal;
			} else {
				// If we are at the end of the text, set the index to one beyond
				//	so we don't loop ad infinitum
				nNormalIndex = pBibleDatabase->bibleEntry().m_nNumWrd + 1;
			}
			continue;
		}
		if (nBk != ndxPhrase.book()) {
			nBk = ndxPhrase.book();
			nChp = 0;
			std::cerr << "\n" << pBibleDatabase->bookName(ndxPhrase).toUtf8().data();
		}
		if (nChp != ndxPhrase.chapter()) {
			nChp = ndxPhrase.chapter();
			std::cerr << ".";
			bNeedNewline = true;
		}

		CMyPhraseSearchList lstSearchPhrases;

		bool bSearchConverged = false;		// Set to 'true' when all phrases have converged to a single occurrence

		while (!bSearchConverged) {
			if (lstSearchPhrases.isEmpty()) {
				// First time at this Bible Word Index, start all new search phrases:
				for (int ndxNext = 0; ndxNext < nPhraseCount; ++ndxNext) {
					// Start with these running at the bCaseSensitive==true and bAccentSensitive==true if
					//		we are going to parse the cases with and without those.  We start with them
					//		set because those will always yield the same or less occurrences than without
					//		them set.  So start with the set, and clear them in subsequent rounds:
					if (ndxNext == 0) {
						lstSearchPhrases.append(CMyPhraseSearch(pBibleDatabase, nNormalIndex,
																(bToggleCaseSensitive || bPreserveCaseSensitive),
																(bToggleAccentSensitive || bPreserveAccentSensitive)));
					} else {
						lstSearchPhrases.append(CMyPhraseSearch(pBibleDatabase, nNormalIndex + lstSearchPhrases.at(ndxNext-1).targetLength(),
																(bToggleCaseSensitive || bPreserveCaseSensitive),
																(bToggleAccentSensitive || bPreserveAccentSensitive)));
					}
					lstSearchPhrases.last().nextPhraseLength();		// Set the search phrase, bump length, and perform search
				}
			} else {
				// Bump last phrase that hasn't converged.  When it converges,
				//	bump the phrase before it and restart the phrases after it.
				CMyPhraseSearchList::iterator itrLastNonconverged = lstSearchPhrases.end()-1;	// This is safe because minimum nPhraseCount is 1
				for (; itrLastNonconverged != lstSearchPhrases.begin(); --itrLastNonconverged) {
					if (!itrLastNonconverged->hasConverged(searchCriteria, bSearchWithinIsEntireBible)) break;
				}
				//	First, exhaust all combinations of AccentSensitive and CaseSensitive:
				if (bToggleAccentSensitive && itrLastNonconverged->isAccentSensitive()) {
					itrLastNonconverged->setAccentSensitive(false);
					itrLastNonconverged->FindWords();
				} else if (bToggleCaseSensitive && itrLastNonconverged->isCaseSensitive()) {
					itrLastNonconverged->setCaseSensitive(false);
					if (bToggleAccentSensitive) itrLastNonconverged->setAccentSensitive(true);
					itrLastNonconverged->FindWords();
				} else {
					// Here we need to see if we've reached convergence, and if not
					//	we must bump it and everything after us must start over in
					//	their new word positions:
					if (!itrLastNonconverged->hasConverged(searchCriteria, bSearchWithinIsEntireBible)) {
						// Not converged so bump it:
						if (bToggleAccentSensitive) {		// check flag instead of call so we don't clear the search results if not changing
							itrLastNonconverged->setAccentSensitive(bToggleAccentSensitive);
						}
						if (bToggleCaseSensitive) {			// check flag instead of call so we don't clear the search results if not changing
							itrLastNonconverged->setCaseSensitive(bToggleCaseSensitive);
						}
						if (!itrLastNonconverged->nextPhraseLength()) {	// Set the search phrase, bump length, and perform search
							// If we hit the end of the text and our last phrase
							//	hasn't converged, pretend that it has so we'll
							//	bail out of our loop:
							bSearchConverged = true;
						}
						// Remove the trailing phrases that have all converged:
						int ndxLastNonconverged = (itrLastNonconverged-lstSearchPhrases.begin());
						for (int ndxNext = lstSearchPhrases.size(); ndxNext > (ndxLastNonconverged+1); --ndxNext) {
							lstSearchPhrases.removeLast();
						}
						// Add new search phrases after this one starting them over
						//	on their new word positions:
						for (int ndxNext = ndxLastNonconverged+1; ndxNext < nPhraseCount; ++ndxNext) {
							lstSearchPhrases.append(CMyPhraseSearch(pBibleDatabase, nNormalIndex + lstSearchPhrases.at(ndxNext-1).targetLength(),
																	(bToggleCaseSensitive || bPreserveCaseSensitive),
																	(bToggleAccentSensitive || bPreserveAccentSensitive)));
							lstSearchPhrases.last().nextPhraseLength();		// Set the search phrase, bump length, and perform search
						}
					} else {
						// If the "LastNonconverged" was already converged, it
						//	means we are on the first phrase of the list and
						//	the search has completely converged:
						bSearchConverged = true;
					}
				}
			}

			bool bNoAccentOrCase = true;		// True if all search phrases are ignoring accent and case this cycle or aren't toggling it
			if (bToggleAccentSensitive || bToggleCaseSensitive) {
				for (int ndx = 0; (bNoAccentOrCase && (ndx < lstSearchPhrases.size())); ++ndx) {
					if ((lstSearchPhrases.at(ndx).isAccentSensitive() && bToggleAccentSensitive) ||
						(lstSearchPhrases.at(ndx).isCaseSensitive() && bToggleCaseSensitive))
						bNoAccentOrCase = false;
				}
			}

#if DEBUG_MODE
			{
				if (bNeedNewline) std::cerr << "\n";
				for (int ndx = 0; ndx < lstSearchPhrases.size(); ++ndx) {
					if (ndx) std::cerr << " / ";
					CPhraseEntry phraseEntry(lstSearchPhrases.at(ndx));
					std::cerr << renderResult(fmtRender, phraseEntry).toUtf8().data();
				}
				std::cerr << "\n";
			}
#endif

			// At this point, we have a list of Search Phrases that have valid
			//	search results.  We should now sum-up the results and see if it's
			//	an even modulus of our modulus-value.  If so, we should add the
			//	results to the output:
			unsigned int nTotalMatches = 0;
			for (int ndx = 0; ndx < lstSearchPhrases.size(); ++ndx) {
				lstSearchPhrases[ndx].buildWithinResultsInParsedPhrase(searchCriteria, bSearchWithinIsEntireBible);
				nTotalMatches += lstSearchPhrases.at(ndx).GetNumberOfMatchesWithin();

				if (lstSearchPhrases.at(ndx).GetNumberOfMatchesWithin() > MIN_SEARCH_WITHIN_CACHE_LIMIT) {
					CPhraseEntry phraseEntry(lstSearchPhrases.at(ndx));
					g_hashSearchPhraseCache[phraseEntry] = lstSearchPhrases.at(ndx);
				}
			}
			if (bNoAccentOrCase && (nTotalMatches < static_cast<unsigned int>(nModulus))) {
				// If the maximum search (i.e. no accent/case) yields a result
				//	less than the modulus, this search pass has converged:
				bSearchConverged = true;
			} else if ((nTotalMatches != 0) && ((nTotalMatches % nModulus) == 0)) {
				if (bNeedNewline && bVerbose) {
					std::cerr << "\n";
					bNeedNewline = false;
				}

				CPhraseList lstResult;
				for (int ndx = 0; ndx < lstSearchPhrases.size(); ++ndx) {
					CPhraseEntry phraseEntry(lstSearchPhrases.at(ndx));
					phraseEntry.setExtraInfo(QVariant(lstSearchPhrases.at(ndx).GetNumberOfMatchesWithin()));
					lstResult.append(phraseEntry);
				}
				lstOverallResults.append(lstResult);

				if (bVerbose) {
					std::cerr << renderResult(fmtRender, lstResult, nModulus).toUtf8().data();
					std::cerr << "\n";
				}
			}
		}	// while !bSearchConverged

		++nNormalIndex;
	}
	if (bNeedNewline) {
		std::cerr << "\n";
	}

	// Sort results:
	if (bVerbose) std::cerr << "Sorting...\n";
	std::sort(lstOverallResults.begin(), lstOverallResults.end(),
				bOrderByModulus ? ascendingLessThanModulusFirst : ascendingLessThanTextFirst);

	// Remove Duplicates:
	if (bVerbose) std::cerr << "\nRemoving Duplicates...\n";
	for (int ndx = 0; ndx < lstOverallResults.size()-1; ++ndx) {
		// Since we just sorted them, we already know that @ndx < @ndx+1,
		//	OR @ndx == @ndx+1.  Since the sort functions are strict weak
		//	ordering, it will return false for > or >=.  So if we compare
		//	@ndx < @ndx+1 and it returns false, then @ndx must be the
		//	same as @ndx+1, and are thus duplicates
		bool bSame = (bOrderByModulus ?
						!ascendingLessThanModulusFirst(lstOverallResults.at(ndx), lstOverallResults.at(ndx+1)) :
						!ascendingLessThanTextFirst(lstOverallResults.at(ndx), lstOverallResults.at(ndx+1)));
		if (bSame) {
			if (bVerbose) {
				std::cerr << "    " << renderResult(fmtRender, lstOverallResults.at(ndx), nModulus).toUtf8().data() << "    and    "
									<< renderResult(fmtRender, lstOverallResults.at(ndx+1), nModulus).toUtf8().data() << "\n";
			}
			lstOverallResults.removeAt(ndx+1);
			--ndx;			// Decrement the index so we will compare the new one in this slot with the next one
		}
	}

	// Print Final Results:
	std::cerr << QString("\nFound %1 unique solutions having a Modulus of %2:\n").arg(lstOverallResults.size()).arg(nModulus).toUtf8().data();

	for (int ndx = 0; ndx < lstOverallResults.size(); ++ndx) {
		std::cout << renderResult(fmtRender, lstOverallResults.at(ndx), nModulus).toUtf8().data();
		std::cout << "\n";
	}

	// ------------------------------------------------------------------------

//	return a.exec();
	return 0;
}
