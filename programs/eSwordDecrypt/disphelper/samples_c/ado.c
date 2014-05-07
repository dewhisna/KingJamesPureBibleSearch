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
ado.c:
  Demonstrates reading and manipulating data from a data source using ActiveX Data Objects.

  To ensure that most people are able to view the contents of our 'database', we use an
Excel spread sheet as our data source in these samples. However, Excel files do not
provide a robust or fast database format and one should generally avoid using them
as a database.

  The preferred method is to use an Access file database or a server database such as
MySql. A list of connection strings for data sources can be found at
http://www.connectionstrings.com/ or http://www.able-consulting.com/ADO_Conn.htm
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>

#define HR_TRY(func) if (FAILED(func)) { printf("\n## Fatal error on line %d.\n", __LINE__); goto cleanup; }


/* **************************************************************************
 * AdoRead:
 *   Demonstrates obtaining data in a recordset and printing it out using ADO.
 *
 ============================================================================ */
void AdoRead(void)
{
	DISPATCH_OBJ(conn);
	DISPATCH_OBJ(rs);
	BOOL bEOF;
	struct Whale { 	/* Structure to hold a row of information */
		int id;
		LPWSTR szSpecies, szPopulation, szStatus;
	} WhaleRow;

	HR_TRY( dhCreateObject(L"ADODB.Connection", NULL, &conn) );

	/* Connect to the data source */
	HR_TRY( dhCallMethod(conn, L".Open(%S)", L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Whales.xls;Extended Properties=\"Excel 8.0;HDR=Yes;IMEX=1;\"") );

	/* Execute a select statement to retrieve a recordset */
	HR_TRY( dhGetValue(L"%o", &rs, conn, L".Execute(%S)", L"SELECT ID, Species, Population, Status FROM [Whales$] ORDER BY ID") );

	/* Loop through each row until EOF is TRUE */
	while (SUCCEEDED(dhGetValue(L"%b", &bEOF, rs, L".EOF")) && !bEOF)
	{
		/* Clear out the structure that will hold the information */
		ZeroMemory(&WhaleRow, sizeof(WhaleRow));

		/* Get the value of each field that we want */
		dhGetValue(L"%d", &WhaleRow.id,           rs, L".Fields(%S).Value", L"ID");
		dhGetValue(L"%S", &WhaleRow.szSpecies,    rs, L".Fields(%S).Value", L"Species");
		dhGetValue(L"%S", &WhaleRow.szPopulation, rs, L".Fields(%S).Value", L"Population");
		dhGetValue(L"%S", &WhaleRow.szStatus,     rs, L".Fields(%S).Value", L"Status");

		/* Output information to console */
		wprintf(L"ID: %d\nSpecies: %s\nPopulation: %s\nStatus: %s\n\n",
			WhaleRow.id, WhaleRow.szSpecies, WhaleRow.szPopulation, WhaleRow.szStatus);

		/* Free returned strings */
		dhFreeString(WhaleRow.szSpecies);
		dhFreeString(WhaleRow.szPopulation);
		dhFreeString(WhaleRow.szStatus);

		/* Move to next record */
		HR_TRY( dhCallMethod(rs, L".MoveNext") );
	}

cleanup:
	if (conn)
	{
		int nState = 0;

		/* Get the connection state */
		dhGetValue(L"%d", &nState, conn, L".State");;

		/* Closing the connection also closes active recordsets associated with it */
		if (nState != 0 /* adStateClosed */) dhCallMethod(conn, L".Close");
	}

	SAFE_RELEASE(rs);
	SAFE_RELEASE(conn);
}


/* **************************************************************************
 * AdoDump:
 *   Similar to the above sample, this sample differs by demonstrating
 * how to dump a recordset to output when you do not know its structure.
 *
 ============================================================================ */
