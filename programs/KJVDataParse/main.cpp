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

#include "../KJVCanOpener/dbstruct.h"
#include "../KJVCanOpener/dbDescriptors.h"
#include "../KJVCanOpener/ParseSymbols.h"
#include "../KJVCanOpener/Translator.h"
#include "../KJVCanOpener/CSV.h"
#include "../KJVCanOpener/BibleLayout.h"

#include "xc_KJVDataParse.h"

#include <QProcessEnvironment>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QObject>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#if QT_VERSION >= 0x050F00
#include <QRegularExpression>
#else
#include <QRegExp>
#endif
#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QStringList>
#include <QtGlobal>
#include <QSettings>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif
#include <QBuffer>
#include <QByteArray>

#include <iostream>
#include <set>
#include <map>

#define CHECK_INDEXES 0

const unsigned int VERSION = 10000;		// Version 1.0.0

namespace {
	// Env constants:
	// --------------
	const QString constrBuildDBPathEnvKey("KJPBS_BUILDDB_PATH");

}	// namespace

// Note: When writing the book TOC, the abbreviation field will now be a semicolon
//			separated list.  The FIRST entry will always be the untranslated OSIS Abbreviation!
//			The rest will be translated "Common Abbreviation" values.  There may be duplicates,
//			particularly between the OSIS Abbreviation and the Common Abbreviations.  In the
//			Common Abbreviations, the FIRST of that list (i.e. the SECOND entry overall) will
//			be the "preferred" abbreviation that will be rendered when using abbreviated book
//			name mode.

// ============================================================================
// ============================================================================

// Note: Other Parse Symbols are in ParseSymbols.cpp:

const QChar g_chrParseTag = QChar('|');			// Special tag to put into the verse text to mark parse tags -- must NOT exist in the text

// ============================================================================
// ============================================================================

const char *g_constrTranslationsPath = "../../KJVDataParse/translations/";
const char *g_constrTranslationFilenamePrefix = "kjvdataparse";

// ============================================================================
// ============================================================================

