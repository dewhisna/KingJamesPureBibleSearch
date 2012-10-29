//
// KJVDataGen.cpp
//
//	Generates the Bk/Chp Keys and Data for the KJVCanOpener LAYOUT Table,
//				Chp/Vrs Keys and Data for individual KJVCanOpener BOOK Tables,
//				Word data for the WORDS Table,
//				Word mapping indices for the MAP Table,
//				and can create word and book dumps.
//
//	Reads a specialized (with '@' symbols) Sword OutPlain dump file with OSIS
//		Key names and text from <Filename-In>.  Sends the following to <Filename-Out>
//		depending on the specified mode:
//		layout :	Writes the LAYOUT table (sans footnotes)
//		book :		Writes the BOOK table for the specified book name
//		words :		Writes the WORDS table
//		worddump :	Dumps all words, trimmed on individual lines
//		bookdump :	Dumps the content of the specified book, one verse per line
//
// Input should be formatted so the OSIS reference is preceeded by "$$$" and
//	the verse text is preceeded by "@" and ended with "@".  Newline (\a) characters
//	at the beginning of the verse text is treated as the Paragraph mark (pilcrow)
//	as seen, for example, at the beginning of Gen 1:3.  Note that these only
//	occur in the KJV text at the beginning of the verse, just after the verse #!
//	Other newline (\a) characters are used to separate verse text from footnote
//	text, etc.  Thus, parsing is halted if another newline is found.
//
// Input Format:
//	$$$Gen.1.1
//	@In the beginning God created the heaven and the earth.@
//
//	$$$Gen.1.2
//	@And the earth was without form, and void; and darkness was upon the face of the deep. And the Spirit of God moved upon the face of the waters.@
//	
//	$$$Gen.1.3
//	@
//	And God said, Let there be light: and there was light.@
//
// When outputing BOOK tables, input stream should include rich text as well:
//
//	$$$Gen.1.1
//	@In the beginning God created the heaven and the earth.@
//	@In the beginning God created the heaven and the earth.@
//	$$$Gen.1.2
//	@And the earth was without form, and void; and darkness was upon the face of the deep. And the Spirit of God moved upon the face of the waters.@
//	@And the earth was without form, and void; and darkness <i>was</i> upon the face of the deep. And the Spirit of God moved upon the face of the waters.@
//	$$$Gen.1.3
//	@
//	And God said, Let there be light: and there was light.@
//	@¶And God said, Let there be light: and there was light.@
//	$$$Gen.1.4
//	@And God saw the light, that it was good: and God divided the light from the darkness.@
//	@And God saw the light, that <i>it was</i> good: and God divided the light from the darkness.@
//	$$$Gen.1.5
//	@And God called the light Day, and the darkness he called Night. And the evening and the morning were the first day.@
//	@And God called the light Day, and the darkness he called Night. And the evening and the morning were the first day.@
//
// Output Format (for LAYOUT):
//	{nBk|nChp},countVrs,countWrd,BkAbbr$,nChp
//
// Example:
//	257,31,797,Gen,1
//	258,25,632,Gen,2
//	259,24,695,Gen,3
//	260,26,632,Gen,4
//	261,32,504,Gen,5
//	262,22,579,Gen,6
//	263,24,584,Gen,7
//
// Output Format (for BOOK):
//	{nChp|nVrs},countWrd,bPilcrow,PlainText$,RichText$,Footnote$
//
// Example:
//
//
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <stdint.h>
//#include <locale>

#ifndef stricmp
#define stricmp _stricmp
#endif

#ifndef uint32_t
#define uint32_t unsigned int
#endif

// PILCROW_SUMMARY : Set to 1 to output Pilcrow report summary to <stderr> or 0 to suppress
#define PILCROW_SUMMARY 0
// OUTPUT_HEBREW_PS119 : Set to 1 to output Hebrew characters in addition to English for Psalm119 in Rich text
#define OUTPUT_HEBREW_PS119 1

#define NUM_BK 66
#define NUM_BK_OT 39
#define NUM_BK_NT 27

// ============================================================================
// Global Constants

const char *g_arrstrBkAbbr[NUM_BK] =
		{	"Gen",
			"Exod",
			"Lev",
			"Num",
			"Deut",
			"Josh",
			"Judg",
			"Ruth",
			"1Sam",
			"2Sam",
			"1Kgs",
			"2Kgs",
			"1Chr",
			"2Chr",
			"Ezra",
			"Neh",
			"Esth",
			"Job",
			"Ps",
			"Prov",
			"Eccl",
			"Song",
			"Isa",
			"Jer",
			"Lam",
			"Ezek",
			"Dan",
			"Hos",
			"Joel",
			"Amos",
			"Obad",
			"Jonah",
			"Mic",
			"Nah",
			"Hab",
			"Zeph",
			"Hag",
			"Zech",
			"Mal",
			"Matt",
			"Mark",
			"Luke",
			"John",
			"Acts",
			"Rom",
			"1Cor",
			"2Cor",
			"Gal",
			"Eph",
			"Phil",
			"Col",
			"1Thess",
			"2Thess",
			"1Tim",
			"2Tim",
			"Titus",
			"Phlm",
			"Heb",
			"Jas",
			"1Pet",
			"2Pet",
			"1John",
			"2John",
			"3John",
			"Jude",
			"Rev"
		};

const char *g_arrstrBkNames[NUM_BK] =
		{	"Genesis",
			"Exodus",
			"Leviticus",
			"Numbers",
			"Deuteronomy",
			"Joshua",
			"Judges",
			"Ruth",
			"1 Samuel",
			"2 Samuel",
			"1 Kings",
			"2 Kings",
			"1 Chronicles",
			"2 Chronicles",
			"Ezra",
			"Nehemiah",
			"Esther",
			"Job",
			"Psalms",
			"Proverbs",
			"Ecclesiastes",
			"Song of Solomon",
			"Isaiah",
			"Jeremiah",
			"Lamentaions",
			"Ezekiel",
			"Daniel",
			"Hosea",
			"Joel",
			"Amos",
			"Obadiah",
			"Jonah",
			"Micah",
			"Nahum",
			"Habakkuk",
			"Zephaniah",
			"Haggai",
			"Zechariah",
			"Malachi",
			"Matthew",
			"Mark",
			"Luke",
			"John",
			"Acts",
			"Romans",
			"1 Corinthians",
			"2 Corinthians",
			"Galatians",
			"Ephesians",
			"Philippians",
			"Colossians",
			"1 Thessalonians",
			"2 Thessalonians",
			"1 Timothy",
			"2 Timothy",
			"Titus",
			"Philemon",
			"Hebrews",
			"James",
			"1 Peter",
			"2 Peter",
			"1 John",
			"2 John",
			"3 John",
			"Jude",
			"Revelation"
		};


