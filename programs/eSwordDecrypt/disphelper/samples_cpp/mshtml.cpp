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
mshtml.cpp:
  Demonstrates ui-less html parsing and manipulation of the html document object
model(DOM) using MSHTML.
  Provides functions to parse html from a string, a website or a file.
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>
#include <iostream>
#include <string>
using namespace std;


#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }


/* **************************************************************************
 * WaitForHTMLDocToLoad:
 *   This function waits for an html document's readystate to equal 'complete' 
 * so that its dom can be used.
 *   You may wish to implement a timeout.
 *
 ============================================================================ */
HRESULT WaitForHTMLDocToLoad(IDispatch * pDoc)
{
	HRESULT hr;

	while (TRUE)
	{
		MSG msg;
		CDhStringW szReadyState;

		hr = dhGetValue(L"%S", &szReadyState, pDoc, L".readyState");

		if (FAILED(hr) || !szReadyState || 0 == wcsicmp(szReadyState, L"complete"))
		{
			break;
		}

		/* We must pump the message loop while the document is downloading */
		if (WAIT_TIMEOUT != MsgWaitForMultipleObjects(0, NULL, FALSE, 250, QS_ALLEVENTS))
		{
			while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	return hr;
}


/* **************************************************************************
 * HTMLParseString:
 *   This function parses a string of HTML into an IHTMLDocument. This allows
 * you to navigate and manipulate the HTML using the HTML Document Object Model.
 *
 ============================================================================ */
HRESULT HTMLParseString(LPCTSTR szHTML, IDispatch ** ppDoc)
{
	HRESULT hr;

	if (SUCCEEDED(hr = dhCreateObject(L"htmlFile", NULL, ppDoc)))
	{
		hr = dhCallMethod(*ppDoc, L".Write(%T)", szHTML);
		dhCallMethod(*ppDoc, L".Close");
		if (FAILED(hr)) SAFE_RELEASE(*ppDoc);
	}

	return hr;
}


#ifndef __GNUC__
/* MinGw doesn't declare IPersistMoniker so this will not currently compile. */

/* **************************************************************************
 * HTMLParseURL:
 *   This function downloads a web page and parses in into an IHTMLDocument.
 * The url must include the protocol, such as "http://" or "ftp://".
 *
 ============================================================================ */
HRESULT HTMLParseURL(LPCWSTR szURL, IDispatch ** ppDoc)
{
	IPersistMoniker * ppm  = NULL;
	IMoniker * pMoniker    = NULL;
	IBindCtx * pbc         = NULL;
	HRESULT hr;

	*ppDoc = NULL;

	HR_TRY( hr = CreateURLMoniker(NULL, szURL, &pMoniker) );
	HR_TRY( hr = CreateBindCtx(0, &pbc) );
	HR_TRY( hr = dhCreateObjectEx(L"htmlFile", IID_IPersistMoniker, CLSCTX_INPROC_SERVER, NULL, (void **) &ppm) );
	HR_TRY( hr = ppm->Load(TRUE, pMoniker, pbc, 0) );
	HR_TRY( hr = ppm->QueryInterface(IID_IDispatch, (void **) ppDoc) );

cleanup:
	SAFE_RELEASE(pMoniker);
	SAFE_RELEASE(ppm);
	SAFE_RELEASE(pbc);
	if (FAILED(hr)) SAFE_RELEASE(*ppDoc);

	return hr;
}
#endif


/* **************************************************************************
 * HTMLParseFile:
 *   This function loads a file and parses in into an IHTMLDocument. File
 * path must be absolute.
 *
 ============================================================================ */
HRESULT HTMLParseFile(LPCWSTR szFile, IDispatch ** ppDoc)
{
	return dhGetObject(szFile, L"htmlFile", ppDoc);
}


/* **************************************************************************
 * WalkAll:
 *   Demonstrates enumerating each element in a document and getting certain
 * properties.
 * http://msdn.microsoft.com/downloads/samples/internet/browser/walkall/default.asp
 *
 ============================================================================ */
void WalkAll(IDispatch * pDoc)
{
	struct HTMLElem {
		CDhStringA szTagName, szHref, szId, szFont;
		CDhStringA szAction, szType;
	};

	dhToggleExceptions(FALSE);

	FOR_EACH(pElem, pDoc, L".all")
	{	
		HTMLElem elem = { 0 };

		dhGetValue(L"%s", &elem.szTagName, pElem, L".tagName");
		dhGetValue(L"%s", &elem.szHref,    pElem, L".href");
		dhGetValue(L"%s", &elem.szId,      pElem, L".id");
		dhGetValue(L"%s", &elem.szFont,    pElem, L".style.font");

		if (elem.szTagName) cout << "\nTag: "  << elem.szTagName << endl;
		if (elem.szId)      cout << "ID: "     << elem.szId      << endl;
		if (elem.szHref)    cout << "HREF: "   << elem.szHref    << endl;
		if (elem.szFont && elem.szFont[0]) cout << "Font: " <<  elem.szFont << endl;

		if (elem.szTagName && 0 == stricmp("FORM", elem.szTagName))
		{
			/* If the element is a form element get its action */

			dhGetValue(L"%s", &elem.szAction, pElem, L".action");
			cout << "Action: " << elem.szAction << endl;
		}
		else if (elem.szTagName && 0 == stricmp("INPUT", elem.szTagName))
		{
			/* If the element is an input element get its type */

			dhGetValue(L"%s", &elem.szType, pElem, L".type");
			cout << "Type: " << elem.szType << endl;
		}

	} NEXT(pElem);

	dhToggleExceptions(TRUE);
}


/* **************************************************************************
 * HTML_STRING:
 *   A snippet of HTML that we use in the Manipulate() sample.
 *
 ============================================================================ */
const TCHAR * HTML_STRING = TEXT(" \
  <html><head><title>DispHelper Samples</title></head><body> \
  <table id=\"samples\"> \
  <tr><th>File Name</th><th>COM Technology</th></tr> \
  <tr><td>xml.c</td><td>MSXML XML Parser</td></tr> \
  <tr><td>wia.c</td><td>WIA Image Manipulation</td></tr> \
  <tr><td>pocketSoap.c</td><td>PocketSoap</td></tr> \
  <tr><td>ado.c</td><td>ADO Database Manipulation</td></tr> \
  </table></body></html>");


/* **************************************************************************
 * Manipulate:
 *   Demonstrates how to alter and add to an html document using the DOM.
 *
 ============================================================================ */
void Manipulate(void)
{
	CDispPtr pDoc;
	CDhStringA szHTML;

	try
	{
		dhCheck( HTMLParseString(HTML_STRING, &pDoc) );

		cout << "\nPrinting current contents of table...\n" << endl;

		/* First print out the current contents of the table by enumerating each row */
		FOR_EACH1(pRow, pDoc, L".all(%S).rows", L"samples")
		{
			CDhStringA szFile, szTech;

			dhGetValue(L"%s", &szFile, pRow, L".cells(%d).innerText", 0);
			dhGetValue(L"%s", &szTech, pRow, L".cells(%d).innerText", 1);

			cout << szFile << "\t - " << szTech << endl;

		} NEXT_THROW(pRow);

		cout << "\nAltering table and HTML document...\n" << endl;

		WITH1(pTable, pDoc, L".all(%S)", L"samples")
		{
			/* Edit a row */
			dhPutValue(pTable, L".rows(%d).cells(%d).innerText = %S", 3, 1, L"Pocket Soap Toolkit");

			/* Add a row and insert columns */
			WITH(pRow, pTable, L".insertRow")
			{
				dhPutValue(pRow, L".insertCell().innerText = %S", L"mshtml.c");
				dhPutValue(pRow, L".insertCell().innerText = %S", L"MSHTML Parser");

			} END_WITH(pRow);

			/* Delete a row */
			dhCallMethod(pTable, L".deleteRow(%d)", 4);

			/* Add a caption to the table */
			WITH(pCaption, pTable, L".createCaption")
			{
				dhPutValue(pCaption, L".vAlign = %S", L"bottom");
				dhPutValue(pCaption, L".innerText = %S", L"Table of DispHelper Samples");

			} END_WITH(pCaption);

		} END_WITH_THROW(pTable);

		/* Add a title heading to the page */
		dhCallMethod(pDoc, L".body.insertAdjacentHTML(%S, %S)", L"afterBegin", L"<h1>DispHelper Samples</h1>");

		cout << "\nPrinting new HTML...\n" << endl;

		/* Print out the new HTML */
		dhGetValue(L"%s", &szHTML, pDoc, L".documentElement.outerHTML");
		cout << szHTML << endl;
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
	CDispPtr pDoc;

	dhToggleExceptions(TRUE);

#ifndef __GNUC__
	cout << "Walking www.google.com/ie?q=test..." << endl;
	if (SUCCEEDED(HTMLParseURL(L"http://www.google.com/ie?q=test", &pDoc)))
	{
		if (SUCCEEDED(WaitForHTMLDocToLoad(pDoc)))
		{
			WalkAll(pDoc);
		}
	}
#else
	cout << "\nWalking HTML File..." << endl;
	if (SUCCEEDED(HTMLParseFile(L"C:\\windows\\web\\tip.htm", &pDoc)))
	{
		if (SUCCEEDED(WaitForHTMLDocToLoad(pDoc)))
		{
			WalkAll(pDoc);
		}
	}
#endif

	cout << "\nPress ENTER to run Manipulate sample..." << endl;
	cin.get();
	Manipulate();

	cout << "\n\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}
