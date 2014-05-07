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
email.cpp:
  Demonstrates sending an email with CDO, Outlook and Eudora.

  You must provide appropriate values in int main() before using this sample.
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
using namespace std;


/* **************************************************************************
 * EmailCDO:
 *   Demonstrates sending an email using CDO for Windows 2000. Requires
 * Windows 2000/XP+.
 * See http://www.microsoft.com/technet/community/scriptcenter/entscr/scrent16.mspx
 *
 ============================================================================ */
HRESULT EmailCDO(LPCTSTR szServer, LPCTSTR szFrom, LPCTSTR szTo, LPCTSTR szSubject, LPCTSTR szBody)
{
	CDispPtr cdoMsg;
	HRESULT hr;

	try
	{
		dhCheck( hr = dhCreateObject(L"CDO.Message", NULL, &cdoMsg) );

		dhPutValue(cdoMsg, L".Configuration.Fields(%S) = %d", L"http://schemas.microsoft.com/cdo/configuration/sendusing", 2); /* cdoSendUsingPort */
		dhPutValue(cdoMsg, L".Configuration.Fields(%S) = %T", L"http://schemas.microsoft.com/cdo/configuration/smtpserver", szServer);
		dhCallMethod(cdoMsg, L".Configuration.Fields.Update");

		dhPutValue(cdoMsg, L".MimeFormatted = %b", TRUE);

		dhPutValue(cdoMsg, L".Sender = %T",   szFrom);
		dhPutValue(cdoMsg, L".To = %T",       szTo);
		dhPutValue(cdoMsg, L".Subject = %T",  szSubject);
		dhPutValue(cdoMsg, L".TextBody = %T", szBody);   /* Use .HTMLBody if needed. */

		/* To add an attachment:
		 * dhCallMethod(cdoMsg, L".AddAttachment(%T)", TEXT("file://C:\\my file.zip"));
		 */

		dhCheck( hr = dhCallMethod(cdoMsg, L".Send") );
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	return hr;
}


/* **************************************************************************
 * EmailOutlook:
 *   Demonstrates sending an email using Outlook. Note that the user will be
 * prompted as a security measure.
 *
 ============================================================================ */
HRESULT EmailOutlook(LPCTSTR szTo, LPCTSTR szSubject, LPCTSTR szBody)
{
	CDispPtr olApp, olMsg;
	HRESULT hr;

	if (SUCCEEDED(hr = dhCreateObject(L"Outlook.Application", NULL, &olApp) ) &&
	    SUCCEEDED(hr = dhGetValue(L"%o", &olMsg, olApp, L".CreateItem(%d)", 0) ) ) /* olMailItem */
	{
		dhPutValue(olMsg, L".Subject = %T", szSubject);
		dhPutValue(olMsg, L".To = %T",      szTo);
		dhPutValue(olMsg, L".Body = %T",    szBody);

		/* To add an attachment:
		 * dhCallMethod(olMsg, L".Attachments.Add(%T)", "FilePath.ext");
		 */

		/* Display and attempt to send email */
		dhCallMethod(olMsg, L".Display");
		hr = dhCallMethod(olMsg, L".Send");
	}

	return hr;
}


/* **************************************************************************
 * EmailEudora:
 *   Demonstrates sending an email using Eudora. Automation must be enabled in
 * Eudora's options. See http://www.scripting.com/specials/eudoraAutomation.html
 *
 ============================================================================ */
HRESULT EmailEudora(LPCTSTR szTo, LPCTSTR szSubject, LPCTSTR szBody)
{
	CDispPtr euApp;
	HRESULT hr;

	if (SUCCEEDED(hr = dhCreateObject(L"Eudora.EuApplication.1", NULL, &euApp)))
	{
		/* Note: Empty arguments are CC, BCC and Attachments respectively. */
		hr = dhCallMethod(euApp, L".QueueMessage(%T,%T,%T,%T,%T,%T)",
					 szTo, szSubject, TEXT(""), TEXT(""), TEXT(""), szBody);

		dhCallMethod(euApp, L"SendQueuedMessages");
		dhCallMethod(euApp, L"CloseEudora");
	}

	return hr;
}


/* ============================================================================ */
int main(void)
{
	CDhInitialize init;
	dhToggleExceptions(TRUE);

	const TCHAR * szServer  = TEXT("smtp_server.myisp.com");
	const TCHAR * szFrom    = TEXT("myemail@mydomain.com");
	const TCHAR * szTo      = TEXT("jane@doe.com");
	const TCHAR * szSubject = TEXT("DispHelper Test email");

	cout << "Sending Email using CDO..." << endl;
	EmailCDO(szServer, szFrom, szTo, szSubject, TEXT("Test email using CDO."));

	cout << "Sending Email using Outlook..." << endl;
	EmailOutlook(szTo, szSubject, TEXT("Test email using Outlook."));
	
	cout << "Sending Email using Eudora..." << endl;
	EmailEudora(szTo, szSubject, TEXT("Test email using Eudora"));

	return 0;
}




