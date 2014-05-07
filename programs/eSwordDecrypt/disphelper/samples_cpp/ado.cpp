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
ado.cpp:
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
#include <iostream>
#include <string>
using namespace std;


/* **************************************************************************
 * AdoRead:
 *   Demonstrates obtaining data in a recordset and printing it out using ADO.
 *
 ============================================================================ */
void AdoRead(void)
{
	CDispPtr conn, rs;
	BOOL bEOF;
	struct Whale { 	/* Structure to hold a row of information */
		int id;
		CDhStringA szSpecies, szPopulation, szStatus;
	};

	try 
	{
		dhCheck( dhCreateObject(L"ADODB.Connection", NULL, &conn) );

		/* Connect to the data source */
		dhCheck( dhCallMethod(conn, L".Open(%S)", L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Whales.xls;Extended Properties=\"Excel 8.0;HDR=Yes;IMEX=1;\"") );

		/* Execute a select statement to retrieve a recordset */
		dhCheck( dhGetValue(L"%o", &rs, conn, L".Execute(%S)", L"SELECT ID, Species, Population, Status FROM [Whales$] ORDER BY Species") );

		/* Loop through each row until EOF is TRUE */
		while (dhCheck(dhGetValue(L"%b", &bEOF, rs, L".EOF")) && !bEOF)
		{
			Whale WhaleRow = { 0 };

			/* Get the value of each field that we want */
			dhGetValue(L"%d", &WhaleRow.id,           rs, L".Fields(%S).Value", L"ID");
			dhGetValue(L"%s", &WhaleRow.szSpecies,    rs, L".Fields(%S).Value", L"Species");
			dhGetValue(L"%s", &WhaleRow.szPopulation, rs, L".Fields(%S).Value", L"Population");
			dhGetValue(L"%s", &WhaleRow.szStatus,     rs, L".Fields(%S).Value", L"Status");

			/* Output information to console */
			cout << "ID:         " << WhaleRow.id           << endl <<
			        "Species:    " << WhaleRow.szSpecies    << endl <<
			        "Population: " << WhaleRow.szPopulation << endl <<
			        "Status:     " << WhaleRow.szStatus     << endl << endl;

			/* Move to next record */
			dhCheck( dhCallMethod(rs, L".MoveNext") );
		}
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	if (conn)
	{
		int nState = 0;

		/* Get the connection state */
		dhGetValue(L"%d", &nState, conn, L".State");

		/* Closing the connection also closes active recordsets associated with it */
		if (nState != 0 /* adStateClosed */) dhCallMethod(conn, L".Close");
	}
}


/* **************************************************************************
 * AdoDump:
 *   Similar to the above sample, this sample differs by demonstrating
 * how to dump a recordset to output when you do not know its structure.
 *
 ============================================================================ */
void AdoDump(void)
{
	CDispPtr conn, rs;
	BOOL bEOF;

	try
	{
		dhCheck( dhCreateObject(L"ADODB.Connection", NULL, &conn) );

		dhCheck( dhCallMethod(conn, L".Open(%S)", L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Whales.xls;Extended Properties=\"Excel 8.0;HDR=Yes;IMEX=1;\"") );

		dhCheck( dhGetValue(L"%o", &rs, conn, L".Execute(%S)", L"SELECT * FROM [Whales$]") );

		while (dhCheck(dhGetValue(L"%b", &bEOF, rs, L".EOF")) && !bEOF)
		{
			/* Enumerate each field and dump its name/value pair */
			FOR_EACH(field, rs, L".Fields")
			{
				CDhStringA szFieldName, szFieldValue;

				dhGetValue(L"%s", &szFieldName,  field, L".Name");
				dhGetValue(L"%s", &szFieldValue, field, L".Value");

				cout << szFieldName << ": " << szFieldValue << endl;

			} NEXT_THROW(field);

			cout << endl;

			dhCheck( dhCallMethod(rs, L".MoveNext") );
		}
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	if (conn)
	{
		int nState = 0;

		dhGetValue(L"%d", &nState, conn, L".State");;

		if (nState != 0 /* adStateClosed */) dhCallMethod(conn, L".Close");
	}
}


/* **************************************************************************
 * AdoWrite:
 *   Demonstrates various methods of adding and altering records using ADO.
 *
 ============================================================================ */
void AdoWrite(void)
{
	CDispPtr conn, rs;

	try
	{
		dhCheck( dhCreateObject(L"ADODB.Connection", NULL, &conn) );

		/* Open the data source. Make sure we have write access. */
		dhCheck( dhCallMethod(conn, L".Open(%S)", L"Provider=Microsoft.Jet.OLEDB.4.0;Data Source=Whales.xls;Extended Properties=\"Excel 8.0;HDR=Yes;IMEX=0;\"") );

		/* Add some records using SQL statements */
		dhCallMethod(conn, L".Execute(%S)", L"INSERT INTO [AddressList$] VALUES ('Jane Doe', '1 Donkey St', 25)");
		dhCallMethod(conn, L".Execute(%S)", L"INSERT INTO [AddressList$] VALUES ('John Citizen', '1 Cheetah St', 74)");

		/* Alter a record using an SQL statement */
		dhCallMethod(conn, L".Execute(%S)", L"UPDATE [AddressList$] SET Address = '1 Elephant St' WHERE Name = 'John Citizen'");

		/* Add 5 to each age using an SQL statement */
		dhCallMethod(conn, L".Execute(%S)", L"UPDATE [AddressList$] SET Age = Age + 5");
	
		/* Using SQL is preferable but now we demonstrate how to update using a recordset */

		/* Create a recordset. The conn.Execute method can only return a read only recordset */
		dhCheck( dhCreateObject(L"ADODB.Recordset", NULL, &rs) );

		/* Open the recordset with write access */
		dhCheck( dhCallMethod(rs, L".Open(%S, %o, %d, %d)", L"[AddressList$]", (IDispatch*) conn, 3, 3) );

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
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	if (conn)
	{
		int nState = 0;

		dhGetValue(L"%d", &nState, conn, L".State");;

		if (nState != 0 /* adStateClosed */) dhCallMethod(conn, L".Close");
	}
}


/* ============================================================================ */
int main(void)
{
	CDhInitialize init;
	dhToggleExceptions(TRUE);

	cout << "Running AdoRead sample..." << endl;
	AdoRead();

	cout << "Press ENTER to run AdoDump sample..." << endl;
	cin.get();
	AdoDump();

	cout << "Running AdoWrite sample..." << endl;
	AdoWrite();

	cout << "Press ENTER to exit..." << endl;
	cin.get();

	return 0;
}