const char *g_strCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-";		// Accept: [alphanumeric, -, '], we'll handle UTF-8 conversion and translate those to ASCII as appropriate

static bool isSpecialWord(const std::string &strWord)
{
	if (stricmp(strWord.c_str(), "abominations") == 0) return true;
	if (stricmp(strWord.c_str(), "am") == 0) return true;
	if (stricmp(strWord.c_str(), "amen") == 0) return true;
	if (stricmp(strWord.c_str(), "ancient") == 0) return true;
	if (stricmp(strWord.c_str(), "and") == 0) return true;
	if (stricmp(strWord.c_str(), "angel") == 0) return true;
	if (stricmp(strWord.c_str(), "Babylon") == 0) return true;
	if (stricmp(strWord.c_str(), "bishop") == 0) return true;
	if (stricmp(strWord.c_str(), "branch") == 0) return true;
	if (stricmp(strWord.c_str(), "cherub") == 0) return true;
	if (stricmp(strWord.c_str(), "comforter") == 0) return true;
	if (stricmp(strWord.c_str(), "creator") == 0) return true;
	if (stricmp(strWord.c_str(), "day") == 0) return true;
	if (stricmp(strWord.c_str(), "days") == 0) return true;
	if (stricmp(strWord.c_str(), "devil") == 0) return true;
	if (stricmp(strWord.c_str(), "dominion") == 0) return true;
	if (stricmp(strWord.c_str(), "duke") == 0) return true;
	if (stricmp(strWord.c_str(), "earth") == 0) return true;
	if (stricmp(strWord.c_str(), "elect") == 0) return true;
	if (stricmp(strWord.c_str(), "father") == 0) return true;
	if (stricmp(strWord.c_str(), "father's") == 0) return true;
	if (stricmp(strWord.c_str(), "fathers") == 0) return true;
	if (stricmp(strWord.c_str(), "ghost") == 0) return true;
	if (stricmp(strWord.c_str(), "God") == 0) return true;
	if (stricmp(strWord.c_str(), "gods") == 0) return true;
	if (stricmp(strWord.c_str(), "great") == 0) return true;
	if (stricmp(strWord.c_str(), "harlots") == 0) return true;
	if (stricmp(strWord.c_str(), "heaven") == 0) return true;
	if (stricmp(strWord.c_str(), "hell") == 0) return true;
	if (stricmp(strWord.c_str(), "highest") == 0) return true;
	if (stricmp(strWord.c_str(), "him") == 0) return true;
	if (stricmp(strWord.c_str(), "himself") == 0) return true;
	if (stricmp(strWord.c_str(), "his") == 0) return true;
	if (stricmp(strWord.c_str(), "holiness") == 0) return true;
	if (stricmp(strWord.c_str(), "holy") == 0) return true;
	if (stricmp(strWord.c_str(), "is") == 0) return true;
	if (stricmp(strWord.c_str(), "Jesus") == 0) return true;
	if (stricmp(strWord.c_str(), "Jews") == 0) return true;
	if (stricmp(strWord.c_str(), "judge") == 0) return true;
	if (stricmp(strWord.c_str(), "king") == 0) return true;
	if (stricmp(strWord.c_str(), "kings") == 0) return true;
	if (stricmp(strWord.c_str(), "kings'") == 0) return true;
	if (stricmp(strWord.c_str(), "lamb") == 0) return true;
	if (stricmp(strWord.c_str(), "legion") == 0) return true;
	if (stricmp(strWord.c_str(), "lion") == 0) return true;
	if (stricmp(strWord.c_str(), "lord") == 0) return true;
	if (stricmp(strWord.c_str(), "lord's") == 0) return true;
	if (stricmp(strWord.c_str(), "lords") == 0) return true;
	if (stricmp(strWord.c_str(), "lot") == 0) return true;
	if (stricmp(strWord.c_str(), "man") == 0) return true;
	if (stricmp(strWord.c_str(), "man's") == 0) return true;
	if (stricmp(strWord.c_str(), "master") == 0) return true;
	if (stricmp(strWord.c_str(), "masters") == 0) return true;
	if (stricmp(strWord.c_str(), "men") == 0) return true;
	if (stricmp(strWord.c_str(), "men's") == 0) return true;
	if (stricmp(strWord.c_str(), "mighty") == 0) return true;
	if (stricmp(strWord.c_str(), "moon") == 0) return true;
	if (stricmp(strWord.c_str(), "mother") == 0) return true;
	if (stricmp(strWord.c_str(), "mystery") == 0) return true;
	if (stricmp(strWord.c_str(), "Nazareth") == 0) return true;
	if (stricmp(strWord.c_str(), "of") == 0) return true;
	if (stricmp(strWord.c_str(), "one") == 0) return true;
	if (stricmp(strWord.c_str(), "our") == 0) return true;
	if (stricmp(strWord.c_str(), "righteousness") == 0) return true;
	if (stricmp(strWord.c_str(), "sanctuary") == 0) return true;
	if (stricmp(strWord.c_str(), "saviour") == 0) return true;
	if (stricmp(strWord.c_str(), "sceptre") == 0) return true;
	if (stricmp(strWord.c_str(), "shepherd") == 0) return true;
	if (stricmp(strWord.c_str(), "son") == 0) return true;
	if (stricmp(strWord.c_str(), "spirit") == 0) return true;
	if (stricmp(strWord.c_str(), "spirits") == 0) return true;
	if (stricmp(strWord.c_str(), "sun") == 0) return true;
	if (stricmp(strWord.c_str(), "tabernacle") == 0) return true;
	if (stricmp(strWord.c_str(), "that") == 0) return true;
	if (stricmp(strWord.c_str(), "the") == 0) return true;
	if (stricmp(strWord.c_str(), "this") == 0) return true;
	if (stricmp(strWord.c_str(), "thy") == 0) return true;
	if (stricmp(strWord.c_str(), "unknown") == 0) return true;
	if (stricmp(strWord.c_str(), "unto") == 0) return true;
	if (stricmp(strWord.c_str(), "word") == 0) return true;
	if (stricmp(strWord.c_str(), "wormwood") == 0) return true;

	return false;
}

