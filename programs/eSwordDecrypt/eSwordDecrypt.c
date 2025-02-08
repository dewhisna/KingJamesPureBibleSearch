/****************************************************************************
**
** Copyright (C) 2014-2025 Donna Whisnant, a.k.a. Dewtronics.
** Contact: http://www.dewtronics.com/
**
** This file is part of the KJVCanOpener Application as originally written
** and developed for Bethel Church, Festus, MO.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU General Public License
** version 3.0 as published by the Free Software Foundation and appearing
** in the file gpl-3.0.txt included in the packaging of this file. Please
** review the following information to ensure the GNU General Public License
** version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and
** Dewtronics.
**
****************************************************************************/

// ============================================================================

// ----------------
// To Compile this:
// ----------------
//
// On Microsoft Visual C++ from Cygwin:
//	cl eSwordDecrypt.c disphelper/single_file_source/disphelper.c /link ole32.lib oleaut32.lib uuid.lib user32.lib
//
//	Note: Technically don't need user32.lib unless displaying messageBox prompts for exceptions
//
// On Linux with MinGW32 from rubenvb's build:
//	/opt/mingw32_4.7.4_rubenvb/bin/i686-w64-mingw32-gcc eSwordDecrypt.c disphelper/single_file_source/disphelper.c -o eSwordDecrypt.exe -lole32 -loleaut32 -luuid -luser32
//

#include <stdlib.h>
#include <stdio.h>
#include "./disphelper/single_file_source/disphelper.h"

#undef ARRAYSIZE
#define ARRAYSIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define VERSION 	10000			// Version 1.0.0
#define APP_NAME	L"eSwordDecrypt"

#define SQL_DLL		L"sqltv3713.dll"

#define LICENSE_KEY	L"556F123C313626"
#define CRYPT_KEY	L"Heb_4:12"

#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }

typedef enum {
	eOpenReadOnly = 1,
	eOpenReadWrite = 2,
	eOpenCreate = 4,
	eOpenDeleteOnClose = 8,
	eOpenExclusive = 16,
	eOpenMainDb = 256,
	eOpenTempDb = 512,
	eOpenTransientDb = 1024,
	eOpenMainJournal = 2048,
	eOpenTempJournal = 4096,
	eOpenSubJournal = 8192,
	eOpenMasterJournal = 16384,
	eOpenNoMutex = 32768,
	eOpenFullMutex = 0x00010000
} OpenFlags;

// ============================================================================

void myExceptionCallback(PDH_EXCEPTION pException)
{
	WCHAR szMessage[1024];

	dhFormatExceptionW(pException, szMessage, ARRAYSIZE(szMessage), TRUE);

	fwprintf(stderr, L"------------------------------\n");
	fwprintf(stderr, L"    EXCEPTION ENCOUNTERED:\n");
	fwprintf(stderr, L"------------------------------\n");
	fwprintf(stderr, L"%s\n", szMessage);
	fwprintf(stderr, L"------------------------------\n");
}