static bool isSpecialWord(const CBibleDatabase *pBibleDatabase, const CWordEntry &entryWord)
{
	Q_ASSERT(pBibleDatabase != nullptr);
	if (pBibleDatabase->langID() == LIDE_ENGLISH) {
		for (int ndx = 0; ndx < entryWord.m_lstAltWords.size(); ++ndx) {
			QString strDecomposedWord = StringParse::decompose(entryWord.m_lstAltWords.at(ndx), true);

			if (strDecomposedWord.compare("abominations", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("am", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("amen", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("ancient", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("and", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("angel", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Babylon", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("bishop", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("branch", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("cherub", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("comforter", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("creator", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("day", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("days", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("devil", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("dominion", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("duke", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("earth", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("elect", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("father", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("father's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("fathers", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("ghost", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("God", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("gods", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("great", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("harlots", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("heaven", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("hell", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("highest", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("him", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("himself", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("his", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("holiness", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("holy", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("is", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Jesus", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Jews", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("judge", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("king", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("kings", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("kings'", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lamb", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("legion", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lion", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lord", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lord's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lords", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("lot", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("man", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("man's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("master", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("masters", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("men", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("men's", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("mighty", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("moon", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("mother", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("mystery", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("Nazareth", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("of", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("one", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("our", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("righteousness", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("sanctuary", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("saviour", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("sceptre", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("shepherd", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("son", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("spirit", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("spirits", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("sun", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("tabernacle", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("that", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("the", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("this", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("thy", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("unknown", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("unto", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("word", Qt::CaseInsensitive) == 0) return true;
			if (strDecomposedWord.compare("wormwood", Qt::CaseInsensitive) == 0) return true;
		}
	}

	return false;
}

static bool isProperWord(const CBibleDatabase *pBibleDatabase, const CWordEntry &entryWord)
{
	Q_ASSERT(pBibleDatabase != nullptr);
	bool bIsProperWord = true;

	BIBLE_DESCRIPTOR_ENUM nBDE = bibleDescriptorFromUUID(pBibleDatabase->compatibilityUUID());
	if ((nBDE == BDE_KJV) || (nBDE == BDE_KJV_FULL) || (nBDE == BDE_KJVPCE) || (nBDE == BDE_KJVA)) {
		Q_ASSERT(pBibleDatabase->langID() == LIDE_ENGLISH);

		for (int ndx = 0; ((bIsProperWord) && (ndx < entryWord.m_lstAltWords.size())); ++ndx) {
			QString strDecomposedWord = StringParse::decompose(entryWord.m_lstAltWords.at(ndx), true);
			if (!strDecomposedWord.at(0).isUpper()) {
				bIsProperWord = false;
			} else {
				//	Lists of "Ordinary" words as extracted:
				//
				//	Sword 1769:				Sword PCE:
				//	-----------				----------
				//	Godward					Godward
				//	jointheirs				jointheirs
				//	theeward
				//	usward					usward
				//	youward					youward

				if (strDecomposedWord.compare("Godward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("jointheirs", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("theeward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("usward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				} else if (strDecomposedWord.compare("youward", Qt::CaseInsensitive) == 0) {
					bIsProperWord = false;
				}
			}
		}
	} else if (nBDE == BDE_OSHB) {
		Q_ASSERT(pBibleDatabase->langID() == LIDE_HEBREW);

		// The OSHB uses OSHM code morphology.  Here, we just go through each word in the
		//	given entry and if any of them have a OSHM similar to "HNp" (proper Hebrew noun),
		//	we exit early with 'True'.
		bIsProperWord = false;			// Switch default state to 'false'
		for (auto const & ndxNormal : entryWord.m_ndxNormalizedMapping) {
			CRelIndex ndxRel = pBibleDatabase->DenormalizeIndex(ndxNormal);
			const CLemmaEntry *pLemma = pBibleDatabase->lemmaEntry(ndxRel);
			if (pLemma) {
				QStringList slMorph = pLemma->morph(MSE_OSHM);
				for (auto const & morph : slMorph) {
					// According to the OSHM dictionary, the following is supposed
					//	to be an exhaustive list of proper noun types:
					//	AC/Np		AC/R/Np		AC/To/Np		ANp
					//	ANp/Td		AR/Np		ATo/Np			HC/Np
					//	HC/Np/Sd	HC/Np/Sh	HC/Np/Sp3mp		HC/R/Np
					//	HC/R/Np/Sd	HC/Rd/Np	HNp				HNp/Sd
					//	HNp/Sp1cs	HNp/Sp2ms	HNp/Sp3mp		HNp/Sp3ms
					//	HR/Np		HR/Np/Sd	HR/R/Np			HR/Td/Np
					//	HRd/Np		HTd/Np		HTd/Np/Sd		HTi/Np
					//	HTr/Np		HTr/R/Np

					// The first letter is always the language (Hebrew or Aramaic)
					//	and the rest is a list separated by "/":
					QStringList slEntry = morph.mid(1).split('/');
					for (auto const & pos : slEntry) {
						if (pos.compare("Np", Qt::CaseInsensitive) == 0) {
							bIsProperWord = true;
							break;
						}
					}
					if (bIsProperWord) break;
				}
			}
			if (bIsProperWord) break;
		}
	} else {
		for (int ndx = 0; ((bIsProperWord) && (ndx < entryWord.m_lstAltWords.size())); ++ndx) {
			QString strDecomposedWord = StringParse::decompose(entryWord.m_lstAltWords.at(ndx), true);
			if (!strDecomposedWord.at(0).isUpper()) {
				bIsProperWord = false;
			}
		}
	}

	return bIsProperWord;
}

// ============================================================================
// ============================================================================

// TWordListSet will be used here for a set containing all of the case-forms of a given
//		word.  It's easier to map them here as a set than in the list that the database
//		itself uses.  The TAltWordListMap will be indexed by the Lower-Case word key
//		and will map to the set of word forms for that key:
typedef std::map<QString, TWordListSet, CWordEntry::SortPredicate> TAltWordListMap;

// WordFromWordSet - Drives word toward lower-case and returns the resulting word.  The
//		theory is that proper names will always be capitalized and non-proper names will
//		have mixed case, being capital only when they start a new sentence.  Thus, if we
//		drive toward lower-case, we should have an all-lower-case word for non-proper
//		name words and mixed-case for proper names:
static QString WordFromWordSet(const TWordListSet &setAltWords)
{
	QString strWord;

	for (TWordListSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
		QString strDeCantillatedWord = StringParse::deCantillate(*itrAltWrd);
		if ((strWord.isEmpty()) ||
			(((strDeCantillatedWord).compare(strWord)) > 0)) strWord = strDeCantillatedWord;
	}

	return strWord;
}

// ============================================================================
// ============================================================================

// Like the QXmlAttributes::index() function, but case-insensitive
static int findAttribute(const QXmlAttributes &attr, const QString &strName)
{
	for (int i = 0; i < attr.count(); ++i) {
		if (attr.localName(i).compare(strName, Qt::CaseInsensitive) == 0) return i;
	}
	return -1;
}

static QString stringifyAttributes(const QXmlAttributes &attr)
{
	QString strTemp;
	for (int i=0; i<attr.count(); ++i) {
		if (i) strTemp += ',';
		strTemp += attr.localName(i) + '=' + attr.value(i);
	}
	return strTemp;
}

//static QXmlAttributes attributesFromString(const QString &str)
//{
//	QXmlAttributes attrs;
//	QStringList lstPairs = str.split(',');
//	for (int i=0; i<lstPairs.count(); ++i) {
//		QStringList lstEntry = lstPairs.at(i).split('=');
//		Q_ASSERT(lstEntry.count() == 2);
//		if (lstEntry.count() != 2) {
//			std::cerr << "\n*** Error: Attributes->String failure\n";
//			continue;
//		}
//		attrs.append(lstEntry.at(0), QString(), lstEntry.at(0), lstEntry.at(1));
//	}
//	return attrs;
//}

// ============================================================================
// ============================================================================

class CStrongsImpXmlHandler : public QXmlDefaultHandler
{
	enum STRONGS_IMP_PARSER_STATE {
		SIPSE_ENTRYFREE = 0,
		SIPSE_TITLE = 1,
		SIPSE_ORTH = 2,
		SIPSE_TRANSLITERATION = 3,
		SIPSE_PRONUNCIATION = 4,
		SIPSE_DEFINITION = 5,
		SIPSE_REFERENCE = 6,
		SIPSE_RENDER = 7,
	};

public:
	CStrongsImpXmlHandler(const QString &strExpectedTextIndex)
		:	m_strongsEntry(strExpectedTextIndex),
			m_bRefIsStrongs(false),
			m_strExpectedTextIndex(strExpectedTextIndex)
	{ }

	virtual ~CStrongsImpXmlHandler()
	{ }

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual bool characters(const QString &ch) override;
	virtual bool error(const QXmlParseException &exception) override;
	virtual bool fatalError(const QXmlParseException &exception) override;
	virtual bool warning(const QXmlParseException &exception) override;
	virtual QString errorString() const override
	{
		return (!m_strErrorString.isEmpty() ? m_strErrorString : QXmlDefaultHandler::errorString());
	}
	virtual bool endDocument() override;

	const CStrongsEntry &strongsEntry() const { return m_strongsEntry; }

private:
	void beginRenderElement(const QString &strRendType);
	void endRenderElement();

private:
	CStrongsEntry m_strongsEntry;

	QVector<STRONGS_IMP_PARSER_STATE> m_vctParseState;	// Parse State Stack
	bool m_bRefIsStrongs;						// Set to True in the SIPSE_REFERENCE state if the reference is for Strongs
	QString m_strRefText;						// Text inside the <a> tags for references for Strongs format conversion
	QString m_strExpectedTextIndex;				// Strongs Text Index expected from IMP file during construction of this parser (ex: 'G0001')
	QString m_strErrorString;

	QString m_strEntryTextIndex;				// derived from entryFree 'n' attribute, compared with expected TextIndex and with 'title' element (ex: 'G0001')
	QString m_strCurrentMapIndex;				// derived from text on 'title' element (ex: 'G1'), compared with expected TextIndex

	QString m_strEntryOrthographicIndex;		// derived from entryFree 'n' attribute, compared with 'orth' element, is PlainText whereas the 'orth' element in the StrongsEntry is RichText

	QStringList m_lstRenderElementStack;		// Corresponding Render Element to output when we hit endRenderElement(), pushed in beginRenderElement()
};

void CStrongsImpXmlHandler::beginRenderElement(const QString &strRendType)
{
	if (strRendType.compare("bold", Qt::CaseInsensitive) == 0) {
		characters("<b>");
		m_lstRenderElementStack.push_back("</b>");
	} else if (strRendType.compare("italic", Qt::CaseInsensitive) == 0) {
		characters("<i>");
		m_lstRenderElementStack.push_back("</i>");
	} else if (strRendType.compare("super", Qt::CaseInsensitive) == 0) {
		characters("<sup>");
		m_lstRenderElementStack.push_back("</sup>");
	} else if (strRendType.compare("sub", Qt::CaseInsensitive) == 0) {
		characters("<sub>");
		m_lstRenderElementStack.push_back("</sub>");
	} else {
		// Unknown rendering types placeholder:
		m_lstRenderElementStack.push_back(QString());		// Keep stack balanced
	}
	m_vctParseState.push_back(SIPSE_RENDER);
}

void CStrongsImpXmlHandler::endRenderElement()
{
	Q_ASSERT(!m_vctParseState.isEmpty());
	Q_ASSERT(m_vctParseState.back() == SIPSE_RENDER);
	m_vctParseState.pop_back();
	Q_ASSERT(!m_lstRenderElementStack.isEmpty());
	characters(m_lstRenderElementStack.back());
	m_lstRenderElementStack.pop_back();
}

bool CStrongsImpXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

/*

	Strongs IMP Format:
	-------------------
	$$$G0001
	<entryFree n="G0001|Α"> <title>G1</title> <orth>Α</orth> <orth rend="bold" type="trans">A</orth>
	<pron rend="italic">al'-fah</pron><lb/> <def>Of Hebrew origin; the first letter of the alphabet:
	figuratively only (from its use as a numeral) the <hi rend="italic">first</hi>. Often used (usually
	“an” before a vowel) also in composition (as a contraction from <ref target="Strong:G0427">G427</ref>)
	in the sense of <hi rend="italic">privation</hi>; so in many words beginning with this letter;
	occasionally in the sense of <hi rend="italic">union</hi> (as a contraction of
	<ref target="Strong:G0260">G260</ref>): - Alpha.</def> </entryFree>

*/

	int ndx = -1;

	if (localName.compare("entryFree", Qt::CaseInsensitive) == 0) {
		if (!m_vctParseState.isEmpty()) {
			std::cerr << QString("*** Error: Strongs Imp Parse State not empty at entryFree start for %1\n").arg(m_strExpectedTextIndex).toUtf8().data();
			m_vctParseState.clear();
		}

		m_vctParseState.push_back(SIPSE_ENTRYFREE);

		m_strCurrentMapIndex.clear();
		m_strongsEntry.setOrthography(QString());

		ndx = findAttribute(atts, "n");
		if (ndx != -1) {
			QStringList lstIndex = atts.value(ndx).split('|');
			if (lstIndex.size() < 2) {
				std::cerr << QString("*** Warning: On %1 - Strongs Imp entryFree missing proper 'n' attribute: %2\n").arg(m_strExpectedTextIndex).arg(stringifyAttributes(atts)).toUtf8().data();
			} else {
				m_strEntryTextIndex = lstIndex.at(0);
				m_strEntryOrthographicIndex = lstIndex.at(1);
				if (lstIndex.size() > 2) {
					std::cerr << QString("*** Warning: On %1 - Strongs Imp entryFree with extraneous 'n' attribute: %2\n").arg(m_strExpectedTextIndex).arg(atts.value(ndx)).toUtf8().data();
				}
			}
		} else {
			m_strEntryTextIndex.clear();
			m_strEntryOrthographicIndex.clear();
		}
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		m_vctParseState.push_back(SIPSE_TITLE);

		m_strCurrentMapIndex.clear();		// Redundant, but we'll do it in case an entry has multiple 'title' elements
	} else if (localName.compare("orth", Qt::CaseInsensitive) == 0) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) && (atts.value(ndx).compare("trans", Qt::CaseInsensitive) == 0)) {
			m_vctParseState.push_back(SIPSE_TRANSLITERATION);
			m_strongsEntry.setTransliteration(QString());
		} else {
			m_vctParseState.push_back(SIPSE_ORTH);
			m_strongsEntry.setOrthography(QString());
		}

		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("pron", Qt::CaseInsensitive) == 0) {
		m_vctParseState.push_back(SIPSE_PRONUNCIATION);
		m_strongsEntry.setPronunciation(QString());

		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("def", Qt::CaseInsensitive) == 0) {
		m_vctParseState.push_back(SIPSE_DEFINITION);
		m_strongsEntry.setDefinition(QString());

		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("hi", Qt::CaseInsensitive) == 0) {
		ndx = findAttribute(atts, "rend");
		if (ndx != -1) beginRenderElement(atts.value(ndx));
	} else if (localName.compare("ref", Qt::CaseInsensitive) == 0) {
		ndx = findAttribute(atts, "target");
		if (ndx != -1) {
			// Push the reference attribute into current text element before we push the reference state
			m_bRefIsStrongs = (atts.value(ndx).indexOf("Strong:", 0, Qt::CaseInsensitive) >= 0);
			m_strRefText.clear();
			characters(QString("<a href=\"%1\">").arg(atts.value(ndx).replace("Strong:", "strong://", Qt::CaseInsensitive)));
			m_vctParseState.push_back(SIPSE_REFERENCE);
		}	// Ignore references with no target
	}
	// Ignore other elements, like 'lb' (lb will get converted in the endElement)

	return true;
}

bool CStrongsImpXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

	if (localName.compare("entryFree", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_ENTRYFREE) {
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected entryFree endElement";
			return false;
		}
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_TITLE) {
			// Make sure all of our indexes match:
			StrongsIndexSortPredicate isp;
			if ((isp(m_strCurrentMapIndex, m_strEntryTextIndex) != false) ||
				(isp(m_strEntryTextIndex, m_strCurrentMapIndex) != false) ||
				(isp(m_strCurrentMapIndex, m_strExpectedTextIndex) != false) ||
				(isp(m_strExpectedTextIndex, m_strCurrentMapIndex) != false)) {
				std::cerr << QString("\n*** Mismatched Current, Title, and Expected Text Indexes : "
										"Current=%1, Title=%2, Expected=%3\n")
										.arg(m_strCurrentMapIndex)
										.arg(m_strEntryTextIndex)
										.arg(m_strExpectedTextIndex).toUtf8().data();
			}
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected title endElement";
			return false;
		}

	} else if (localName.compare("orth", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
			Q_ASSERT(!m_vctParseState.isEmpty());
		}
		if ((m_vctParseState.back() == SIPSE_ORTH) ||
			(m_vctParseState.back() == SIPSE_TRANSLITERATION)) {
			// Make sure orthography indexes match:
			if (m_vctParseState.back() == SIPSE_ORTH) {
				if (m_strongsEntry.orthographyPlainText() != m_strEntryOrthographicIndex) {
					std::cerr << QString("\n*** Mismatched Orthography on %1 : n=\"%2\", orth=\"%3\", orthPlainText=\"%4\"\n")
											.arg(m_strExpectedTextIndex)
											.arg(m_strEntryOrthographicIndex)
											.arg(m_strongsEntry.orthography())
											.arg(m_strongsEntry.orthographyPlainText()).toUtf8().data();
				}
			}
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected orth endElement";
			return false;
		}
	} else if (localName.compare("pron", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
			Q_ASSERT(!m_vctParseState.isEmpty());
		}
		if (m_vctParseState.back() == SIPSE_PRONUNCIATION) {
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected pron endElement";
			return false;
		}
	} else if (localName.compare("def", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
			Q_ASSERT(!m_vctParseState.isEmpty());
		}
		if (m_vctParseState.back() == SIPSE_DEFINITION) {
			m_vctParseState.pop_back();
		} else {
			m_strErrorString = "Expected def endElement";
			return false;
		}
	} else if (localName.compare("hi", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_RENDER) {
			endRenderElement();
		}
	} else if (localName.compare("ref", Qt::CaseInsensitive) == 0) {
		Q_ASSERT(!m_vctParseState.isEmpty());
		if (m_vctParseState.back() == SIPSE_REFERENCE) {
			if (m_bRefIsStrongs) {
				m_bRefIsStrongs = false;		// Clear flag so character push goes to parent:
				CStrongsEntry tmpEntry(m_strRefText);
				characters(tmpEntry.strongsTextIndex());
			}
			m_vctParseState.pop_back();
			characters("</a>");
		}
	} else if (localName.compare("lb", Qt::CaseInsensitive) == 0) {
		characters("<br/>");
	}
	// Ignore other elements

	return true;
}

bool CStrongsImpXmlHandler::characters(const QString &ch)
{
	Q_ASSERT(!m_vctParseState.isEmpty());
	STRONGS_IMP_PARSER_STATE parseState = m_vctParseState.back();

	if ((parseState == SIPSE_REFERENCE) && m_bRefIsStrongs) {
		m_strRefText += ch;
		return true;
	}

	// Push data inside references and render elements up to
	//	the parent element:
	int ndxParent = 2;
	while ((parseState == SIPSE_REFERENCE) ||
			(parseState == SIPSE_RENDER)) {
		Q_ASSERT(m_vctParseState.size() >= ndxParent);
		parseState = m_vctParseState.at(m_vctParseState.size()-ndxParent);
		++ndxParent;
	}

	switch (parseState) {
		case SIPSE_ENTRYFREE:
			// Ignore characters (like spaces) in our entry not assigned to a specific function
			break;
		case SIPSE_TITLE:
			m_strCurrentMapIndex.append(ch);
			break;
		case SIPSE_ORTH:
			m_strongsEntry.setOrthography(m_strongsEntry.orthography() + ch);
			break;
		case SIPSE_TRANSLITERATION:
			m_strongsEntry.setTransliteration(m_strongsEntry.transliteration() + ch);
			break;
		case SIPSE_PRONUNCIATION:
			m_strongsEntry.setPronunciation(m_strongsEntry.pronunciation() + ch);
			break;
		case SIPSE_DEFINITION:
			m_strongsEntry.setDefinition(m_strongsEntry.definition() + ch);
			break;
		case SIPSE_REFERENCE:
		case SIPSE_RENDER:
			// This state can't happen due to parent seeking above
			Q_ASSERT(false);
			return false;
	}

	return true;
}

bool CStrongsImpXmlHandler::error(const QXmlParseException &exception)
{
	std::cerr << QString("\n\n*** %1\n").arg(exception.message()).toUtf8().data();
	std::cerr << QString("Line: %1  Column: %2\nPublicID: \"%3\"\nSystemID: \"%4\"\n")
					.arg(exception.lineNumber()).arg(exception.columnNumber())
					.arg(exception.publicId()).arg(exception.systemId()).toUtf8().data();
	return true;
}

bool CStrongsImpXmlHandler::fatalError(const QXmlParseException &exception)
{
	return error(exception);
}

bool CStrongsImpXmlHandler::warning(const QXmlParseException &exception)
{
	return error(exception);
}

bool CStrongsImpXmlHandler::endDocument()
{
	return true;
}

// ============================================================================
// ============================================================================

class COSISXmlHandler : public QXmlDefaultHandler
{
public:
	enum XML_FORMAT_TYPE_ENUM {
		XFTE_UNKNOWN = -1,
		XFTE_OSIS = 0,
		XFTE_ZEFANIA = 1
	};

	COSISXmlHandler(const TBibleDescriptor &bblDesc)
		:	m_xfteFormatType(XFTE_UNKNOWN),
			m_nCurrentBookIndex(-1),
			m_bInHeader(false),
			m_bCaptureTitle(false),
			m_bCaptureLang(false),
			m_bOpenEndedChapter(false),
			m_bInVerse(false),
			m_bOpenEndedVerse(false),
			m_bInLemma(false),
			m_bInTransChangeAdded(false),
			m_bInNotes(false),
			m_bInBracketNotes(false),
			m_bInColophon(false),
			m_bOpenEndedColophon(false),
			m_bInSuperscription(false),
			m_bOpenEndedSuperscription(false),
			m_bInForeignText(false),
			m_bInWordsOfJesus(false),
			m_bInDivineName(false),
			m_nDelayedPilcrow(CVerseEntry::PTE_NONE),
			m_strLanguage("en"),
			m_bNoColophonVerses(false),
			m_bUseBracketColophons(false),
			m_bDisableColophons(false),
			m_bNoSuperscriptionVerses(false),
			m_bDisableSuperscriptions(false),
			m_bBracketItalics(false),
			m_bNoArabicNumeralWords(false),
			m_bInlineFootnotes(false),
			m_bUseBracketFootnotes(false),
			m_bUseBracketFootnotesExcluded(false),
			m_bExcludeDeuterocanonical(false),
			m_bFoundSegVariant(false)
	{
		m_pBibleDatabase = QSharedPointer<CBibleDatabase>(new CBibleDatabase(bblDesc));		// Note: We'll set the name and description later in the reading of the data
	}

	virtual ~COSISXmlHandler()
	{

	}

	// Properties:
	void setNoColophonVerses(bool bNoColophonVerses) { m_bNoColophonVerses = bNoColophonVerses; }
	bool noColophonVerses() const { return m_bNoColophonVerses; }
	void setUseBracketColophons(bool bUseBracketColophons) { m_bUseBracketColophons = bUseBracketColophons; }
	bool useBracketColophons() const { return m_bUseBracketColophons; }
	void setDisableColophons(bool bDisableColophons) { m_bDisableColophons = bDisableColophons; }
	bool disableColophons() const { return m_bDisableColophons; }
	void setNoSuperscriptionVerses(bool bNoSuperscriptionVerses) { m_bNoSuperscriptionVerses = bNoSuperscriptionVerses; }
	bool noSuperscriptionVerses() const { return m_bNoSuperscriptionVerses; }
	void setDisableSuperscriptions(bool bDisableSuperscriptions) { m_bDisableSuperscriptions = bDisableSuperscriptions; }
	bool disableSuperscriptions() const { return m_bDisableSuperscriptions; }
	void setBracketItalics(bool bBracketItalics) { m_bBracketItalics = bBracketItalics; }
	bool bracketItalics() const { return m_bBracketItalics; }
	void setNoArabicNumeralWords(bool bNoArabicNumeralWords) { m_bNoArabicNumeralWords = bNoArabicNumeralWords; }
	bool noArabicNumeralWords() const { return m_bNoArabicNumeralWords; }
	void setInlineFootnotes(bool bInlineFootnotes) { m_bInlineFootnotes = bInlineFootnotes; }
	bool inlineFootnotes() const { return m_bInlineFootnotes; }
	void setUseBracketFootnotes(bool bUseBracketFootnotes) { m_bUseBracketFootnotes = bUseBracketFootnotes; }
	bool useBracketFootnotes() const { return m_bUseBracketFootnotes; }
	void setUseBracketFootnotesExcluded(bool bUseBracketFootnotesExcluded) { m_bUseBracketFootnotesExcluded = bUseBracketFootnotesExcluded; }
	bool useBracketFootnotesExcluded() const { return m_bUseBracketFootnotesExcluded; }
	void setExcludeDeuterocanonical(bool bExcludeDeuterocanonical) { m_bExcludeDeuterocanonical = bExcludeDeuterocanonical; }
	bool excludeDeuterocanonical() const { return m_bExcludeDeuterocanonical; }
	void setSegVariant(const QString &strSegVariant) { m_strSegVariant = strSegVariant; }
	QString segVariant() const { return m_strSegVariant; }
	bool foundSegVariant() const { return m_bFoundSegVariant; }
	void setStrongsImpFilepath(const QString &strFilepath) { m_strStrongsImpFilepath = strFilepath; }
	QString strongsImpFilepath() const { return m_strStrongsImpFilepath; }

	// Parsing:
	QStringList elementNames() const { return m_lstElementNames; }
	QStringList attrNames() const { return m_lstAttrNames; }

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual bool characters(const QString &ch) override;
	virtual bool error(const QXmlParseException &exception) override;
	virtual bool fatalError(const QXmlParseException &exception) override;
	virtual bool warning(const QXmlParseException &exception) override;
	virtual QString errorString() const override
	{
		return (!m_strErrorString.isEmpty() ? m_strErrorString : QXmlDefaultHandler::errorString());
	}
	virtual bool endDocument() override;

	const CBibleDatabase *bibleDatabase() const { return m_pBibleDatabase.data(); }

	QString parsedUTF8Chars() const { return m_strParsedUTF8Chars; }

	const CBookEntry *addBookToBibleDatabase(int nBk);

protected:
	void startVerseEntry(const CRelIndex &relIndex, bool bOpenEnded);
	void charactersVerseEntry(const CRelIndex &relIndex, const QString &strText);
	void endVerseEntry(CRelIndex &relIndex);
	CRelIndex &activeVerseIndex()
	{
		// Note: Can have nested superscriptions/colophon in verse, but not
		//			vice versa.  So, do these in order of precedence:
		if (m_bInColophon) return m_ndxColophon;
		if (m_bInSuperscription) return m_ndxSuperscription;
		Q_ASSERT(m_bInVerse);
		return m_ndxCurrent;
	}
	CVerseEntry &activeVerseEntry()
	{
		CRelIndex &ndxActive = activeVerseIndex();
		return (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[ndxActive.book()-1])[CRelIndex(ndxActive.book(), ndxActive.chapter(), ndxActive.verse(), 0)];
	}

private:
	XML_FORMAT_TYPE_ENUM m_xfteFormatType;
	QString m_strErrorString;

	QString m_strNamespace;
	QStringList m_lstElementNames;
	QStringList m_lstAttrNames;

	int m_nCurrentBookIndex;				// 0-based Index into g_arrBibleBooks for the current book being processed (-1 = not defined or no current book)
	CRelIndex m_ndxCurrent;
	CRelIndex m_ndxColophon;
	CRelIndex m_ndxSuperscription;
	bool m_bInHeader;
	bool m_bCaptureTitle;
	bool m_bCaptureLang;
	bool m_bOpenEndedChapter;
	bool m_bInVerse;
	bool m_bOpenEndedVerse;
	bool m_bInLemma;
	bool m_bInTransChangeAdded;
	bool m_bInNotes;
	bool m_bInBracketNotes;
	bool m_bInColophon;
	bool m_bOpenEndedColophon;				// Open-ended colophons use sID/eID attributes in <div /> tags to start stop them rather than enclosing the whole colophon in a single specific <div></div> section
	bool m_bInSuperscription;
	bool m_bOpenEndedSuperscription;
	bool m_bInForeignText;
	bool m_bInWordsOfJesus;
	bool m_bInDivineName;
	CVerseEntry::PILCROW_TYPE_ENUM m_nDelayedPilcrow;		// Used to flag a pilcrow to appear in the next verse -- used for <CM> tag from German Schlachter text
	QString m_strParsedUTF8Chars;		// UTF-8 (non-Ascii) characters encountered -- used for report

	CBibleDatabasePtr m_pBibleDatabase;
	QString m_strTitle;					// Used only for capture of title from XML -- after that it will be stored in the Bible Database Descriptor
	QString m_strLanguage;				// Used only for capture of language from XML -- after that it will be stored in the Bible Database Descriptor
	bool m_bNoColophonVerses;			// Note: This is colophons as "pseudo-verses" only not colophons in general, which are also written as footnotes
	bool m_bUseBracketColophons;		// Treat "[" and "]" as a colophon marker
	bool m_bDisableColophons;			// Disable all colophon output (both pseudo-verses and footnote version)
	bool m_bNoSuperscriptionVerses;		// Note: This is superscriptions as "pseudo-verses" only not superscriptions in general, which are also written as footnotes
	bool m_bDisableSuperscriptions;		// Disable all superscription output (both pseudo-verses and footnote version)
	bool m_bBracketItalics;
	bool m_bNoArabicNumeralWords;		// Skip "words" made entirely of Arabic numerals and don't count them as words
	bool m_bInlineFootnotes;			// True if inlining footnotes as uncounted parentheticals
	bool m_bUseBracketFootnotes;		// True if treating "[" and "]" as inline footnote markers
	bool m_bUseBracketFootnotesExcluded;// True if treating "[" and "]" as inline footnote markers and excluding them (m_bUseBracketFootnotes will also be true)
	bool m_bExcludeDeuterocanonical;	// Exclude Apocrypha/Deuterocanonical Text
	QString m_strSegVariant;			// OSIS <seg> tag variant to export (or empty to export all)
	QString m_strCurrentSegVariant;		// Current OSIS <seg> tag variant we are in (or empty if not in a seg)
	bool m_bFoundSegVariant;			// Set to true if any <seg> tag variant found when no SegVariant was specifed.  Otherwise, set to true when the specified Seg Variant was found.
	QString m_strStrongsImpFilepath;	// Strongs Imp Database to parse (if empty, no Strongs Database will be used)
};

// osisAbbrFromBookIndex -- returns the OSIS abbreviation
//	for the 0-based book number from the g_arrBibleBooks array.
static QString osisAbbrFromBookIndex(int nBk)			// 0-based book array index number
{
	Q_ASSERT((nBk >= 0) && (nBk < g_arrBibleBooks.size()));
	if ((nBk < 0) || (nBk >= g_arrBibleBooks.size())) return QString();
	return g_arrBibleBooks.at(nBk).m_lstOsisAbbr.at(0);
}

// bookIndexFromOSISAbbr -- returns the 0-based index into the
//	g_arrBibleBooks array for the specified OSIS Abbreviation:
static int bookIndexFromOSISAbbr(const QString &strOSISAbbr)
{
	for (int i=0; i<g_arrBibleBooks.size(); ++i) {
		if (g_arrBibleBooks.at(i).m_lstOsisAbbr.contains(strOSISAbbr)) return i;
	}
	return -1;
}

// bookStartingIndex -- returns the CRelIndex for the 0-based
//	book number from the g_arrBibleBooks array.  If the entry
//	belongs to another book, the book in the CRelIndex will
//	be non-zero:
static CRelIndex bookStartingIndex(int nBk)				// 0-based book array index number
{
	Q_ASSERT((nBk >= 0) && (nBk < g_arrBibleBooks.size()));
	if ((nBk < 0) || (nBk >= g_arrBibleBooks.size())) return CRelIndex();
	CRelIndex ndxStartingIndex = CRelIndex(g_arrBibleBooks.at(nBk).m_ndxStartingIndex);
	if (ndxStartingIndex.isSet()) {
		// Word must be zero for the KJVDataParse logic
		//	to work.  Check BibleLayout if this asserts:
		Q_ASSERT(ndxStartingIndex.word() == 0);
		ndxStartingIndex.setWord(0);
		if (ndxStartingIndex.book() == 0) ndxStartingIndex.setBook(nBk+1);
	}
	return ndxStartingIndex.isSet() ? ndxStartingIndex : CRelIndex(nBk+1, 1, 1, 0);	// Default to first Chapter, first Verse of this book
}

// resolveChapter -- returns the chapter CRelIndex for the
//	specified 0-based g_arrBibleBooks array book and OSIS
//	chapter spec.  This is to handle the duality entries
//	in g_arrBibleBooks where chapter and verse are offsets:
static unsigned int resolveChapter(int nBk, unsigned int nChp)
{
	if (static_cast<uint32_t>(nBk) < NUM_BK) {
		return nChp;
	}
	return nChp + bookStartingIndex(nBk).chapter() - 1;
}

// resolveVerse -- returns the verse CRelIndex for the
//	specified 0-based g_arrBibleBooks array book and OSIS
//	verse spec.  This is to handle the duality entries
//	in g_arrBibleBooks where chapter and verse are offsets:
static unsigned int resolveVerse(int nBk, unsigned int nChp, unsigned int nVrs)
{
	if ((static_cast<uint32_t>(nBk) < NUM_BK) ||
		(nChp != bookStartingIndex(nBk).chapter())) {
		return nVrs;
	}
	return nVrs + bookStartingIndex(nBk).verse() - 1;
}

static unsigned int bookIndexToTestamentIndex(unsigned int nBk)		// 1-based book number
{
	unsigned int nTst = 0;
	if (nBk == 0) return 0;			// Special-case for "no book"

	if (nBk <= NUM_BK_OT) {
		nTst = 1;
	} else if (nBk <= (NUM_BK_OT + NUM_BK_NT)) {
		nTst = 2;
	} else if (nBk <= (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
		nTst = 3;
	} else {
		nTst = 0;
		Q_ASSERT(false);			// Can't happen if our NUM_BK_xx values are correct!
	}

	return nTst;
}

static unsigned int bookIndexToTestamentBookIndex(unsigned int nBk)		// 1-based book number
{
	// Note: nBk is one-based
	if (nBk <= NUM_BK_OT) {
		return nBk;
	} else if (nBk <= (NUM_BK_OT + NUM_BK_NT)) {
		return nBk-NUM_BK_OT;
	} else if (nBk <= (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
		return nBk-NUM_BK_OT-NUM_BK_NT;
	} else {
		return nBk-NUM_BK_OT-NUM_BK_NT-NUM_BK_APOC;
	}
}

static bool bookIsDeuterocanonical(unsigned int nBk)		// 1-based book number
{
	// Note: nBk is one-based
	if (nBk > (NUM_BK_OT + NUM_BK_NT)) return true;
	return false;
}

const CBookEntry *COSISXmlHandler::addBookToBibleDatabase(int nBk)				// 0-based book array index number
{
	unsigned int nBkText = bookStartingIndex(nBk).book() - 1;		// Make nBkText 0-based to simplify
	unsigned int nTst = bookIndexToTestamentIndex(nBkText+1);
	unsigned int nBkPri = (static_cast<unsigned int>(nBk) == nBkText) ? nBk : nBkText;			// Primary book index for dualities

	if ((nBkText >= m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size()) ||
		(!m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_bCreated)) {
		// Only create the book if we haven't done so already:
		m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumBk++;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[nTst-1].m_nNumBk++;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.resize(qMax(nBkText+1, m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size()));
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_bCreated = true;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_nTstBkNdx = bookIndexToTestamentBookIndex(nBkText+1);
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_nTstNdx = nTst;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_strBkName = g_arrBibleBooks.at(nBkPri).m_strName;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_lstBkAbbr.append(osisAbbrFromBookIndex(nBkPri));	// Our main OSIS abbreviation must always be the first entry!
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_lstBkAbbr.append(g_arrBibleBooks.at(nBkPri).m_strCommonAbbr.split(QChar(';'), My_QString_SkipEmptyParts));
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_strTblName = g_arrBibleBooks.at(nBkPri).m_strTableName;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_strDesc = g_arrBibleBooks.at(nBkPri).m_strDescription;
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses.resize(qMax(nBkText+1, m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses.size()));
	}
	if (nBkPri != static_cast<unsigned int>(nBk)) {
		// If this is a duality entry, add its abbreviates to its primary:
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText].m_lstBkAbbr.append(g_arrBibleBooks.at(nBk).m_strCommonAbbr.split(QChar(';'), My_QString_SkipEmptyParts));
	}

	return &m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[nBkText];
}

bool COSISXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);

/*

	OSIS File Format Header:
	------------------------

	{osis}[schemaLocation=http://www.bibletechnologies.net/2003/OSIS/namespace http://www.bibletechnologies.net/osisCore.2.1.1.xsd]

	{osisText}[osisIDWork=KJV,osisRefWork=defaultReferenceScheme,lang=en]

		{header}[]
			{work}[osisWork=KJV]
				{title}[]King James Version (1769) with Strongs Numbers and Morphology{/title}

				{identifier}[type=OSIS]Bible.KJV{/identifier}

				{refSystem}[]Bible.KJV{/refSystem}

			{/work}

			{work}[osisWork=defaultReferenceScheme]
				{refSystem}[]Bible.KJV{/refSystem}

			{/work}

		{/header}



	Zefania XML Format Header:
	--------------------------

	<XMLBIBLE xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="zef2005.xsd" version="2.0.1.18" revision="2" status="v" biblename="Luther 1545 mit Strongs" type="x-bible">
	  <INFORMATION>
		<title>Luther 1545 mit Strongs</title>
		<creator></creator>
		<subject>Heilige Schrift</subject>
		<description>Dieser Bibeltext ist eine eingedeutschte Version der letzten von Luther
			 1545 in Druck gegebenen Übersetzung der Bibel. Ursprünglich erschien sie als:
			"Martin Luther: Biblia: das ist: Die gantze Heilige Schrifft. Deudsch Auffs new zugericht.
			Wittenberg: Hans Lufft 1545 (Ausgabe letzter Hand)".
		</description>
		<publisher>FREE BIBLE SOFTWARE GROUP</publisher>
		<contributors>Michael Bolsinger (Michael.Bolsinger@t-online.de)</contributors>
		<date>2009-01-20</date>
		<type>Bible</type>
		<format>Zefania XML Bible Markup Language</format>
		<identifier>luth1545str</identifier>
		<source>Text: http://www.luther-bibel-1545.de/
					 Strongs: http://www.winbibel.de/de/lutherstrong/lu1545adds.htm</source>
		<language>GER</language>
		<coverage>provide the Bible to the nations of the world</coverage>
		<rights>We believe that this Bible is found in the Public Domain.</rights>
	  </INFORMATION>

*/

	int ndx = -1;
	unsigned int nTst = bookIndexToTestamentIndex(m_ndxCurrent.book());

	if (localName.compare("osis", Qt::CaseInsensitive) == 0) {
		if (m_xfteFormatType != XFTE_UNKNOWN) {
			m_strErrorString = "*** Error: Ambiguous XML File Type!  Check Source!\n\n";
			return false;
		}
		m_strNamespace = "http://www.bibletechnologies.net/2003/OSIS/namespace";		// TODO : Verify Namespace?
		m_xfteFormatType = XFTE_OSIS;
		std::cerr << "XMLType: OSIS\n";
	} else if (localName.compare("XMLBIBLE", Qt::CaseInsensitive) == 0) {
		if (m_xfteFormatType != XFTE_UNKNOWN) {
			m_strErrorString = "*** Error: Ambiguous XML File Type!  Check Source!\n\n";
			return false;
		}
		m_strNamespace = "zef2005.xsd";		// TODO : Verify Namespace?
		m_xfteFormatType = XFTE_ZEFANIA;
		std::cerr << "XMLType: ZEFANIA\n";
	} else if ((m_xfteFormatType == XFTE_OSIS) && (localName.compare("osisText", Qt::CaseInsensitive) == 0))  {
		ndx = findAttribute(atts, "osisIDWork");
		if (ndx != -1) m_pBibleDatabase->m_descriptor.m_strWorkID = atts.value(ndx);
		std::cerr << "Work: " << atts.value(ndx).toUtf8().data() << "\n";
		ndx = findAttribute(atts, "lang");
		if  (ndx != -1) {
			LANGUAGE_ID_ENUM nOrigLangID = m_pBibleDatabase->langID();
			m_pBibleDatabase->m_descriptor.m_strLanguage = atts.value(ndx);
			std::cerr << "Language: " << m_pBibleDatabase->language().toUtf8().data();
			if (nOrigLangID != m_pBibleDatabase->langID()) {
				std::cerr << "    *** Warning: Original Bible Descriptor Language doesn't match database language\n";
			}
			if (CTranslatorList::instance()->setApplicationLanguage(toQtLanguageName(m_pBibleDatabase->langID()))) {
				std::cerr << " (Loaded Translations)\n";
			} else {
				std::cerr << " (NO Translations Found!)\n";
			}
		}
	} else if (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("header", Qt::CaseInsensitive) == 0)) ||
			   ((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("INFORMATION", Qt::CaseInsensitive) == 0))) {
		m_bInHeader = true;
	} else if ((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("language", Qt::CaseInsensitive) == 0) && (m_bInHeader)) {
		m_bCaptureLang = true;
		m_strLanguage.clear();
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		if (!m_ndxCurrent.isSet()) {
			if (m_bInHeader) {
				m_bCaptureTitle = true;
				m_strTitle.clear();
			}
		} else if (m_xfteFormatType == XFTE_OSIS) {
			// Older format (embedded in closed-form verse tag): canonical="true" subType="x-preverse" type="section":
			//		<chapter osisID="Ps.3">
			//		<verse osisID="Ps.3.1"><title canonical="true" subType="x-preverse" type="section">A Psalm of David, when he fled from Absalom his son.</title>
			//
			// Newer format (using OpenEnded markers, but preceding the verse-tags):
			//		<chapter osisID="Job.42" chapterTitle="CHAPTER 42.">
			//		<title type="chapter">CHAPTER 42.</title>
			//
			// Newer format (using OpenEnded markers, but inside the verse-tags):
			//		<chapter osisID="Ps.7">
			//		<verse osisID="Ps.7.1">
			//		<div type="x-milestone" subType="x-preverse" sID="pv5"/>
			//		<title canonical="true" type="psalm">Shiggaion of David, which he sang unto the <divineName>Lord</divineName>, concerning the words of Cush the Benjamite.</title>
			//		<div type="x-milestone" subType="x-preverse" eID="pv5"/>
			//		O <divineName>Lord</divineName> my God, in thee do I put my trust: save me from all them that persecute me, and deliver me:
			//		<note type="study">words: or, business</note>
			//		</verse>
			//
			//		<chapter osisID="Ps.3" chapterTitle="PSALM 3.">
			//		<title type="chapter">PSALM 3.</title>
			//		<title type="psalm" canonical="true">A Psalm of David, when he fled from Absalom his son.</title>
			//
			//		<verse osisID="Ps.119.1" sID="Ps.119.1"/><title type="acrostic" canonical="true"><foreign n="א">ALEPH.</foreign></title>
			ndx = findAttribute(atts, "type");
			if ((ndx != -1) &&
				((atts.value(ndx).compare("section", Qt::CaseInsensitive) == 0) ||
				 (atts.value(ndx).compare("psalm", Qt::CaseInsensitive) == 0))) {
				m_ndxSuperscription = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0);		// Superscriptions are for the chapter, not the first verse in it, even though that's were this tag exists (in the old closed-form format)
				startVerseEntry(m_ndxSuperscription, false);
			} else if ((ndx != -1) &&
					   ((atts.value(ndx).compare("chapter", Qt::CaseInsensitive) == 0) ||
						(atts.value(ndx).compare("acrostic", Qt::CaseInsensitive) == 0))) {
				// Ignore Chapter titles (as it just has things like "Chapter 1", etc), and is somewhat useless...
				// Ignore verse acrostics on new-format OSIS files (old formats get ignored via foreign language tags below)
			} else {
				std::cerr << QString("\n*** Encountered unknown Title tag inside chapter and/or verse body : %1\n").arg(m_ndxCurrent.index()).toUtf8().data();
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && (localName.compare("foreign", Qt::CaseInsensitive) == 0)) {
		m_bInForeignText = true;				// Old format way of handling acrostics
	} else if ((m_xfteFormatType == XFTE_OSIS) &&
			   (((localName.compare("div", Qt::CaseInsensitive) == 0) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("colophon", Qt::CaseInsensitive) == 0)) ||
				(localName.compare("closer", Qt::CaseInsensitive) == 0))) {
		// Note: This must come here as colophon's may (old form) or may not (new form) have m_ndxCurrent set depending on placement relative to books, chapters, and verses:
		ndx = findAttribute(atts, "osisID");
		if (ndx != -1) {
			QStringList lstOsisID = atts.value(ndx).split('.');
			if ((lstOsisID.size() < 1) || ((m_nCurrentBookIndex = bookIndexFromOSISAbbr(lstOsisID.at(0))) == -1)) {
				std::cerr << "\n*** Unknown Colophon osisID : " << atts.value(ndx).toUtf8().data() << "\n";
				m_ndxColophon = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);
			} else {
				if (!m_bExcludeDeuterocanonical || !bookIsDeuterocanonical(bookStartingIndex(m_nCurrentBookIndex).book())) {
					bool bOK = true;
					unsigned int nChp = 0;
					unsigned int nVrs = 0;
					m_ndxColophon = CRelIndex(bookStartingIndex(m_nCurrentBookIndex).book(), 0, 0, 0);
					if ((lstOsisID.size() >= 2) && ((nChp = lstOsisID.at(1).toUInt(&bOK)) != 0) && (bOK)) {
						m_ndxColophon.setChapter(resolveChapter(m_nCurrentBookIndex, nChp));
						if ((lstOsisID.size() >= 3) && ((nVrs = lstOsisID.at(2).toUInt(&bOK)) != 0) && (bOK)) {
							m_ndxColophon.setVerse(resolveVerse(m_nCurrentBookIndex, m_ndxColophon.chapter(), nVrs));
						}
					}
				} else {
					m_nCurrentBookIndex = -1;
				}
			}
		} else {
			m_ndxColophon = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);
		}
		if (findAttribute(atts, "sID") != -1) {
			// Start of open-ended colophon:
			if (m_bInColophon && !m_bUseBracketColophons) {
				std::cerr << "\n*** Start of open-ended colophon before end of colophon : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			startVerseEntry(m_ndxColophon, true);
		} else if (findAttribute(atts, "eID") != -1) {
			// End of open-ended colophon:
			if (!m_bInColophon && !m_bUseBracketColophons) {
				std::cerr << "\n*** End of open-ended colophon before start of colophon : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			} else {
				// We can have nested Words of Jesus with open form:
				if (m_bInWordsOfJesus) {
					CVerseEntry &verse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[m_ndxColophon.book()-1])[CRelIndex(m_ndxColophon.book(), m_ndxColophon.chapter(), m_ndxColophon.verse(), 0)];
					verse.m_strText += g_chrParseTag;
					verse.m_lstParseStack.push_back("j:");
				}
				endVerseEntry(m_ndxColophon);
			}
		} else {
			// Standard Closed-Form Colophon:
			if (m_bOpenEndedColophon) {
				std::cerr << "\n*** Mixing open-ended and closed form colophons : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			startVerseEntry(m_ndxColophon, false);
		}
	} else if ((!m_ndxCurrent.isSet()) &&
			   (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("div", Qt::CaseInsensitive) == 0)) ||
				((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("BIBLEBOOK", Qt::CaseInsensitive) == 0)))) {
		m_nCurrentBookIndex = -1;
		if (m_xfteFormatType == XFTE_OSIS) {
			ndx = findAttribute(atts, "type");
			if ((ndx != -1) && (atts.value(ndx).compare("x-testament", Qt::CaseInsensitive) == 0)) {
//				std::cerr << "Testament Tag\n";
			} else if ((ndx != -1) && (atts.value(ndx).compare("book", Qt::CaseInsensitive) == 0)) {
				// Some OSIS files just have book tags and no x-testament tags, so we'll try to infer
				//		testament here:
				ndx = findAttribute(atts, "osisID");
				if (ndx != -1) {
					QStringList lstOsisID = atts.value(ndx).split('.');
					if ((lstOsisID.size() != 1) || ((m_nCurrentBookIndex = bookIndexFromOSISAbbr(lstOsisID.at(0))) == -1)) {
						std::cerr << "\n*** Invalid Book osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} // else fall-through and create book with nBk != -1 ...
				}
			}
		} else if (m_xfteFormatType == XFTE_ZEFANIA) {
			ndx = findAttribute(atts, "bnumber");
			if (ndx != -1) {
				m_nCurrentBookIndex = atts.value(ndx).toInt() - 1;			// Note: m_nCurrentBookIndex is index into array, not book number
				if ((m_nCurrentBookIndex < 0) || (m_nCurrentBookIndex >= g_arrBibleBooks.size())) {
					std::cerr << "\n**** Invalid Book Index: " << atts.value(ndx).toUtf8().data() << "\n";
					m_nCurrentBookIndex = -1;
				} // else fall-through and create book with nBk != -1 ...
			} else {
				std::cerr << "\n*** Warning: Found BIBLEBOOK tag without a bnumber\n";
			}
		}
		if (m_nCurrentBookIndex != -1) {
			if (bookIsDeuterocanonical(bookStartingIndex(m_nCurrentBookIndex).book()) && m_bExcludeDeuterocanonical) {
				if ((localName.compare("div", Qt::CaseInsensitive) != 0) ||
					((localName.compare("div", Qt::CaseInsensitive) == 0) &&
					 (findAttribute(atts, "eID") == -1))) {
					// Don't log this if this is a <div> tag with "eID" set, as
					//	we will have already written it for "sID":
					std::cerr << "Book: " << osisAbbrFromBookIndex(m_nCurrentBookIndex).toUtf8().data();
					std::cerr << "  >>> Skipping Deuterocanonical Book\n";
				}
				m_nCurrentBookIndex = -1;
			} else {
				std::cerr << "Book: " << osisAbbrFromBookIndex(m_nCurrentBookIndex).toUtf8().data() << "\n";
				// note: nBk is index into array, not book number:
				nTst = bookIndexToTestamentIndex(bookStartingIndex(m_nCurrentBookIndex).book());
				while (m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.size() < nTst) {
					CTestamentEntry aTestament(CBibleTestaments::name(m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.size()+1));
					m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumTst++;
					m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.push_back(aTestament);
					std::cerr << "Adding Testament: " << aTestament.m_strTstName.toUtf8().data() << "\n";
				}
				if (m_xfteFormatType == XFTE_ZEFANIA) m_ndxCurrent.setBook(bookStartingIndex(m_nCurrentBookIndex).book());
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && (m_ndxCurrent.isSet()) && (localName.compare("div", Qt::CaseInsensitive) == 0) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("paragraph", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "sID");			// Paragraph Starts are tagged with sID, Paragraph Ends are tagged with eID -- we only care about the starts for our Pilcrows -- example text: Reina-Valera 1909
		if (ndx != -1) {
			if (m_ndxCurrent.verse() == 0) {
				std::cerr << "\n*** Pilcrow marker outside of verse at: " << m_pBibleDatabase->PassageReferenceText(m_ndxCurrent).toUtf8().data() << "\n";
			} else {
				CVerseEntry &verse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
				verse.m_nPilcrow = CVerseEntry::PTE_MARKER;
			}
		}
	} else if ((localName.compare("chapter", Qt::CaseInsensitive) == 0) &&
			   ((m_xfteFormatType == XFTE_OSIS) ||
				((m_ndxCurrent.isSet()) && (m_ndxCurrent.chapter() == 0) && (m_xfteFormatType == XFTE_ZEFANIA)))) {
		// Note: Coming into this function, either ndxCurrent isn't set and we set both book and chapter (OSIS) or book only is set and we set chapter (ZEFANIA)
		if ((m_xfteFormatType == XFTE_OSIS) && ((ndx = findAttribute(atts, "eID")) != -1)) {
			// End of open-ended chapter:
			if ((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() == 0)) {
				std::cerr << "\n*** End of open-ended chapter before start of chapter : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			m_bOpenEndedChapter = false;
			// At the end of closed-form, leave the chapter number set (indicating bInChapter) and process closing in the endElement for this end-tag.  But don't start new chapter here
		} else {
			if (m_xfteFormatType == XFTE_OSIS) {
				ndx = findAttribute(atts, "osisID");
				if (ndx != -1) {
					QStringList lstOsisID = atts.value(ndx).split('.');
					if ((lstOsisID.size() != 2) || ((m_nCurrentBookIndex = bookIndexFromOSISAbbr(lstOsisID.at(0))) == -1)) {
						m_ndxCurrent = CRelIndex();
						std::cerr << "\n*** Unknown Chapter osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} else {
						if (!m_bExcludeDeuterocanonical || !bookIsDeuterocanonical(bookStartingIndex(m_nCurrentBookIndex).book())) {
							if (findAttribute(atts, "sID") != -1) {
								// Start of open-ended chapter:
								if (m_ndxCurrent.chapter()) {
									std::cerr << "\n*** Start of open-ended chapter before end of chapter : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
								}
								m_bOpenEndedChapter = true;
							} else {
								// Standard Closed-Form chapter:
								if (m_bOpenEndedChapter) {
									std::cerr << "\n*** Mixing open-ended and closed form chapter : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
								}
								m_bOpenEndedChapter = false;
							}
							m_ndxCurrent = CRelIndex(bookStartingIndex(m_nCurrentBookIndex).book(), resolveChapter(m_nCurrentBookIndex, lstOsisID.at(1).toUInt()), 0, 0);
						} else {
							m_nCurrentBookIndex = -1;
						}
					}
				} else {
					m_ndxCurrent = CRelIndex();
					std::cerr << "\n*** Chapter with no osisID : ";
					std::cerr << stringifyAttributes(atts).toUtf8().data() << "\n";
				}
			} else if (m_xfteFormatType == XFTE_ZEFANIA) {
				Q_ASSERT((m_ndxCurrent.verse() == 0) && (m_ndxCurrent.word() == 0));
				ndx = findAttribute(atts, "cnumber");
				if (ndx != -1) {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), atts.value(ndx).toUInt(), 0, 0);
					if (m_ndxCurrent.chapter() == 0) {
						std::cerr << QString("\n*** Invalid Chapter Number: \"%1\"\n").arg(atts.value(ndx)).toUtf8().data();
					}
				} else {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);		// Leave the book set for other chapters...
					std::cerr << "\n*** Warning: Found chapter tag without a cnumber\n";
				}
			}

			if (m_ndxCurrent.chapter() != 0) {
				std::cerr << "Book: " << osisAbbrFromBookIndex(m_nCurrentBookIndex).toUtf8().data() << " Chapter: " << QString("%1").arg(m_ndxCurrent.chapter()).toUtf8().data();
				nTst = bookIndexToTestamentIndex(bookStartingIndex(m_nCurrentBookIndex).book());
				m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[m_ndxCurrent];			// Make sure the chapter entry is created, even though we have nothing to put in it yet
				if ((m_ndxCurrent.chapter() == bookStartingIndex(m_nCurrentBookIndex).chapter()) ||
					(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[m_ndxCurrent.book()-1].m_nTstNdx == 0)) {
					addBookToBibleDatabase(m_nCurrentBookIndex);
				}
				Q_ASSERT(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size() >= bookStartingIndex(m_nCurrentBookIndex).book());
				// If revisiting earlier chapters, don't double count them:
				if (m_ndxCurrent.chapter() > m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[bookStartingIndex(m_nCurrentBookIndex).book()-1].m_nNumChp) {
					if (m_ndxCurrent.chapter() == bookStartingIndex(m_nCurrentBookIndex).chapter()) {
						for (CRelIndex ndxAddChp = CRelIndex(bookStartingIndex(m_nCurrentBookIndex).book(), 1, 0, 0); ndxAddChp != m_ndxCurrent; ndxAddChp.setChapter(ndxAddChp.chapter()+1)) {
							m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[ndxAddChp];			// Make sure the chapter entry is created for the empty chapters
						}
						m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumChp += bookStartingIndex(m_nCurrentBookIndex).chapter();
						m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[nTst-1].m_nNumChp += bookStartingIndex(m_nCurrentBookIndex).chapter();
						m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[bookStartingIndex(m_nCurrentBookIndex).book()-1].m_nNumChp += bookStartingIndex(m_nCurrentBookIndex).chapter();
					} else {
						m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumChp++;
						m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[nTst-1].m_nNumChp++;
						m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[bookStartingIndex(m_nCurrentBookIndex).book()-1].m_nNumChp++;
					}
				}
			}
		}
	} else if ((m_ndxCurrent.isSet()) &&
			   (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) ||
				((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("vers", Qt::CaseInsensitive) == 0)))) {
		if ((m_xfteFormatType == XFTE_OSIS) && ((ndx = findAttribute(atts, "eID")) != -1)) {
			// End of open-ended verse:
			if (!m_bInVerse) {
				std::cerr << "\n*** End of open-ended verse before start of verse : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
			}
			m_bOpenEndedVerse = false;
			// We can have nested Words of Jesus with open form:
			if ((m_bInWordsOfJesus) && (m_bInVerse)) {
				CVerseEntry &verse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[m_ndxCurrent.book()-1])[CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0)];
				verse.m_strText += g_chrParseTag;
				verse.m_lstParseStack.push_back("j:");
			}
			// At the end of closed-form, leave m_bInVerse set and process closing in the endElement for this end-tag.  But don't start new verse here
		} else {
			// We need a separate flag here to denote new verse and can't just use whether or not
			//		the verse number is set like we could with the others.  This is because this
			//		same code also processes the split sID/eID tags for open-ended verse logic:
			bool bFoundNewVerse = false;
			bool bOpenEnded = false;
			if (m_xfteFormatType == XFTE_OSIS) {
				ndx = findAttribute(atts, "osisID");
				if (ndx != -1) {
					QStringList lstOsisID = atts.value(ndx).split('.');
					if ((lstOsisID.size() != 3) || ((m_nCurrentBookIndex = bookIndexFromOSISAbbr(lstOsisID.at(0))) == -1)) {
						std::cerr << "\n*** Unknown Verse osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} else if ((m_ndxCurrent.book() != bookStartingIndex(m_nCurrentBookIndex).book()) ||
							   (m_ndxCurrent.chapter() != resolveChapter(m_nCurrentBookIndex, lstOsisID.at(1).toUInt()))) {
						m_ndxCurrent.setVerse(0);
						m_ndxCurrent.setWord(0);
						std::cerr << "\n*** Verse osisID doesn't match Chapter osisID : " << atts.value(ndx).toUtf8().data() << "\n";
					} else {
						if (findAttribute(atts, "sID") != -1) {
							// Start of open-ended verse:
							if (m_bInVerse) {
								std::cerr << "\n*** Start of open-ended verse before end of verse : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
							}
							bOpenEnded = true;
						} else {
							// Standard Closed-Form verse:
							if (m_bOpenEndedVerse) {
								std::cerr << "\n*** Mixing open-ended and closed form verses : osisID=" << atts.value(ndx).toUtf8().data() << "\n";
							}
							m_bOpenEndedVerse = false;
						}

						m_ndxCurrent.setVerse(resolveVerse(m_nCurrentBookIndex, m_ndxCurrent.chapter(), lstOsisID.at(2).toUInt()));
						m_ndxCurrent.setWord(0);
						bFoundNewVerse = true;
					}
				}
			} else if (m_xfteFormatType == XFTE_ZEFANIA) {
				ndx = findAttribute(atts, "vnumber");
				if (ndx != -1) {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), atts.value(ndx).toUInt(), 0);
					if (m_ndxCurrent.verse() == 0) {
						std::cerr << QString("\n*** Invalid Verse Number: \"%1\"\n").arg(atts.value(ndx)).toUtf8().data();
					} else {
						bFoundNewVerse = true;
					}
				} else {
					m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0);		// Leave the book/chapter set for other verses...
					std::cerr << "\n*** Warning: Found verse tag without a vnumber\n";
				}
			}

			if (bFoundNewVerse) {
				if (m_ndxCurrent.verse() == 0) {
					// Special case to allow an edited OSIS file with verse #0 to be used for
					//	superscriptions, such as: Ps.3.0
					//	This facilitates editing OSIS files like the OSHB database which has
					//	the superscriptions actually entered as verses, but with a shifting
					//	of all of the verse indexes that follow it.  Without this, we will
					//	crash with a SegFault because we'll end up with m_bInSuperscription
					//	set, but not m_ndxSuperscription.
					m_ndxSuperscription = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 0, 0);
				}

				if (((m_ndxCurrent.verse() % 5) == 0) && (m_ndxCurrent.verse() != 0)) {
					std::cerr << QString("%1").arg(m_ndxCurrent.verse() / 5).toUtf8().data();
				} else {
					std::cerr << ".";
				}

				bool bPreExisted = ((m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[m_ndxCurrent.book()-1]).find(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0))
										!= (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[m_ndxCurrent.book()-1]).end());

				if (CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0) == bookStartingIndex(m_nCurrentBookIndex)) {
					for (CRelIndex ndxAddVrs = CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), 1, 0); ndxAddVrs != m_ndxCurrent; ndxAddVrs.setVerse(ndxAddVrs.verse()+1)) {
						// Create all intentionally missing verses:
						(m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[ndxAddVrs.book()-1])[CRelIndex(ndxAddVrs.book(), ndxAddVrs.chapter(), ndxAddVrs.verse(), 0)];
					}
				}

				if (bPreExisted &&
					m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[m_ndxCurrent.book()-1].find(CRelIndex(m_ndxCurrent.book(), m_ndxCurrent.chapter(), m_ndxCurrent.verse(), 0))->second.m_nNumWrd) {
					std::cerr << QString("\n*** Warning: Duplicate Verse Entry: \"%1\"\n").arg(m_pBibleDatabase->PassageReferenceText(m_ndxCurrent)).toUtf8().data();
				}

				startVerseEntry(m_ndxCurrent, bOpenEnded);
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (localName.compare("note", Qt::CaseInsensitive) == 0)) {
		m_bInNotes = true;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("N:");
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("milestone", Qt::CaseInsensitive) == 0)) {
		//	Note: If we already have text on this verse, then set a flag to put the pilcrow on the next verse
		//			so we can handle the strange <CM> markers used on the German Schlachter text
		//
		//	PTE_MARKER			Example: {verse}[osisID=Gen.5.21]{milestone}[marker=¶,type=x-p]{/milestone}
		//	PTE_MARKER_ADDED	Example: {verse}[osisID=Gen.5.3]{milestone}[marker=¶,subType=x-added,type=x-p]{/milestone}
		//	PTE_EXTRA			Example: {verse}[osisID=Gen.5.6]{milestone}[type=x-extra-p]{/milestone}

		CVerseEntry::PILCROW_TYPE_ENUM nPilcrow = CVerseEntry::PTE_NONE;

		if (((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("x-p", Qt::CaseInsensitive) == 0)) {
			if (((ndx = findAttribute(atts, "subType")) != -1) && (atts.value(ndx).compare("x-added", Qt::CaseInsensitive) == 0)) {
				nPilcrow = CVerseEntry::PTE_MARKER_ADDED;
			} else{
				nPilcrow = CVerseEntry::PTE_MARKER;
			}
		} else if ((m_xfteFormatType == XFTE_OSIS) && ((ndx = findAttribute(atts, "type")) != -1) && (atts.value(ndx).compare("x-extra-p", Qt::CaseInsensitive) == 0)) {
			nPilcrow = CVerseEntry::PTE_EXTRA;
		}

		CVerseEntry &verse = activeVerseEntry();
		QString strTempText = verse.m_strText;
		strTempText.remove(g_chrParseTag);		// To check for text for delayed pilcrow, remove any parseTag markers as we may have encountered some text modifier tags, but not the actual text yet
		if (strTempText.isEmpty()) {
			verse.m_nPilcrow = nPilcrow;
		} else {
			m_nDelayedPilcrow = nPilcrow;
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("seg", Qt::CaseInsensitive) == 0)) {
		// <seg subType="x-1" type="x-variant">
		ndx = findAttribute(atts, "type");		// TODO : In addition to 'x-variant', add support for full OSIS 'variant', which is currently a work-in-progress
		if ((ndx != -1) && (atts.value(ndx).compare("x-variant", Qt::CaseInsensitive) == 0)) {
			ndx = findAttribute(atts, "subType");
			if (ndx != -1) {
				m_strCurrentSegVariant = atts.value(ndx);
				if ((m_strSegVariant.isEmpty()) || (m_strCurrentSegVariant.compare(m_strSegVariant, Qt::CaseInsensitive) == 0)) {
					m_bFoundSegVariant = true;
				}
			}
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("w", Qt::CaseInsensitive) == 0)) {
		m_bInLemma = true;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("L:" + stringifyAttributes(atts));
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) &&	// Note: Allow transChangeAdded inside of inline bracketed notes
			   ((localName.compare("transChange", Qt::CaseInsensitive) == 0) ||
				(localName.compare("hi", Qt::CaseInsensitive) == 0))) {
		ndx = findAttribute(atts, "type");
		if ((ndx != -1) &&
			(((localName.compare("transChange", Qt::CaseInsensitive) == 0) &&
			  (atts.value(ndx).compare("added", Qt::CaseInsensitive) == 0)) ||		// <transChange type="added">
			 ((localName.compare("hi", Qt::CaseInsensitive) == 0) &&
			  (atts.value(ndx).compare("italic", Qt::CaseInsensitive) == 0)))) {	// <hi type="italic">
			m_bInTransChangeAdded = true;
			CVerseEntry &verse = activeVerseEntry();
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("T:");
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("q", Qt::CaseInsensitive) == 0)) {
		ndx = findAttribute(atts, "who");
		if ((ndx != -1) && (atts.value(ndx).compare("Jesus", Qt::CaseInsensitive) == 0)) {
			m_bInWordsOfJesus = true;
			CVerseEntry &verse = activeVerseEntry();
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("J:");
		}
	} else if ((m_xfteFormatType == XFTE_OSIS) && ((m_bInVerse) ||
												   (m_bInColophon && !m_bNoColophonVerses && !m_bDisableColophons) ||
												   (m_bInSuperscription && !m_bNoSuperscriptionVerses && !m_bDisableSuperscriptions)) &&
			   (!m_bInNotes) && (!m_bInBracketNotes) && (localName.compare("divineName", Qt::CaseInsensitive) == 0)) {
		m_bInDivineName = true;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("D:");
	}

	// Note: In the m_lstParseStack, we'll push values on as follows:
	//			L:<attrs>		-- Lemma Start
	//			l:				-- Lemma End
	//			T:				-- TransChange Added Start
	//			t:				-- TransChange Added End
	//			J:				-- Words of Jesus Start
	//			j:				-- Words of Jesus End
	//			D:				-- Divine Name Start
	//			d:				-- Divine Name End
	//			R:				-- Search Result Start (reserved placeholder, see VerseRichifier)
	//			r:				-- Search Result End (reserved placeholder, see VerseRichifier)
	//			A:				-- Anchor Start (reserved placeholder, see VerseRichifier)
	//			a:				-- Anchor End (reserved placeholder, see VerseRichifier)
	//			M:				-- Hebrew Psalm 119 Marker
	//			N:				-- Inline Parenthetical Note Start (inline footnote, not counted as verse text proper)
	//			n:				-- Inline Parenthetical Note End



/*
	m_lstElementNames.append(localName);
	std::cout << "{" << localName.toUtf8().data() << "}";
	std::cout << "[";
	for (int i = 0; i < atts.count(); ++i) {
		if (i) std::cout << ",";
		std::cout << atts.localName(i).toUtf8().data() << "=" << atts.value(i).toUtf8().data();
//		if (atts.localName(i).compare("type", Qt::CaseInsensitive) == 0) {
			m_lstAttrNames.append(atts.localName(i) + "=" + atts.value(i));
//		}
	}
	std::cout << "]";
*/



	return true;
}

bool COSISXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(qName);


	if (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("header", Qt::CaseInsensitive) == 0)) ||
		((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("INFORMATION", Qt::CaseInsensitive) == 0))) {
		m_bInHeader = false;
	} else if ((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("language", Qt::CaseInsensitive) == 0) && (m_bInHeader)) {
		// Convert Language:
		LANGUAGE_ID_ENUM nOrigLangID = m_pBibleDatabase->langID();
		m_pBibleDatabase->m_descriptor.m_strLanguage = m_strLanguage;
		std::cerr << "Language: " << m_pBibleDatabase->language().toUtf8().data();
		if (nOrigLangID != m_pBibleDatabase->langID()) {
			std::cerr << "    *** Warning: Original Bible Descriptor Language doesn't match database language\n";
		}
		if (CTranslatorList::instance()->setApplicationLanguage(toQtLanguageName(m_pBibleDatabase->langID()))) {
			std::cerr << " (Loaded Translations)\n";
		} else {
			std::cerr << " (NO Translations Found!)\n";
		}
		m_bCaptureLang = false;
	} else if (localName.compare("title", Qt::CaseInsensitive) == 0) {
		if (m_bInHeader) {
			std::cerr << "Title: " << m_strTitle.toUtf8().data() << "\n";
			if (m_strTitle.compare(m_pBibleDatabase->description(), Qt::CaseInsensitive) != 0) {
				std::cerr << "    *** Warning: Original Bible Description doesn't match database:\n        \""
							<< m_pBibleDatabase->description().toUtf8().data() << "\"\n";
			}
			m_pBibleDatabase->m_descriptor.m_strDBDesc = m_strTitle;
		}
		m_bCaptureTitle = false;
		if ((m_bInSuperscription) && (!m_bOpenEndedSuperscription)) {
			endVerseEntry(m_ndxSuperscription);
		}
	} else if (localName.compare("foreign", Qt::CaseInsensitive) == 0) {
		m_bInForeignText = false;
	} else if ((m_bInColophon) && (!m_bOpenEndedColophon) && (!m_bUseBracketColophons) && (localName.compare("div", Qt::CaseInsensitive) == 0)) {
		endVerseEntry(m_ndxColophon);
	} else if ((m_xfteFormatType == XFTE_ZEFANIA) && (!m_bInVerse) && (localName.compare("BIBLEBOOK", Qt::CaseInsensitive) == 0)) {
		m_ndxCurrent = CRelIndex();
	} else if ((!m_bInVerse) && (localName.compare("chapter", Qt::CaseInsensitive) == 0) && (!m_bOpenEndedChapter)) {
		if (m_ndxCurrent.book() != 0) {
			std::cerr << "\n";
		}
		if (m_bInBracketNotes) std::cerr << "\n*** Error: Missing end of Bracketed Footnotes within chapter\n";
		m_bInBracketNotes = false;
		if (m_xfteFormatType == XFTE_OSIS) {
			m_ndxCurrent = CRelIndex();
		} else if (m_xfteFormatType == XFTE_ZEFANIA) {
			m_ndxCurrent = CRelIndex(m_ndxCurrent.book(), 0, 0, 0);
		}
// Technically, we shouldn't have a chapter inside verse, but some modules use it as a special inner marking (like FrePGR, for example):
//		Q_ASSERT(m_bInVerse == false);
//		if (m_bInVerse) {
//			std::cerr << "\n*** End-of-Chapter found before End-of-Verse\n";
//			m_bInVerse = false;
//		}
	} else if ((((m_bInVerse) && (!m_bOpenEndedVerse)) || (m_bInSuperscription)) &&		// Check m_bInSuperscription here for verse 0 declared superscriptions, like Ps.3.0
			   (((m_xfteFormatType == XFTE_OSIS) && (localName.compare("verse", Qt::CaseInsensitive) == 0)) ||
				((m_xfteFormatType == XFTE_ZEFANIA) && (localName.compare("vers", Qt::CaseInsensitive) == 0)))) {
		endVerseEntry(m_ndxCurrent);
	} else if ((m_bInNotes) && (localName.compare("note", Qt::CaseInsensitive) == 0)) {
		m_bInNotes = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("n:");
	} else if (localName.compare("seg", Qt::CaseInsensitive) == 0) {
		m_strCurrentSegVariant.clear();
	} else if ((m_bInLemma) && (localName.compare("w", Qt::CaseInsensitive) == 0)) {
		m_bInLemma = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("l:");
	} else if ((m_bInTransChangeAdded) &&
			   ((localName.compare("transChange", Qt::CaseInsensitive) == 0) ||
				(localName.compare("hi", Qt::CaseInsensitive) == 0))) {
		m_bInTransChangeAdded = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("t:");
	} else if ((m_bInWordsOfJesus) && (localName.compare("q", Qt::CaseInsensitive) == 0)) {
		m_bInWordsOfJesus = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("j:");
	} else if ((m_bInDivineName) && (localName.compare("divineName", Qt::CaseInsensitive) == 0)) {
		m_bInDivineName = false;
		CVerseEntry &verse = activeVerseEntry();
		verse.m_strText += g_chrParseTag;
		verse.m_lstParseStack.push_back("d:");
	}



//	std::cout << "{/" << localName.toUtf8().data() << "}\n";

	return true;
}

bool COSISXmlHandler::characters(const QString &ch)
{
	QString strTemp = ch;
// TODO : REMOVE
//	strTemp.replace('\n', ' ');

	if (m_bCaptureTitle) {
		m_strTitle += strTemp;
	} else if (m_bCaptureLang) {
		m_strLanguage += strTemp;
	} else if ((!m_strCurrentSegVariant.isEmpty()) && (!m_strSegVariant.isEmpty()) && (m_strCurrentSegVariant.compare(m_strSegVariant, Qt::CaseInsensitive) != 0)) {
		// Eat the characters if we are in a <seg> variant other than the one specified to capture,
		//		and we were actually given a <seg> variant to capture...
	} else if (m_bInColophon && !m_bUseBracketColophons) {
		if (!m_bDisableColophons) {
			Q_ASSERT(m_ndxColophon.isSet());
			charactersVerseEntry(m_ndxColophon, strTemp);
		}
	} else if ((m_bInSuperscription) && (!m_bInForeignText)) {
		if (!m_bDisableSuperscriptions) {
			Q_ASSERT(m_ndxSuperscription.isSet());
			charactersVerseEntry(m_ndxSuperscription, strTemp);
		}
	} else if ((m_bInVerse) && (!m_bInNotes) && (!m_bInForeignText)) {
		Q_ASSERT((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() != 0) && (m_ndxCurrent.verse() != 0));
		charactersVerseEntry(m_ndxCurrent, strTemp);
//		std::cout << strTemp.toUtf8().data();
	} else if (((m_bInVerse) || (m_bInColophon && !m_bDisableColophons) || (m_bInSuperscription && !m_bDisableSuperscriptions)) && (m_bInNotes)) {
		if (m_bInVerse || (m_bInColophon && !m_bNoColophonVerses) || (m_bInSuperscription && !m_bNoSuperscriptionVerses)) {
			Q_ASSERT((m_ndxCurrent.book() != 0) && (m_ndxCurrent.chapter() != 0) && (m_ndxCurrent.verse() != 0));
			charactersVerseEntry(m_ndxCurrent, strTemp);
		}
	}



//	std::cout << ch.toUtf8().data();

	return true;
}

bool COSISXmlHandler::error(const QXmlParseException &exception)
{
	std::cerr << QString("\n\n*** %1\n").arg(exception.message()).toUtf8().data();
	std::cerr << QString("Line: %1  Column: %2\nPublicID: \"%3\"\nSystemID: \"%4\"\n")
					.arg(exception.lineNumber()).arg(exception.columnNumber())
					.arg(exception.publicId()).arg(exception.systemId()).toUtf8().data();
	return true;
}

bool COSISXmlHandler::fatalError(const QXmlParseException &exception)
{
	return error(exception);
}

bool COSISXmlHandler::warning(const QXmlParseException &exception)
{
	return error(exception);
}

bool COSISXmlHandler::endDocument()
{
	// Nothing else needs to be done here for the main database XML,
	//	but now that that's complete, we'll parse any given Strongs
	//	Database here to go with it:
	if (m_strStrongsImpFilepath.isEmpty()) return true;

	QFile fileStrongsImp;

	fileStrongsImp.setFileName(m_strStrongsImpFilepath);
	if (!fileStrongsImp.open(QIODevice::ReadOnly)) {
		m_strErrorString = QString("*** Failed to open Strongs Imp Database \"%1\"\n").arg(m_strStrongsImpFilepath).toUtf8().data();
		return false;
	}

	std::cerr << "Strongs: ";

	int nProgress = 0;
	QString strIndexLine;
	QByteArray baDataLine;
	while (!fileStrongsImp.atEnd()) {
		if ((nProgress % 100) == 0) std::cerr << ".";
		++nProgress;

		strIndexLine = QString(fileStrongsImp.readLine()).trimmed();
		baDataLine = fileStrongsImp.readLine();
		if (strIndexLine.isEmpty()) continue;		// Handle extra newline at end of file

		if (strIndexLine.left(3) != "$$$") {
			std::cerr << QString("\n\n*** Malformed Strongs Index: %1\n").arg(strIndexLine).toUtf8().data();
			continue;
		} else {
			strIndexLine = strIndexLine.mid(3);
		}
		CStrongsImpXmlHandler xmlHandler(strIndexLine);
		if (xmlHandler.strongsEntry().strongsIndex() == 0) {
// Note: All link indexes in database of orthography will generate this warning, so
//	leave this commented out -- but leave the line here in case we need to turn it on
//	to debug a new database:
//			std::cerr << QString("\n*** Unknown Strongs Index: %1\n").arg(strIndexLine).toUtf8().data();
			continue;
		}

		QBuffer xmlBuffer(&baDataLine);
		QXmlInputSource xmlInput(&xmlBuffer);
		QXmlSimpleReader xmlReader;

		xmlReader.setContentHandler(&xmlHandler);
		xmlReader.setErrorHandler(&xmlHandler);
		if (xmlReader.parse(xmlInput)) {
			if (m_pBibleDatabase->m_mapStrongsEntries.find(xmlHandler.strongsEntry().strongsMapIndex()) !=
				m_pBibleDatabase->m_mapStrongsEntries.cend()) {
				std::cerr << QString("\n*** Duplicate Strongs Map Index: %1\n").arg(strIndexLine).toUtf8().data();
			}
			m_pBibleDatabase->m_mapStrongsEntries[xmlHandler.strongsEntry().strongsMapIndex()] = xmlHandler.strongsEntry();
			m_pBibleDatabase->m_mapStrongsOrthographyMap.insert(xmlHandler.strongsEntry().orthographyPlainText(), xmlHandler.strongsEntry().strongsMapIndex());
		} else {
			std::cerr << QString("\n\n*** Failed to parse Strongs Index: %1\n").arg(strIndexLine).toUtf8().data();
		}
	}
	std::cerr << "\n";

	return true;
}

// ----------------------------------------------------------------------------

enum VTYPE_ENUM {
	VT_VERSE = 0,
	VT_SUPERSCRIPTION = 1,
	VT_COLOPHON = 2
};

void COSISXmlHandler::startVerseEntry(const CRelIndex &relIndex, bool bOpenEnded)
{
	VTYPE_ENUM nVT = VT_VERSE;
	if (relIndex.verse() == 0) {
		nVT = ((relIndex.chapter() == 0) ? VT_COLOPHON : VT_SUPERSCRIPTION);
	}

	if (m_bInLemma) std::cerr << "\n*** Error: Missing end of Lemma\n";
	m_bInLemma = false;
	if (m_bInTransChangeAdded) std::cerr << "\n*** Error: Missing end of TransChange Added\n";
	m_bInTransChangeAdded = false;
	if (m_bInNotes) std::cerr << "\n*** Error: Missing end of Notes\n";
	m_bInNotes = false;
	if (m_bInForeignText) std::cerr << "\n*** Error: Missing end of Foreign text\n";
	m_bInForeignText = false;
	if (m_bInDivineName) std::cerr << "\n*** Error: Missing end of Divine Name\n";
	m_bInDivineName = false;

	if ((nVT == VT_COLOPHON) && (m_bNoColophonVerses || m_bDisableColophons)) {
		if (!m_bOpenEndedColophon) {
			if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus at " << m_pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data() << "\n";
			m_bInWordsOfJesus = false;
		}

		if (m_bInSuperscription) std::cerr << "\n*** Error: Missing end of Superscription\n";
		m_bInSuperscription = false;
		m_bOpenEndedSuperscription = false;

		m_bInColophon = true;
		if (bOpenEnded) m_bOpenEndedColophon = true;

		return;
	}

	if (nVT == VT_SUPERSCRIPTION) {
		bool bLocalDisableSuperscription = false;
		if ((m_ndxCurrent.verse() != 0) && (m_ndxCurrent.verse() != 1)) {
			// Note: Some texts use "in-chapter titles" to break the chapter
			//	into subdivisions.  These are not superscriptions, so only
			//	allow superscriptions if we are at the top of the chapter
			//	itself or in the first verse, such as verse 1 preverse text:
			bLocalDisableSuperscription = true;
		}
		if (bLocalDisableSuperscription || m_bNoSuperscriptionVerses || m_bDisableSuperscriptions) {
			if (!m_bOpenEndedSuperscription) {
				if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus at " << m_pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data() << "\n";
				m_bInWordsOfJesus = false;
			}

			if (m_bInColophon) std::cerr << "\n*** Error: Missing end of Colophon\n";
			m_bInColophon = false;
			m_bOpenEndedColophon = false;

			m_bInSuperscription = true;
			if (bOpenEnded) m_bOpenEndedSuperscription = true;

			return;
		}
	}

	bool bPreExisted = ((m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1]).find(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0))
							!= (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1]).end());
	CVerseEntry &verse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)];

	if (m_nDelayedPilcrow != CVerseEntry::PTE_NONE) {
		verse.m_nPilcrow = m_nDelayedPilcrow;
		m_nDelayedPilcrow = CVerseEntry::PTE_NONE;
	}

	if (nVT == VT_VERSE) {
		m_bInVerse = true;
		if (bOpenEnded) m_bOpenEndedVerse = true;
	} else {
// Note: Can have nested Superscriptions/Colophons inside of Verse tag!  So, don't
//		do this check:
//		if (m_bInVerse) std::cerr << "\n*** Error: Missing end of Verse\n";
//		m_bInVerse = false;
//		m_bOpenEndedVerse = false;
	}

	if (nVT == VT_COLOPHON) {
		m_bInColophon = true;
		if (bOpenEnded) m_bOpenEndedColophon = true;
	} else {
		if (m_bInColophon) std::cerr << "\n*** Error: Missing end of Colophon\n";
		m_bInColophon = false;
		m_bOpenEndedColophon = false;
	}

	if (nVT == VT_SUPERSCRIPTION) {
		m_bInSuperscription = true;
		if (bOpenEnded) m_bOpenEndedSuperscription = true;
	} else {
		if (m_bInSuperscription) std::cerr << "\n*** Error: Missing end of Superscription\n";
		m_bInSuperscription = false;
		m_bOpenEndedSuperscription = false;
	}

	if (((nVT == VT_VERSE) && (!m_bOpenEndedVerse)) ||
		((nVT == VT_COLOPHON) && (!m_bOpenEndedColophon)) ||
		((nVT == VT_SUPERSCRIPTION) && (!m_bOpenEndedSuperscription))) {
		if (m_bInWordsOfJesus) std::cerr << "\n*** Error: Missing end of Words-of-Jesus at " << m_pBibleDatabase->PassageReferenceText(relIndex).toUtf8().data() << "\n";
		m_bInWordsOfJesus = false;
	} else {
		if (m_bInWordsOfJesus) {
			// We can have nested Words of Jesus with open form:
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("J:");
		}
	}

	if (m_bInBracketNotes) {
		// If still within BracketNotes from previous verse,
		//	extended it into this one:
		verse.m_strText.append(g_chrParseTag);
		verse.m_lstParseStack.push_back("N:");
	}

	if (nVT == VT_VERSE) {
		if (!bPreExisted) {			// Only increment verse counts if this isn't a duplicate (pre-existing) verse.  Otherwise, we'll crash in the word output and summary phase
			unsigned int nVerseOffset = 1;
			unsigned int nTst = bookIndexToTestamentIndex(relIndex.book());
			if (CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0) == bookStartingIndex(m_nCurrentBookIndex)) nVerseOffset = bookStartingIndex(m_nCurrentBookIndex).verse();
			m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumVrs += nVerseOffset;
			Q_ASSERT(static_cast<unsigned int>(nTst) <= m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments.size());
			m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[nTst-1].m_nNumVrs += nVerseOffset;
			Q_ASSERT(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size() > static_cast<unsigned int>(relIndex.book()-1));
			m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[relIndex.book()-1].m_nNumVrs += nVerseOffset;
			m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[CRelIndex(relIndex.book(), relIndex.chapter(), 0, 0)].m_nNumVrs += nVerseOffset;
		}
		if ((relIndex.book() == PSALMS_BOOK_NUM) && (relIndex.chapter() == 119) && (((relIndex.verse()-1)%8) == 0)) {
			verse.m_strText += g_chrParseTag;
			verse.m_lstParseStack.push_back("M:");
		}
	}

	if (nVT == VT_COLOPHON) {
		Q_ASSERT(m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.size() > static_cast<unsigned int>(relIndex.book()-1));
		m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[relIndex.book()-1].m_bHaveColophon = true;
	}

	if (nVT == VT_SUPERSCRIPTION) {
		m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[CRelIndex(relIndex.book(), relIndex.chapter(), 0, 0)].m_bHaveSuperscription = true;
	}
}

void COSISXmlHandler::charactersVerseEntry(const CRelIndex &relIndex, const QString &strText)
{
	VTYPE_ENUM nVT = VT_VERSE;
	if (relIndex.verse() == 0) {
		nVT = ((relIndex.chapter() == 0) ? VT_COLOPHON : VT_SUPERSCRIPTION);
	}

	if ((nVT == VT_COLOPHON) && (m_bNoColophonVerses || m_bDisableColophons)) {
		return;
	}
	if (nVT == VT_SUPERSCRIPTION) {
		bool bLocalDisableSuperscription = false;
		if ((m_ndxCurrent.verse() != 0) && (m_ndxCurrent.verse() != 1)) {
			// Note: Some texts use "in-chapter titles" to break the chapter
			//	into subdivisions.  These are not superscriptions, so only
			//	allow superscriptions if we are at the top of the chapter
			//	itself or in the first verse, such as verse 1 preverse text:
			bLocalDisableSuperscription = true;
		}
		if (bLocalDisableSuperscription || m_bNoSuperscriptionVerses || m_bDisableSuperscriptions) return;
	}

	Q_ASSERT(!strText.contains(g_chrParseTag, Qt::CaseInsensitive));
	if (strText.contains(g_chrParseTag, Qt::CaseInsensitive)) {
		std::cerr << "\n*** ERROR: Text contains the special parse tag!!  Change the tag in KJVDataParse and try again!\n";
	}

	QString strTempText = strText;

	CVerseEntry &verse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)];

	// Do bracket footnotes ahead of bracket colophons
	if (m_bUseBracketFootnotes) {		// Note: Bracket-Footnotes will appear in normal verse text during parsing
		int nBracketRefCount = 0;
		int ndxStart;
		int ndxEnd;
		while (m_bInBracketNotes || ((ndxStart = strTempText.indexOf(QChar('['))) != -1)) {		// If continued from previous verse or starting in this verse
			if (!m_bInBracketNotes) {
				strTempText.replace(ndxStart, 1, g_chrParseTag);
				verse.m_lstParseStack.push_back("N:");
			} else {
				// Here if this is a continuation from a previous entry.  If
				//	so, the starting parse tag will have already been written.
				//	In this case, set up to find the ending:
				ndxStart = -1;		// Set to -1 so end search is correct below
				m_bInBracketNotes = false;		// Clear flag to break out of this loop
			}
			++nBracketRefCount;
			ndxEnd = strTempText.indexOf(QChar(']'), ndxStart+1);
			if (ndxEnd != -1) {
				strTempText.replace(ndxEnd, 1, g_chrParseTag);
				verse.m_lstParseStack.push_back("n:");
				--nBracketRefCount;
			}
		}
		if (nBracketRefCount) m_bInBracketNotes = true;		// Keep it set if still in bracketing
		// Allow one marker carryover without warning:
		if ((nBracketRefCount > 1) || (strTempText.contains(QChar('['))) || (strTempText.contains(QChar(']')))) {
			std::cerr << "\n*** Warning: Mismatched Bracket-Footnote Markers\n";
		}
	} else
	  // Do bracket colophons ahead of bracket italics
	  if ((m_bUseBracketColophons) && (nVT == VT_VERSE)) {		// Note: Bracket-Colophons will appear in normal verse text during parsing, not a "real colophon"
		if (!m_bInColophon) {
			int ndxStart;
			int ndxEnd;
			if ((ndxStart = strTempText.indexOf(QChar('['))) != -1) {
				ndxEnd = strTempText.mid(ndxStart+1).indexOf(QChar(']'));
				if (!m_bNoColophonVerses) {
					m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[relIndex.book()-1].m_bHaveColophon = true;
					CVerseEntry &colophonVerse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), 0, 0, 0)];
					colophonVerse.m_strText += strTempText.mid(ndxStart+1).mid(0, ndxEnd);
				}
				m_ndxColophon = CRelIndex(relIndex.book(), 0, 0, 0);
				if (ndxEnd != -1) {
					m_bInColophon = true;			// Though short lived here, set the flags for endVerseEntry() processing, etc...
					m_bOpenEndedColophon = true;
					strTempText = strTempText.mid(0, ndxStart-1) + strTempText.mid(ndxStart+1).mid(ndxEnd+1);
					endVerseEntry(m_ndxColophon);
					m_ndxColophon.clear();
					m_bInColophon = false;
					m_bOpenEndedColophon = false;
				} else {
					strTempText.clear();
					m_bInColophon = true;
					m_bOpenEndedColophon = true;
				}
			}
		} else if (m_bInColophon) {
			int ndxEnd;
			CVerseEntry &colophonVerse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1])[m_ndxColophon];
			if ((ndxEnd = strTempText.indexOf(QChar(']'))) != -1) {
				if (!m_bNoColophonVerses) {
					colophonVerse.m_strText += strTempText.mid(0, ndxEnd);
				}
				strTempText = strTempText.mid(ndxEnd+1);
				endVerseEntry(m_ndxColophon);
				m_ndxColophon.clear();
				m_bInColophon = false;
				m_bOpenEndedColophon = false;
			} else {
				if (!m_bNoColophonVerses) {
					colophonVerse.m_strText += strTempText;
				}
				strTempText.clear();
			}
		}
	} else if (m_bBracketItalics) {
		int nItalicRefCount = 0;
		int ndxStart;
		int ndxEnd;
		while ((ndxStart = strTempText.indexOf(QChar('['))) != -1) {
			strTempText.replace(ndxStart, 1, g_chrParseTag);
			verse.m_lstParseStack.push_back("T:");
			++nItalicRefCount;
			ndxEnd = strTempText.indexOf(QChar(']'), ndxStart+1);
			if (ndxEnd != -1) {
				strTempText.replace(ndxEnd, 1, g_chrParseTag);
				verse.m_lstParseStack.push_back("t:");
				--nItalicRefCount;
			}
		}
		if ((nItalicRefCount != 0) || (strTempText.contains(QChar('['))) || (strTempText.contains(QChar(']')))) {
			std::cerr << "\n*** Warning: Mismatched Bracket-Italic Markers\n";
		}
	}

	//	verse.m_strText += strText;
	verse.m_strText += (m_bInDivineName ? strTempText.toUpper() : strTempText);
}

