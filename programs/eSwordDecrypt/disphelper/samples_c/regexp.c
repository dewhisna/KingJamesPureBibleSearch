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
 * You can browse the source code and samples, including a C++ version of this sample, online at:
 * http://cvs.sourceforge.net/viewcvs.py/disphelper/disphelper/
 */


/* --
regexp.c:
  Demonstrates using the VBScript.RegExp object to provide support for regular
expressions. Provides a function to extract hrefs from a web page using a
regular expression.
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>

#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }


/* **************************************************************************
 * RegExp:
 *   Executes a regular expression and prints out matches.
 *
 ============================================================================ */
void RegExp(LPCTSTR szPattern, LPCTSTR szString, BOOL bIgnoreCase)
{
	DISPATCH_OBJ(regEx);

	HR_TRY( dhCreateObject(L"VBScript.RegExp", NULL, &regEx) );

	HR_TRY( dhPutValue(regEx, L".Pattern = %T", szPattern) );
	HR_TRY( dhPutValue(regEx, L".IgnoreCase = %b", bIgnoreCase) );
	HR_TRY( dhPutValue(regEx, L".Global = %b", TRUE) );

	FOR_EACH1(match, regEx, L".Execute(%T)", szString)
	{
		INT nFirstIndex = 0, nLength = 0;
		LPWSTR szValue = NULL;

		dhGetValue(L"%d", &nFirstIndex, match, L".FirstIndex");
		dhGetValue(L"%d", &nLength,     match, L".Length");
		dhGetValue(L"%S", &szValue,     match, L".Value");

		wprintf(L"Match found at characters %d to %d. Match value is '%s'.\n",
		            nFirstIndex, nFirstIndex + nLength, szValue);

		dhFreeString(szValue);

	} NEXT(match);

cleanup:
	SAFE_RELEASE(regEx);
}


/* **************************************************************************
 * Replace:
 *   Performs a regular expression replace and prints out the result.
 *
 ============================================================================ */
void Replace(LPCTSTR szPattern, LPCTSTR szString, LPCTSTR szReplacement, BOOL bIgnoreCase)
{
	DISPATCH_OBJ(regEx);
	LPWSTR szResult = NULL;

	HR_TRY( dhCreateObject(L"VBScript.RegExp", NULL, &regEx) );
	HR_TRY( dhPutValue(regEx, L".Pattern = %T", szPattern) );
	HR_TRY( dhPutValue(regEx, L".IgnoreCase = %b", bIgnoreCase) );
	HR_TRY( dhPutValue(regEx, L".Global = %b", TRUE) );

	dhGetValue(L"%S", &szResult, regEx, L".Replace(%T, %T)", szString, szReplacement);

	wprintf(L"Text after replacement is '%s'\n", szResult);

	dhFreeString(szResult);

cleanup:
	SAFE_RELEASE(regEx);
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
	DISPATCH_OBJ(regEx);
	DISPATCH_OBJ(objHTTP);
	LPWSTR szResponse = NULL;
	const WCHAR * FIND_HREFS_PATTERN = L"[^\\.]href\\s*=\\s*[\"']?([^\"'\\s>]+)";

	/* Download web page */
	HR_TRY( dhCreateObject(L"MSXML2.XMLHTTP", NULL, &objHTTP) );
	HR_TRY( dhCallMethod(objHTTP, L".Open(%S, %T, %b)", L"GET", szURL, FALSE) );
	HR_TRY( dhCallMethod(objHTTP, L".Send") );
	HR_TRY( dhGetValue(L"%S", &szResponse, objHTTP, L".ResponseText") );

	/* Set up regular expression */
	HR_TRY( dhCreateObject(L"VBScript.RegExp", NULL, &regEx) );
	HR_TRY( dhPutValue(regEx, L".Pattern = %S", FIND_HREFS_PATTERN) );
	HR_TRY( dhPutValue(regEx, L".IgnoreCase = %b", TRUE) );
	HR_TRY( dhPutValue(regEx, L".Global = %b", TRUE) );

	FOR_EACH1(match, regEx, L".Execute(%S)", szResponse)
	{
		LPWSTR szHref = NULL;

		/* A sub match is created by enclosing part of the regex in brackets */
		dhGetValue(L"%S", &szHref, match, L".SubMatches(%d)", 0);
		wprintf(L"%s\n\n", szHref);
		dhFreeString(szHref);

	} NEXT(match);

cleanup:
	dhFreeString(szResponse);
	SAFE_RELEASE(regEx);
	SAFE_RELEASE(objHTTP);
}


/* ============================================================================ */
int main(void)
{
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);

	printf("Running regular expression find sample...\n");
	RegExp(TEXT("is."), TEXT("IS1 is2 IS3 is4"), TRUE);

	printf("\nRunning regular expression replace sample...\n");
	Replace(TEXT("fox"), TEXT("The quick brown fox jumped over the lazy dog."), TEXT("cat"), TRUE);

	printf("\nRunning GetHrefs sample...\n");
	ExtractHREFs(TEXT("http://www.newscientist.com"));

	printf("\nPress ENTER to exit...\n");
	getchar();

	dhUninitialize(TRUE);
	return 0;
}




