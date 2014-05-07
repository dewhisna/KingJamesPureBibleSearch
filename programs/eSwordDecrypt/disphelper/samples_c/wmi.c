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
wmi.c:
  Demonstrates using Windows Management Instrumentation(WMI).

  Samples include enumerating installed hot fixes, purging print queues, starting
a program, monitoring starting and terminating processes, and monitoring file creation.
All of these samples can target the local or a remote computer.

  To use these samples on a remote computer you must have administrator
priviliges on the remote computer.
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>

#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }


/* **************************************************************************
 * getWmiStr:
 *   Helper function to create wmi moniker incorporating computer name.
 *
 ============================================================================ */
static LPWSTR getWmiStr(LPCWSTR szComputer)
{
	static WCHAR szWmiStr[256];

	wcscpy(szWmiStr, L"winmgmts:{impersonationLevel=impersonate}!\\\\");

	if (szComputer) wcsncat(szWmiStr, szComputer, 128);
	else            wcscat (szWmiStr, L".");

	wcscat(szWmiStr, L"\\root\\cimv2");

	return szWmiStr;
}


/* **************************************************************************
 * HotFixes:
 *   Enumerate hot fixes installed on local or remote computer.
 * http://www.microsoft.com/technet/community/scriptcenter/compmgmt/scrcm15.mspx
 *
 ============================================================================ */