void COSISXmlHandler::endVerseEntry(CRelIndex &relIndex)
{
	VTYPE_ENUM nVT = VT_VERSE;
	if (relIndex.verse() == 0) {
		nVT = ((relIndex.chapter() == 0) ? VT_COLOPHON : VT_SUPERSCRIPTION);
	}

	if ((nVT == VT_COLOPHON) && (m_bNoColophonVerses || m_bDisableColophons)) {
		m_bInColophon = false;
		return;
	}
	if (nVT == VT_SUPERSCRIPTION) {
		bool bLocalDisableSuperscription = false;
		if ((m_ndxCurrent.verse() != 0) && (m_ndxCurrent.verse() != 1)) {
			// Note: Some texts use "in-chapter titles" to break the chapter
			//	into subdivisions.  These are not superscriptions, so only
			//	allow superscriptions if we are at the top of the chapter
			//	itself or in the first verse, such as verse 1 preverse text:
			bLocalDisableSuperscription = true;
		}
		if (bLocalDisableSuperscription || m_bNoSuperscriptionVerses || m_bDisableSuperscriptions) {
			m_bInSuperscription = false;
			return;
		}
	}

	CVerseEntry &verse = (m_pBibleDatabase->m_itrCurrentLayout->m_lstBookVerses[relIndex.book()-1])[CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)];
	QString strTemp = verse.m_strText;

	if (m_bInBracketNotes) {
		// If BracketNotes crosses verse boundary, put matching
		//	tag at the end of the verse:
		strTemp.append(g_chrParseTag);
		verse.m_lstParseStack.push_back("n:");
	}

	unsigned int nWordCount = 0;
	bool bInWord = false;
	bool bInlineNote = false;
	CRelIndex ndxLastFootnoteActive = relIndex;
	CRelIndex ndxFootnoteActive = relIndex;
	ndxFootnoteActive.setWord(nWordCount+1);
	QString strWord;
	QString strRichWord;
	QStringList lstWords;

	TPhraseTag tagLemmaEntry;
	QString strLemmaAttr;

	QStringList lstRichWords;
	bool bHaveDoneTemplateWord = false;				// Used to tag words crossing parse-stack boundary (i.e. half the word is inside the parse operator and half is outside, like the word "inasmuch")

	// ----------------------------------------------------

	// fncDoTemplateWord -- Makes sure that the template has
	//	been populated with the 'w' word marker and that the
	//	current word has been counted.  It also sets the
	//	active footnote index to the next word.  This does
	//	NOT push the word onto the list of words.  It only
	//	makes sure the template position for the current word
	//	is set so that other markers will be in the correct
	//	position -- such as ending the word prior to Words of
	//	Jesus markup or finishing the last word after that
	//	markup is finished:
	auto &&fncDoTemplateWord = [&]() {
		if (bInWord) {
			if (!bHaveDoneTemplateWord) {
				++nWordCount;
				ndxFootnoteActive.setWord(nWordCount+1);
				verse.m_strTemplate += QString("w");
			}
			bHaveDoneTemplateWord = true;
		}
	};

	// ----------------------------------------------------

	// fncCompleteWord -- If we are currently in a word, this
	//	function will push the word on the list of words for
	//	the verse and clears the word variables and flags
	//	ready for the next word:
	auto &&fncCompleteWord = [&]() {
		if (bInWord) {
			Q_ASSERT(!strRichWord.isEmpty());
			Q_ASSERT(!strWord.isEmpty());

			if ((strRichWord.size() == 1) &&
				((g_strHyphens.contains(strRichWord.at(0))) ||
				 (g_strApostrophes.contains(strRichWord.at(0))))) {
				// Don't count words that are only a hyphen or apostrophe:
				verse.m_strTemplate += strRichWord;
	#if QT_VERSION >= 0x050F00
			} else if (m_bNoArabicNumeralWords && (QRegularExpression("\\d*").match(strWord).hasMatch())) {
	#else
			} else if (m_bNoArabicNumeralWords && (QRegExp("\\d*").exactMatch(strWord))) {
	#endif
				// If we aren't counting Arabic Numerals as words, move them out to the verse template for rendering but not counting:
				verse.m_strTemplate += strWord;		// It shouldn't matter here if we use Word or RichWord (unlike apostrophes above)
			} else {
				QString strPostTemplate;		// Needed so we get the "w" marker in the correct place
				// Remove trailing hyphens from words and put them in the template.
				//		We'll keep trailing apostophes for posessive words, like: "Jesus'":
				while ((!strRichWord.isEmpty()) && (g_strHyphens.contains(strRichWord.at(strRichWord.size()-1)))) {
					Q_ASSERT(!strWord.isEmpty());
					strPostTemplate += strRichWord.at(strRichWord.size()-1);
					strRichWord = strRichWord.left(strRichWord.size()-1);
					strWord = strWord.left(strWord.size()-1);
				}
				if (!strRichWord.isEmpty()) {
					fncDoTemplateWord();		// Make sure we've written our template word
					relIndex.setWord(verse.m_nNumWrd + nWordCount);
					lstWords.append(strWord);
					lstRichWords.append(strRichWord);
				}
				verse.m_strTemplate += strPostTemplate;
			}
			strWord.clear();
			strRichWord.clear();
			bInWord = false;
		}
		bHaveDoneTemplateWord = false;
	};

	// ----------------------------------------------------

	while (!strTemp.isEmpty()) {
		bool bIsHyphen = g_strHyphens.contains(strTemp.at(0));
		bool bIsApostrophe = g_strApostrophes.contains(strTemp.at(0));
		if (strTemp.at(0) == g_chrParseTag) {
			fncDoTemplateWord();		// Write our current template word tag before processing this tag so things like Words of Jesus are correct

			Q_ASSERT(!verse.m_lstParseStack.isEmpty());
			if (!verse.m_lstParseStack.isEmpty()) {
				QString strParse = verse.m_lstParseStack.at(0);
				verse.m_lstParseStack.pop_front();
				int nPos = strParse.indexOf(':');
				Q_ASSERT(nPos != -1);		// Every ParseStack entry must contain a ':'
				QString strOp = strParse.left(nPos);
				if (strOp.compare("L") == 0) {
					if (bInWord) {
						verse.m_strTemplate += " ";		// Need a space to avoid "ww" templates that don't track correctly (which can happen when missing a space during manual OSIS edit)
						fncCompleteWord();		// Complete the word for this lemma sequence, as they must be distinct words
					}
					tagLemmaEntry.setRelIndex(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), nWordCount+1));
					tagLemmaEntry.setCount(1);
					strLemmaAttr = strParse.mid(nPos+1);
				} else if (strOp.compare("l") == 0) {
					tagLemmaEntry.setCount((nWordCount+1)-tagLemmaEntry.relIndex().word());
					m_pBibleDatabase->m_mapLemmaEntries[tagLemmaEntry.relIndex()] = CLemmaEntry(tagLemmaEntry, strLemmaAttr);
					fncCompleteWord();		// Complete the word for this lemma sequence, as they must be distinct words
				} else if (strOp.compare("T") == 0) {
					if (!bInlineNote) {
						verse.m_strTemplate += "T";
					} else {
						// Convert TransChangeAdded in footnotes to brackets:
						CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
						footnote.setText(footnote.text() + QChar('['));
					}
				} else if (strOp.compare("t") == 0) {
					if (!bInlineNote) {
						verse.m_strTemplate += "t";
					} else {
						// Convert TransChangeAdded in footnotes to brackets:
						CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
						footnote.setText(footnote.text() + QChar(']'));
					}
				} else if (strOp.compare("J") == 0) {
					verse.m_strTemplate += "J";
				} else if (strOp.compare("j") == 0) {
					verse.m_strTemplate += "j";
				} else if (strOp.compare("D") == 0) {
					verse.m_strTemplate += "D";
				} else if (strOp.compare("d") == 0) {
					verse.m_strTemplate += "d";
				} else if (strOp.compare("M") == 0) {
					verse.m_strTemplate += "M";
					// For special Ps 119 Hebrew markers, add x-extra-p Pilcrow to
					//		add a pseudo-paragraph break if there currently isn't
					//		one, as it makes these more readable:
					if (verse.m_nPilcrow == CVerseEntry::PTE_NONE)
						verse.m_nPilcrow = CVerseEntry::PTE_EXTRA;
				} else if (strOp.compare("N") == 0) {
					if ((!m_bUseBracketFootnotes && m_bInlineFootnotes) ||
						(m_bUseBracketFootnotes && !m_bUseBracketFootnotesExcluded)) {
						if (ndxFootnoteActive != ndxLastFootnoteActive) {
							verse.m_strTemplate += "N";
						}
					} else {
						// If not outputting the inline note, remove the
						//	extra space from the text that preceeded it:
						if (!verse.m_strTemplate.isEmpty() &&
							verse.m_strTemplate.at(verse.m_strTemplate.size()-1).isSpace()) {
							verse.m_strTemplate = verse.m_strTemplate.left(verse.m_strTemplate.size()-1);
						}
					}
					bInlineNote = true;

					// If the active footnote index already has text, add a
					//	separator so that this next footnote starting won't
					//	be jammed up against the first.  This is necessary
					//	for texts, like the KJV-1769, where multiple study
					//	notes exists back-to-back at the end of a verse:
					CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
					if (!footnote.text().isEmpty()) {
						footnote.setText(footnote.text() + "; ");
					}
				} else if (strOp.compare("n") == 0) {
					if ((!m_bUseBracketFootnotes && m_bInlineFootnotes) ||
						(m_bUseBracketFootnotes && !m_bUseBracketFootnotesExcluded)) {
						if (ndxFootnoteActive != ndxLastFootnoteActive) {
							verse.m_strTemplate += "n";
						}
					}
					bInlineNote = false;

					ndxLastFootnoteActive = ndxFootnoteActive;
				} else {
					Q_ASSERT(false);		// Unknown ParseStack Operator!
				}
			}
		} else if (bInlineNote) {
			CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxFootnoteActive];
			footnote.setText(footnote.text() + strTemp.at(0));
		} else if ((strTemp.at(0).unicode() < 128) ||
			(g_strNonAsciiNonWordChars.contains(strTemp.at(0))) ||
			(strTemp.at(0) == g_chrPilcrow) ||
			(strTemp.at(0) == g_chrParseTag) ||
			(bIsHyphen) ||
			(bIsApostrophe)) {

			if (nVT == VT_COLOPHON) {
				// Special footnote version of colophon
				CRelIndex ndxColophon = relIndex;
				ndxColophon.setWord(0);
				CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxColophon];
				footnote.setText(footnote.text() + strTemp.at(0));
			} else if (nVT == VT_SUPERSCRIPTION) {
				// Special footnote version of superscription
				CRelIndex ndxSuperscription = relIndex;
				ndxSuperscription.setWord(0);
				CFootnoteEntry &footnote = m_pBibleDatabase->m_mapFootnotes[ndxSuperscription];
				footnote.setText(footnote.text() + strTemp.at(0));
			}

			if ((g_strAsciiWordChars.contains(strTemp.at(0))) ||
				((bIsHyphen) && (!strRichWord.isEmpty())) ||				// Don't let words start with hyphen or apostrophe
				((bIsApostrophe) && (!strRichWord.isEmpty()))) {
				bInWord = true;
				if (bIsHyphen) {
					strWord += '-';
				} else if (bIsApostrophe) {
					strWord += '\'';
				} else strWord += strTemp.at(0);
				strRichWord += strTemp.at(0);
			} else {
				// We've hit a non-word character, so write our
				//	word to the stack:
				fncCompleteWord();

				if (strTemp.at(0) != g_chrPilcrow) {
					if (strTemp.at(0) == g_chrParseTag) {		// TODO : Can this case even happen here??
						std::cerr << "\n*** WARNING: Text contains our special parse tag character and may cause parsing issues\nTry recompiling using a different g_chrParseTag character!\n";
					}
					verse.m_strTemplate += strTemp.at(0);
				} else {
					// If we see a pilcrow marker in the text, but the OSIS didn't declare it, go ahead and add it
					//	as a marker, but flag it of type "added":
					if (verse.m_nPilcrow == CVerseEntry::PTE_NONE) verse.m_nPilcrow = CVerseEntry::PTE_MARKER_ADDED;
				}
			}
		} else {
			// Add characters to our current word:

			if (!m_strParsedUTF8Chars.contains(strTemp.at(0))) m_strParsedUTF8Chars += strTemp.at(0);
			bInWord = true;
			strWord += StringParse::deLigature(strTemp.at(0));	// Translate ligatures, but leave other UTF-8 untranslated
			strRichWord += strTemp.at(0);
		}

		strTemp = strTemp.right(strTemp.size()-1);
	}

	Q_ASSERT(verse.m_lstParseStack.isEmpty());		// We should have exhausted the stack above!

	// If we are in a word, finish writing it before
	//	doing database counts:
	fncCompleteWord();

	// Calculate counts for this verse for the database:
	m_pBibleDatabase->m_itrCurrentLayout->m_EntireBible.m_nNumWrd += nWordCount;
	m_pBibleDatabase->m_itrCurrentLayout->m_lstTestaments[m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks.at(relIndex.book()-1).m_nTstNdx-1].m_nNumWrd += nWordCount;
	m_pBibleDatabase->m_itrCurrentLayout->m_lstBooks[relIndex.book()-1].m_nNumWrd += nWordCount;
	if (relIndex.chapter() != 0) {
		m_pBibleDatabase->m_itrCurrentLayout->m_mapChapters[CRelIndex(relIndex.book(), relIndex.chapter(), 0, 0)].m_nNumWrd += nWordCount;
	}
	verse.m_nNumWrd += nWordCount;
	verse.m_lstWords.append(lstWords);
	verse.m_lstRichWords.append(lstRichWords);



