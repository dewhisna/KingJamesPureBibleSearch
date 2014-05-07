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
dexplore.cpp:
  Demonstrates controlling Microsoft's new help system for developers, dexplore.
This is also known as MS Help 2 and ships with Visual Studio and the platform SDK.
 -- */


#include "disphelper.h"
#include <iostream>
using namespace std;


/* **************************************************************************
 * ShowHelpTopic:
 *   Function to show a Platform SDK help topic in dExplore(MS Help 2).
 *
 ============================================================================ */
void ShowHelpTopic(LPCTSTR szKeyword)
{
	CDispPtr dExplore;

	dhToggleExceptions(FALSE);
		dhGetObject(NULL, L"DExplore.AppObj", &dExplore);
	dhToggleExceptions(TRUE);

	if (!dExplore) dhCreateObject(L"DExplore.AppObj", NULL, &dExplore);

	if (dExplore)
	{
		WITH(helpHost, dExplore, L".Help")
		{
			dhCallMethod(helpHost, L".SetCollection(%S, %S)", L"ms-help://MS.PSDK.1033", L"Platform SDK");
			dhCallMethod(helpHost, L".SyncIndex(%T, %d)", szKeyword, 1);
			dhCallMethod(helpHost, L".DisplayTopicFromKeyword(%T)", szKeyword);

		} END_WITH(helpHost);
	}

	/* dexplore 'may' exit when we release the object so we wait */
	cout << "Press ENTER to exit..." << endl;
	cin.get();
}


/* ============================================================================ */
int main(void)
{
	CDhInitialize init;
	dhToggleExceptions(TRUE);

	ShowHelpTopic(TEXT("RegCreateKeyEx"));

	return 0;
}
