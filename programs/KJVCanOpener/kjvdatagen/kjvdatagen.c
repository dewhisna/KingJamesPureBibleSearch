//
// KJVDataGen.c
//
//	Generates the Bk/Chp Keys and Data for the KJVCanOpener LAYOUT Table,
//				Chp/Vrs Keys and Data for individual KJVCanOpener BOOK Tables,
//				Word data for the WORDS Table,
//				Word mapping indices for the MAP Table,
//				and can create word and book dumps.
//
//	Reads a specialized (with '@' symbols) Sword OutPlain dump file with OSIS
//		Key names and text from <stdin>.  Sends the following to <stdout>
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

#define FALSE (0)
#define TRUE (!FALSE)
#ifndef BOOL
#define BOOL int
#endif

// PILCROW_SUMMARY : Set to 1 to output Pilcrow report summary to <stderr> or 0 to suppress
#define PILCROW_SUMMARY 0
// OUTPUT_HEBREW_PS119 : Set to 1 to output Hebrew characters in addition to English for Psalm119 in Rich text
#define OUTPUT_HEBREW_PS119 1

#define NUM_BK 66
#define NUM_BK_OT 39
#define NUM_BK_NT 27

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

const char *g_strCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'-";		// Accept: [alphanumeric, -, '], we'll handle UTF-8 conversion and translate those to ASCII as appropriate

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
	int m;

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