//std::cout << m_pBibleDatabase->PassageReferenceText(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)).toUtf8().data() << "\n";
//std::cout << verse..m_strText.toUtf8().data() << "\n" << verse.m_strTemplate.toUtf8().data << "\n" << verse.m_lstWords.join(",").toUtf8().data() << "\n" << QString("Words: %1\n").arg(verse.m_nNumWrd).toUtf8().data();

	// The number of words in our template must match the word count:
	Q_ASSERT(static_cast<unsigned int>(verse.m_strTemplate.count('w')) == verse.m_nNumWrd);
	if (static_cast<unsigned int>(verse.m_strTemplate.count('w')) != verse.m_nNumWrd) {
		std::cerr << "\n" << m_pBibleDatabase->PassageReferenceText(CRelIndex(relIndex.book(), relIndex.chapter(), relIndex.verse(), 0)).toUtf8().data();
		std::cerr << "\n*** Error: Verse word count doesn't match template word count!!!\n";
	}


	relIndex.setVerse(0);
	relIndex.setWord(0);

	if (nVT == VT_VERSE) {
		m_bInVerse = false;
	} else if (nVT == VT_SUPERSCRIPTION) {
		m_bInSuperscription = false;
		m_bInBracketNotes = false;			// Don't carry bracketed footnotes across superscription boundary
	} else if (nVT == VT_COLOPHON) {
		m_bInColophon = false;
		m_bInBracketNotes = false;			// Don't carry bracketed footnotes across colophon boundary
	}
}

