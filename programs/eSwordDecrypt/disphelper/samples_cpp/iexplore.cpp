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
iexplore.cpp:
  Demonstrates controlling Internet Explorer via COM. Demonstrates using an Internet
Explorer window to display or retrieve information from the user.
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
using namespace std;


/* **************************************************************************
 * IESample
 *   Simple demonstration of controlling Internet Explorer.
 *
 ============================================================================ */
void IESample(void)
{
	CDispPtr ieApp;
	BOOL bBusy;

	try
	{
		dhCheck( dhCreateObject(L"InternetExplorer.Application", NULL, &ieApp) );

		dhPutValue(ieApp, L"Visible = %b", TRUE);

		dhCheck( dhCallMethod(ieApp, L"Navigate(%S)", L"http://www.google.com") );

		/* Wait for the page to load */
		while (dhCheck(dhGetValue(L"%b", &bBusy, ieApp, L".Busy")) && bBusy) Sleep(150);

		/* Turn off scroll bars, turn on TheatreMode and show the favorites bar */
		dhPutValue(ieApp, L".document.body.scroll = %S", L"no");
		dhPutValue(ieApp, L".TheaterMode = %b", TRUE);
		dhCallMethod(ieApp, L"ShowBrowserBar(%S, %b)", L"{EFA24E61-B078-11D0-89E4-00C04FC9E26E}", TRUE);

		WITH(pDoc, ieApp, L".Document")
		{
			CDhStringW szLastMod, szInnerText;
			HWND hwndIe = NULL;

			/* hwndIe = ieApp.HWND - Note use of %p */
			dhGetValue(L"%p", &hwndIe, ieApp, L".HWND");

			dhPutValue(pDoc, L".body.style.color = %S", L"firebrick");
			dhPutValue(pDoc, L".body.style.backgroundColor = %S", L"black");

			dhGetValue(L"%S", &szLastMod,   pDoc, L".lastModified");
			dhGetValue(L"%S", &szInnerText, pDoc, L".documentElement.innerText");

			MessageBoxW(hwndIe, szInnerText, szLastMod, 0);

		} END_WITH_THROW(pDoc);

		dhPutValue(ieApp, L".StatusText = %S", L"We are finally done!");
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * Search:
 *   Opens alltheweb.com, fills in the search box and submits the form.
 *
 ============================================================================ */
void Search(LPCTSTR szQuery)
{
	CDispPtr ieApp;
	BOOL bBusy;

	try
	{
		dhCheck( dhCreateObject(L"InternetExplorer.Application", NULL, &ieApp) );
		dhCheck( dhPutValue(ieApp, L".Visible = %b", TRUE) );

		/* Navigate to alltheweb.com */
		dhCheck( dhCallMethod(ieApp, L".Navigate(%S)", L"alltheweb.com") );

		/* Wait for the page to load */
		while (dhCheck(dhGetValue(L"%b", &bBusy, ieApp, L".Busy")) && bBusy) Sleep(150);
	
		/* Fill in the search box */
		dhCheck( dhPutValue(ieApp, L".document.all(%S).value = %T", L"q", szQuery) );

		/* Submit the form */
		dhCheck( dhCallMethod(ieApp, L".document.forms(%d).submit", 0) );
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * EnumerateIE:
 *   Enumerate open Internet Explorer windows.
 *
 ============================================================================ */
void EnumerateIE(void)
{
	CDispPtr pShell;

	if (SUCCEEDED(dhCreateObject(L"Shell.Application", NULL, &pShell)))
	{
		FOR_EACH(pIE, pShell, L".Windows")
		{
			CDhStringA szURL, szName;

			dhGetValue(L"%s", &szURL,  pIE, L".LocationURL");
			dhGetValue(L"%s", &szName, pIE, L".LocationName");

			cout << "Name: " << szName << endl <<
			        "URL: "  << szURL  << endl << endl;

			dhPutValue(pIE, L".StatusText= %S", L"You have been ENUMERATED!");

		} NEXT(pIE);
	}
}


/* **************************************************************************
 * SetIEOptions:
 *   Helper function to set various Internet Explorer options.
 *
 ============================================================================ */
void SetIEOptions(IDispatch * ieApp, BOOL bToolbar, BOOL bStatusbar, INT nWidth,
                  INT nHeight, INT nLeft, INT nTop)
{
	dhPutValue(ieApp, L".Toolbar = %b", bToolbar);
	dhPutValue(ieApp, L".StatusBar = %b", bStatusbar);
	dhPutValue(ieApp, L".Width = %d", nWidth);
	dhPutValue(ieApp, L".Height = %d", nHeight);
	dhPutValue(ieApp, L".Left = %d", nLeft);
	dhPutValue(ieApp, L".Top = %d", nTop);
}



/* **************************************************************************
 * DisplayData:
 *   Demonstrates displaying data using Internet Explorer. In this case a list
 * of services on the local computer.
 * See http://www.microsoft.com/technet/community/scriptcenter/entscr/scrent19.mspx
 *
 ============================================================================ */
void DisplayData(void)
{
	CDispPtr ieApp, wmiSvc;
	BOOL bBusy;

	try
	{
		dhCheck( dhCreateObject(L"InternetExplorer.Application", NULL, &ieApp) );
		dhCheck( dhGetObject(L"winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2", NULL, &wmiSvc) );
		dhCheck( dhCallMethod(ieApp, L".Navigate(%S)", L"About:blank") );

		SetIEOptions(ieApp, FALSE, FALSE, 800, 570, 0, 0);
		dhPutValue(ieApp, L".Visible = %b", TRUE);

		while (dhCheck(dhGetValue(L"%b", &bBusy, ieApp, L".Busy")) && bBusy) Sleep(150);

		WITH(pDoc, ieApp, L".Document")
		{
			dhCallMethod(pDoc, L".Open");

			/* Note: For performance, these should all be combined into one call */
			dhCallMethod(pDoc, L".Writeln(%S)", L"<html><head><title>Service Status</title></head>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"<body bgcolor='white'>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"<table width='100%'>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"<tr>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"<td width='50%'><b>Service</b></td>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"<td width='50%'><b>State</b></td>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"</tr>");

			FOR_EACH1(objService, wmiSvc, L".ExecQuery(%S)", L"SELECT * FROM Win32_Service")
			{
				BSTR bstrDisplayName = NULL, bstrState = NULL;

				dhGetValue(L"%B", &bstrDisplayName, objService, L".DisplayName");
				dhGetValue(L"%B", &bstrState,       objService, L".State");

				if (bstrState && _wcsicmp(bstrState, L"Running") == 0)
					dhCallMethod(pDoc, L".Writeln(%S)", L"<tr style=\"color:green;\"><td width='50%'>");
				else
					dhCallMethod(pDoc, L".Writeln(%S)", L"<tr style=\"color:red;\"><td width='50%'>");

				dhCallMethod(pDoc, L".Writeln(%B)", bstrDisplayName);
				dhCallMethod(pDoc, L".Writeln(%S)", L"</td><td width='50%'>");
				dhCallMethod(pDoc, L".Writeln(%B)", bstrState);
				dhCallMethod(pDoc, L".Writeln(%S)", L"</td></tr>");

				dhFreeString(bstrDisplayName);
				dhFreeString(bstrState);

			} NEXT_THROW(objService);

			dhCallMethod(pDoc, L".Writeln(%S)", L"</table>");
			dhCallMethod(pDoc, L".Writeln(%S)", L"</body></html>");
			dhCallMethod(pDoc, L".Write");
			dhCallMethod(pDoc, L".Close");

		} END_WITH_THROW(pDoc);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * HTML Snippet used in RetrieveData sample.
 *
 ============================================================================ */
const WCHAR * HTML_PAGE = L" \
<BODY>  \
Please enter your password: <INPUT TYPE=password Name='PasswordBox' size='20'>  \
<P><INPUT NAME='OKButton' TYPE='BUTTON' VALUE='OK' onclick='OKClicked.value=1'> \
<P><input type='hidden' name='OKClicked' value='0' size='10'>  \
</BODY>";


/* **************************************************************************
 * RetrieveData:
 *   Demonstrates retrieving information from the user using Internet Explorer.
 * See http://www.microsoft.com/technet/community/scriptcenter/entscr/scrent10.mspx
 *
 ============================================================================ */
void RetrieveData(void)
{
	CDispPtr ieApp;
	HRESULT hr;
	BOOL bBusy;
	INT nClicked;

	try
	{
		dhCheck( dhCreateObject(L"InternetExplorer.Application", NULL, &ieApp) );
		dhCheck( dhCallMethod(ieApp, L".Navigate(%S)", L"about:blank") );

		SetIEOptions(ieApp, FALSE, TRUE, 400, 250, 0, 0);
		dhPutValue(ieApp, L".Visible = %b", TRUE);

		while (dhCheck(dhGetValue(L"%b", &bBusy, ieApp, L".Busy")) && bBusy) Sleep(150);

		dhCheck( dhCallMethod(ieApp, L".document.write(%S)", HTML_PAGE) );
		dhCallMethod(ieApp, L".document.close");

		/* Wait for user to press button */
		while (SUCCEEDED(hr = dhGetValue(L"%d", &nClicked, ieApp, L".document.all.OKClicked.value")) && !nClicked) Sleep(250);

		if (SUCCEEDED(hr))
		{
			CDhStringA szPassword;
			dhCheck( dhGetValue(L"%s", &szPassword, ieApp, L".document.all.PasswordBox.value") );
			dhCallMethod(ieApp, L".Quit");
			cout << "Password is '" << szPassword << "'" << endl;
		}
		else
		{
			cout << "User closed dialog." << endl;
		}
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

	cout << "Enumerating Internet Explorer windows...\n" << endl;
	EnumerateIE();

	cout << "\nPress ENTER to run IESample..." << endl;
	cin.get();
	IESample();

	cout << "\nPress ENTER to run Search sample..." << endl;
	cin.get();
	Search(TEXT("test"));

	cout << "\nPress ENTER to run DisplayData sample..." << endl;
	cin.get();
	DisplayData();

	cout << "\nPress ENTER to run RetrieveData sample..." << endl;
	cin.get();
	RetrieveData();

	cout << "\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}