// ============================================================================
// Special Types

typedef std::vector<uint32_t> TIndexList;			// Index List for words into book/chapter/verse/word

class CWordEntry
{
public:
	CWordEntry()
	{
		for (int i=0; i<NUM_BK; ++i) m_arrUsage[i] = 0;
	}

// TODO : CLEAN
//	std::string m_strWord;		// Word Text
//	std::string m_strAltWords;	// CSV of alternate synonymous words for searching (such as hyphenated and non-hyphenated)
	TIndexList m_ndxMapping;	// Indexes into text
	TIndexList m_ndxNormalized;	// Normalized index into entire Bible (Number of entries here should match number of entries in index above)
	int m_arrUsage[NUM_BK];		// Word usage count by book.  Sum of the values in this array should match size of index above

	struct SortPredicate {
		bool operator() (const std::string &s1, const std::string &s2) const
		{
			return (strcmp(s1.c_str(), s2.c_str()) < 0);
		}
	};

};

// WordListMap will be for ALL forms of all words so that we can get mapping/counts
//	for all unique forms of words.  Then, we'll store the alternate words in
//	TAltWordSet objects in the TAltWordListMap which is indexed by only the lower-case
//	form.  We'll figure out the base "Word Text" and "AltWords" from the set data.
typedef std::map<std::string, CWordEntry, CWordEntry::SortPredicate> TWordListMap;
typedef std::set<std::string, CWordEntry::SortPredicate> TAltWordSet;
typedef std::map<std::string, TAltWordSet, CWordEntry::SortPredicate> TAltWordListMap;

struct XformLower {
	int operator()(int c)
	{
		return tolower(c);
	}
};

// WordFromWordSet - Drives word toward lower-case and returns the resulting word.  The
//		theory is that proper names will always be capitalized and non-proper names will
//		have mixed case, being capital only when they start a new sentence.  Thus, if we
//		drive toward lower-case, we should have an all-lower-case word for non-proper
//		name words and mixed-case for proper names:
std::string WordFromWordSet(const TAltWordSet &setAltWords)
{
	std::string strWord;

	for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
		if ((strWord.empty()) ||
			(strcmp((*itrAltWrd).c_str(), strWord.c_str()) > 0)) strWord = *itrAltWrd;
	}

	return strWord;
}

// ============================================================================
// Global Variables

TWordListMap g_mapWordList;			// Our one and only master word list
TAltWordListMap g_mapAltWordList;	// Our alternate word list, mapping various case forms of words
TIndexList g_NormalizationVerification;		// Mapping of normalized indexes to Bk/Chp/Vrs/Wrd index.  Used to verify the normalization process


// ============================================================================

uint32_t MakeIndex(uint32_t nN3, uint32_t nN2, uint32_t nN1, uint32_t nN0)
{
	return (((nN3 & 0xFF) << 24) | ((nN2 & 0xFF) << 16) | ((nN1 & 0xFF) << 8) | (nN0 & 0xFF));
}


// ============================================================================

int fgetcUTF8(FILE *file)
{
	int j;
	int c;
	int c2;
	int cResult;

	c = fgetc(file);
	if (c == EOF) return EOF;
	if ((c & 0x0080) == 0) return c;		// Single bytes leave as-is
	c = ((c & 0x007f) << 1);

	cResult=0;
	j=-1;					// Start with -1 because we've already shifted c by 1 bit
	while (c & 0x0080) {
		c2 = fgetc(file);
		if (c2 == EOF) return EOF;			// Exit if we hit the end of the file prematurely (really, this is a character encoding error)
		cResult = (cResult << 6) | (c2 & 0x3F);
		j+=5;				// Shift left 5, 6 for the 6 bits we are adding, minus 1 because we're already shifted c
		c = ((c & 0x007f) << 1);
	}
	cResult = cResult | (c << j);

	return cResult;
}

int sputcUTF8(int c, char *outbuf)
{
	unsigned int c1 = c;
	unsigned int c2;
	char buff[10];
	int n;
	unsigned int m;

	if (c1 < 0x80) {
		outbuf[0] = c1;
		outbuf[1] = 0;
		return 1;
	}

	buff[6] = 0;
	n = 5;
	m = 32;
	c2 = 0x1F80;
	while (c1) {
		buff[n] = (c1 & 0x3f) | 0x80;
		c1 = c1 >> 6;
		c2 = c2 >> 1;
		if (c1 < m) {
			n--;
			buff[n] = ((c2 & 0xFF) | c1);
			c1 = 0;
		} else {
			n--;
			m /= 2;
		}
	}
	strcpy(outbuf, &buff[n]);
	return strlen(&buff[n]);
}

void fputcUTF8(int c, FILE *file)
{
	char buff[10];

	sputcUTF8(c, buff);
	fprintf(file, "%s", buff);
}

// ============================================================================