// ============================================================================
// ============================================================================

enum OUTPUT_ERROR_CODES_ENUM {
	ERR_CODE_NONE = 0,
	ERR_CODE_USAGE = -1,							// Usage/Command-line error
	ERR_CODE_BUILD_PATH = -2,						// Output path bad
	ERR_CODE_OSIS_FILE_MISSING = -3,				// OSIS Input File Open Failed
	ERR_CODE_INFO_FILE_MISSING = -4,				// Info Open Failed or Copy Failed
	ERR_CODE_OSIS_PARSE_FAILED = -5,				// OSIS Parse Failed
	ERR_CODE_DBINFO_FILE_WRITE_FAILED = -6,			// DBInfo File Write Failed
	ERR_CODE_TESTAMENT_FILE_WRITE_FAILED = -7,		// Testament File Write Failed
	ERR_CODE_BOOK_FILE_WRITE_FAILED = -8,			// Book (TOC) File Write Failed
	ERR_CODE_CHAPTER_FILE_WRITE_FAILED = -9,		// Chapter (Layout) File Write Failed
	ERR_CODE_VERSE_FILE_WRITE_FAILED = -10,			// Verse (Book) File Write Failed
	ERR_CODE_WORDS_FILE_WRITE_FAILED = -11,			// Words File Write Failed
	ERR_CODE_WORD_SUMMARY_FILE_WRITE_FAILED = -12,	// Word Summary File Write Failed
	ERR_CODE_FOOTNOTES_FILE_WRITE_FAILED = -13,		// Footnotes File Write Failed
	ERR_CODE_PHRASES_FILE_WRITE_FAILED = -14,		// Phrases File Write Failed
	ERR_CODE_LEMMAS_FILE_WRITE_FAILED = -15,		// Lemmas File Write Failed
	ERR_CODE_STRONGS_FILE_WRITE_FAILED = -16,		// Strongs File Write Failed
	ERR_CODE_VERSIFICATION_FILE_WRITE_FAILED = -17,	// Versification File Write Failed
};

