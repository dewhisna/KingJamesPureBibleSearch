/******************************************************************************
 * This example shows how to output the plain text entries from a SWORD module.
 * This small program outputs a SWORD module in 'imp' format, e.g.,
 *
 * $$$Gen.1.1
 * In the beginning God created
 * the heavens and the earth
 *
 * $$$Gen.1.2
 * ...
 *
 * Class SWMgr manages installed modules for a frontend.
 * The developer may use this class to query what modules are installed
 * and to retrieve an (SWModule *) for any one of these modules
 *
 * SWMgr makes its modules available as an STL Map.
 * The Map definition is typedef'ed as ModMap
 * ModMap consists of: FIRST : SWBuf moduleName
 *                     SECOND: SWModule *module
 *
 * $Id: swmgr.h 2321 2009-04-13 01:17:00Z scribe $
 *
 * Copyright 1998-2009 CrossWire Bible Society (http://www.crosswire.org)
 *	CrossWire Bible Society
 *	P. O. Box 2528
 *	Tempe, AZ  85280-2528
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include <iostream>

#include <swmgr.h>
#include <swmodule.h>
#include <versekey.h>
#include <markupfiltmgr.h>

using namespace sword;
using namespace std;

int main(int argc, char **argv) {
	int bDoPlain = 0;
	int bDoRich = 0;
	int bNeedUsage = 0;

//	SWMgr manager(new MarkupFilterMgr(sword::FMT_HTMLHREF, sword::ENC_UTF16));
//	SWMgr manager(new MarkupFilterMgr(sword::FMT_WEBIF, sword::ENC_UTF16));
	SWMgr manager(new MarkupFilterMgr(sword::FMT_HTMLHREF));

	if (argc < 2) {
		bNeedUsage = 1;
	} else if (argc == 2) {
		// Default to doing both:
		bDoPlain = 1;
		bDoRich = 1;
	} else {
		if ((stricmp(argv[2], "plain") == 0) || (stricmp(argv[2], "both") == 0)) {
			bDoPlain = 1;
		}
		if ((stricmp(argv[2], "rich") == 0) || (stricmp(argv[2], "both") == 0)) {
			bDoRich = 1;
		}
		if ((bDoPlain == 0) && (bDoRich == 0)) bNeedUsage = 1;
	}

	if (bNeedUsage) {
		cout << "Usage: outplain <bible> [<mode>]\n\n";
		cout << "   Where <mode> is:\n";
		cout << "         plain = plain text only\n";
		cout << "         rich = rich text only\n";
		cout << "         both = both plain and rich (default)\n";
		cout << "\n";
		return -1;
	}

	const char *bookName = (argc > 1) ? argv[1] : "WLC";
	SWModule *b = manager.getModule(bookName);
	if (!b) return -1;
	SWModule &book = *b;
	book.processEntryAttributes(false);
	VerseKey *vk = SWDYNAMIC_CAST(VerseKey, book.getKey());
	for (book = TOP; !book.Error() && !book.getRawEntryBuf().size(); book++);
	if (!book.getRawEntryBuf().size()) return -2; 	// empty module
	for (;!book.Error(); book++) {
		cout << "$$$";
		if (vk) cout << vk->getOSISRef();
		else    cout << book.getKeyText();
		cout << "\n";
//		cout << book.StripText() << "\n";
		book.RenderText();
		if (bDoPlain) {
			cout << "@" << book.StripText() << "@\n";
		} else {
			cout << "@@\n";
		}
		if (bDoRich) {
			cout << "@" << book.RenderText() << "@\n";
		} else {
			cout << "@@\n";
		}
	}

	return 0;
}
