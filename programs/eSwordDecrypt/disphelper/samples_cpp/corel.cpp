/* This file contains sample code that demonstrates use of the DispHelper COM helper library.
 * DispHelper allows you to call COM objects with an extremely simple printf style syntax.
 * DispHelper can be used from C++ or even plain C. It works with most Windows compilers
 * including Dev-CPP, Visual C++ and LCC-WIN32. Including DispHelper in your project
 * couldn't be simpler as it is available in a compacted single file version.
 *
 * Included with DispHelper are over 20 samples that demonstrate using COM objects
 * including ADO, CDO, Outlook, Eudora, Excel, Word, Internet Explorer, MSHTML,
 * PocketSoap, Word Perfect, MS Agent, SAPI, MSXML, WIA, dexplorer and WMI.
 *
 * DispHelper is free open source software provided under the BSD license.
 *
 * Find out more and download DispHelper at:
 * http://sourceforge.net/projects/disphelper/
 * http://disphelper.sourceforge.net/
 *
 * You can browse the source code and samples, including a C version of this sample, online at:
 * http://cvs.sourceforge.net/viewcvs.py/disphelper/disphelper/
 */


/* --
corel.cpp:
  Demonstrates outputting formatted text to a WordPerfect document.
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
using namespace std;


/* **************************************************************************
 * Wp Sample:
 *   Demonstrates outputting text to a word perfect document.
 *
 ============================================================================ */
int WpSample(void)
{
	CDhInitialize init;
	CDispPtr wp;

	dhToggleExceptions(TRUE);

	try
	{
		dhCheck( dhCreateObject(L"WordPerfect.PerfectScript", NULL, &wp) );

		dhCallMethod(wp, L".AppMaximize");

		dhCallMethod(wp, L".AttributeAppearanceOn(%d)", 12); /* Turn bold on. */
		dhCallMethod(wp, L".FontSize(%d)", 500);             /* WordPerfect units. */
		dhCallMethod(wp, L".TextColor(%S)", L"Red");

		dhCallMethod(wp, L".Type(%S)", L"DispHelper WordPerfect Sample!");

		dhCallMethod(wp, L".AttributeAppearanceOff(%d)", 12); /* Turn bold off. */
		dhCallMethod(wp, L".FontSize(%d)", 200);              /* WordPerfect units. */
		dhCallMethod(wp, L".Type(%s)", "\n\nThis sample shows how to use DispHelper to automate WordPerfect.");
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	return 0;
}


/* **************************************************************************
 * Qp Sample:
 *   This was to have been a Quattro Pro sample.
 *
 ============================================================================ */
int QpSample(void)
{
	CDhInitialize init;
	CDispPtr qp, qpApp;

	dhToggleExceptions(TRUE);

	dhCreateObject(L"QuattroPro.Application", NULL, &qpApp);

	/* Unfortunately, this line gives:
	 * Object doesn't support this property or method: 'Visible'
	 * which is why this sample is rather brief. I get the same
	 * error when I try to get the PerfectScript object on the next
	 * line. I have reproduced this in vbscript so this is not an issue
	 * with DispHelper. It works only when I use early binding in VBA or elsewhere.
	 * I am using version 11 of Quattro Pro and I have seen references of this
	 * working in versions 9 and 10 so it may be an issue with version 11. */
	dhPutValue(qpApp, L".Visible = %b", TRUE);

	dhGetValue(L"%o", &qp, qpApp, L".PerfectScript");

	dhCallMethod(qp, L".Let(%S, %S)", L"B:A2", L"qwe");

	return 0;
}


/* ============================================================================ */
int main(void)
{
	cout << "Running WordPerfect Sample..." << endl;
	WpSample();

	return 0;
}