void HotFixes(LPCWSTR szComputer)
{
	DISPATCH_OBJ(wmiSvc);
	DISPATCH_OBJ(colQuickFixes);
	struct QFix {
		LPWSTR szCSName, szDescription, szHotFixID, szInstalledBy, szFixComments;
	} QuickFix;

	HR_TRY( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

	HR_TRY( dhGetValue(L"%o", &colQuickFixes, wmiSvc, L".ExecQuery(%S)",
	                                L"SELECT * FROM Win32_QuickFixEngineering") );

	FOR_EACH(objQuickFix, colQuickFixes, NULL)
	{
		ZeroMemory(&QuickFix, sizeof(QuickFix));

		dhGetValue(L"%S", &QuickFix.szCSName,      objQuickFix, L".CSName");
		dhGetValue(L"%S", &QuickFix.szDescription, objQuickFix, L".Description");
		dhGetValue(L"%S", &QuickFix.szHotFixID,    objQuickFix, L".HotFixID");
		dhGetValue(L"%S", &QuickFix.szFixComments, objQuickFix, L".FixComments");
		dhGetValue(L"%S", &QuickFix.szInstalledBy, objQuickFix, L".InstalledBy");

		wprintf(L"Computer: %s\nDescription: %s\nHot Fix ID: %s\nInstalled By: %s\nComments: %s\n\n",
		           QuickFix.szCSName, QuickFix.szDescription, QuickFix.szHotFixID,
		           QuickFix.szInstalledBy, QuickFix.szFixComments);

		dhFreeString(QuickFix.szCSName);
		dhFreeString(QuickFix.szDescription);
		dhFreeString(QuickFix.szHotFixID);
		dhFreeString(QuickFix.szFixComments);
		dhFreeString(QuickFix.szInstalledBy);

	} NEXT(objQuickFix);

cleanup:
	SAFE_RELEASE(colQuickFixes);
	SAFE_RELEASE(wmiSvc);
}


/* **************************************************************************
 * AddRemoteEvent:
 *   Add an entry to a local or remote event log.
 * http://www.microsoft.com/technet/community/scriptcenter/logs/scrlog21.mspx
 *
 ============================================================================ */
void AddRemoteEvent(LPCWSTR szComputer, LPCWSTR szEventText)
{
	DISPATCH_OBJ(objShell);
	const UINT EVENT_SUCCESS = 0;

	if (SUCCEEDED(dhCreateObject(L"Wscript.Shell", NULL, &objShell)))
	{
		dhCallMethod(objShell, L".LogEvent(%d, %S, %S)", EVENT_SUCCESS, szEventText, szComputer ? szComputer : L".");
		SAFE_RELEASE(objShell);
	}
}


/* **************************************************************************
 * PurgePrintQueues:
 *   Purge print queues on a computer.
 * http://www.microsoft.com/technet/community/scriptcenter/printing/scrprn34.mspx
 *
 ============================================================================ */
void PurgePrintQueue(LPCWSTR szComputer)
{
	DISPATCH_OBJ(wmiSvc);
	DISPATCH_OBJ(colInstalledPrinters);

	HR_TRY( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

	HR_TRY( dhGetValue(L"%o", &colInstalledPrinters, wmiSvc, L".ExecQuery(%S)",
	                                L"SELECT * FROM Win32_Printer") );

	FOR_EACH(objPrinter, colInstalledPrinters, NULL)
	{
		dhCallMethod(objPrinter, L".CancelAllJobs");

	} NEXT(objPrinter);

cleanup:
	SAFE_RELEASE(colInstalledPrinters);
	SAFE_RELEASE(wmiSvc);
}


/* **************************************************************************
 * StartProcess:
 *   Start a program on local or remote computer.
 * http://www.microsoft.com/technet/community/scriptcenter/process/scrpcs04.mspx
 *
 ============================================================================ */
void StartProcess(LPCWSTR szComputer, LPCWSTR szProcess)
{
	DISPATCH_OBJ(wmiSvc);
	LPWSTR szWmiStr;
	int nResult;

	szWmiStr = getWmiStr(szComputer);
	wcscat(szWmiStr, L":Win32_Process");

	HR_TRY( dhGetObject(szWmiStr, NULL, &wmiSvc) );

	HR_TRY( dhGetValue(L"%d", &nResult, wmiSvc, L".Create(%S)", szProcess) );

	if (nResult == 0)
		wprintf(L"The process was started.\n");
	else 
		wprintf(L"The process failed to start due to error %d.\n", nResult);

cleanup:
	SAFE_RELEASE(wmiSvc);
}


/* **************************************************************************
 * MonitorProcessActivity:
 *   Monitors process startup and termination on local or remote computer.
 * http://www.microsoft.com/technet/community/scriptcenter/monitor/scrmon24.mspx
 * http://www.microsoft.com/technet/community/scriptcenter/monitor/scrmon25.mspx
 *
 ============================================================================ */
void MonitorProcessActivity(LPCWSTR szComputer)
{
	DISPATCH_OBJ(wmiSvc);
	DISPATCH_OBJ(wmiEventSrc);

	HR_TRY( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

	HR_TRY( dhGetValue(L"%o", &wmiEventSrc, wmiSvc, L".ExecNotificationQuery(%S)",
	         L"SELECT * FROM __InstanceOperationEvent WITHIN 1 WHERE (__Class = '__InstanceCreationEvent' OR __Class = '__InstanceDeletionEvent') AND TargetInstance ISA 'Win32_Process'") );

	while (TRUE)
	{
		DISPATCH_OBJ(wmiLatestEvent);
		LPWSTR szProcessName = NULL;
		LPWSTR szClassName   = NULL;

		HR_TRY( dhGetValue(L"%o", &wmiLatestEvent, wmiEventSrc, L".NextEvent") );

		dhGetValue(L"%S", &szProcessName, wmiLatestEvent, L".TargetInstance.Name");
		dhGetValue(L"%S", &szClassName,   wmiLatestEvent, L".SystemProperties_(%S)", L"__Class");

		if (szClassName && 0 == wcscmp(L"__InstanceDeletionEvent", szClassName))
			wprintf(L"TERMINATED: \t%s\n", szProcessName);
		else
			wprintf(L"STARTED:    \t%s\n", szProcessName);

		dhFreeString(szProcessName);
		dhFreeString(szClassName);
		SAFE_RELEASE(wmiLatestEvent);
	}

cleanup:
	SAFE_RELEASE(wmiEventSrc);
	SAFE_RELEASE(wmiSvc);
}


/* **************************************************************************
 * MonitorFileCreation:
 *   Monitors creation of files in a directory on local or remote computer.
 * http://www.microsoft.com/technet/community/scriptcenter/filefolder/scrff44.mspx
 *
 * Note: The directory name must be split up with 4 back slashes, which is 8 in
 * a string literal. eg. L"C:\\\\\\\\Test\\\\\\\\Programs"
 ============================================================================ */
void MonitorFileCreation(LPCWSTR szComputer, LPCWSTR szDirectory)
{
	DISPATCH_OBJ(wmiSvc);
	DISPATCH_OBJ(wmiEventSrc);
	WCHAR szWmiQuery[MAX_PATH+256];

	HR_TRY( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

	wcscpy(szWmiQuery, L"SELECT * FROM __InstanceCreationEvent WITHIN 10 WHERE ");
	wcscat(szWmiQuery, L"Targetinstance ISA 'CIM_DirectoryContainsFile' AND ");
	wcscat(szWmiQuery, L"TargetInstance.GroupComponent= 'Win32_Directory.Name=\"");
	wcsncat(szWmiQuery, szDirectory, MAX_PATH);
	wcscat(szWmiQuery, L"\"'");

	HR_TRY( dhGetValue(L"%o", &wmiEventSrc, wmiSvc, L".ExecNotificationQuery(%S)", szWmiQuery) );

	while (TRUE)
	{
		DISPATCH_OBJ(wmiLatestEvent);
		LPWSTR szFileName = NULL;

		HR_TRY( dhGetValue(L"%o", &wmiLatestEvent, wmiEventSrc, L".NextEvent") );

		dhGetValue(L"%S", &szFileName, wmiLatestEvent, L".TargetInstance.PartComponent");

		wprintf(L"%s was created.\n", szFileName);

		dhFreeString(szFileName);
		SAFE_RELEASE(wmiLatestEvent);
	}

cleanup:
	SAFE_RELEASE(wmiEventSrc);
	SAFE_RELEASE(wmiSvc);
}


/* ============================================================================ */
int main(void)
{
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);

	printf("Enumerating installed hot fixes...\n");
	HotFixes(L".");

	printf("\nAdding event to event log...\n");
	AddRemoteEvent(L".", L"DispHelper Test Event");

	printf("\nStarting calc.exe process...\n");
	StartProcess(L".", L"calc.exe");

	printf("\nMonitoring process activity...\n");
	printf("Start a program to demonstrate.\n");
	printf("Press CTRL-C to exit.\n\n");
	MonitorProcessActivity(L".");

	/* printf("\nPurging print queue...\n"); */
	/* PurgePrintQueue(L"."); */
	
	/* printf("\nMonitoring file creations...\n"); */
	/* MonitorFileCreation(L".", L"C:\\\\\\\\Documents and Settings\\\\\\\\user_name\\\\\\\\My Documents"); */

	dhUninitialize(TRUE);
	return 0;
}




