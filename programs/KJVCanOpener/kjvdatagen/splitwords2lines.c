//
// splitwords2lines.c
//
//	This program reads text from <stdin>, finds words, splits them into
//		individual lines.  Supports UTF-8

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define FALSE (0)
#define TRUE (!FALSE)
#ifndef BOOL
#define BOOL int
#endif

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

void fputcUTF8(int c, FILE *file)
{
	int c2;
	char buff[10];
	int n;

	if (c < 0x80) {
		fputc(c, file);
		return;
	}

	buff[6] = 0;
	n = 5;
	c2 = 0x0FF80;
	while (c) {
		buff[n] = (c & 0x3f) | 0x80;
		c = c >> 6;
		c2 = c2 >> 1;
		if (c < (2^n)) {
			c = ((c2 & 0xFF) | c);
			n--;
			buff[n] = c;
			c = 0;
		}
	}
	fprintf(file, "%s", &buff[n]);
}

int main(int argc, const char *argv[])
{
	int c;
	int n = 0;

	while (!feof(stdin)) {
		c = fgetcUTF8(stdin);
		if (c == EOF) break;

		if (c == 0x2013) {				// Translate "en dash" to (-)
			fputcUTF8('-', stdout);
			continue;
		}
		if (c == 0x2015) {				// Translate "Horizontal Bar" to (--) TODO : Figure out what to do with this one!
			fputcUTF8('-', stdout);
			fputcUTF8('-', stdout);
			continue;
		}
		if (c == 0xe6) {				// Translate "æ" to "ae":
			fputcUTF8('a', stdout);
			fputcUTF8('e', stdout);
			continue;
		}
		if (c == 0xc6) {				// Translate "Æ" to "Ae":
			fputcUTF8('A', stdout);
			fputcUTF8('e', stdout);
			continue;
		}
		if (c > 127) {
			fprintf(stderr, "Unexpected UTF-8 symbol (%04lx) at position %d [0x%04lx]\n", c, ftell(stdin), ftell(stdin));
		}
		if (strchr(g_strCharset, c) != NULL) {
			fputcUTF8(c, stdout);
			n++;
		} else {
			if (n) {
				fputcUTF8('\n', stdout);
				n=0;
			}
		}
	}
	if (n) fputcUTF8('\n', stdout);

	return 0;
}