void AdoDump(void)
{
	DISPATCH_OBJ(conn);
	DISPATCH_OBJ(rs);
	BOOL bEOF;

	HR_TRY( dhCreateObject(L"ADODB.Connection", NULL, &conn) );

	HR_TRY( dhCallMethod(conn, L".Open(%S)", L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Whales.xls;Extended Properties=\"Excel 8.0;HDR=Yes;IMEX=1;\"") );

	HR_TRY( dhGetValue(L"%o", &rs, conn, L".Execute(%S)", L"SELECT * FROM [Whales$]") );

	while (SUCCEEDED(dhGetValue(L"%b", &bEOF, rs, L".EOF")) && !bEOF)
	{
		/* Enumerate each field and dump its name/value pair */
		FOR_EACH(field, rs, L".Fields")
		{
			LPWSTR szFieldName, szFieldValue;

			if (SUCCEEDED(dhGetValue(L"%S", &szFieldName, field, L".Name")))
			{
				wprintf(L"%s: ", szFieldName);
				dhFreeString(szFieldName);
			}
			if (SUCCEEDED(dhGetValue(L"%S", &szFieldValue, field, L".Value")))
			{
				wprintf(L"%s\n", szFieldValue);
				dhFreeString(szFieldValue);
			}

		} NEXT(field);

		wprintf(L"\n");

		HR_TRY( dhCallMethod(rs, L".MoveNext") );
	}

cleanup:
	if (conn)
	{
		int nState = 0;

		dhGetValue(L"%d", &nState, conn, L".State");;

		if (nState != 0 /* adStateClosed */) dhCallMethod(conn, L".Close");
	}

	SAFE_RELEASE(rs);
	SAFE_RELEASE(conn);
}


/* **************************************************************************
 * AdoWrite:
 *   Demonstrates various methods of adding and altering records using ADO.
 *
 ============================================================================ */
void AdoWrite(void) {

	DISPATCH_OBJ(conn);
	DISPATCH_OBJ(rs);

	HR_TRY( dhCreateObject(L"ADODB.Connection", NULL, &conn) );

	/* Open the data source. Make sure we have write access. */
	HR_TRY( dhCallMethod(conn, L".Open(%S)", L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Whales.xls;Extended Properties=\"Excel 8.0;HDR=Yes;IMEX=0;\"") );

	/* Add some records using SQL statements */
	dhCallMethod(conn, L"Execute(%S)", L"INSERT INTO [AddressList$] VALUES ('Jane Doe', '1 Donkey St', 25)");
	dhCallMethod(conn, L"Execute(%S)", L"INSERT INTO [AddressList$] VALUES ('John Citizen', '1 Cheetah St', 74)");

	/* Alter a record using an SQL statement */
	dhCallMethod(conn, L"Execute(%S)", L"UPDATE [AddressList$] SET Address = '1 Elephant St' WHERE Name = 'John Citizen'");

	/* Add 5 to each age using an SQL statement */
	dhCallMethod(conn, L"Execute(%S)", L"UPDATE [AddressList$] SET Age = Age + 5");
	
	/* Using SQL is preferable but now we demonstrate how to update using a recordset */

	/* Create a recordset. The conn.Execute method can only return a read only recordset */
	HR_TRY( dhCreateObject(L"ADODB.Recordset", NULL, &rs) );

	/* Open the recordset with write access */
	HR_TRY( dhCallMethod(rs, L"Open(%S, %o, %d, %d)", L"[AddressList$]", conn, 3, 3) );

	/* Add a new record */
	if (SUCCEEDED(dhCallMethod(rs, L".AddNew")))
	{
		dhPutValue(rs, L".Fields(%S).Value = %S", L"Name", L"Joe Smith");
		dhPutValue(rs, L".Fields(%S).Value = %S", L"Address", L"1 Dung Beetle Way");
		dhPutValue(rs, L".Fields(%S).Value = %d", L"Age", 32);
	}

	/* Move to the first record and alter it */
	if (SUCCEEDED(dhCallMethod(rs, L".MoveFirst")))
	{
		dhPutValue(rs, L".Fields(%S).Value = %S", L"Address", L"1 Kangaroo Rd");
		dhPutValue(rs, L".Fields(%S).Value = %d", L"Age", 45);
	}

	/* Save changes to the last record we edited */
	dhCallMethod(rs, L".Update");

cleanup:
	if (conn)
	{
		int nState = 0;

		dhGetValue(L"%d", &nState, conn, L".State");;

		if (nState != 0 /* adStateClosed */) dhCallMethod(conn, L".Close");
	}

	SAFE_RELEASE(rs);
	SAFE_RELEASE(conn);
}


/* ============================================================================ */
int main(void)
{
	dhInitialize(TRUE);
	dhToggleExceptions(TRUE);

	printf("Running AdoRead sample...\n");
	AdoRead();

	printf("Press ENTER to run AdoDump sample...\n");
	getchar();
	AdoDump();

	printf("Running AdoWrite sample...\n");
	AdoWrite();

	printf("Press ENTER to exit...\n");
	getchar();

	dhUninitialize(TRUE);
	return 0;
}
