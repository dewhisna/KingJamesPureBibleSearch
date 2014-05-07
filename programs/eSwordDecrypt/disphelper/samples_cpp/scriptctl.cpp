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
scriptctl.cpp:
  Demonstrates using the MSScriptControl to run a VBScript or JScript.
 -- */


#include "disphelper.h"
#include <iostream>
using namespace std;


/* **************************************************************************
 * RunScript:
 *   Run a script using the MSScriptControl. Optionally return a result.
 *
 ============================================================================ */
void RunScript(LPCWSTR szRetIdentifier, LPVOID pResult, LPCTSTR szScript, LPCTSTR szLanguage)
{
	CDispPtr scrCtl;

	if (SUCCEEDED(dhCreateObject(L"MSScriptControl.ScriptControl", NULL, &scrCtl)) &&
	    SUCCEEDED(dhPutValue(scrCtl, L".Language = %T", szLanguage)))
	{
		dhPutValue(scrCtl, L".AllowUI = %b", TRUE);
		dhPutValue(scrCtl, L".UseSafeSubset = %b", FALSE);

		if (!pResult)
			dhCallMethod(scrCtl, L".Eval(%T)", szScript);
		else
			dhGetValue(szRetIdentifier, pResult, scrCtl, L".Eval(%T)", szScript);
	}
}


/* ============================================================================ */
int main(void)
{
	CDhInitialize init;
	dhToggleExceptions(TRUE);
	int nResult;

	/* VBScript sample */
	RunScript(NULL, NULL, TEXT("MsgBox(\"This is a VBScript test.\" & vbcrlf & \"It worked!\",64 Or 3)"), TEXT("VBScript"));

	/* JScript sample */
	RunScript(L"%d", &nResult, TEXT("Math.round(Math.pow(5, 2) * Math.PI)"), TEXT("JScript"));
	cout << "Result = " << nResult << endl;

	cout << "\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}