int main(int argc, const char *argv[])
{
	char buffPlain[10000];		// Plain Text version
	char buffRich[10000];		// Rich Text version
	char buffFootnote[2000];	// Footnote Text
	char word[50];
	int countChp[NUM_BK];		// # of Chapters in books
	int countVrs[NUM_BK];		// # of Verses in entire book for Book mode or # of Verses in current Chapter for Layout mode
	int countWrd[NUM_BK];		// # of Words in entire book for Book mode or # of Words in current Chapter for Layout mode
	BOOL bDoingLayout = FALSE;		// TRUE if outputing LAYOUT table
	BOOL bDoingBook = FALSE;		// TRUE if outputing BOOK table (nBkNdx = book to output)
	BOOL bDoingWords = FALSE;		// TRUE if outputing WORDS table
	BOOL bDoingWordDump = FALSE;	// TRUE if dumping words used (nBkNdx = book to output or 0 for all)
	BOOL bDoingBookDump = FALSE;	// TRUE if dumping book content (nBkNdx = book to output or 0 for all)
	BOOL bNeedUsage = FALSE;		// TRUE if user needs usage info
	int nBkNdx = 0;		// Index of Book to Output (Book Mode)
	int nCurBk = 0;		// Current book output (Layout Mode)
	int nCurChp = 0;	// Current chapter output (Layout Mode)
	int nBk;			// Book counter
	int nChp;			// Chapter counter
	int nVrs;			// Verse counter
	int nWrd;			// Word counter
	int ndx;			// Index into buffer;
	BOOL bIsPilcrow;		// Set to TRUE if the current verse is a Pilcrow (¶) start point, else is FALSE (either Plain/Rich)
	BOOL bIsPilcrowPlain;	// Set to TRUE if the current verse is a Pilcrow (¶) start point, else is FALSE (in Plain Text)
	BOOL bIsPilcrowRich;	// Set to TRUE if the current verse is a Pilcrow (¶) start point, else is FALSE (in Rich Text)
	BOOL bPilcrowMismatch;	// Set to TRUE if the plain specifies a pilcrow and the rich doesn't or vice versa
	int nPilcrowPlain = 0;	// Number of Pilcrows in Plain
	int nPilcrowRich = 0;	// Number of Pilcrows in Rich
	int nPilcrowTotal = 0;	// Total Number of Pilcrows
	int i;
	int c;				// Character from input file
	char *pTemp;
	char *pTemp2;

	if (argc < 2) {
		bNeedUsage = TRUE;
	} else if (stricmp(argv[1], "layout") == 0) {
		bDoingLayout = TRUE;
	} else if (stricmp(argv[1], "book") == 0) {
		bDoingBook = TRUE;
		if (argc < 3) bNeedUsage = TRUE;		// Must have book name for this one
	} else if (stricmp(argv[1], "words") == 0) {
		bDoingWords = TRUE;
	} else if (stricmp(argv[1], "worddump") == 0) {
		bDoingWordDump = TRUE;
	} else if (stricmp(argv[1], "bookdump") == 0) {
		bDoingBookDump = TRUE;
	} else {
		bNeedUsage = TRUE;
	}

	if (argc > 2) {
		for (nBk = 0; nBk < NUM_BK; ++nBk) {
			if (stricmp(g_arrstrBkAbbr[nBk], argv[2]) == 0) {
				nBkNdx = nBk + 1;
				break;
			}
		}
		if (nBkNdx == 0) {
			fprintf(stderr, "Invalid book name.  Valid books are:\n\n");
			for (nBk = 0; nBk < NUM_BK; ++nBk) {
				fprintf(stderr, "    %s\n", g_arrstrBkAbbr[nBk]);
			}
			return -1;
		}
	}
	// By here, nBkNum = Book number to output, or 0 for all books on modes that allow it

	if (bNeedUsage) {
		fprintf(stderr, "Usage:  KJVDataGen <mode> [<bookname>]\n\n");
		fprintf(stderr, "  Where <mode> is one of:\n\n");
		fprintf(stderr, "      layout   -- Dump LAYOUT table data (book name not used)\n");
		fprintf(stderr, "      book     -- Dump BOOK table data for the specified book\n");
		fprintf(stderr, "      words    -- Dump WORDS table data (book name not used)\n");
		fprintf(stderr, "      worddump -- Dumps all words (book name optional)\n");
		fprintf(stderr, "      bookdump -- Dumps the content of specified book (Verse per line)\n");
		fprintf(stderr, "\n\n");
		fprintf(stderr, "  I/O is from/to stdin/stdout.  Input should be specialized\n");
		fprintf(stderr, "  Sword outplain dump file.\n");
		return -1;
	}

/////////////////////////////////////
// Preprocessing:
/////////////////////////////////////

	if (bDoingBook) {
		// Excel will normally try to treat the file as Latin-1, but since we
		//		have some embedded UTF-8, we need to output a BOM:
		fputcUTF8(0x0FEFF, stdout);
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
	while (!feof(stdin)) {

/////////////////////////////////////
// Find and Parse Bk/Chp/Vrs Header
/////////////////////////////////////

		for (i=0; i<3; ++i) {		// looking for 3 '$' symbols
			do {
				c = fgetcUTF8(stdin);
			} while ((c != '$') && (c != EOF));
			if (c == EOF) break;
		}
		if (c == EOF) break;

		fgets(buffPlain, sizeof(buffPlain), stdin);	// Note: The reference shouldn't be UTF-8, so we can just use fgets
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
			c = fgetcUTF8(stdin);
		} while ((c != '@') && (c != EOF));
		if (c == EOF) break;

		// Read and fill buffPlain with verse text up to final "@" marker:
		bIsPilcrowPlain = FALSE;
		ndx = 0;
		do {
			c = fgetcUTF8(stdin);
			if ((c != '@') && (c != EOF)) {
				if (c == 0x0a) {
					if (ndx > 0) break;			// A newline after text means footnote marker!  So skip the rest!!
					bIsPilcrowPlain = TRUE;		// A newline at the start denotes a Pilcrow (¶) mark
					continue;					// We set the flag, but we'll exclude writing the newline to the output
				}
				if (c == 0xb6) {				// Translate Pilcrow
					bIsPilcrowPlain = TRUE;
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
					fprintf(stderr, "Unexpected UTF-8 symbol in Plain Text (%04lx) at position %d [0x%04lx] in file: %s %0d.%0d [%0d]\n", c, ftell(stdin), ftell(stdin), g_arrstrBkAbbr[nBk-1], nChp, nVrs, ndx);
				}
				buffPlain[ndx] = c;
				ndx++;
			}
		} while ((!feof(stdin)) && (ndx < sizeof(buffPlain)-1) && (c != '@'));
		buffPlain[ndx] = 0;			// End main text
		if (c != '@') {				// If we didn't find the end marker, we must be in a footnote
			ndx = 0;
			do {
				c = fgetcUTF8(stdin);
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
						fprintf(stderr, "Unexpected UTF-8 symbol in Footnote Text (%04lx) at position %d [0x%04lx] in file: %s %0d.%0d [%0d]\n", c, ftell(stdin), ftell(stdin), g_arrstrBkAbbr[nBk-1], nChp, nVrs, ndx);
					}
					buffFootnote[ndx] = c;
					ndx++;
				}
			} while ((!feof(stdin)) && (ndx < sizeof(buffFootnote)-1) && (c != '@'));
			buffFootnote[ndx] = 0;			// End footnote
		}