// ============================================================================
// ============================================================================

static int writeDBInfoFile(const QDir &dirOutput, const TBibleDescriptor &bblDescriptor, const QString &strInfoFilename)
{
	QFileInfo fiInfoFile(strInfoFilename);
	if (!strInfoFilename.isEmpty()) {
		std::cerr << strInfoFilename.toUtf8().data() << "\n";
		if ((!fiInfoFile.exists()) || (!fiInfoFile.isFile())) {
			std::cerr << QString("\n\n*** Info Filename \"%1\" doesn't exist\n\n").arg(strInfoFilename).toUtf8().data();
			return ERR_CODE_INFO_FILE_MISSING;
		}
		if (!QFile::copy(fiInfoFile.absoluteFilePath(), dirOutput.absoluteFilePath(fiInfoFile.fileName()))) {
			std::cerr << QString("\n\n*** Failed to copy Info File from \"%1\" to \"%2\"\n\n")
						 .arg(fiInfoFile.absoluteFilePath())
						 .arg(dirOutput.absoluteFilePath(fiInfoFile.fileName()))
						 .toUtf8().data();
			return ERR_CODE_INFO_FILE_MISSING;
		}
	}

	std::cerr << "DBInfo.ini\n";
	QSettings settingsDBInfo(dirOutput.absoluteFilePath("DBInfo.ini"), QSettings::IniFormat);
#if QT_VERSION < 0x060000
	settingsDBInfo.setIniCodec("UTF-8");
#endif
	settingsDBInfo.clear();
	settingsDBInfo.beginGroup("BibleDBInfo");
	settingsDBInfo.setValue("Language", bblDescriptor.m_strLanguage);
	QString strDir;
	if (bblDescriptor.m_nTextDir == Qt::LeftToRight) {
		strDir = "ltr";
	} else if (bblDescriptor.m_nTextDir == Qt::RightToLeft) {
		strDir = "rtl";
	}	// Note: "Auto" stays as an empty string instead of using "auto"
	settingsDBInfo.setValue("Direction", strDir);
	settingsDBInfo.setValue("Name", bblDescriptor.m_strDBName);
	settingsDBInfo.setValue("Description", bblDescriptor.m_strDBDesc);
	settingsDBInfo.setValue("UUID", bblDescriptor.m_strUUID);
	settingsDBInfo.setValue("InfoFilename", (!strInfoFilename.isEmpty() ? fiInfoFile.fileName() : QString()));
	settingsDBInfo.setValue("Versification", CBibleVersifications::uuid(bblDescriptor.m_nMainVersification));
	settingsDBInfo.endGroup();

	if (settingsDBInfo.status() != QSettings::NoError) return ERR_CODE_DBINFO_FILE_WRITE_FAILED;

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writeTestamentsFile(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase)
{
	QFile fileTestaments;	// Testaments CSV being written

	fileTestaments.setFileName(dirOutput.absoluteFilePath("TESTAMENT.csv"));
	if (!fileTestaments.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Testament Output File \"%1\"\n").arg(fileTestaments.fileName()).toUtf8().data();
		return ERR_CODE_TESTAMENT_FILE_WRITE_FAILED;
	}

	CCSVStream csvFileTestaments(&fileTestaments);

	fileTestaments.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
	csvFileTestaments << QStringList({ "TstNdx", "TstName" });
	for (unsigned int nTst=1; nTst<=pBibleDatabase->bibleEntry().m_nNumTst; ++nTst) {
		csvFileTestaments << QStringList({
								QString("%1").arg(nTst),
								pBibleDatabase->testamentEntry(nTst)->m_strTstName
							});
	}
	std::cerr << QFileInfo(fileTestaments).fileName().toUtf8().data() << "\n";
	fileTestaments.close();

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

struct TWordListBaton
{
	// mapWordList will be for ALL forms of all words so that we can get mapping/counts
	//	for all unique forms of words.  Words in this map will NOT be indexed by the
	//	lowercase nor the base-form of the word, but by the actual word itself.
	//	Then, once we've used this to build all of the unique indexes for the different
	//	word forms, we'll consolidate them and ultimately build the database entries which will
	//	contain the wordlists by lowercase word and have the main word value and all
	//	alternates:
	TWordListMap mapWordList;				// mapWordList is indexed by the Word form as-is (no changes in case or cantillation)
	TAltWordListMap mapAltWordList;			// mapAltWordList is indexed by the LowerCase decantillated form of the Word
};

static int doBooksChaptersVerses(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase,
									TWordListBaton &wordListBaton,
									COSISXmlHandler &xmlHandler, bool bMissingOK,
									bool bWriteFiles)		// If bWriteFiles is false -- don't write files, just compute counts
{
	QFile fileBooks;		// Books CSV being written (Originally known as "TOC")
	CCSVStream csvFileBooks(&fileBooks);
	QFile fileChapters;		// Chapters CSV being written (Originally known as "Layout")
	CCSVStream csvFileChapters(&fileChapters);
	QFile fileVerses;		// Verses CSV being written (Originally known as "BOOKS")
	CCSVStream csvFileVerses(&fileVerses);

	if (bWriteFiles) {
		fileBooks.setFileName(dirOutput.absoluteFilePath("TOC.csv"));
		if (!fileBooks.open(QIODevice::WriteOnly)) {
			std::cerr << QString("\n\n*** Failed to open Books Output File \"%1\"\n").arg(fileBooks.fileName()).toUtf8().data();
			return ERR_CODE_BOOK_FILE_WRITE_FAILED;
		}

		fileBooks.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
		csvFileBooks << QStringList({ "BkNdx", "TstBkNdx", "TstNdx", "BkName", "BkAbbr", "TblName", "NumChp", "NumVrs", "NumWrd", "Cat", "Desc" });

		fileChapters.setFileName(dirOutput.absoluteFilePath("LAYOUT.csv"));
		if (!fileChapters.open(QIODevice::WriteOnly)) {
			std::cerr << QString("\n\n*** Failed to open Chapters Output File \"%1\"\n").arg(fileChapters.fileName()).toUtf8().data();
			return ERR_CODE_CHAPTER_FILE_WRITE_FAILED;
		}

		fileChapters.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
		csvFileChapters << QStringList({ "BkChpNdx", "NumVrs", "NumWrd", "BkAbbr", "ChNdx" });
	}

	TBibleChapterVerseCounts lstChapterVerseCounts;

	unsigned int nWordAccum = 0;
	for (unsigned int nBk=1; nBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		if (nBk > NUM_BK) {
			std::cerr << QString("\n*** WARNING: Module has extra Book : %1\n").arg(nBk).toUtf8().data();
			lstChapterVerseCounts.push_back(QStringList());
		} else {
			// Predefined books from our list:
			switch (pBibleDatabase->descriptor().m_nMainVersification) {
				case BVTE_KJV:
					lstChapterVerseCounts.push_back(CKJVBibleChapterVerseCounts::instance()->at(nBk-1));
					break;
				case BVTE_HEBREW_MASORETIC:
					lstChapterVerseCounts.push_back(CMTBibleChapterVerseCounts::instance()->at(nBk-1));
					break;
				case BVTE_SYNODAL:
					lstChapterVerseCounts.push_back(CSynodalBibleChapterVerseCounts::instance()->at(nBk-1));
					break;
				case BVTE_COUNT:
				default:
					Q_ASSERT(false);		// Invalid Versification Index
					lstChapterVerseCounts.push_back(QStringList());
					break;
			}
		}
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		bool bHadBook = true;
		if ((pBook == nullptr) || (pBook->m_strTblName.isEmpty())) {
			bHadBook = false;
			pBook = xmlHandler.addBookToBibleDatabase(nBk-1);
			std::cerr << QString("\n*** WARNING: Module is missing Book : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0, 0))).toUtf8().data();
		}
		(const_cast<CBookEntry*>(pBook))->m_nWrdAccum = nWordAccum;

		if (bWriteFiles) {
			fileVerses.setFileName(dirOutput.absoluteFilePath(QString("BOOK_%1_%2.csv").arg(nBk, 2, 10, QChar('0')).arg(pBook->m_strTblName)));
			if (!fileVerses.open(QIODevice::WriteOnly)) {
				std::cerr << QString("\n\n*** Failed to open Verses Output File \"%1\"\n").arg(fileVerses.fileName()).toUtf8().data();
				return ERR_CODE_VERSE_FILE_WRITE_FAILED;
			}

			fileVerses.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
			csvFileVerses << QStringList({ "ChpVrsNdx", "NumWrd", "nPilcrow", "PText", "RText", "TText" });

			std::cerr << QFileInfo(fileVerses).fileName().toUtf8().data();
		} else {
			std::cerr << pBook->m_strTblName.toUtf8().data();
		}

		unsigned int nChapterWordAccum = 0;
		unsigned int nChaptersExpected = qMax(pBook->m_nNumChp, static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size()));
		if (bMissingOK) nChaptersExpected = pBook->m_nNumChp;
		for (unsigned int nChp=(pBook->m_bHaveColophon ? 0 : 1); nChp<=nChaptersExpected; ++nChp) {
			if (!bMissingOK && (nChp != 0) && (nChp > static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size()))) {
				std::cerr << QString("\n*** WARNING: Module has extra Chapter : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toUtf8().data();
			}
			const CChapterEntry *pChapter = ((nChp != 0) ? pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0)) : nullptr);
			bool bChapterMissing = false;
			Q_UNUSED(bChapterMissing);
			if ((nChp != 0) && (pChapter == nullptr)) {
				bChapterMissing = true;
				if ((nChp >= bookStartingIndex(bookIndexFromOSISAbbr(pBibleDatabase->bookOSISAbbr(CRelIndex(nBk, 0, 0, 0)))).chapter()) && (bHadBook)) {
					std::cerr << QString("\n*** WARNING: Module is missing Chapter : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toUtf8().data();
				}
				pChapter = pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0), true);
				if (pChapter == nullptr) {
					std::cerr << "*** Unable to create missing chapter\n";
					continue;
				} else {
					(const_cast<CBookEntry*>(pBook))->m_nNumChp++;
				}
			}
			if (pChapter != nullptr) {
				(const_cast<CChapterEntry*>(pChapter))->m_nWrdAccum = nWordAccum;

				std::cerr << ".";
			}

//			std::cout << QString("%1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0))).toUtf8().data();
			unsigned int nVerseWordAccum = 0;
			unsigned int nVersesExpected = ((pChapter != nullptr) ? qMax(pChapter->m_nNumVrs, static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0)) : 0);
			if (bMissingOK) nVersesExpected = pChapter->m_nNumVrs;

			// Remove empty non-canonical verses that got added posthumous during parsing
			//	that are trailing at the end of the chapter (leave any in the middle of
			//	the chapter, as they are placeholders):
			if ((nChp > 0) && !bMissingOK) {		// Do this only for non-colophons
				for (unsigned int nVrs=nVersesExpected; nVrs > static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0); --nVrs) {
					const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
					if ((pVerse != nullptr) && (pVerse->m_nNumWrd == 0) && (pVerse->m_strTemplate.trimmed().isEmpty())) {
						// Ideally, we would also delete the verse here from pBibleDatabase->m_lstBookVerses,
						//	but that's a private member, and since we are using the counts to output the
						//	verses below, rather than the extents of that member, we can simply decrement
						//	the counts and it will be correctly removed when writing:
						std::cerr << QString("\n*** Removing empty extra verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
						(const_cast<CBookEntry*>(pBook))->m_nNumVrs--;
						(const_cast<CChapterEntry*>(pChapter))->m_nNumVrs--;
						nVersesExpected--;		// This count is used for the loop below, so it needs decrementing too
					} else {
						break;		// Stop when we find the first one that isn't empty as any other empties are needed as placeholders
					}
				}
			}

			for (unsigned int nVrs=((pChapter != nullptr) ? (pChapter->m_bHaveSuperscription ? 0 : 1) : 0); nVrs<=nVersesExpected; ++nVrs) {
				if (!bMissingOK && (nVrs != 0) && (nVrs > static_cast<unsigned int>((nChp <= static_cast<unsigned int>(lstChapterVerseCounts.at(nBk-1).size())) ? lstChapterVerseCounts.at(nBk-1).at(nChp-1).toUInt() : 0))) {
					std::cerr << QString("\n*** WARNING: Module has extra Verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
				}
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				bool bVerseMissing = false;
				if (pVerse == nullptr) {
					if ((nChp == 0) || (nVrs == 0)) Q_ASSERT(false);
					bVerseMissing = true;
					if ((CRelIndex(nBk, nChp, nVrs, 0) >= bookStartingIndex(bookIndexFromOSISAbbr(pBibleDatabase->bookOSISAbbr(CRelIndex(nBk, 0, 0, 0)))))
						&& (bHadBook)) {
						std::cerr << QString("\n*** WARNING: Module is missing Verse : %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
					}
					pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0), true);
					if (pVerse == nullptr) {
						std::cerr << "*** Unable to create missing verse\n";
						continue;
					} else {
						(const_cast<CBookEntry*>(pBook))->m_nNumVrs++;
						(const_cast<CChapterEntry*>(pChapter))->m_nNumVrs++;
					}
				}
//				std::cout << QString("%1 : \"%2\"\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).arg(pVerse->m_strTemplate).toUtf8().data();

				(const_cast<CVerseEntry*>(pVerse))->m_nWrdAccum = nWordAccum;
				nVerseWordAccum += pVerse->m_nNumWrd;
				nWordAccum += pVerse->m_nNumWrd;

				if (pVerse->m_nNumWrd > 0) {
					uint32_t nNormal = pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, 1));
					if (nNormal != pVerse->m_nWrdAccum+1) {
						std::cerr << QString("\n**** Error: Normal for CRelIndex(%1, %2, %3, 1)->%4 != %5\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nNormal).arg(pVerse->m_nWrdAccum+1).toUtf8().data();
						Q_ASSERT(nNormal == (pVerse->m_nWrdAccum+1));
					}
					CRelIndex ndxDenormal = pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum+1);
					if (ndxDenormal != CRelIndex(nBk, nChp, nVrs, 1)) {
						std::cerr << QString("\n*** Error: Denormal for %1  !=  CRelIndex(%2, %3, %4, 1)->%5\n")
									 .arg(ndxDenormal.index()).arg(nBk).arg(nChp).arg(nVrs).arg(CRelIndex(nBk, nChp, nVrs, 1).index()).toUtf8().data();
						Q_ASSERT(ndxDenormal == CRelIndex(nBk, nChp, nVrs, 1));
					}
				} else {
					if (!bVerseMissing &&
						(CRelIndex(nBk, nChp, nVrs, 0) >= bookStartingIndex(bookIndexFromOSISAbbr(pBibleDatabase->bookOSISAbbr(CRelIndex(nBk, 0, 0, 0)))))) {
						if (nVrs == 0) {
							if (nChp == 0) {
								std::cerr << QString("\n*** Warning: Colophon has no text: %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
							} else {
								std::cerr << QString("\n*** Warning: Superscription has no text: %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
							}
						} else {
							std::cerr << QString("\n*** Warning: Verse has no text: %1\n").arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0))).toUtf8().data();
						}
					}
				}

				if (bWriteFiles) {
					// ChpVrsNdx,NumWrd,nPilcrow,PText,RText,TText
					csvFileVerses << QStringList({
									QString("%1").arg(CRelIndex(0,0,nChp,nVrs).index()),	// 1
									QString("%1").arg(pVerse->m_nNumWrd),					// 2
									QString("%1").arg(pVerse->m_nPilcrow),					// 3
									QString(),												// 4	(PlainText)
									QString(),												// 5	(RichText)
									pVerse->m_strTemplate.trimmed()							// 6	(TemplateText)
							});
				}


				// Now use the words we've gathered from this verse to build the Word Lists and Concordance:
				Q_ASSERT(pVerse->m_nNumWrd == static_cast<unsigned int>(pVerse->m_lstWords.size()));
				Q_ASSERT(pVerse->m_nNumWrd == static_cast<unsigned int>(pVerse->m_lstRichWords.size()));
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					//QString strWord = pVerse->m_lstWords.at(nWrd-1);
					QString strRichWord = pVerse->m_lstRichWords.at(nWrd-1);
					QString strDeCantillatedWord = StringParse::deCantillate(strRichWord);
					CWordEntry &wordEntry = wordListBaton.mapWordList[strRichWord];
					TWordListSet &wordSet = wordListBaton.mapAltWordList[strDeCantillatedWord.toLower()];
					wordSet.insert(strRichWord);
					wordEntry.m_ndxNormalizedMapping.push_back(pVerse->m_nWrdAccum+nWrd);
				}

				if (pVerse->m_nNumWrd > CRelIndex::maxWordCount()) {
					std::cerr << QString("\n*** Warning: Verse word count (%1) exceeds maximum allowed (%2) : ").arg(pVerse->m_nNumWrd).arg(CRelIndex::maxWordCount()).toUtf8().data()
							  << pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, nVrs, 0)).toUtf8().data() << "\n";
				}
			}
			if (pChapter != nullptr) {
				if (nVerseWordAccum != pChapter->m_nNumWrd) {
					std::cerr << QString("\n*** Error: %1 Chapter Word Count (%2) doesn't match sum of Verse Word Counts (%3)!\n")
													.arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0)))
													.arg(pChapter->m_nNumWrd)
													.arg(nVerseWordAccum)
													.toUtf8().data();
				}
				nChapterWordAccum += pChapter->m_nNumWrd;

				if (pChapter->m_nNumVrs > CRelIndex::maxVerseCount()) {
					std::cerr << QString("\n*** Warning: Chapter verse count (%1) exceeds maximum allowed (%2) : ").arg(pChapter->m_nNumVrs).arg(CRelIndex::maxVerseCount()).toUtf8().data()
							  << pBibleDatabase->PassageReferenceText(CRelIndex(nBk, nChp, 0, 0)).toUtf8().data() << "\n";
				}
			} else {
				nChapterWordAccum += nVerseWordAccum;		// Book has words of Colophons that aren't part of chapters proper
			}

			if (bWriteFiles) {
				if (pChapter != nullptr) {
					// BkChpNdx,NumVrs,NumWrd,BkAbbr,ChNdx
					csvFileChapters << QStringList({
									QString("%1").arg(CRelIndex(0,0,nBk,nChp).index()),		// 1
									QString("%1").arg(pChapter->m_nNumVrs),					// 2
									QString("%1").arg(pChapter->m_nNumWrd),					// 3
									pBook->m_lstBkAbbr.at(0),								// 4 -- OSIS Abbr Only!
									QString("%1").arg(nChp)									// 5
							});
				}
			}
		}
		if (nChapterWordAccum != pBook->m_nNumWrd) {
			std::cerr << QString("\n*** Error: %1 Book Word Count (%2) doesn't match sum of Chapter Word Counts (%3)!\n")
												.arg(pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0 ,0)))
												.arg(pBook->m_nNumWrd)
												.arg(nChapterWordAccum)
												.toUtf8().data();
		}
		if (pBook->m_nNumChp > CRelIndex::maxChapterCount()) {
			std::cerr << QString("\n*** Warning: Book chapter count (%1) exceeds maximum allowed (%2) : ").arg(pBook->m_nNumChp).arg(CRelIndex::maxChapterCount()).toUtf8().data()
					  << pBibleDatabase->PassageReferenceText(CRelIndex(nBk, 0, 0, 0)).toUtf8().data() << "\n";
		}

		if (bWriteFiles) {
			fileVerses.close();

			// BkNdx,TstBkNdx,TstNdx,BkName,BkAbbr,TblName,NumChp,NumVrs,NumWrd,Cat,Desc
			csvFileBooks << QStringList({
									QString("%1").arg(nBk),							// 1
									QString("%1").arg(pBook->m_nTstBkNdx),			// 2
									QString("%1").arg(pBook->m_nTstNdx),			// 3
									pBook->m_strBkName,								// 4
									pBook->m_lstBkAbbr.join(";"),					// 5
									pBook->m_strTblName,							// 6
									QString("%1").arg(pBook->m_nNumChp),			// 7
									QString("%1").arg(pBook->m_nNumVrs),			// 8
									QString("%1").arg(pBook->m_nNumWrd),			// 9
									pBibleDatabase->bookCategoryName(CRelIndex(nBk, 0, 0, 0)),	// 10 - Note: Category is deprecated, but write it for compatibility with creating databases for old KJPBS versions
									pBook->m_strDesc								// 11
							});
		}


		std::cerr << "\n";
	}
	if (pBibleDatabase->bibleEntry().m_nNumBk > CRelIndex::maxBookCount()) {
		std::cerr << QString("\n*** Warning: Total book count (%1) exceeds maximum allowed (%2)!\n").arg(pBibleDatabase->bibleEntry().m_nNumBk).arg(CRelIndex::maxBookCount()).toUtf8().data();
	}

	if (bWriteFiles) {
		std::cerr << QFileInfo(fileChapters).fileName().toUtf8().data() << "\n";
		fileChapters.close();

		std::cerr << QFileInfo(fileBooks).fileName().toUtf8().data() << "\n";
		fileBooks.close();
	}

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writeWordsFiles(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase,
									TWordListBaton &wordListBaton)
{
	QFile fileWords;		// Words CSV being written

	fileWords.setFileName(dirOutput.absoluteFilePath("WORDS.csv"));
	if (!fileWords.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Words Output File \"%1\"\n").arg(fileWords.fileName()).toUtf8().data();
		return ERR_CODE_WORDS_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(fileWords).fileName().toUtf8().data();

	CCSVStream csvFileWords(&fileWords);

	fileWords.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	csvFileWords << QStringList({ "WrdNdx", "Word", "bIndexCasePreserve", "NumTotal", "AltWords", "AltWordCounts", "NormalMap" });

	unsigned int nWordIndex = 0;

	// We've now built a list of word indexes and alternate forms, and now we
	//	need to take this list and convert it to the form of the database:
	TWordListMap &mapDbWordList = const_cast<TWordListMap &>(pBibleDatabase->mapWordList());
	for (TAltWordListMap::const_iterator itrUniqWrd = wordListBaton.mapAltWordList.begin(); itrUniqWrd != wordListBaton.mapAltWordList.end(); ++itrUniqWrd) {
		const TWordListSet &setAltWords = itrUniqWrd->second;
		CWordEntry &wordEntryDb = mapDbWordList[itrUniqWrd->first];
		for (TWordListSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
			TWordListMap::const_iterator itrWrd = wordListBaton.mapWordList.find(*itrAltWrd);
			if (itrWrd == wordListBaton.mapWordList.end()) {
				std::cerr << QString("\n*** Error: %1 -> %2 -- Couldn't Find it (something bad happened!)\n").arg(itrUniqWrd->first).arg(*itrAltWrd).toUtf8().data();
				continue;
			}
			wordEntryDb.m_lstAltWords.push_back(*itrAltWrd);
			wordEntryDb.m_lstAltWordCount.push_back(itrWrd->second.m_ndxNormalizedMapping.size());
			wordEntryDb.m_ndxNormalizedMapping.insert(wordEntryDb.m_ndxNormalizedMapping.end(), itrWrd->second.m_ndxNormalizedMapping.begin(), itrWrd->second.m_ndxNormalizedMapping.end());
			wordEntryDb.m_strWord = WordFromWordSet(setAltWords);
		}
		wordEntryDb.m_bCasePreserve = isSpecialWord(pBibleDatabase, wordEntryDb);
		wordEntryDb.m_bIsProperWord = isProperWord(pBibleDatabase, wordEntryDb);

		Q_ASSERT(wordEntryDb.m_lstAltWords.size() == wordEntryDb.m_lstAltWordCount.size());
		Q_ASSERT(wordEntryDb.m_lstAltWords.size() > 0);

		if ((nWordIndex % 100) == 0) std::cerr << ".";

		// WrdNdx,Word,bIndexCasePreserve,NumTotal,AltWords,AltWordCounts,NormalMap

		nWordIndex++;
		int nSpecFlags = (wordEntryDb.m_bCasePreserve ? 1 :0) + (wordEntryDb.m_bIsProperWord ? 2 :0);		// Setup sepcial bit-flags field
		QStringList lstAltWordCounts;
		for (int i=0; i<wordEntryDb.m_lstAltWordCount.size(); ++i) {
			lstAltWordCounts.append(QString("%1").arg(wordEntryDb.m_lstAltWordCount.at(i)));
		}
		QStringList lstNormalMapping;
		for (unsigned int i=0; i<wordEntryDb.m_ndxNormalizedMapping.size(); ++i) {
			lstNormalMapping.append(QString("%1").arg(wordEntryDb.m_ndxNormalizedMapping.at(i)));
		}
		csvFileWords << QStringList({
									QString("%1").arg(nWordIndex),		// 1 : WrdNdx
									wordEntryDb.m_strWord,				// 2 : Word
									QString("%1").arg(nSpecFlags),		// 3 : SpecFlags [bIndexCasePreserve]
									QString("%1").arg(wordEntryDb.m_ndxNormalizedMapping.size()),	// 4 : NumTotal
									wordEntryDb.m_lstAltWords.join(","),	// 5 : AltWords
									lstAltWordCounts.join(","),			// 6 : AltWordCounts
									lstNormalMapping.join(",")			// 7 : NormalMap
							});
	}

	fileWords.close();
	std::cerr << "\n";

	// ------------------------------------------------------------------------

	QFile fileWordSummary;	// Words Summary CSV being written

	fileWordSummary.setFileName(dirOutput.absoluteFilePath("WORDS_summary.csv"));
	if (!fileWordSummary.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Words Summary Output File \"%1\"\n").arg(fileWordSummary.fileName()).toUtf8().data();
		return ERR_CODE_WORD_SUMMARY_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(fileWordSummary).fileName().toUtf8().data();

	CCSVStream csvFileWordSummary(&fileWordSummary);

	unsigned int nTotalWordCount = 0;
	unsigned int arrTotalTestamentWordCounts[NUM_TST];
	memset(arrTotalTestamentWordCounts, 0, sizeof(arrTotalTestamentWordCounts));
	unsigned int arrTotalBookWordCounts[NUM_BK];
	memset(arrTotalBookWordCounts, 0, sizeof(arrTotalBookWordCounts));

	bool bHaveApoc = (pBibleDatabase->bibleEntry().m_nNumBk > (NUM_BK_OT_NT));
	unsigned int nRepTst = (bHaveApoc ? NUM_TST : (NUM_TST-1));
	unsigned int nRepBk = (bHaveApoc ? NUM_BK : (NUM_BK_OT_NT));

	fileWordSummary.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	QStringList lstSummaryLine({ "Word", "AltWords", "Entire\nBible" });
	for (unsigned int nTst=0; nTst<nRepTst; ++nTst) {
		QString strTemp = CBibleTestaments::name(nTst+1);
		strTemp.replace(' ', '\n');
		lstSummaryLine.append(strTemp);
	}
	for (unsigned int nBk=0; nBk<nRepBk; ++nBk) {
		lstSummaryLine.append(g_arrBibleBooks.at(nBk).m_strName);
	}
	csvFileWordSummary << lstSummaryLine;

	nWordIndex = 0;

// Use previously defined mapDbWordList:
//	TWordListMap &mapDbWordList = const_cast<TWordListMap &>(pBibleDatabase->mapWordList());
	for (TAltWordListMap::const_iterator itrUniqWrd = wordListBaton.mapAltWordList.begin(); itrUniqWrd != wordListBaton.mapAltWordList.end(); ++itrUniqWrd) {
		lstSummaryLine.clear();
		const TWordListSet &setAltWords = itrUniqWrd->second;
		CWordEntry &wordEntryDb = mapDbWordList[itrUniqWrd->first];
		QString strAltWords;
		for (TWordListSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
			if (!strAltWords.isEmpty()) strAltWords += ",";
			strAltWords += *itrAltWrd;
		}

		lstSummaryLine.append(wordEntryDb.m_strWord);
		lstSummaryLine.append(strAltWords);
		lstSummaryLine.append(QString("%1").arg(wordEntryDb.m_ndxNormalizedMapping.size()));

		Q_ASSERT(wordEntryDb.m_lstAltWords.size() == wordEntryDb.m_lstAltWordCount.size());
		Q_ASSERT(wordEntryDb.m_lstAltWords.size() > 0);

		if ((nWordIndex % 100) == 0) std::cerr << ".";
		nWordIndex++;

		unsigned int arrTestamentWordCounts[NUM_TST] = { };
		unsigned int arrBookWordCounts[NUM_BK] = { };

		for (TNormalizedIndexList::const_iterator itr = wordEntryDb.m_ndxNormalizedMapping.begin(); itr != wordEntryDb.m_ndxNormalizedMapping.end(); ++itr) {
			CRelIndex ndx(pBibleDatabase->DenormalizeIndex(*itr));
			Q_ASSERT(ndx.isSet());
			Q_ASSERT(ndx.book() != 0);
			if (ndx.book() <= NUM_BK_OT) {
				arrTestamentWordCounts[0]++;
				arrTotalTestamentWordCounts[0]++;
			} else if (ndx.book() <= (NUM_BK_OT + NUM_BK_NT)) {
				arrTestamentWordCounts[1]++;
				arrTotalTestamentWordCounts[1]++;
			} else if (ndx.book() <= (NUM_BK_OT + NUM_BK_NT + NUM_BK_APOC)) {
				arrTestamentWordCounts[2]++;
				arrTotalTestamentWordCounts[2]++;
			} else {
				// Word in unknown Testament -- assert here??
			}
			if (ndx.book() <= NUM_BK) {
				arrBookWordCounts[ndx.book()-1]++;
				arrTotalBookWordCounts[ndx.book()-1]++;
			} else {
				// Word in unknwon Book -- assert here??
			}
			nTotalWordCount++;
		}

		for (unsigned int nTst=0; nTst<nRepTst; ++nTst) {
			lstSummaryLine.append(QString("%1").arg(arrTestamentWordCounts[nTst]));
		}
		for (unsigned int nBk=0; nBk<nRepBk; ++nBk) {
			lstSummaryLine.append(QString("%1").arg(arrBookWordCounts[nBk]));
		}

		csvFileWordSummary << lstSummaryLine;
	}
	lstSummaryLine.clear();
	lstSummaryLine.append(QString());
	lstSummaryLine.append(QString());
	lstSummaryLine.append(QString("%1").arg(nTotalWordCount));
	for (unsigned int nTst=0; nTst<nRepTst; ++nTst) {
		lstSummaryLine.append(QString("%1").arg(arrTotalTestamentWordCounts[nTst]));
	}
	for (unsigned int nBk=0; nBk<nRepBk; ++nBk) {
		lstSummaryLine.append(QString("%1").arg(arrTotalBookWordCounts[nBk]));
	}
	csvFileWordSummary << lstSummaryLine;

	fileWordSummary.close();
	std::cerr << "\n";

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writeFootnotesFile(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase)
{
	QFile fileFootnotes;	// Footnotes CSV being written
	unsigned int nFootnoteIndex = 0;

	fileFootnotes.setFileName(dirOutput.absoluteFilePath("FOOTNOTES.csv"));
	if (!fileFootnotes.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Footnotes Output File \"%1\"\n").arg(fileFootnotes.fileName()).toUtf8().data();
		return ERR_CODE_FOOTNOTES_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(fileFootnotes).fileName().toUtf8().data();

	CCSVStream csvFileFootnotes(&fileFootnotes);

	fileFootnotes.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	csvFileFootnotes << QStringList({ "BkChpVrsWrdNdx", "PFootnote", "RFootnote" });

	const TFootnoteEntryMap &mapFootnotes = pBibleDatabase->footnotesMap();
	for (TFootnoteEntryMap::const_iterator itrFootnotes = mapFootnotes.begin(); itrFootnotes != mapFootnotes.end(); ++itrFootnotes) {
		QStringList lstTempFootnote = (itrFootnotes->second).text().split('\"');
		QString strTempFootnote = lstTempFootnote.join("\"\"");
		// BkChpVrsWrdNdx,PFootnote,RFootnote
		csvFileFootnotes << QStringList({
								QString("%1").arg((itrFootnotes->first).index()),		// 1
								strTempFootnote,										// 2			-- TODO : FIX
								strTempFootnote											// 3			-- TODO : FIX
						});

		if ((nFootnoteIndex % 100) == 0) std::cerr << ".";
		nFootnoteIndex++;
	}

	fileFootnotes.close();
	std::cerr << "\n";

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writePhrasesFile(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase)
{
	Q_UNUSED(pBibleDatabase);

	QFile filePhrases;		// Default search phrases CSV being written (deprecated)

	// Phrases are somewhat deprecated.  Write an empty PHRASES file so that the
	//	KJPBS build will succeed.  The person doing the build can always override
	//	it with a meaningful phrases file.
	filePhrases.setFileName(dirOutput.absoluteFilePath("PHRASES.csv"));
	if (!filePhrases.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Phrases Output File \"%1\"\n").arg(filePhrases.fileName()).toUtf8().data();
		return ERR_CODE_PHRASES_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(filePhrases).fileName().toUtf8().data();

	CCSVStream csvFilePhrases(&filePhrases);

	filePhrases.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	csvFilePhrases << QStringList({ "Ndx", "Phrase", "CaseSensitive", "AccentSensitive", "Exclude" });

	filePhrases.close();
	std::cerr << "\n";

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writeLemmasFile(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase)
{
	QFile fileLemmas;		// Lemma list being written

	// Write Lemmas:
	fileLemmas.setFileName(dirOutput.absoluteFilePath("LEMMAS.csv"));
	if (!fileLemmas.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Lemma Output File \"%1\"\n").arg(fileLemmas.fileName()).toUtf8().data();
		return ERR_CODE_LEMMAS_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(fileLemmas).fileName().toUtf8().data();

	CCSVStream csvFileLemmas(&fileLemmas);

	fileLemmas.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM
	csvFileLemmas << QStringList({ "BkChpVrsWrdNdx", "Count", "Attrs" });

	unsigned int nLemmaBk = 0;
	const TLemmaEntryMap &mapLemmas = pBibleDatabase->lemmaMap();
	for (TLemmaEntryMap::const_iterator itrLemmas = mapLemmas.cbegin();
			itrLemmas != mapLemmas.cend(); ++itrLemmas) {
		if (nLemmaBk != itrLemmas->second.tag().relIndex().book()) {
			nLemmaBk = itrLemmas->second.tag().relIndex().book();
			std::cerr << ".";
		}
		QStringList lstTempLemma = (itrLemmas->second).lemmaAttrs().split('\"');
		QString strTempLemma = lstTempLemma.join("\"\"");
		// Ndx,Count,Attrs
		csvFileLemmas << QStringList({
							QString("%1").arg(itrLemmas->second.tag().relIndex().index()),
							QString("%1").arg(itrLemmas->second.tag().count()),
							strTempLemma
						});
	}

	fileLemmas.close();
	std::cerr << "\n";

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writeStrongs(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase)
{
	QFile fileStrongs;		// Strongs database list being written

	// Write Strongs Database:
	fileStrongs.setFileName(dirOutput.absoluteFilePath("STRONGS.csv"));
	if (!fileStrongs.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Strongs Output File \"%1\"\n").arg(fileStrongs.fileName()).toUtf8().data();
		return ERR_CODE_STRONGS_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(fileStrongs).fileName().toUtf8().data();

	fileStrongs.write(QString(QChar(0xFEFF)).toUtf8());		// UTF-8 BOM

	CCSVStream csvFileStrongs(&fileStrongs);
	csvFileStrongs << QStringList({ "StrongsMapNdx", "Orth", "Trans", "Pron", "Def" });

	int nStrongsProgress = 0;
	for (TStrongsIndexMap::const_iterator itrStrongs = pBibleDatabase->strongsIndexMap().cbegin();
			itrStrongs != pBibleDatabase->strongsIndexMap().cend();
			++itrStrongs) {
		csvFileStrongs << QStringList({ itrStrongs->second.strongsMapIndex(),
										itrStrongs->second.orthography(),
										itrStrongs->second.transliteration(),
										itrStrongs->second.pronunciation(),
										itrStrongs->second.definition() });
		if ((nStrongsProgress % 100) == 0) std::cerr << ".";
		++nStrongsProgress;
	}

	fileStrongs.close();
	std::cerr << "\n";

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

static int writeVersification(const QDir &dirOutput, const CBibleDatabase *pBibleDatabase, BIBLE_VERSIFICATION_TYPE_ENUM nV11nType)
{
	QFile fileVersification;	// Versification CSV being written (Will be appended to allow multiple passes)

	fileVersification.setFileName(dirOutput.absoluteFilePath("VERSIFICATION.csv"));
	bool bExists = fileVersification.exists();
	if (!fileVersification.open(QIODevice::WriteOnly | QIODevice::Append)) {
		std::cerr << QString("\n\n*** Failed to open Versification Output File \"%1\"\n").arg(fileVersification.fileName()).toUtf8().data();
		return ERR_CODE_VERSIFICATION_FILE_WRITE_FAILED;
	}
	std::cerr << QFileInfo(fileVersification).fileName().toUtf8().data();

	if (!bExists) {
		fileVersification.write(QString(QChar(0xFEFF)).toUtf8());			// UTF-8 BOM
	}

	QString strFileVersification;
	CCSVStream csvFileVersification(&strFileVersification, QIODevice::WriteOnly);
	unsigned int nEntryCount = 0;

	// Don't write the CSV headers so we can have running concatenated versification tables
	//csvFileVersification << QStringList({ "BkChpVrsNdx", "BkAbbr", "NumChp", "NumVrs", "NumWrd", "nPilcrow", "TText" });

	for (unsigned int nBk = 1; nBk <= pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		Q_ASSERT(pBook != nullptr);

		if (!pBook->m_bHaveColophon) {
			// If there's no Colophon, write the book entry here.
			//	Otherwise, it will be written with a colophon below:

			CRelIndex ndxCurrent(nBk, 0, 0, 0);
			csvFileVersification << QStringList({
					QString("%1").arg(ndxCurrent.index()),			// 1 : BkChpVrsNdx
					pBook->m_lstBkAbbr.at(0),						// 2 : BkAbbr (OSIS)
					QString("%1").arg(pBook->m_nNumChp),			// 3 : NumChp
					QString("%1").arg(pBook->m_nNumVrs),			// 4 : NumVrs
					QString("%1").arg(pBook->m_nNumWrd),			// 5 : NumWrd
					"0",											// 6 : nPilcrow
					""												// 7 : TText
				});
			++nEntryCount;
		}

		for (unsigned int nChp=(pBook->m_bHaveColophon ? 0 : 1); nChp<=pBook->m_nNumChp; ++nChp) {
			const CChapterEntry *pChapter = ((nChp != 0) ? pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0)) : nullptr);
			unsigned int nStartVerse = 1;
			if (nChp != 0) {
				Q_ASSERT(pChapter != nullptr);
				nStartVerse = (pChapter->m_bHaveSuperscription ? 0 : 1);
			} else {
				nStartVerse = 0;
			}

			// Note: pChapter will be nullptr below when writing colophon:

			if ((pChapter != nullptr) && (!pChapter->m_bHaveSuperscription)) {
				// If there's no Superscription, write the chapter entry here.
				//	Otherwise, it will be written with a superscription below:

				CRelIndex ndxCurrent(nBk, nChp, 0, 0);
				csvFileVersification << QStringList({
						QString("%1").arg(ndxCurrent.index()),		// 1 : BkChpVrsNdx
						pBook->m_lstBkAbbr.at(0),					// 2 : BkAbbr (OSIS)
						QString("%1").arg(nChp),					// 3 : NumChp
						QString("%1").arg(pChapter->m_nNumVrs),		// 4 : NumVrs
						QString("%1").arg(pChapter->m_nNumWrd),		// 5 : NumWrd
						"0",										// 6 : nPilcrow
						""											// 7 : TText
					});
				++nEntryCount;
			}

			for (unsigned int nVrs=nStartVerse; nVrs<= ((pChapter != nullptr) ? pChapter->m_nNumVrs : 0); ++nVrs) {
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				Q_ASSERT(pVerse != nullptr);

				// Round-trip Index Check: (could be disabled/removed)
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					uint32_t nNormalAccum = pBibleDatabase->NormalizeIndex(CRelIndex(nBk, nChp, nVrs, nWrd));
					uint32_t nNormalNoAccum = pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, nWrd));
					if (nNormalAccum != nNormalNoAccum) {
						std::cerr << QString("\n*** Error: CRelIndex(%1, %2, %3, %4) : NormalAccum->%5 != NormalNoAccum->%6\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(nNormalAccum).arg(nNormalNoAccum).toUtf8().data();
						Q_ASSERT(nNormalAccum == nNormalNoAccum);
					}
					CRelIndex ndxDenormalAccum = pBibleDatabase->DenormalizeIndex(pVerse->m_nWrdAccum+nWrd);
					CRelIndex ndxDenormalNoAccum = pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum+nWrd);
					if (ndxDenormalAccum != ndxDenormalNoAccum) {
						std::cerr << QString("\n*** Error: CRelIndex(%1, %2, %3, %4)->Accum:%5  DenormalAccum->%6 != DenormalNoAccum=%7\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(pVerse->m_nWrdAccum+nWrd).arg(ndxDenormalAccum.index()).arg(ndxDenormalNoAccum.index()).toUtf8().data();
						Q_ASSERT(ndxDenormalAccum == ndxDenormalNoAccum);
					}
					CRelIndex ndxTest = CRelIndex(nBk, nChp, nVrs, nWrd);
					uint32_t nNormal = pBibleDatabase->NormalizeIndex(ndxTest);
					CRelIndex ndxDenormal = pBibleDatabase->DenormalizeIndex(nNormal);
					if (ndxDenormal != ndxTest) {
						std::cerr << QString("\n*** Error: Roundtrip : CRelIndex(%1, %2, %3, %4)=%5 -> Normal=%6 -> Denormal=%7\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(ndxTest.index()).arg(nNormal).arg(ndxDenormal.index()).toUtf8().data();
						Q_ASSERT(ndxDenormal == ndxTest);
					}
				}

				CRelIndex ndxCurrent(nBk, nChp, nVrs, 0);
				csvFileVersification << QStringList({
						QString("%1").arg(ndxCurrent.index()),					// 1 : BkChpVrsNdx
						pBook->m_lstBkAbbr.at(0),								// 2 : BkAbbr (OSIS)
						QString("%1").arg(nChp ? nChp : pBook->m_nNumChp),		// 3 : NumChp
						QString("%1").arg(nVrs ? nVrs : (nChp ? pChapter->m_nNumVrs : pBook->m_nNumVrs)),	// 4 : NumVrs
						QString("%1").arg(nVrs ? pVerse->m_nNumWrd : (nChp ? pChapter->m_nNumWrd : pBook->m_nNumWrd)),	// 5 : NumWrd
						QString("%1").arg(pVerse->m_nPilcrow),					// 6 : nPilcrow
						pVerse->m_strTemplate.trimmed()							// 7 : TText
					});
				++nEntryCount;
			}
		}

		std::cerr << ".";
	}
	std::cerr << "\n";

	fileVersification.write(QString("VERSIFICATION,%1,%2\n").arg(nEntryCount).arg(CBibleVersifications::uuid(nV11nType)).toUtf8());
	fileVersification.write(strFileVersification.toUtf8());
	fileVersification.close();

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

int checkIndexes(const CBibleDatabase *pBibleDatabase)		// Not static to avoid unused warning when CHECK_INDEXES is disabled
{
	std::cerr << "Checking Indexes";
	for (unsigned int nBk=1; nBk<=pBibleDatabase->bibleEntry().m_nNumBk; ++nBk) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(nBk);
		Q_ASSERT(pBook != nullptr);
		for (unsigned int nChp=(pBook->m_bHaveColophon ? 0 : 1); nChp<=pBook->m_nNumChp; ++nChp) {
			const CChapterEntry *pChapter = ((nChp != 0) ? pBibleDatabase->chapterEntry(CRelIndex(nBk, nChp, 0, 0)) : nullptr);
			unsigned int nStartVerse = 1;
			if (nChp != 0) {
				Q_ASSERT(pChapter != nullptr);
				nStartVerse = (pChapter->m_bHaveSuperscription ? 0 : 1);
			} else {
				nStartVerse = 0;
			}

			for (unsigned int nVrs=nStartVerse; nVrs<= ((pChapter != nullptr) ? pChapter->m_nNumVrs : 0); ++nVrs) {
				const CVerseEntry *pVerse = pBibleDatabase->verseEntry(CRelIndex(nBk, nChp, nVrs, 0));
				Q_ASSERT(pVerse != nullptr);
				for (unsigned int nWrd=1; nWrd<=pVerse->m_nNumWrd; ++nWrd) {
					uint32_t nNormalAccum = pBibleDatabase->NormalizeIndex(CRelIndex(nBk, nChp, nVrs, nWrd));
					uint32_t nNormalNoAccum = pBibleDatabase->NormalizeIndexNoAccum(CRelIndex(nBk, nChp, nVrs, nWrd));
					if (nNormalAccum != nNormalNoAccum) {
						std::cerr << QString("\n*** Error: CRelIndex(%1, %2, %3, %4) : NormalAccum->%5 != NormalNoAccum->%6\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(nNormalAccum).arg(nNormalNoAccum).toUtf8().data();
						Q_ASSERT(nNormalAccum == nNormalNoAccum);
					}
					CRelIndex ndxDenormalAccum = pBibleDatabase->DenormalizeIndex(pVerse->m_nWrdAccum+nWrd);
					CRelIndex ndxDenormalNoAccum = pBibleDatabase->DenormalizeIndexNoAccum(pVerse->m_nWrdAccum+nWrd);
					if (ndxDenormalAccum != ndxDenormalNoAccum) {
						std::cerr << QString("\n*** Error: CRelIndex(%1, %2, %3, %4)->Accum:%5  DenormalAccum->%6 != DenormalNoAccum=%7\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(pVerse->m_nWrdAccum+nWrd).arg(ndxDenormalAccum.index()).arg(ndxDenormalNoAccum.index()).toUtf8().data();
						Q_ASSERT(ndxDenormalAccum == ndxDenormalNoAccum);
					}
					CRelIndex ndxTest = CRelIndex(nBk, nChp, nVrs, nWrd);
					uint32_t nNormal = pBibleDatabase->NormalizeIndex(ndxTest);
					CRelIndex ndxDenormal = pBibleDatabase->DenormalizeIndex(nNormal);
					if (ndxDenormal != ndxTest) {
						std::cerr << QString("\n*** Error: Roundtrip : CRelIndex(%1, %2, %3, %4)=%5 -> Normal=%6 -> Denormal=%7\n")
									 .arg(nBk).arg(nChp).arg(nVrs).arg(nWrd).arg(ndxTest.index()).arg(nNormal).arg(ndxDenormal.index()).toUtf8().data();
						Q_ASSERT(ndxDenormal == ndxTest);
					}
				}
			}
		}
		std::cerr << ".";
	}
	std::cerr << "\n";

	return ERR_CODE_NONE;
}

// ----------------------------------------------------------------------------

int dumpIndexes(const CBibleDatabase *pBibleDatabase)		// Not static to avoid unused warning when not called
{
	std::cout << QString("Bible:  Testaments: %1  Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->bibleEntry().m_nNumTst)
						.arg(pBibleDatabase->bibleEntry().m_nNumBk)
						.arg(pBibleDatabase->bibleEntry().m_nNumChp)
						.arg(pBibleDatabase->bibleEntry().m_nNumVrs)
						.arg(pBibleDatabase->bibleEntry().m_nNumWrd)
						.toUtf8().data();

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumTst; ++i) {
		std::cout << QString("%1 : Books: %2  Chapters: %3  Verses: %4  Words: %5\n")
						.arg(pBibleDatabase->testamentEntry(i)->m_strTstName)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumBk)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->testamentEntry(i)->m_nNumWrd)
						.toUtf8().data();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		std::cout << QString("%1 : Chapters: %2  Verses: %3  Words: %4\n")
						.arg(pBibleDatabase->bookEntry(i)->m_strBkName)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumChp)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumVrs)
						.arg(pBibleDatabase->bookEntry(i)->m_nNumWrd)
						.toUtf8().data();
	}

	for (unsigned int i=1; i<=pBibleDatabase->bibleEntry().m_nNumBk; ++i) {
		const CBookEntry *pBook = pBibleDatabase->bookEntry(i);
		Q_ASSERT(pBook != nullptr);
		for (unsigned int j=1; j<=pBook->m_nNumChp; ++j) {
			std::cout << QString("%1 Chapter %2 : Verses: %3  Words: %4\n")
						.arg(pBook->m_strBkName)
						.arg(j)
						.arg(pBibleDatabase->chapterEntry(CRelIndex(i, j, 0, 0))->m_nNumVrs)
						.arg(pBibleDatabase->chapterEntry(CRelIndex(i, j, 0, 0))->m_nNumWrd)
						.toUtf8().data();
		}
	}

	return ERR_CODE_NONE;
}

