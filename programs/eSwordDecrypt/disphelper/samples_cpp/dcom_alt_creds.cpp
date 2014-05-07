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
dcom_alt_creds.cpp:
  Demonstrates one way of accessing a remote COM object using alternate credentials.

  These samples are not about basic DCOM. If you simply want to use a remote COM
object simply provide dhCreateObject with a machine name and use the object
as usual. Example: dhCreateObject(L"Word.Document", L"machine_name", &wdDoc);

  This is not the preferred method of using alternate credentials
as it only works on Windows 2000/XP and later and the alternate credentials are
used process wide. However, currently this is the only method that is
supported by DispHelper.

  You can see this thread for a discussion on using alternate credentials.
http://groups.google.com/groups?threadm=b6aab89b.0311101708.4a123c75%40posting.google.com

  You should not use this method to access WMI. WMI has its own way of providing
alternate credentials. Search for 'WMI alternate credentials' or see
http://www.microsoft.com/resources/documentation/windows/2000/server/scriptguide/en-us/sas_wmi_ciga.mspx

  By default, an object will run in the context of the lauching user. This means
that it will not be visible on the target computer. You can change this by
running dcomcnfg and setting the object to run in the context of the interactive
user under the identity tab. You should be aware of the security implications
of doing this.

  You should be extremely cautious about compiling a user name and password into
an executable as anybody can read them.
 -- */


#define _WIN32_DCOM
#include "disphelper.h"
#include <iostream>
#include <string>
using namespace std;


static SEC_WINNT_AUTH_IDENTITY_W AuthIdentity;


/* **************************************************************************
 * InitIdentity:
 *   Initialises the static identity structure with the supplied values.
 * Strings are not copied and must be valid for the lifetime of the structure.
 *
 ============================================================================ */
HRESULT InitIdentity(LPWSTR szUserName, LPWSTR szPassword, LPWSTR szDomain)
{
	ZeroMemory(&AuthIdentity, sizeof(AuthIdentity));

	AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
	
	if ((AuthIdentity.User = (USHORT*) szUserName) != NULL)
		AuthIdentity.UserLength = wcslen(szUserName);

	if ((AuthIdentity.Password = (USHORT*) szPassword) != NULL)
		AuthIdentity.PasswordLength = wcslen(szPassword);

	if ((AuthIdentity.Domain = (USHORT*) szDomain) != NULL)
		AuthIdentity.DomainLength = wcslen(szDomain);

	return NOERROR;
}


/* **************************************************************************
 * InitSecurity:
 *   Set up COM to use our alternate credentials for COM calls. You must call
 * InitIdentity before calling this function.
 *
 ============================================================================ */
HRESULT InitSecurity(void)
{
	SOLE_AUTHENTICATION_LIST sal = { 0 };
	SOLE_AUTHENTICATION_INFO sai = { 0 };

	sal.cAuthInfo = 1;
	sal.aAuthInfo = &sai;

	sai.dwAuthnSvc = RPC_C_AUTHN_WINNT;
	sai.dwAuthzSvc = RPC_C_AUTHZ_NONE;
	sai.pAuthInfo = &AuthIdentity;

	return CoInitializeSecurity(NULL, -1, NULL, NULL, 
	                 RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE, 
	                 &sal, 0, NULL);
}


/* **************************************************************************
 * CreateObjectWithAltCredentials:
 *   Create a COM object on a remote machine using the alternate credentials
 * supplied in the call to InitIdentity. Both InitIdentity and InitSecurity 
 * must have been called before using this function.
 *
 ============================================================================ */
HRESULT CreateObjectWithAltCredentials(LPCOLESTR szProgId, LPWSTR szMachine, IDispatch ** ppDisp)
{
	COSERVERINFO ServerInfo     = { 0 };
	COAUTHINFO AuthInfo = {   /* Use default settings according to MSDN. */
		RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
		RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL, EOAC_NONE
	};

	ServerInfo.pwszName = szMachine;

	if (AuthIdentity.User)
	{
		ServerInfo.pAuthInfo = &AuthInfo;

#ifdef __GNUC__
		/* MinGW currently uses AUTH_IDENTITY instead of COAUTHIDENTITY */
		AuthInfo.pAuthIdentityData = (AUTH_IDENTITY *) &AuthIdentity;
#else
		AuthInfo.pAuthIdentityData = (COAUTHIDENTITY *) &AuthIdentity;
#endif
	}

	return dhCreateObjectEx(szProgId, IID_IDispatch,  CLSCTX_REMOTE_SERVER, &ServerInfo, (void **) ppDisp);
}


/* **************************************************************************
 * main:
 *   Our sample using alternate credentials. You must replace machine_name, 
 * user_name and password with appropriate values. The provided user_name
 * usually needs administrator priviliges on the remote computer.
 *
 ============================================================================ */
int main(void)
{
	CDhInitialize init;
	CDispPtr wdApp, wdDoc;

	dhToggleExceptions(TRUE);

	try
	{
		/* Use the domain if you authenticate against a domain, else the remote machine */
		dhCheck( InitIdentity(L"user_name", L"password", L"domain_or_machine_name") );
		dhCheck( InitSecurity() );

		dhCheck( CreateObjectWithAltCredentials(L"Word.Document", L"machine_name", &wdDoc) );

		dhCheck( dhGetValue(L"%o", &wdApp, wdDoc, L".Application") );

		dhPutValue(wdApp, L".Visible = %b", TRUE);

		dhPutValue(wdDoc, L".Range.Font.Size = %d", 50);
		dhPutValue(wdDoc, L".Range.Font.Color = %d", RGB(0,0,255));
		dhPutValue(wdDoc, L".Range.Text = %S", L"Hello!");

		dhCallMethod(wdApp, L".Documents.Add");
		dhPutValue(wdApp, L".Selection.Font.Size = %d", 25);
		dhPutValue(wdApp, L".Selection.Font.Color = %d", RGB(255,0,0));
		dhCallMethod(wdApp, L".Selection.TypeText(%S)", L"This is a ghost. BOO!\n");

		FOR_EACH(wdEnumDoc, wdApp, L"Documents")
		{
			CDhStringW szText;

			dhGetValue(L"%S", &szText, wdEnumDoc, L".Range.Text");
			MessageBoxW(NULL, szText, L"Remote Word Document Text", 0);

			dhPutValue(wdEnumDoc, L".Saved = %b", TRUE);

		} NEXT_THROW(wdEnumDoc);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	cout << "Press ENTER to exit..." << endl;
	cin.get();
	return 0;
}