int main(int argc, const char *argv[])
{
	char buffPlain[10000];		// Plain Text version
	char buffRich[10000];		// Rich Text version
	char buffFootnote[2000];	// Footnote Text
	char word[50];
	int countChp[NUM_BK];		// # of Chapters in books
	int countVrs[NUM_BK];		// # of Verses in entire book for Book mode or # of Verses in current Chapter for Layout mode
	int countWrd[NUM_BK];		// # of Words in entire book for Book mode or # of Words in current Chapter for Layout mode
	bool bDoingLayout = false;		// TRUE if outputing LAYOUT table
	bool bDoingBook = false;		// TRUE if outputing BOOK table (nBkNdx = book to output)
	bool bDoingWords = false;		// TRUE if outputing WORDS table
	bool bDoingWordDump = false;	// TRUE if dumping words used (nBkNdx = book to output or 0 for all)
	bool bDoingWordDumpKeys = false;	// TRUE if dumping unique Key words used (nBkNdx = book to output or 0 for all)
	bool bDoingWordDumpUnique = false;	// TRUE if dumping unique words used (nBkNdx = book to output or 0 for all)
	bool bDoingWordDumpUniqueLC = false;	// TRUE if dumping unique words used as lower-case keys (nBkNdx = book to output or 0 for all)
	bool bDoingAltWordDump = false;	// TRUE if dumping the Alt Words List (nBkNdx = book to output or 0 for all)
	bool bDoingBookDump = false;	// TRUE if dumping book content (nBkNdx = book to output or 0 for all)
	bool bDoingSummary = false;		// TRUE if dumping word usage summary (always for all books)
	bool bNeedUsage = false;		// TRUE if user needs usage info
	int nBkNdx = 0;		// Index of Book to Output (Book Mode)
	int nCurBk = 0;		// Current book output (Layout Mode)
	int nCurChp = 0;	// Current chapter output (Layout Mode)
	int nBk;			// Book counter
	int nChp;			// Chapter counter
	int nVrs;			// Verse counter
	int nWrd;			// Word counter
	unsigned int ndx;	// Index into buffer;
	uint32_t nNormalizedNdx = 0;	// Normalized index -- incremented for each word processed and added to collection
	bool bIsPilcrow;		// Set to TRUE if the current verse is a Pilcrow (¶) start point, else is FALSE (either Plain/Rich)
	bool bIsPilcrowPlain;	// Set to TRUE if the current verse is a Pilcrow (¶) start point, else is FALSE (in Plain Text)
	bool bIsPilcrowRich;	// Set to TRUE if the current verse is a Pilcrow (¶) start point, else is FALSE (in Rich Text)
	bool bPilcrowMismatch;	// Set to TRUE if the plain specifies a pilcrow and the rich doesn't or vice versa
	int nPilcrowPlain = 0;	// Number of Pilcrows in Plain
	int nPilcrowRich = 0;	// Number of Pilcrows in Rich
	int nPilcrowTotal = 0;	// Total Number of Pilcrows
	int i;
	int c;				// Character from input file
	char *pTemp;
	char *pTemp2;
	FILE *fileIn = NULL;
	FILE *fileOut = NULL;
	const char *strInFilename = NULL;
	const char *strOutFilename = NULL;

	if ((argc != 4) && (argc != 5)) {
		bNeedUsage = true;
	} else if (stricmp(argv[1], "layout") == 0) {
		bDoingLayout = true;
		// Book name is ignored
	} else if (stricmp(argv[1], "book") == 0) {
		if (argc != 5) bNeedUsage = true;		// Must have book name for this one
		bDoingBook = true;
	} else if (stricmp(argv[1], "words") == 0) {
		bDoingWords = true;
		// Book name is ignored
	} else if (stricmp(argv[1], "worddump") == 0) {
		bDoingWordDump = true;
		// Book name is optional -- used if specified, or "all" if not
	} else if (stricmp(argv[1], "worddumpkeys") == 0) {
		bDoingWordDumpKeys = true;
		// Book name is optional -- used if specified, or "all" if not
	} else if (stricmp(argv[1], "worddumpunique") == 0) {
		bDoingWordDumpUnique = true;
		// Book name is optional -- used if specified, or "all" if not
	} else if (stricmp(argv[1], "worddumpuniqlc") == 0) {
		bDoingWordDumpUniqueLC = true;
		// Book name is optional -- used if specified, or "all" if not
	} else if (stricmp(argv[1], "altworddump") == 0) {
		bDoingAltWordDump = true;
		// Book name is optional -- used if specified, or "all" if not
	} else if (stricmp(argv[1], "bookdump") == 0) {
		bDoingBookDump = true;
		// Book name is optional -- used if specified, or "all" if not
	} else if (stricmp(argv[1], "summary") == 0) {
		bDoingSummary = true;
		// Book name is ignored
	} else {
		bNeedUsage = true;
	}

	if (argc >= 5) {				// Must have at least 5 to have a bookname.  If less, it's invalid or they omitted it
		nBkNdx = 0;					// 0 = All books for modes that allow it
		if (stricmp(argv[2], "all") != 0) {			// Allow special keyword "all" for all books
			for (nBk = 0; nBk < NUM_BK; ++nBk) {
				if (stricmp(g_arrstrBkAbbr[nBk], argv[2]) == 0) {
					nBkNdx = nBk + 1;
					break;
				}
			}
			if (nBkNdx == 0) {
				fprintf(stderr, "Invalid book name.  Valid books are:\n\n");
				fprintf(stderr, "    ALL\n");
				for (nBk = 0; nBk < NUM_BK; ++nBk) {
					fprintf(stderr, "    %s\n", g_arrstrBkAbbr[nBk]);
				}
				return -1;
			}
		}
	}
	// By here, nBkNum = Book number to output, or 0 for all books on modes that allow it

	if (bNeedUsage) {
		fprintf(stderr, "Usage:  kjvdatagen <mode> [<bookname>] <Filename-In> <Filename-Out>\n\n");
		fprintf(stderr, "  Where <mode> is one of:\n\n");
		fprintf(stderr, "      layout         -- Dump LAYOUT table data (book name not used)\n");
		fprintf(stderr, "      book           -- Dump BOOK table data for the specified book\n");
		fprintf(stderr, "      words          -- Dump WORDS table data (book name not used)\n");
		fprintf(stderr, "      worddump       -- Dumps all words (book name optional)\n");
		fprintf(stderr, "      worddumpkeys   -- Dumps all key words (book name optional)\n");
		fprintf(stderr, "      worddumpunique -- Dumps all unique words (book name optional)\n");
		fprintf(stderr, "      worddumpuniqlc -- Dumps all unique words lowercase (book name optional)\n");
		fprintf(stderr, "      altworddump    -- Dumps the alternate words table (book name optional)\n");
		fprintf(stderr, "      bookdump       -- Dumps the content of specified book (Verse per line)\n");
		fprintf(stderr, "      summary        -- Dump word usage Summary CSV (book name ignored)\n");
		fprintf(stderr, "\n\n");
		fprintf(stderr, "  Input should be specialized Sword dump file with both plain\n");
		fprintf(stderr, "    and rich text.  Use '-' for filename for <stdin>/<stdout>.\n");
		fprintf(stderr, "\n");
		return -1;
	}

	strInFilename = argv[((argc == 4) ? 2 : 3)];
	strOutFilename = argv[((argc == 4) ? 3 : 4)];

	if (strcmp(strInFilename, "-") != 0) {
		fileIn = fopen(strInFilename, "rb");
	} else {
		fileIn = stdin;
		strInFilename = "<stdin>";
	}
	if (fileIn == NULL) {
		fprintf(stderr, "Error: Can't open '%s' for reading.\n", strInFilename);
		return -2;
	}

	if (strcmp(strOutFilename, "-") != 0) {
		fileOut = fopen(strOutFilename, "wb");
	} else {
		_setmode(_fileno(stdout), _O_BINARY);
		fileOut = stdout;
		strOutFilename = "<stdout>";
	}
	if (fileOut == NULL) {
		fprintf(stderr, "Error: Can't open '%s' for writing.\n", strOutFilename);
		if ((fileIn != NULL) && (fileIn != stdin)) fclose(fileIn);
		return -3;
	}

/////////////////////////////////////
// Preprocessing:
/////////////////////////////////////

//	if (bDoingTOC) {
//		fprintf(fileOut, "BkNdx,TstBkNdx,TstNdx,BkName,BkAbbr,TblName,NumChp,NumVrs,NumWrd,Cat,Desc\r\n");
//	}

	if (bDoingLayout) {
		fprintf(fileOut, "BkChpNdx,NumVrs,NumWrd,BkAbbr,ChNdx\r\n");
	}

	if (bDoingBook) {
		// Excel will normally try to treat the file as Latin-1, but since we
		//		have some embedded UTF-8, we need to output a BOM:
		fputcUTF8(0x0FEFF, fileOut);
		fprintf(fileOut, "ChpVrsNdx,NumWrd,bPilcrow,PText,RText,Footnote\r\n");
	}

/////////////////////////////////////
// Main Processing Loop
/////////////////////////////////////

	// Initialize total counts for the book
	for (nBk = 0; nBk < NUM_BK; ++nBk) {
		countChp[nBk] = 0;
		countVrs[nBk] = 0;
		countWrd[nBk] = 0;
	}

	nBk = nChp = nVrs = 0;		// Start with 0.0 and set with parsed data when parsing OSIS
	if (bDoingLayout) {
		nCurChp = 1;				// For layout mode, pretend we're currently on chapter 1 already
		nCurBk = 1;					//	and in book 1
	}
	while (!feof(fileIn)) {

/////////////////////////////////////
// Find and Parse Bk/Chp/Vrs Header
/////////////////////////////////////

		for (i=0; i<3; ++i) {		// looking for 3 '$' symbols
			do {
				c = fgetcUTF8(fileIn);
			} while ((c != '$') && (c != EOF));
			if (c == EOF) break;
		}
		if (c == EOF) break;

		fgets(buffPlain, sizeof(buffPlain), fileIn);	// Note: The reference shouldn't be UTF-8, so we can just use fgets
		nBk = nChp = nVrs = 0;
		pTemp = strchr(buffPlain, '.');			// Find book name
		if (pTemp) {
			*pTemp = 0;
			pTemp++;
			for (nBk = 0; nBk < NUM_BK; ++nBk) {
				if (stricmp(g_arrstrBkAbbr[nBk], buffPlain) == 0) break;
			}
			if (nBk == NUM_BK) {
				nBk = 0;			// Set to 0 if we didn't find it
			} else {
				nBk++;				// Otherwise, store as 1's orig
			}
			pTemp2 = strchr(pTemp, '.');		// Find chp #
			if (pTemp2) {
				*pTemp2 = 0;
				pTemp2++;
				nChp = atoi(pTemp);
				nVrs = atoi(pTemp2);
			} else {
				pTemp--;
				*pTemp = '.';		// unwind parse so we can print error text
			}
		}
		if ((nBk == 0) || (nChp == 0) || (nVrs == 0)) {
			fprintf(stderr, "Unable to parse: %s\n\n", buffPlain);
			continue;
		}

		if (nChp > countChp[nBk-1]) countChp[nBk-1] = nChp;
		if (nVrs > countVrs[nBk-1]) countVrs[nBk-1] = nVrs;

/////////////////////////////////////
// Find and Parse Plain Text
/////////////////////////////////////

		do {							// Find "@" marker
			c = fgetcUTF8(fileIn);
		} while ((c != '@') && (c != EOF));
		if (c == EOF) break;

		// Read and fill buffPlain with verse text up to final "@" marker:
		bIsPilcrowPlain = false;
		ndx = 0;
		do {
			c = fgetcUTF8(fileIn);
			if ((c != '@') && (c != EOF)) {
				if (c == 0x0a) {
					if (ndx > 0) break;			// A newline after text means footnote marker!  So skip the rest!!
					bIsPilcrowPlain = true;		// A newline at the start denotes a Pilcrow (¶) mark
					continue;					// We set the flag, but we'll exclude writing the newline to the output
				}
				if (c == 0xb6) {				// Translate Pilcrow
					bIsPilcrowPlain = true;
					continue;
				}
				if (c == 0x2013) c = '-';		// Translate "en dash" to (-)
				if (c == 0x2015) {				// Translate "Horizontal Bar" to (--) TODO : Figure out what to do with this one!
					buffPlain[ndx] = '-';
					ndx++;
					c = '-';
				}
				if (c == 0xe6) {				// Translate "æ" to "ae":
					buffPlain[ndx] = 'a';
					ndx++;
					c = 'e';
				}
				if (c == 0xc6) {				// Translate "Æ" to "Ae":
					buffPlain[ndx] = 'A';
					ndx++;
					c = 'e';
				}
				if (c > 127) {
					fprintf(stderr, "Unexpected UTF-8 symbol in Plain Text (%04x) at position %ld [0x%04lx] in file: %s %0d.%0d [%0d]\n", c, ftell(fileIn), ftell(fileIn), g_arrstrBkAbbr[nBk-1], nChp, nVrs, ndx);
				}
				buffPlain[ndx] = c;
				ndx++;
			}
		} while ((!feof(fileIn)) && (ndx < sizeof(buffPlain)-1) && (c != '@'));
		buffPlain[ndx] = 0;			// End main text
		if (c != '@') {				// If we didn't find the end marker, we must be in a footnote
			ndx = 0;
			do {
				c = fgetcUTF8(fileIn);
				if ((c != '@') && (c != EOF)) {
					if (c == 0xb6) continue;			// Translate Pilcrow (ignore them in footnote, shouldn't be here anyway)
					if (c == 0x2013) c = '-';		// Translate "en dash" to (-)
					if (c == 0x2015) {				// Translate "Horizontal Bar" to (--) TODO : Figure out what to do with this one!
						buffFootnote[ndx] = '-';
						ndx++;
						c = '-';
					}
					if (c == 0xe6) {				// Translate "æ" to "ae":
						buffFootnote[ndx] = 'a';
						ndx++;
						c = 'e';
					}
					if (c == 0xc6) {				// Translate "Æ" to "Ae":
						buffFootnote[ndx] = 'A';
						ndx++;
						c = 'e';
					}
					if (c > 127) {
						fprintf(stderr, "Unexpected UTF-8 symbol in Footnote Text (%04x) at position %ld [0x%04lx] in file: %s %0d.%0d [%0d]\n", c, ftell(fileIn), ftell(fileIn), g_arrstrBkAbbr[nBk-1], nChp, nVrs, ndx);
					}
					buffFootnote[ndx] = c;
					ndx++;
				}
			} while ((!feof(fileIn)) && (ndx < sizeof(buffFootnote)-1) && (c != '@'));
			buffFootnote[ndx] = 0;			// End footnote
		}

/////////////////////////////////////
// Find and Parse Rich Text
/////////////////////////////////////

		do {							// Find "@" marker
			c = fgetcUTF8(fileIn);
		} while ((c != '@') && (c != EOF));
		if (c == EOF) break;

		// Read and fill buffRich with verse text up to final "@" marker:
		bIsPilcrowRich = false;
		ndx = 0;

		// First, fill in missing Hebrew alphabet in Psalm 119 in both Hebrew and English for text to be rendered:
		if ((nBk == 19) && (nChp == 119) && (((nVrs-1)%8) == 0)) {

#if (OUTPUT_HEBREW_PS119)
			switch ((nVrs-1)/8) {
				case 0:
					// ALEPH
					ndx += sputcUTF8(0x005D0, &buffRich[ndx]);
					break;
				case 1:
					// BETH
					ndx += sputcUTF8(0x005D1, &buffRich[ndx]);
					break;
				case 2:
					// GIMEL
					ndx += sputcUTF8(0x005D2, &buffRich[ndx]);
					break;
				case 3:
					// DALETH
					ndx += sputcUTF8(0x005D3, &buffRich[ndx]);
					break;
				case 4:
					// HE
					ndx += sputcUTF8(0x005D4, &buffRich[ndx]);
					break;
				case 5:
					// VAU
					ndx += sputcUTF8(0x005D5, &buffRich[ndx]);
					break;
				case 6:
					// ZAIN
					ndx += sputcUTF8(0x005D6, &buffRich[ndx]);
					break;
				case 7:
					// CHETH
					ndx += sputcUTF8(0x005D7, &buffRich[ndx]);
					break;
				case 8:
					// TETH
					ndx += sputcUTF8(0x005D8, &buffRich[ndx]);
					break;
				case 9:
					// JOD
					ndx += sputcUTF8(0x005D9, &buffRich[ndx]);
					break;
				case 10:
					// CAPH
					ndx += sputcUTF8(0x005DB, &buffRich[ndx]);		// Using nonfinal-CAPH
					break;
				case 11:
					// LAMED
					ndx += sputcUTF8(0x005DC, &buffRich[ndx]);
					break;
				case 12:
					// MEM
					ndx += sputcUTF8(0x005DE, &buffRich[ndx]);		// Using nonfinal-Mem
					break;
				case 13:
					// NUN
					ndx += sputcUTF8(0x005E0, &buffRich[ndx]);		// Using nonfinal-Nun
					break;
				case 14:
					// SAMECH
					ndx += sputcUTF8(0x005E1, &buffRich[ndx]);
					break;
				case 15:
					// AIN
					ndx += sputcUTF8(0x005E2, &buffRich[ndx]);
					break;
				case 16:
					// PE
					ndx += sputcUTF8(0x005E4, &buffRich[ndx]);		// Using nonfinal-Pe
					break;
				case 17:
					// TZADDI
					ndx += sputcUTF8(0x005E6, &buffRich[ndx]);		// Using nonfinal-Tzaddi
					break;
				case 18:
					// KOPH
					ndx += sputcUTF8(0x005E7, &buffRich[ndx]);
					break;
				case 19:
					// RESH
					ndx += sputcUTF8(0x005E8, &buffRich[ndx]);
					break;
				case 20:
					// SCHIN
					ndx += sputcUTF8(0x005E9, &buffRich[ndx]);
					break;
				case 21:
					// TAU
					ndx += sputcUTF8(0x005EA, &buffRich[ndx]);
					break;
			}
			strcpy(&buffRich[ndx], " ");
			ndx += strlen(&buffRich[ndx]);
#endif

			switch ((nVrs-1)/8) {
				case 0:
					strcpy(&buffRich[ndx], "(ALEPH). ");
					break;
				case 1:
					strcpy(&buffRich[ndx], "(BETH). ");
					break;
				case 2:
					strcpy(&buffRich[ndx], "(GIMEL). ");
					break;
				case 3:
					strcpy(&buffRich[ndx], "(DALETH). ");
					break;
				case 4:
					strcpy(&buffRich[ndx], "(HE). ");
					break;
				case 5:
					strcpy(&buffRich[ndx], "(VAU). ");
					break;
				case 6:
					strcpy(&buffRich[ndx], "(ZAIN). ");
					break;
				case 7:
					strcpy(&buffRich[ndx], "(CHETH). ");
					break;
				case 8:
					strcpy(&buffRich[ndx], "(TETH). ");
					break;
				case 9:
					strcpy(&buffRich[ndx], "(JOD). ");
					break;
				case 10:
					strcpy(&buffRich[ndx], "(CAPH). ");
					break;
				case 11:
					strcpy(&buffRich[ndx], "(LAMED). ");
					break;
				case 12:
					strcpy(&buffRich[ndx], "(MEM). ");
					break;
				case 13:
					strcpy(&buffRich[ndx], "(NUN). ");
					break;
				case 14:
					strcpy(&buffRich[ndx], "(SAMECH). ");
					break;
				case 15:
					strcpy(&buffRich[ndx], "(AIN). ");
					break;
				case 16:
					strcpy(&buffRich[ndx], "(PE). ");
					break;
				case 17:
					strcpy(&buffRich[ndx], "(TZADDI). ");
					break;
				case 18:
					strcpy(&buffRich[ndx], "(KOPH). ");
					break;
				case 19:
					strcpy(&buffRich[ndx], "(RESH). ");
					break;
				case 20:
					strcpy(&buffRich[ndx], "(SCHIN). ");
					break;
				case 21:
					strcpy(&buffRich[ndx], "(TAU). ");
					break;
				default:
					buffRich[ndx] = 0;		// Safeguard in case something's wrong with the text
					break;
			}
			ndx += strlen(&buffRich[ndx]);

			bIsPilcrowRich = true;			// Add Pilcrow break for these as it looks best on output since each letter should start new paragraph
		}

		do {
			c = fgetcUTF8(fileIn);
			if ((c != '@') && (c != EOF)) {
				if (ndx == 0) {
					if (c == 0xb6) {				// Translate Pilcrow (but only at start of verse!)
						bIsPilcrowRich = true;
					} else {
						// Source text tends to have mismatches of pilcrows between plain and rich.  So
						//		we'll make our rich output the combined pilcrows so it's complete:
						if (bIsPilcrowPlain) ndx += sputcUTF8(0x00b6, &buffRich[ndx]);
					}
				}
				if (c == '\"') {				// Escape " with "" by adding one more to our buffer
					buffRich[ndx] = '\"';
					ndx++;
				}

// Can't do this or else we'll lose our special hyphens and such.  Replaced with BOM output above.  Left here for reference:
//				// Excel apparently expects Latin-1 for CSV files, so we won't output UTF-8
//				if (c > 255) {
//					fprintf(stderr, "Unexpected UTF-8 symbol in Rich Text (%04x) at position %ld [0x%04lx] in file: %s %0d.%0d [%0d]\n", c, ftell(fileIn), ftell(fileIn), g_arrstrBkAbbr[nBk-1], nChp, nVrs, ndx);
//				} else {
//					buffRich[ndx] = c;
//					ndx++;
//				}

				ndx += sputcUTF8(c, &buffRich[ndx]);	// UTF8 transfer character

			}
		} while ((!feof(fileIn)) && (ndx < sizeof(buffRich)-1) && (c != '@'));
		buffRich[ndx] = 0;

/////////////////////////////////////
// Parse and Write Output File
/////////////////////////////////////

		bIsPilcrow = (bIsPilcrowPlain || bIsPilcrowRich);
		bPilcrowMismatch = ((bIsPilcrowPlain != bIsPilcrowRich) ? true : false);

		if (bIsPilcrow) ++nPilcrowTotal;
		if (bIsPilcrowPlain) ++nPilcrowPlain;
		if (bIsPilcrowRich) ++nPilcrowRich;

#if (PILCROW_SUMMARY)
		if (bPilcrowMismatch) {
			fprintf(stderr, "Pilcrow Mismatch in file: %s %0d.%0d  (%s)\n", g_arrstrBkAbbr[nBk-1], nChp, nVrs, (bIsPilcrowPlain ? "Plain" : "Rich"));
		}
#endif

		if (bDoingLayout) {
			// Output the current one we were processing just before transitioning:
			if ((nCurBk != nBk) || (nCurChp != nChp)) {
				fprintf(fileOut, "%d,%d,%d,%s,%d\r\n", nCurBk*256+nCurChp, countVrs[nCurBk-1], countWrd[nCurBk-1], g_arrstrBkAbbr[nCurBk-1], nCurChp);
				countVrs[nCurBk-1] = 0;				// For Layout mode, these are counts for the chapter only, so reset them
				countWrd[nCurBk-1] = 0;
				nCurBk = nBk;
				nCurChp = nChp;
			}
		}

		if ((bDoingBookDump) && ((nBkNdx == 0) || (nBk == nBkNdx))) {
			fprintf(fileOut, "%s\r\n", buffPlain);
		}

		// Do this after the printing because either we are on the same
		//		chp/bk in which case we didn't print the output line above
		//		and it doesn't matter on the order, or else this word count
		//		is for the next chp or bk and should count toward it instead
		//		after we've written the output for our current Chp/Bk.
		nWrd = 0;
		pTemp = strpbrk(buffPlain, g_strCharset);
		while (pTemp) {								// pTemp = start of word
			nWrd++;
			pTemp2 = pTemp+1;
			while ((*pTemp2 != 0) && (strchr(g_strCharset, *pTemp2) != NULL)) ++pTemp2;
			memcpy(word, pTemp, pTemp2-pTemp);
			word[pTemp2-pTemp] = 0;
			if ((bDoingWordDump) && ((nBkNdx == 0) || (nBk == nBkNdx))) fprintf(fileOut, "%s\r\n", word);
			if ((nBkNdx == 0) || (nBk == nBkNdx)) {
				std::string strWordKey(word);

				// Set possible alternate word form:
				std::transform(strWordKey.begin(), strWordKey.end(), strWordKey.begin(), XformLower());
				TAltWordSet &wrdSet = g_mapAltWordList[strWordKey];
				wrdSet.insert(word);

				CWordEntry &wrdEntry = g_mapWordList[word];

				wrdEntry.m_arrUsage[nBk-1]++;

				wrdEntry.m_ndxMapping.push_back(MakeIndex(nBk, nChp, nVrs, nWrd));
				++nNormalizedNdx;
				wrdEntry.m_ndxNormalized.push_back(nNormalizedNdx);

				g_NormalizationVerification.push_back(MakeIndex(nBk, nChp, nVrs, nWrd));
			}
			pTemp = strpbrk(pTemp2, g_strCharset);
		}		// Here, nWrd = Number of words in this verse
		countWrd[nBk-1] += nWrd;			// Add in the number of words we found

		if ((bDoingBook) && ((nBkNdx == 0) || (nBk == nBkNdx))) {
			fprintf(fileOut, "%d,%d,%d,\"%s\",\"%s\",\"%s\"\r\n", nChp*256+nVrs, nWrd, (bIsPilcrow ? 1 : 0), buffPlain, buffRich, buffFootnote);
		}

	}

	if (bDoingLayout) {
		if ((nChp != 0) && (nBk != 0)) {			// Output our final entry if one was still pending
			fprintf(fileOut, "%d,%d,%d,%s,%d\r\n", nBk*256+nChp, countVrs[nBk-1], countWrd[nBk-1], g_arrstrBkAbbr[nBk-1], nChp);
		}
	}

	if (bDoingWordDumpKeys) {
		for (TAltWordListMap::const_iterator itrUniqWrd = g_mapAltWordList.begin(); itrUniqWrd != g_mapAltWordList.end(); ++itrUniqWrd) {
			const TAltWordSet &setAltWords = itrUniqWrd->second;
			fprintf(fileOut, "%s\r\n", WordFromWordSet(setAltWords).c_str());
		}
	}

	if (bDoingWordDumpUnique) {
		for (TWordListMap::const_iterator itr = g_mapWordList.begin(); itr != g_mapWordList.end(); ++itr) {
			fprintf(fileOut, "%s\r\n", itr->first.c_str());
		}
	}

	if (bDoingWordDumpUniqueLC) {
		for (TAltWordListMap::const_iterator itr = g_mapAltWordList.begin(); itr != g_mapAltWordList.end(); ++itr) {
			fprintf(fileOut, "%s\r\n", itr->first.c_str());
		}
	}

	if (bDoingAltWordDump) {
		for (TAltWordListMap::const_iterator itrUniqWrd = g_mapAltWordList.begin(); itrUniqWrd != g_mapAltWordList.end(); ++itrUniqWrd) {
			const TAltWordSet &setAltWords = itrUniqWrd->second;
			std::string strAltWords;

			if (setAltWords.size() < 2) continue;

			for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
				if (!strAltWords.empty()) strAltWords += ",";
				strAltWords += *itrAltWrd;
			}
			fprintf(fileOut, "%s %s\r\n", WordFromWordSet(setAltWords).c_str(), strAltWords.c_str());
		}
	}

	if (bDoingWords) {
		fprintf(fileOut, "WrdNdx,Word,bIndexCasePreserve,NumTotal,AltWords,AltWordCounts,Mapping,NormalMap\r\n");
		nWrd = 0;
		for (TAltWordListMap::const_iterator itrUniqWrd = g_mapAltWordList.begin(); itrUniqWrd != g_mapAltWordList.end(); ++itrUniqWrd) {
			const TAltWordSet &setAltWords = itrUniqWrd->second;
			std::string strAltWords;
			std::string strAltWordCounts;
			std::string strMapping;
			std::string strNormalMap;
			unsigned int nIndexCount = 0;
			bool bPreserve = false;

			for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
				TWordListMap::const_iterator itrWrd = g_mapWordList.find(*itrAltWrd);
				if (itrWrd == g_mapWordList.end()) {
					fprintf(fileOut, "%s -> %s -- Couldn't find it (something bad happened!)\r\n", itrUniqWrd->first.c_str(), (*itrAltWrd).c_str());
					continue;
				}

				if (!strAltWords.empty()) strAltWords += ",";
				strAltWords += *itrAltWrd;
				if (!strAltWordCounts.empty()) strAltWordCounts += ",";
				sprintf(word, "%d", itrWrd->second.m_ndxMapping.size());
				strAltWordCounts += word;

				if (isSpecialWord(*itrAltWrd)) bPreserve = true;

				for (TIndexList::const_iterator itrNdx = itrWrd->second.m_ndxMapping.begin();
						itrNdx != itrWrd->second.m_ndxMapping.end(); ++itrNdx) {
					if (!strMapping.empty()) strMapping += ",";
					sprintf(word, "%d", *itrNdx);
					strMapping += word;
				}
				for (TIndexList::const_iterator itrNdx = itrWrd->second.m_ndxNormalized.begin();
						itrNdx != itrWrd->second.m_ndxNormalized.end(); ++itrNdx) {
					if (!strNormalMap.empty()) strNormalMap += ",";
					sprintf(word, "%d", *itrNdx);
					strNormalMap += word;
				}

				nIndexCount += itrWrd->second.m_ndxMapping.size();
			}
			++nWrd;
			fprintf(fileOut, "%d,\"%s\",%d,%d,\"%s\",\"%s\",\"%s\",\"%s\"\r\n",
								nWrd,
								WordFromWordSet(setAltWords).c_str(),
								(bPreserve ? 1 : 0),
								nIndexCount,
								strAltWords.c_str(),
								strAltWordCounts.c_str(),
								strMapping.c_str(),
								strNormalMap.c_str());
		}
		for (int i=0; i<(g_NormalizationVerification.size()-1); ++i) {
			if (g_NormalizationVerification[i+1] <= g_NormalizationVerification[i]) {
				fprintf(stderr, "Normalization Verification Failure: %d=%d, %d=%d\n", i, g_NormalizationVerification[i], i+1, g_NormalizationVerification[i+1]);
			}
		}
	}

	if (bDoingSummary) {
		fprintf(fileOut, "\"Word\",\"AltWords\",\"Whole\nBible\",\"Old\nTestament\",\"New\nTestament\"");
		for (nBk=0; nBk<NUM_BK; ++nBk) {
			fprintf(fileOut, ",\"%s\"", g_arrstrBkNames[nBk]);
		}
		fprintf(fileOut, "\r\n");

		for (TAltWordListMap::const_iterator itrUniqWrd = g_mapAltWordList.begin(); itrUniqWrd != g_mapAltWordList.end(); ++itrUniqWrd) {
			const TAltWordSet &setAltWords = itrUniqWrd->second;
			std::string strAltWords;
			int nOTCount = 0;
			int nNTCount = 0;
			int arrUsage[NUM_BK];
			unsigned int nIndexCount = 0;

			for (nBk=0; nBk<NUM_BK; ++nBk)
				arrUsage[nBk] = 0;

			for (TAltWordSet::const_iterator itrAltWrd = setAltWords.begin(); itrAltWrd != setAltWords.end(); ++itrAltWrd) {
				TWordListMap::const_iterator itrWrd = g_mapWordList.find(*itrAltWrd);
				if (itrWrd == g_mapWordList.end()) {
					fprintf(fileOut, "%s -> %s -- Couldn't find it (something bad happened!)\r\n", itrUniqWrd->first.c_str(), (*itrAltWrd).c_str());
					continue;
				}

				if (!strAltWords.empty()) strAltWords += ",";
				strAltWords += *itrAltWrd;

				for (nBk=0; nBk<NUM_BK; ++nBk) {
					arrUsage[nBk] += itrWrd->second.m_arrUsage[nBk];
					if (nBk < NUM_BK_OT) {
						nOTCount += itrWrd->second.m_arrUsage[nBk];
					} else {
						nNTCount += itrWrd->second.m_arrUsage[nBk];
					}
				}

				nIndexCount += itrWrd->second.m_ndxMapping.size();
			}

			fprintf(fileOut, "\"%s\",\"%s\",%d,%d,%d",
								WordFromWordSet(setAltWords).c_str(),
								strAltWords.c_str(),
								nIndexCount,
								nOTCount,
								nNTCount);
			for (nBk=0; nBk<NUM_BK; ++nBk) {
				fprintf(fileOut, ",%d", arrUsage[nBk]);
			}
			fprintf(fileOut, "\r\n");
		}
	}

#if (PILCROW_SUMMARY)
	fprintf(stderr, "Pilcrows:  Plain: %d,  Rich: %d,  Total: %d\n", nPilcrowPlain, nPilcrowRich, nPilcrowTotal);
#endif

	if ((fileIn != NULL) && (fileIn != stdin)) fclose(fileIn);
	if ((fileOut != NULL) && (fileOut != stdout)) fclose(fileOut);

	return 0;
}