// ============================================================================
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

	int nArgsFound = 0;
	bool bUnknownOption = false;
	bool bNoColophonVerses = false;
	bool bUseBracketColophons = false;
	bool bDisableColophons = false;
	bool bNoSuperscriptionVerses = false;
	bool bDisableSuperscriptions = false;
	bool bBracketItalics = false;
	bool bNoArabicNumeralWords = false;
	bool bInlineFootnotes = false;
	bool bUseBracketFootnotes = false;
	bool bUseBracketFootnotesExcluded = false;
	bool bExcludeDeuterocanonical = false;
	bool bMissingOK = false;		// Missing OR Extra Chapters/Verses are OK, (i.e. don't enforce KJV Versification)
	int nDescriptor = -1;
	QString strOSISFilename;
	QString strInfoFilename;
	QString strOutputPath;
	bool bLookingForOutputPath = false;
	QString strDefaultOutputPath = QProcessEnvironment::systemEnvironment().value(constrBuildDBPathEnvKey);
	if (!strDefaultOutputPath.isEmpty()) strDefaultOutputPath = QFileInfo(strDefaultOutputPath, "data").absoluteFilePath();
	QString strStrongsImpPath;
	bool bLookingForSegVariant = false;
	QString strSegVariant;
	bool bLookingForVersification = false;
	QString strV11n;
	bool bLookingForVersificationAdd = false;
	QString strV11nAdd;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			if (bLookingForOutputPath) {
				strOutputPath = strArg;
				bLookingForOutputPath = false;
			} else if (bLookingForSegVariant) {
				strSegVariant = strArg;
				bLookingForSegVariant = false;
			} else if (bLookingForVersification) {
				strV11n = strArg;
				bLookingForVersification = false;
			} else if (bLookingForVersificationAdd) {
				strV11nAdd = strArg;
				bLookingForVersificationAdd = false;
			} else {
				++nArgsFound;
				if (nArgsFound == 1) {
					nDescriptor = strArg.toInt();
				} else if (nArgsFound == 2) {
					strOSISFilename = strArg;
				} else if (nArgsFound == 3) {
					strInfoFilename = strArg;
				} else if (nArgsFound == 4) {
					strStrongsImpPath = strArg;
				}
			}
		} else if (strArg.compare("-o") == 0) {
			bLookingForOutputPath = true;
		} else if (strArg.compare("-c") == 0) {
			bNoColophonVerses = true;
		} else if (strArg.compare("-bc") == 0) {
			bUseBracketColophons = true;
		} else if (strArg.compare("-cd") == 0) {
			bDisableColophons = true;
		} else if (strArg.compare("-s") == 0) {
			bNoSuperscriptionVerses = true;
		} else if (strArg.compare("-sd") == 0) {
			bDisableSuperscriptions = true;
		} else if (strArg.compare("-i") == 0) {
			bBracketItalics = true;
		} else if (strArg.compare("-v") == 0) {
			bLookingForSegVariant = true;
		} else if (strArg.compare("-n") == 0) {
			bNoArabicNumeralWords = true;
		} else if (strArg.compare("-f") == 0) {
			bInlineFootnotes = true;
		} else if (strArg.compare("-bf") == 0) {
			bUseBracketFootnotes = true;
		} else if (strArg.compare("-bfx") == 0) {
			bUseBracketFootnotes = true;
			bUseBracketFootnotesExcluded = true;
		} else if (strArg.compare("-x") == 0) {
			bExcludeDeuterocanonical = true;
		} else if (strArg.compare("-m") == 0) {
			bMissingOK = true;
		} else if (strArg.compare("-v11n") == 0) {
			bLookingForVersification = true;
		} else if (strArg.compare("-v11nadd") == 0) {
			bLookingForVersificationAdd = true;
		} else {
			bUnknownOption = true;
		}
	}

	if (strOutputPath.isEmpty()) strOutputPath = strDefaultOutputPath;

	if (bLookingForSegVariant) bUnknownOption = true;		// Still looking for SegVariant
	if (bLookingForVersification) bUnknownOption = true;	// Still looking for versification index
	if (bLookingForVersificationAdd) bUnknownOption = true;	// Still looking for versification add index

	if ((nArgsFound < 3) || (nArgsFound > 4) || (strOutputPath.isEmpty()) || (bUnknownOption)) {
		std::cerr << QString("KJVDataParse Version %1\n\n").arg(a.applicationVersion()).toUtf8().data();
		std::cerr << QString("Usage: %1 [options] <UUID-Index> <OSIS-Database> <infofile> [<Strongs-Imp-path>]\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << QString("Reads and parses the OSIS database and outputs all of the CSV files\n").toUtf8().data();
		std::cerr << QString("    necessary to import into KJPBS into <datafile-path> (see -o option below)\n\n").toUtf8().data();
		std::cerr << QString("<infofile> is the path/filename to the information file to include\n\n").toUtf8().data();
		std::cerr << QString("Options\n").toUtf8().data();
		std::cerr << QString("    -o <datafile-path> = Data File Output path%1\n").arg(strDefaultOutputPath.isEmpty() ? " (required)" : "").toUtf8().data();
		if (!strDefaultOutputPath.isEmpty()) {
			std::cerr << QString("           (optional, if not specified, will use env path of: \"%1\")\n").arg(strDefaultOutputPath).toUtf8().data();
		}
		std::cerr << QString("    -c  =  Don't generate Colophons as pseudo-verses (only as footnotes)\n").toUtf8().data();
		std::cerr << QString("    -bc =  Enable Bracket Colophons (such as used in the TR text)\n").toUtf8().data();
		std::cerr << QString("           (use with -c to find the bracket colophons and remove them)\n").toUtf8().data();
		std::cerr << QString("           (Note: -bc will take precedence over -i)\n").toUtf8().data();
		std::cerr << QString("    -cd =  Disable all Colophon generation (pseudo-verses and footnote form)\n").toUtf8().data();
		std::cerr << QString("    -s  =  Don't generate Superscriptions as pseudo-verses (only as footnotes)\n").toUtf8().data();
		std::cerr << QString("    -sd =  Disable all Superscription generation (pseudo-verses and footnote form)\n").toUtf8().data();
		std::cerr << QString("    -i  =  Enable Bracket Italic detection conversion to TransChange\n").toUtf8().data();
		std::cerr << QString("    -v <variant> = Export only segment variant of <variant>\n").toUtf8().data();
		std::cerr << QString("    -n  =  Don't detect Arabic numerals as words\n").toUtf8().data();
		std::cerr << QString("    -f  =  Inline footnotes as Uncounted Parentheticals\n").toUtf8().data();
		std::cerr << QString("    -bf =  Enable Bracket Inline Footnotes (such as used in the RusSynodal)\n").toUtf8().data();
		std::cerr << QString("           (Note: -bf will take precedence over -i and -bc, and implies -f)\n").toUtf8().data();
		std::cerr << QString("    -bfx=  Enable Bracket Inline Footnotes and Exclude them\n").toUtf8().data();
		std::cerr << QString("           (Identical to -bf, but excludes them, and overrides -f)\n").toUtf8().data();
		std::cerr << QString("    -x  =  Exclude Apocrypha/Deuterocanonical Text\n").toUtf8().data();
		std::cerr << QString("    -m  =  Missing/Extra Chapters/Verses are OK (don't fit to KJV Versification)\n").toUtf8().data();
		std::cerr << QString("    -v11n <index> = Used to specify versification to override the database's default\n").toUtf8().data();
		std::cerr << QString("           (where <index> is one of the v11n indexes listed below\n").toUtf8().data();
		std::cerr << QString("    -v11nadd <index> = Write only the additional specified versification file\n").toUtf8().data();
		std::cerr << QString("           (where <index> is one of the v11n indexes listed below\n").toUtf8().data();
		std::cerr << QString("\n").toUtf8().data();
		std::cerr << QString("UUID-Index:\n").toUtf8().data();
		for (unsigned int ndx = 0; ndx < bibleDescriptorCount(); ++ndx) {
			BIBLE_DESCRIPTOR_ENUM nBDETemp = static_cast<BIBLE_DESCRIPTOR_ENUM>(ndx);
			std::cerr << QString("    %1 = %2 (%3)\n").arg(ndx).arg(bibleDescriptor(nBDETemp).m_strDBName).arg(bibleDescriptor(nBDETemp).m_strDBDesc).toUtf8().data();
		}
		std::cerr << "\n";
		std::cerr << QString("Versification Index:\n").toUtf8().data();
		for (int ndx = 0; ndx < CBibleVersifications::count(); ++ndx) {
			std::cerr << QString("    %1 = %2\n").arg(ndx).arg(CBibleVersifications::name(static_cast<BIBLE_VERSIFICATION_TYPE_ENUM>(ndx))).toUtf8().data();
		}
		std::cerr << "\n";
		return ERR_CODE_USAGE;
	}

	if ((nDescriptor < 0) || (static_cast<unsigned int>(nDescriptor) >= bibleDescriptorCount())) {
		std::cerr << "Unknown UUID-Index\n";
		return ERR_CODE_USAGE;
	}

	BIBLE_DESCRIPTOR_ENUM nBDE = static_cast<BIBLE_DESCRIPTOR_ENUM>(nDescriptor);
	TBibleDescriptor bblDescriptor = bibleDescriptor(nBDE);

	// Main Versification:
	if (!strV11n.isEmpty()) {
		BIBLE_VERSIFICATION_TYPE_ENUM nV11nType = static_cast<BIBLE_VERSIFICATION_TYPE_ENUM>(strV11n.toUInt());
		if ((nV11nType < 0) || (nV11nType >= CBibleVersifications::count())) {
			std::cerr << "Unknown Versification Type Index specified\n";
			return ERR_CODE_USAGE;
		}

		bblDescriptor.m_nMainVersification = nV11nType;
	}

	// Additional Versification:
	BIBLE_VERSIFICATION_TYPE_ENUM nV11nAddType = static_cast<BIBLE_VERSIFICATION_TYPE_ENUM>(strV11nAdd.toUInt());
	if (!strV11nAdd.isEmpty()) {
		if ((nV11nAddType < 0) || (nV11nAddType >= CBibleVersifications::count())) {
			std::cerr << "Unknown Versification Type Index specified\n";
			return ERR_CODE_USAGE;
		}
		if (!bMissingOK) {
			std::cerr << "*** Warning: You may want to specify -m (Missing OK) when writing alternate versifications\n";
		}
	}

	QDir dirOutput(strOutputPath);
	if (!dirOutput.exists()) {
		std::cerr << QString("\n\n*** Output path \"%1\" doesn't exist\n\n").arg(dirOutput.path()).toUtf8().data();
		return ERR_CODE_BUILD_PATH;
	}

	QFile fileOSIS;

	fileOSIS.setFileName(strOSISFilename);
	if (!fileOSIS.open(QIODevice::ReadOnly)) {
		std::cerr << QString("\n\n*** Failed to open OSIS database \"%1\"\n").arg(strOSISFilename).toUtf8().data();
		return ERR_CODE_OSIS_FILE_MISSING;
	}

	QXmlInputSource xmlInput(&fileOSIS);
	QXmlSimpleReader xmlReader;
	COSISXmlHandler xmlHandler(bblDescriptor);

	xmlHandler.setNoColophonVerses(bNoColophonVerses);
	xmlHandler.setUseBracketColophons(bUseBracketColophons);
	xmlHandler.setDisableColophons(bDisableColophons);
	xmlHandler.setNoSuperscriptionVerses(bNoSuperscriptionVerses);
	xmlHandler.setDisableSuperscriptions(bDisableSuperscriptions);
	xmlHandler.setBracketItalics(bBracketItalics);
	xmlHandler.setNoArabicNumeralWords(bNoArabicNumeralWords);
	xmlHandler.setInlineFootnotes(bInlineFootnotes);
	xmlHandler.setUseBracketFootnotes(bUseBracketFootnotes);
	xmlHandler.setUseBracketFootnotesExcluded(bUseBracketFootnotesExcluded);
	xmlHandler.setExcludeDeuterocanonical(bExcludeDeuterocanonical);
	xmlHandler.setSegVariant(strSegVariant);
	xmlHandler.setStrongsImpFilepath(strStrongsImpPath);

	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.setErrorHandler(&xmlHandler);
//	xmlReader.setFeature("http://www.bibletechnologies.net/2003/OSIS/namespace", true);

	if (!xmlReader.parse(xmlInput)) {
		std::cerr << QString("\n\n*** Failed to parse OSIS database \"%1\"\n%2\n").arg(strOSISFilename).arg(xmlHandler.errorString()).toUtf8().data();
		return ERR_CODE_OSIS_PARSE_FAILED;
	}

	const CBibleDatabase *pBibleDatabase = xmlHandler.bibleDatabase();
	bblDescriptor = pBibleDatabase->descriptor();		// Update descriptor from parsing

	// ------------------------------------------------------------------------

	if (strV11nAdd.isEmpty()) {
		std::cerr << "\nWriting Files:\n";
	} else {
		std::cerr << "\nProcessing Counts:\n";
	}

	int nRetVal = 0;
	TWordListBaton wordListBaton;

	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writeDBInfoFile(dirOutput, bblDescriptor, strInfoFilename);
	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writeTestamentsFile(dirOutput, pBibleDatabase);
	nRetVal = nRetVal ? nRetVal : doBooksChaptersVerses(dirOutput, pBibleDatabase, wordListBaton, xmlHandler, bMissingOK, strV11nAdd.isEmpty());
	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writeWordsFiles(dirOutput, pBibleDatabase, wordListBaton);
	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writeFootnotesFile(dirOutput, pBibleDatabase);
	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writePhrasesFile(dirOutput, pBibleDatabase);
	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writeLemmasFile(dirOutput, pBibleDatabase);
	nRetVal = (nRetVal || !strV11nAdd.isEmpty()) ? nRetVal : writeStrongs(dirOutput, pBibleDatabase);

	if (!strV11nAdd.isEmpty()) {
		std::cerr << "\nWriting Files:\n";
	}

	nRetVal = (nRetVal || strV11nAdd.isEmpty()) ? nRetVal : writeVersification(dirOutput, pBibleDatabase, nV11nAddType);

	// ------------------------------------------------------------------------

	if (!nRetVal) {
		if (strSegVariant.isEmpty() && xmlHandler.foundSegVariant()) {
			std::cerr << "\n"
						 "*** WARNING: Text contains seg variant text tags and no variant to parse was specified!\n"
						 "             Resulting database file will contain all variants run together!!\n\n";
		} else if (!strSegVariant.isEmpty() && !xmlHandler.foundSegVariant()) {
			std::cerr << "\n"
						 "*** WARNING: Specified seg variant wasn't found!  Resulting database file may be missing text!!\n\n";
		}
	}

	// ------------------------------------------------------------------------

	if (!nRetVal) {
		QString strParsedUTF8 = xmlHandler.parsedUTF8Chars();
		std::cerr << "UTF8 Characters Parsed: \"" << strParsedUTF8.toUtf8().data() << "\"\n";
		for (int i = 0; i<strParsedUTF8.size(); ++i) {
			std::cerr << "    \"" << QString(strParsedUTF8.at(i)).toUtf8().data() << "\" (" << QString("%1").arg(strParsedUTF8.at(i).unicode(), 4, 16, QChar('0')).toUtf8().data() << ")\n";
		}
	}

	// ------------------------------------------------------------------------

#if CHECK_INDEXES
	nRetVal = nRetVal ? nRetVal : checkIndexes(pBibleDatabase);
	//nRetVal = nRetVal ? nRetVal : dumpIndexes(pBibleDatabase);
#endif

	// ------------------------------------------------------------------------

/*
	std::cout << "\n============================ Element Names  =================================\n";
	QStringList lstElements = xmlHandler.elementNames();
	lstElements.sort();
	lstElements.removeDuplicates();
	for (int i = 0; i < lstElements.count(); ++i) {
		std::cout << lstElements.at(i).toUtf8().data() << "\n";
	}

	std::cout << "\n\n============================ Attribute Names  =================================\n";
	QStringList lstAttrib = xmlHandler.attrNames();
	lstAttrib.sort();
	lstAttrib.removeDuplicates();
	for (int i = 0; i < lstAttrib.count(); ++i) {
		std::cout << lstAttrib.at(i).toUtf8().data() << "\n";
	}

*/

	// ------------------------------------------------------------------------

//	return a.exec();
	return nRetVal;
}

