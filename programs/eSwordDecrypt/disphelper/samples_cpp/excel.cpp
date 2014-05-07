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
excel.cpp:
  Demonstrates outputting formatted data to Excel and using it to create a
chart. Demonstrates using a safe array to efficiently insert data into Excel.
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
using namespace std;


/* **************************************************************************
 * Excel Sample 1:
 *   Demonstrates outputting data to excel and using it to create a chart.
 *
 ============================================================================ */
int ExcelSample1(void)
{
	CDhInitialize init;
	CDispPtr xlApp, xlRange, xlChart;
	UINT i;
	const WCHAR * szHeadings[] = { L"Mammals", L"Birds", L"Reptiles", L"Fishes", L"Plants" };

	dhToggleExceptions(TRUE);

	try
	{
		dhCheck( dhCreateObject(L"Excel.Application", NULL, &xlApp) );

		dhPutValue(xlApp, L".DisplayFullScreen = %b", TRUE);
		dhPutValue(xlApp, L".Visible = %b", TRUE);

		/* xlApp.Workbooks.Add */
		dhCheck( dhCallMethod(xlApp, L".Workbooks.Add") );

		/* Set the worksheet name */
		dhPutValue(xlApp, L".ActiveSheet.Name = %T", TEXT("Critically Endangered"));

		/* Add the column headings */
		for (i=0;i < 5;i++)
		{
			dhPutValue(xlApp, L".ActiveSheet.Cells(%d, %d) = %S", 1, i + 1, szHeadings[i]);
		}

		/* Format the headings */
		WITH1(xlCells, xlApp, L".ActiveSheet.Range(%S)", L"A1:E1")
		{
			dhPutValue(xlCells, L".Interior.Color = %d", RGB(0xee,0xdd,0x82));
			dhPutValue(xlCells, L".Interior.Pattern = %d", 1);  /* xlSolid */
			dhPutValue(xlCells, L".Font.Size = %d", 13);
			dhPutValue(xlCells, L".Borders.Color = %d", RGB(0,0,0));
			dhPutValue(xlCells, L".Borders.LineStyle = %d", 1); /* xlContinuous */
			dhPutValue(xlCells, L".Borders.Weight = %d", 2);    /* xlThin */

		} END_WITH(xlCells);

		WITH(xlSheet, xlApp, L".ActiveSheet")
		{
			/* Set some values */
			dhPutValue(xlSheet, L".Range(%S).Value = %d", L"A2", 184);
			dhPutValue(xlSheet, L".Range(%S).Value = %d", L"B2", 182);
			dhPutValue(xlSheet, L".Range(%S).Value = %d", L"C2", 57);
			dhPutValue(xlSheet, L".Range(%S).Value = %d", L"D2", 162);
			dhPutValue(xlSheet, L".Range(%S).Value = %d", L"E2", 1276);

			/* Output data source */
			dhCallMethod(xlSheet, L".Range(%S).Merge", L"A4:E4");
			dhPutValue(xlSheet, L".Range(%S).Value = %S", L"A4", L"Source: IUCN Red List 2003 (http://www.redlist.org/info/tables/table2.html)");

			/* Apply a border around everything. Note '%m' means missing. */
			dhCallMethod(xlSheet, L".Range(%S).BorderAround(%d, %d, %m, %d)", L"A1:E2", 1, 2, RGB(0,0,0));

			/* Set column widths */
			dhPutValue(xlSheet, L".Columns(%S).ColumnWidth = %e", L"A:E", 12.5);

		} END_WITH_THROW(xlSheet);

		/* Set xlRange = xlApp.ActiveSheet.Range("A1:E2") */
		dhCheck( dhGetValue(L"%o", &xlRange, xlApp, L".ActiveSheet.Range(%S)", L"A1:E2") );

		/* Set xlChart = xlApp.ActiveWorkbook.Charts.Add */
		dhCheck( dhGetValue(L"%o", &xlChart, xlApp, L".ActiveWorkbook.Charts.Add") );

		/* Set up the chart */
		dhCheck( dhCallMethod(xlChart, L".ChartWizard(%o, %d, %d, %d, %d, %d, %b, %S)",
		                      (IDispatch*) xlRange, -4100, 7, 1, 1, 0, FALSE, L"Critically Endangered Plants and Animals") );

		dhPutValue(xlChart, L".HasAxis(%d) = %b", 3, FALSE); /* xlSeries */

		/* Put the chart on our worksheet */
		dhCallMethod(xlChart, L".Location(%d,%S)", 2, L"Critically Endangered");
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	dhToggleExceptions(FALSE);

	dhPutValue(xlApp, L".ActiveWorkbook.Saved = %b", TRUE);

	return 0;
}


/* **************************************************************************
 * Excel Sample 2:
 *   Demonstrates a much faster way of inserting multiple values into excel
 * using an array.
 *
 ============================================================================ */
int ExcelSample2(void)
{
	CDhInitialize init;
	CDispPtr xlApp;
	int i, j;
	VARIANT arr;

	dhToggleExceptions(TRUE);

	try
	{
		dhCheck( dhCreateObject(L"Excel.Application", NULL, &xlApp) );

		dhPutValue(xlApp, L".Visible = %b", TRUE);

		dhCheck( dhCallMethod(xlApp, L".Workbooks.Add") );

		MessageBoxA(NULL, "First the slow method...", NULL, MB_SETFOREGROUND);

		WITH(xlSheet, xlApp, L"ActiveSheet")
		{
			/* Fill cells with values one by one */
			for (i = 1; i <= 15; i++)
			{
				for (j = 1; j <= 15; j++)
				{
					dhCheck( dhPutValue(xlSheet, L".Cells(%d,%d) = %d", i, j, i * j) );
				}
			}

		} END_WITH_THROW(xlSheet);

		MessageBoxA(NULL, "Now the fast way...", NULL, MB_SETFOREGROUND);

		/* xlApp.ActiveSheet.Range("A1:O15").Clear */
		dhCallMethod(xlApp, L".ActiveSheet.Range(%S).Clear", L"A1:O15");

		/* Create a safe array of VARIANT[15][15] */
		{
		   SAFEARRAYBOUND sab[2];

		   arr.vt = VT_ARRAY | VT_VARIANT;              /* An array of VARIANTs. */
		   sab[0].lLbound = 1; sab[0].cElements = 15;   /* First dimension.  [1 to 15] */
		   sab[1].lLbound = 1; sab[1].cElements = 15;   /* Second dimension. [1 to 15] */
		   arr.parray = SafeArrayCreate(VT_VARIANT, 2, sab);
		}
	
		/* Now fill in the array */
		for(i=1; i <= 15; i++)
		{
			for(j=1; j <= 15; j++)
			{
				VARIANT tmp = {0};
				long indices[2];

				indices[0] = i;  /* Index of first dimension */
				indices[1] = j;  /* Index of second dimension */

				tmp.vt = VT_I4;
				tmp.lVal = i * j + 10;

				SafeArrayPutElement(arr.parray, indices, (void*)&tmp);
			}
		}

		/* Set all values in one shot! */
		/* xlApp.ActiveSheet.Range("A1:O15") = arr */
		dhCheck( dhPutValue(xlApp, L".ActiveSheet.Range(%S) = %v", L"A1:O15", &arr) );

		VariantClear(&arr);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}

	dhToggleExceptions(FALSE);

	dhPutValue(xlApp, L".ActiveWorkbook.Saved = %b", TRUE);

	return 0;
}


/* ============================================================================ */
int main(void)
{
	cout << "Running Excel Sample One..." << endl;
	ExcelSample1();

	cout << "\nPress ENTER to run Excel Sample Two..." << endl;
	cin.get();
	ExcelSample2();

	return 0;
}