/////////////////////////////////////
// Find and Parse Rich Text
/////////////////////////////////////

		do {							// Find "@" marker
			c = fgetcUTF8(stdin);
		} while ((c != '@') && (c != EOF));
		if (c == EOF) break;

		// Read and fill buffRich with verse text up to final "@" marker:
		bIsPilcrowRich = FALSE;
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
		}

		do {
			c = fgetcUTF8(stdin);
			if ((c != '@') && (c != EOF)) {
				if (ndx == 0) {
					if (c == 0xb6) {				// Translate Pilcrow (but only at start of verse!)
						bIsPilcrowRich = TRUE;
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
//					fprintf(stderr, "Unexpected UTF-8 symbol in Rich Text (%04lx) at position %d [0x%04lx] in file: %s %0d.%0d [%0d]\n", c, ftell(stdin), ftell(stdin), g_arrstrBkAbbr[nBk-1], nChp, nVrs, ndx);
//				} else {
//					buffRich[ndx] = c;
//					ndx++;
//				}

				ndx += sputcUTF8(c, &buffRich[ndx]);	// UTF8 transfer character

			}
		} while ((!feof(stdin)) && (ndx < sizeof(buffRich)-1) && (c != '@'));
		buffRich[ndx] = 0;

/////////////////////////////////////
// Parse and Write Output File
/////////////////////////////////////

		bIsPilcrow = (bIsPilcrowPlain || bIsPilcrowRich);
		bPilcrowMismatch = ((bIsPilcrowPlain != bIsPilcrowRich) ? TRUE : FALSE);

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
				fprintf(stdout, "%d,%d,%d,%s,%d\n", nCurBk*256+nCurChp, countVrs[nCurBk-1], countWrd[nCurBk-1], g_arrstrBkAbbr[nCurBk-1], nCurChp);
				countVrs[nCurBk-1] = 0;				// For Layout mode, these are counts for the chapter only, so reset them
				countWrd[nCurBk-1] = 0;
				nCurBk = nBk;
				nCurChp = nChp;
			}
		}

		if ((bDoingBookDump) && ((nBkNdx == 0) || (nBk == nBkNdx))) {
			fprintf(stdout, "%s\n", buffPlain);
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
			if ((bDoingWordDump) && ((nBkNdx == 0) || (nBk == nBkNdx))) fprintf(stdout, "%s\n", word);
			pTemp = strpbrk(pTemp2, g_strCharset);
		}		// Here, nWrd = Number of words in this verse
		countWrd[nBk-1] += nWrd;			// Add in the number of words we found

		if ((bDoingBook) && ((nBkNdx == 0) || (nBk == nBkNdx))) {
			fprintf(stdout, "%d,%d,%d,\"%s\",\"%s\",\"%s\"\n", nChp*256+nVrs, nWrd, (bIsPilcrow ? 1 : 0), buffPlain, buffRich, buffFootnote);
		}

	}

	if (bDoingLayout) {
		if ((nChp != 0) && (nBk != 0)) {			// Output our final entry if one was still pending
			fprintf(stdout, "%d,%d,%d,%s,%d\n", nBk*256+nChp, countVrs[nBk-1], countWrd[nBk-1], g_arrstrBkAbbr[nBk-1], nChp);
		}
	}

#if (PILCROW_SUMMARY)
	fprintf(stderr, "Pilcrows:  Plain: %d,  Rich: %d,  Total: %d\n", nPilcrowPlain, nPilcrowRich, nPilcrowTotal);
#endif

	return 0;
}