int main(int argc, const char *argv[])
{
	int nArgsFound = 0;
	const char *pInFile = NULL;
	const char *pOutFile = NULL;
	int i;
	BOOL bUnknownOption = FALSE;
	BOOL bEOD = TRUE;
	INT nBk = 0;
	INT nChp = 0;

	DH_EXCEPTION_OPTIONS expOpts =
	{	NULL,					//	HWND hwnd;
		APP_NAME,				//	LPCWSTR szAppName;
		FALSE,					//	BOOL bShowExceptions;
		FALSE,					//	BOOL bDisableRecordExceptions;
		myExceptionCallback		//EDH_EXCEPTION_CALLBACK pfnExceptionCallback;
	};

	DISPATCH_OBJ(dbSource);
	DISPATCH_OBJ(dsSource);
	DISPATCH_OBJ(dbDecrypt);
	DISPATCH_OBJ(dsDecrypt);
	DISPATCH_OBJ(dsJunk);

	dhInitialize(TRUE);
//	dhToggleExceptions(TRUE);

	dhSetExceptionOptions(&expOpts);

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] != '-') {
			++nArgsFound;
			if (nArgsFound == 1) {
				pInFile = argv[i];
			} else if (nArgsFound == 2) {
				pOutFile = argv[i];
			}
		} else {
			bUnknownOption = TRUE;
		}
	}

	if ((nArgsFound != 2) || (bUnknownOption) || (pInFile == NULL) || (pOutFile == NULL)) {
		fwprintf(stderr, L"%s v%d.%d.%d\n\n", APP_NAME, VERSION/10000, ((VERSION/100)%100), (VERSION%100));
		fprintf(stderr, "Usage: %s <eSword-In-File> <eSword-Out-File>\n\n", argv[0]);
		fprintf(stderr, "This utility decrypts the eSword SQL Databases into a plaintext SQL Database\n\n");
		fprintf(stderr, "Requires the SQLitePlus sqltp50.dll to be registered and requires associated\n");
		fprintf(stderr, "   sqltv3701.dll  sqltv3713.dll  sqltv3763.dll  processing dll\n\n");
		fwprintf(stderr, L"%s currently using: %s\n\n", APP_NAME, SQL_DLL);
		return -1;
	}

	fprintf(stdout, "Reading: %s\n", pInFile);
	fprintf(stdout, "Writing: %s\n", pOutFile);
	fflush(stdout);
	
	// Create our objects:
	HR_TRY( dhCreateObject(L"SqlitePlus50.SqliteDb", NULL, &dbSource) );
	HR_TRY( dhCreateObject(L"SqlitePlus50.SqliteDb", NULL, &dbDecrypt) );

	// ------------------------------------------------------------------------

	// Initialize source database:
	HR_TRY( dhCallMethod(dbSource, L".Init(%S, %S)", SQL_DLL, LICENSE_KEY) );
	HR_TRY( dhPutValue(dbSource, L".CipherKey = %S", CRYPT_KEY) );

	// Initialize decryption database:
	HR_TRY( dhCallMethod(dbDecrypt, L".Init(%S, %S)", SQL_DLL, LICENSE_KEY) );

	// Must open the source database before our database object so that SQLTP50 will read the
	//		eSword License from their database (which "authorizes" us to use the dll):
	HR_TRY( dhCallMethod(dbSource, L".OpenEx(%s, %d, %m, %m)", pInFile, eOpenReadOnly) );

	HR_TRY( dhCallMethod(dbDecrypt, L".Open(%s, %m, %m)", pOutFile) );

	// ------------------------------------------------------------------------

	// Create tables in our decrypted database:
	if (FAILED(dhGetValue(L"%o", &dsJunk, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"CREATE TABLE Bible (Book INTEGER, Chapter INTEGER, Verse INTEGER, Scripture TEXT)"))) {
		fprintf(stderr, "Error creating Bible Table in decrypted database.\n"
						"Please either delete that table or use a new filename\n\n");
		goto cleanup;
	}
	SAFE_RELEASE(dsJunk);

	if (FAILED(dhGetValue(L"%o", &dsJunk, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"CREATE TABLE Details (Description NVARCHAR(255), Abbreviation NVARCHAR(50), Comments TEXT, Version INT, Font NVARCHAR(50), RightToLeft BOOL, OT BOOL, NT BOOL, Apocrypha BOOL, Strong BOOL)"))) {
		fprintf(stderr, "Error creating Details Table in decrypted database.\n"
						"Please either delete that table or use a new filename\n\n");
		goto cleanup;
	}
	SAFE_RELEASE(dsJunk);

	// ------------------------------------------------------------------------

	// Copy the Details Table:
	HR_TRY( dhGetValue(L"%o", &dsDecrypt, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"open table Details") );
	HR_TRY( dhCallMethod(dsDecrypt, L".MoveFirst") );

	HR_TRY( dhGetValue(L"%o", &dsSource, dbSource, L".Exec(%S, %m, %m, %m)", L"select Description, Abbreviation, Comments, Version, Font, RightToLeft, OT, NT, Apocrypha, Strong from Details") );

	HR_TRY( dhGetValue(L"%o", &dsJunk, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"BEGIN TRANSACTION") );
	SAFE_RELEASE(dsJunk);

	// At this point dsSource will be the dataset of our Source Bible Database (decrypted).
	//		Write it to the target database:
	HR_TRY( dhGetValue(L"%b", &bEOD, dsSource, L".EOD") );
	if (!bEOD) {
		fprintf(stdout, "\nCopying Details");
		do {
			HR_TRY( dhCallMethod(dsDecrypt, L".AddNew") );
			FOR_EACH(objColumn, dsSource, L".Columns")
			{
				LPSTR szColName;
				VARIANT varData;
				VariantInit(&varData);

				HR_TRY( dhGetValue(L"%s", &szColName, objColumn, L".Name") );
				HR_TRY( dhGetValue(L"%v", &varData, objColumn, L".Value") );

				// Note: It seems that SQLitePlus writes BOOL entries as 0 or -1 instead of 0 or 1.
				//		So those will get changed during the decrypt, but shouldn't be an issue
				//		if comparing as 0 or !0.

				HR_TRY( dhCallMethod(dsDecrypt, L".SetValue(%s, %v)", szColName, &varData) );

				VariantClear(&varData);
				dhFreeString(szColName);
			} NEXT(objColumn);

			HR_TRY( dhCallMethod(dsDecrypt, L".Update") );
			HR_TRY( dhCallMethod(dsDecrypt, L".MoveNext") );
			HR_TRY( dhCallMethod(dsSource, L".MoveNext") );

			HR_TRY( dhGetValue(L"%b", &bEOD, dsSource, L".EOD") );
		} while (!bEOD);
		fprintf(stdout, "\n");
	}

	HR_TRY( dhGetValue(L"%o", &dsJunk, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"COMMIT") );
	SAFE_RELEASE(dsJunk);

	SAFE_RELEASE(dsDecrypt);
	SAFE_RELEASE(dsSource);

	// ------------------------------------------------------------------------

	// Copy the main Bible Table:
	HR_TRY( dhGetValue(L"%o", &dsDecrypt, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"open table Bible") );
	HR_TRY( dhCallMethod(dsDecrypt, L".MoveFirst") );

	HR_TRY( dhGetValue(L"%o", &dsSource, dbSource, L".Exec(%S, %m, %m, %m)", L"select Book, Chapter, Verse, DECOMPRESS(DECRYPT(Scripture,'B')) from Bible") );

	HR_TRY( dhGetValue(L"%o", &dsJunk, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"BEGIN TRANSACTION") );
	SAFE_RELEASE(dsJunk);

	// At this point dsSource will be the dataset of our Source Bible Database (decrypted).
	//		Write it to the target database:
	HR_TRY( dhGetValue(L"%b", &bEOD, dsSource, L".EOD") );
	if (!bEOD) {
//		FOR_EACH(objColumn, dsSource, L".Columns")
//		{
//			LPSTR szColName;
//			LPSTR szColTypeName;
//			HR_TRY( dhGetValue(L"%s", &szColName, objColumn, L".Name") );
//			HR_TRY( dhGetValue(L"%s", &szColTypeName, objColumn, L".TypeName") );
//			fprintf(stdout, "%s : %s\n", szColName, szColTypeName);
//			dhFreeString(szColName);
//			dhFreeString(szColTypeName);
//		} NEXT(objColumn);

		do {
			HR_TRY( dhCallMethod(dsDecrypt, L".AddNew") );
			FOR_EACH(objColumn, dsSource, L".Columns")
			{
				LPSTR szColName;
				LPSTR szText;
				INT nValue;

				HR_TRY( dhGetValue(L"%s", &szColName, objColumn, L".Name") );

				if (strcmp(szColName, "DECOMPRESS(DECRYPT(Scripture,'B'))") == 0) {
					HR_TRY( dhGetValue(L"%s", &szText, objColumn, L".ToString") );
					HR_TRY( dhCallMethod(dsDecrypt, L".SetValue(%s, %s)", "Scripture", szText) );
					dhFreeString(szText);
				} else {
					HR_TRY( dhGetValue(L"%d", &nValue, objColumn, L".Value") );
					HR_TRY( dhCallMethod(dsDecrypt, L".SetValue(%s, %d)", szColName, nValue) );
					if (strcmp(szColName, "Book") == 0) {
						if (nBk != nValue) {
							fprintf(stdout, "\nBook %d", nValue);
							fflush(stdout);
							nBk = nValue;
							nChp = 0;
						}
					} else if (strcmp(szColName, "Chapter") == 0) {
						if (nChp != nValue) {
							fprintf(stdout, ".");
							fflush(stdout);
							nChp = nValue;
						}
					}
				}

				dhFreeString(szColName);
			} NEXT(objColumn);

			HR_TRY( dhCallMethod(dsDecrypt, L".Update") );
			HR_TRY( dhCallMethod(dsDecrypt, L".MoveNext") );
			HR_TRY( dhCallMethod(dsSource, L".MoveNext") );

			HR_TRY( dhGetValue(L"%b", &bEOD, dsSource, L".EOD") );
		} while (!bEOD);
		fprintf(stdout, "\n");
	}

	HR_TRY( dhGetValue(L"%o", &dsJunk, dbDecrypt, L".Exec(%S, %m, %m, %m)", L"COMMIT") );
	SAFE_RELEASE(dsJunk);

	// ------------------------------------------------------------------------

	HR_TRY( dhCallMethod(dbSource, L".Close") );
	HR_TRY( dhCallMethod(dbDecrypt, L".Close") );

	fprintf(stdout, "Done\n");

cleanup:
//	dhToggleExceptions(FALSE);

	SAFE_RELEASE(dsJunk);
	SAFE_RELEASE(dsDecrypt);
	SAFE_RELEASE(dbDecrypt);
	SAFE_RELEASE(dsSource);
	SAFE_RELEASE(dbSource);

	dhUninitialize(TRUE);
	
	return 0;
}

// ============================================================================
