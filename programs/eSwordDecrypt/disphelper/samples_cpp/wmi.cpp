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
wmi.cpp:
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
#include <iostream>
#include <string>
using namespace std;


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
	CDispPtr wmiSvc, colQuickFixes;
	struct QFix {
		CDhStringA szCSName, szDescription, szHotFixID, szInstalledBy, szFixComments;
	};

	try
	{
		dhCheck( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

		dhCheck( dhGetValue(L"%o", &colQuickFixes, wmiSvc, L".ExecQuery(%S)",
		                                L"SELECT * FROM Win32_QuickFixEngineering") );

		FOR_EACH(objQuickFix, colQuickFixes, NULL)
		{
			QFix QuickFix = { 0 };

			dhGetValue(L"%s", &QuickFix.szCSName,      objQuickFix, L".CSName");
			dhGetValue(L"%s", &QuickFix.szDescription, objQuickFix, L".Description");
			dhGetValue(L"%s", &QuickFix.szHotFixID,    objQuickFix, L".HotFixID");
			dhGetValue(L"%s", &QuickFix.szFixComments, objQuickFix, L".FixComments");
			dhGetValue(L"%s", &QuickFix.szInstalledBy, objQuickFix, L".InstalledBy");

			cout << "Computer: "     << QuickFix.szCSName      << endl
			     << "Description: "  << QuickFix.szDescription << endl
			     << "Hot Fix ID: "   << QuickFix.szHotFixID    << endl
			     << "Installed By: " << QuickFix.szInstalledBy << endl
			     << "Comments: "     << QuickFix.szFixComments << endl << endl;

		} NEXT_THROW(objQuickFix);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * AddRemoteEvent:
 *   Add an entry to a local or remote event log.
 * http://www.microsoft.com/technet/community/scriptcenter/logs/scrlog21.mspx
 *
 ============================================================================ */
void AddRemoteEvent(LPCWSTR szComputer, LPCWSTR szEventText)
{
	CDispPtr objShell;
	const UINT EVENT_SUCCESS = 0;

	if (SUCCEEDED(dhCreateObject(L"Wscript.Shell", NULL, &objShell)))
	{
		dhCallMethod(objShell, L".LogEvent(%d, %S, %S)", EVENT_SUCCESS, szEventText, szComputer ? szComputer : L".");
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
	CDispPtr wmiSvc, colInstalledPrinters;

	try
	{
		dhCheck( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

		dhCheck( dhGetValue(L"%o", &colInstalledPrinters, wmiSvc, L".ExecQuery(%S)",
		                                L"SELECT * FROM Win32_Printer") );

		FOR_EACH(objPrinter, colInstalledPrinters, NULL)
		{
			dhCallMethod(objPrinter, L".CancelAllJobs");
		} NEXT_THROW(objPrinter);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * StartProcess:
 *   Start a program on local or remote computer.
 * http://www.microsoft.com/technet/community/scriptcenter/process/scrpcs04.mspx
 *
 ============================================================================ */
void StartProcess(LPCWSTR szComputer, LPCWSTR szProcess)
{
	CDispPtr wmiSvc;
	int nResult;

	try 
	{
		LPWSTR szWmiStr = getWmiStr(szComputer);
		wcscat(szWmiStr, L":Win32_Process");

		dhCheck( dhGetObject(szWmiStr, NULL, &wmiSvc) );

		dhCheck( dhGetValue(L"%d", &nResult, wmiSvc, L".Create(%S)", szProcess) );

		if (nResult == 0)
			cout << "The process was started." << endl;
		else 
			cout << "The process failed to start due to error " << nResult << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
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
	CDispPtr wmiSvc, wmiEventSrc;

	try
	{
		dhCheck( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

		dhCheck( dhGetValue(L"%o", &wmiEventSrc, wmiSvc, L".ExecNotificationQuery(%S)",
		                    L"SELECT * FROM __InstanceOperationEvent WITHIN 1 WHERE (__Class = '__InstanceCreationEvent' OR __Class = '__InstanceDeletionEvent') AND TargetInstance ISA 'Win32_Process'") );

		while (TRUE)
		{
			CDispPtr wmiLatestEvent;
			CDhStringA szProcessName, szClassName;

			dhCheck( dhGetValue(L"%o", &wmiLatestEvent, wmiEventSrc, L".NextEvent") );

			dhGetValue(L"%s", &szProcessName, wmiLatestEvent, L".TargetInstance.Name");
			dhGetValue(L"%s", &szClassName,   wmiLatestEvent, L".SystemProperties_(%S)", L"__Class");

			if ((string) szClassName == (string) "__InstanceDeletionEvent")
				cout << "TERMINATED: \t" << szProcessName << endl;
			else
				cout << "STARTED:    \t" << szProcessName << endl;
		}
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
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
	CDispPtr wmiSvc, wmiEventSrc;

	try
	{
		dhCheck( dhGetObject(getWmiStr(szComputer), NULL, &wmiSvc) );

		WCHAR szWmiQuery[MAX_PATH+256] = 
		              L"SELECT * FROM __InstanceCreationEvent WITHIN 10 WHERE "
		              L"Targetinstance ISA 'CIM_DirectoryContainsFile' AND "
		              L"TargetInstance.GroupComponent= 'Win32_Directory.Name=\"";
		wcsncat(szWmiQuery, szDirectory, MAX_PATH);
		wcscat(szWmiQuery, L"\"'");

		dhCheck( dhGetValue(L"%o", &wmiEventSrc, wmiSvc, L".ExecNotificationQuery(%S)", szWmiQuery) );

		while (TRUE)
		{
			CDispPtr wmiLatestEvent;
			CDhStringA szFileName;

			dhCheck( dhGetValue(L"%o", &wmiLatestEvent, wmiEventSrc, L".NextEvent") );

			dhGetValue(L"%s", &szFileName, wmiLatestEvent, L".TargetInstance.PartComponent");

			cout << szFileName << " was created." << endl;
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

	cout << "Enumerating installed hot fixes..." << endl;
	HotFixes(L".");

	cout << "\nAdding event to event log..." << endl;
	AddRemoteEvent(L".", L"DispHelper Test Event");

	cout << "\nStarting calc.exe process...\n" << endl;
	StartProcess(L".", L"calc.exe");

	cout << "\nMonitoring process activity...\n" <<
	        "Start a program to demonstrate.\n"  <<
	        "Press CTRL-C to exit.\n"            << endl;
	MonitorProcessActivity(L".");

	/* cout << "\nPurging print queue..." << endl; */
	/* PurgePrintQueue(L"."); */
	
	/* cout << "\nMonitoring file creations..." << endl; */
	/* MonitorFileCreation(L".", L"C:\\\\\\\\Documents and Settings\\\\\\\\user_name\\\\\\\\My Documents"); */

	return 0;
}




