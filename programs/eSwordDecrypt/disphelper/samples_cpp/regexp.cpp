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
regexp.cpp:
  Demonstrates using the VBScript.RegExp object to provide support for regular
expressions. Provides a function to extract hrefs from a web page using a
regular expression.
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
using namespace std;


/* **************************************************************************
 * RegExp:
 *   Executes a regular expression and prints out matches.
 *
 ============================================================================ */
void RegExp(LPCTSTR szPattern, LPCTSTR szString, BOOL bIgnoreCase)
{
	CDispPtr regEx;

	try
	{
		dhCheck( dhCreateObject(L"VBScript.RegExp", NULL, &regEx) );

		dhCheck( dhPutValue(regEx, L".Pattern = %T", szPattern) );
		dhCheck( dhPutValue(regEx, L".IgnoreCase = %b", bIgnoreCase) );
		dhCheck( dhPutValue(regEx, L".Global = %b", TRUE) );

		FOR_EACH1(match, regEx, L".Execute(%T)", szString)
		{
			INT nFirstIndex = 0, nLength = 0;
			CDhStringA szValue;

			dhGetValue(L"%d", &nFirstIndex, match, L".FirstIndex");
			dhGetValue(L"%d", &nLength,     match, L".Length");
			dhGetValue(L"%s", &szValue,     match, L".Value");

			cout << "Match found at characters " 
			     << nFirstIndex << " to " << (nFirstIndex + nLength)
			     << ". Match value is '"  << szValue << "'." << endl;

		} NEXT_THROW(match);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * Replace:
 *   Performs a regular expression replace and prints out the result.
 *
 ============================================================================ */
void Replace(LPCTSTR szPattern, LPCTSTR szString, LPCTSTR szReplacement, BOOL bIgnoreCase)
{
	CDispPtr regEx;
	CDhStringA szResult;

	try
	{
		dhCheck( dhCreateObject(L"VBScript.RegExp", NULL, &regEx) );
		dhCheck( dhPutValue(regEx, L".Pattern = %T", szPattern) );
		dhCheck( dhPutValue(regEx, L".IgnoreCase = %b", bIgnoreCase) );
		dhCheck( dhPutValue(regEx, L".Global = %b", TRUE) );
		dhCheck( dhGetValue(L"%s", &szResult, regEx, L".Replace(%T, %T)", szString, szReplacement) );

		cout << "Text after replacement is '" << szResult << "'." << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * ExtractHREFs:
 *   Extract hrefs from a web page using a regular expression.
 * The regular expression will truncate a URL with unescaped
 * spaces(malformed by defintion) but should work for most other well formed
 * or mal formed hrefs.
 *
 ============================================================================ */
void ExtractHREFs(LPCTSTR szURL)
{
	CDispPtr regEx, objHTTP;
	CDhStringW szResponse;
	const WCHAR * FIND_HREFS_PATTERN = L"[^\\.]href\\s*=\\s*[\"']?([^\"'\\s>]+)";

	try
	{
		/* Download web page */
		dhCheck( dhCreateObject(L"MSXML2.XMLHTTP", NULL, &objHTTP) );
		dhCheck( dhCallMethod(objHTTP, L".Open(%S, %T, %b)", L"GET", szURL, FALSE) );
		dhCheck( dhCallMethod(objHTTP, L".Send") );
		dhCheck( dhGetValue(L"%S", &szResponse, objHTTP, L".ResponseText") );

		/* Set up regular expression */
		dhCheck( dhCreateObject(L"VBScript.RegExp", NULL, &regEx) );
		dhCheck( dhPutValue(regEx, L".Pattern = %S", FIND_HREFS_PATTERN) );
		dhCheck( dhPutValue(regEx, L".IgnoreCase = %b", TRUE) );
		dhCheck( dhPutValue(regEx, L".Global = %b", TRUE) );

		FOR_EACH1(match, regEx, L".Execute(%S)", (LPWSTR) szResponse)
		{
			CDhStringA szHref;

			/* A sub match is created by enclosing part of the regex in brackets */
			dhGetValue(L"%s", &szHref, match, L".SubMatches(%d)", 0);

			cout << szHref << endl << endl;

		} NEXT_THROW(match);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* ============================================================================ */
int main(void)
{
	CDhInitialize init;
	dhToggleExceptions(TRUE);

	cout << "Running regular expression find sample..." << endl;
	RegExp(TEXT("is."), TEXT("IS1 is2 IS3 is4"), TRUE);

	cout << "\nRunning regular expression replace sample..." << endl;
	Replace(TEXT("fox"), TEXT("The quick brown fox jumped over the lazy dog."), TEXT("cat"), TRUE);

	cout << "\nRunning GetHrefs sample..." << endl;
	ExtractHREFs(TEXT("http://www.newscientist.com"));

	cout << "\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}




