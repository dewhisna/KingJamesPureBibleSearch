//
// KJVDataGen.c
//
//	Generates the Bk/Chp Keys and Data for the KJVCanOpener LAYOUT Table and
//				Chp/Vrs Keys and Data for individual KJVCanOpener BOOK Tables
//
//	Reads a Sword OutPlain dump file with OSIS Key names and text from <stdin>
//		and writes the LAYOUT table (sans footnotes) to <stdout> if "layout"
//		is specified or the appropriate BOOK table if a book name is given.
//
// Input should be formatted so the OSIS reference is preceeded by "$$$" and
//	the verse text is preceeded by "@" and ended with "@".  Newline \a characters
//	within the verse text is treated as the Paragraph mark (as seen at the
//	beginning of Gen 1:3).
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
// Output Format (for LAYOUT):
//	257,50,464,""
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FALSE (0)
#define TRUE (!FALSE)
#ifndef BOOL
#define BOOL int
#endif

#define NUM_BK 66

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

const char *g_strCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-\x96\xE2\x80\x93\xE6\xC3\xA6\xB6#";		// Accept alphanumeric, -, ', #, æ, ¶, and Special x96 and E28093 UTF8 encoding of (-), and Special xE6 and C3A6 UTF8 encoding of (æ), (ugh!)

int main(int argc, const char *argv[])
{
	char buff[10000];
	char word[50];
	int countChp[NUM_BK];		// # of Chapters in books
	int countVrs[NUM_BK];		// # of Verses in entire book for Book mode or # of Verses in current Chapter for Layout mode
	int countWrd[NUM_BK];		// # of Words in entire book for Book mode or # of Words in current Chapter for Layout mode
	BOOL bDoingLayout = FALSE;
	BOOL bDoingWordDump = FALSE;
	int nBkNdx = 0;		// Index of Book to Output (Book Mode)
	int nCurBk = 0;		// Current book output (Layout Mode)
	int nCurChp = 0;	// Current chapter output (Layout Mode)
	int nBk;			// Book counter
	int nChp;			// Chapter counter
	int nVrs;			// Verse counter
	int nWrd;			// Word counter
	int ndx;			// Index into buffer;
	int i;
	int c;				// Character from input file
	char *pTemp;
	char *pTemp2;

	if (argc < 2) {
		fprintf(stderr, "Usage:  KJVDataGen <mode>\n\n");
		fprintf(stderr, "  Where <mode> is either a Bible book name or one of:\n\n");
		fprintf(stderr, "      \"layout\" -- Dump LAYOUT table data\n");
		fprintf(stderr, "      \"words\" -- Dump out all words\n\n");
		fprintf(stderr, "  I/O is from/to stdin/stdout.  Input should be special\n");
		fprintf(stderr, "  Sword outplain dump file.\n");
		return -1;
	}

	if (stricmp(argv[1], "layout") == 0) {
		bDoingLayout = TRUE;
	} else if (stricmp(argv[1], "words") == 0) {
		bDoingWordDump = TRUE;
	} else {
		for (nBk = 0; nBk < NUM_BK; ++nBk) {
			if (stricmp(g_arrstrBkAbbr[nBk], argv[1]) == 0) {
				nBkNdx = nBk + 1;
				break;
			}
		}
		if (nBkNdx == 0) {
			fprintf(stderr, "Invalid mode or book name.  Valid books are:\n\n");
			for (nBk = 0; nBk < NUM_BK; ++nBk) {
				fprintf(stderr, "    %s\n", g_arrstrBkAbbr[nBk]);
			}
			return -1;
		}
	}

	// Initialize total counts for the book
	for (nBk = 0; nBk < NUM_BK; ++nBk) {
		countChp[nBk] = 0;
		countVrs[nBk] = 0;
		countWrd[nBk] = 0;
	}

	nBk = nChp = nVrs = 0;		// Start with 0.0 and set with parsed data when parsing OSIS
	ndx = 0;
	if (bDoingLayout) {
		nCurChp = 1;				// For layout mode, pretend we're currently on chapter 1 already
		nCurBk = 1;					//	and in book 1
	}
	while (!feof(stdin)) {
		for (i=0; i<3; ++i) {		// looking for 3 '$' symbols
			do {
				c = fgetc(stdin);
			} while ((c != '$') && (c != EOF));
			if (c == EOF) break;
		}
		if (c == EOF) break;

		fgets(buff, sizeof(buff), stdin);
		nBk = nChp = nVrs = 0;
		pTemp = strchr(buff, '.');			// Find book name
		if (pTemp) {
			*pTemp = 0;
			pTemp++;
			for (nBk = 0; nBk < NUM_BK; ++nBk) {
				if (stricmp(g_arrstrBkAbbr[nBk], buff) == 0) break;
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
			fprintf(stderr, "Unable to parse: %s\n\n", buff);
			continue;
		}

		if (nChp > countChp[nBk-1]) countChp[nBk-1] = nChp;
		if (nVrs > countVrs[nBk-1]) countVrs[nBk-1] = nVrs;

		do {							// Find "@" marker
			c = fgetc(stdin);
		} while ((c != '@') && (c != EOF));
		if (c == EOF) break;

		// Read and fill buff with verse text up to final "@" marker:
		ndx = 0;
		do {
			c = fgetc(stdin);
			if ((c != '@') && (c != EOF)) {
				buff[ndx] = c;
				ndx++;
			}
		} while ((!feof(stdin)) && (ndx < sizeof(buff)-1) && (c != '@'));
		buff[ndx] = 0;

		if (bDoingLayout) {
			// Output the current one we were processing just before transitioning:
			if ((nCurBk != nBk) || (nCurChp != nChp)) {
				fprintf(stdout, "%d,%d,%d,,%s,%d\n", nCurBk*256+nCurChp, countVrs[nCurBk-1], countWrd[nCurBk-1], g_arrstrBkAbbr[nCurBk-1], nCurChp);
				countVrs[nCurBk-1] = 0;				// For Layout mode, these are counts for the chapter only, so reset them
				countWrd[nCurBk-1] = 0;
				nCurBk = nBk;
				nCurChp = nChp;
			}
		}

		// Do this after the printing because either we are on the same
		//		chp/bk in which case we didn't print the output line above
		//		and it doesn't matter on the order, or else this word count
		//		is for the next chp or bk and should count toward it instead
		//		after we've written the output for our current Chp/Bk.
//fprintf(stdout, ",,,,");
		nWrd = 0;
		pTemp = strpbrk(buff, g_strCharset);
		while (pTemp) {								// pTemp = start of word
			nWrd++;
			pTemp2 = pTemp+1;
			while (strchr(g_strCharset, *pTemp2) != NULL) ++pTemp2;		// pTemp2 = End of word (TODO : for future Word dump)
			strncpy(word, pTemp, pTemp2-pTemp);
			word[pTemp2-pTemp] = 0;
			if (bDoingWordDump) fprintf(stdout, "%s\n", word);
//fprintf(stdout, "%s;", word);
			pTemp = strpbrk(pTemp2, g_strCharset);
		}		// Here, nWrd = Number of words in this verse
		countWrd[nBk-1] += nWrd;			// Add in the number of words we found
//fprintf(stdout, ",%d\n", nWrd);
	}

	if (bDoingLayout) {
		if ((nChp != 0) && (nBk != 0)) {			// Output our final entry if one was still pending
			fprintf(stdout, "%d,%d,%d,,%s,%d\n", nBk*256+nChp, countVrs[nBk-1], countWrd[nBk-1], g_arrstrBkAbbr[nBk-1], nChp);
		}
	}


	return 0;
}

