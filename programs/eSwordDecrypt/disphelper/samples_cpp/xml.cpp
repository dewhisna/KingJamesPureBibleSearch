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
xml.cpp:
  Demonstrates using MSMXL to download a web page, read an RSS feed and read and
manipulate XML with the XML document object model(DOM).
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;


/* **************************************************************************
 * DownloadWebPage:
 *   Demonstrates using IXMLHTTPRequest to download a web page.
 * http://www.microsoft.com/technet/community/scriptcenter/other/scroth16.mspx
 *
 ============================================================================ */
void DownloadWebPage(LPCTSTR szURL)
{
	CDispPtr objHTTP;
	CDhStringA szResponse, szStatus;

	try
	{
		dhCheck( dhCreateObject(L"MSXML2.XMLHTTP", NULL, &objHTTP) );
		dhCheck( dhCallMethod(objHTTP, L".Open(%S, %T, %b)", L"GET", szURL, FALSE) );
		dhCheck( dhCallMethod(objHTTP, L".Send") );

		dhCheck( dhGetValue(L"%s", &szStatus, objHTTP, L".StatusText") );
		cout << "Status: " << szStatus << endl;

		dhCheck( dhGetValue(L"%s", &szResponse, objHTTP, L".ResponseText") );
		cout << szResponse << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * XMLRead:
 *   Demonstrates obtaining data from an xml file using MSXML.
 *
 ============================================================================ */
void XMLRead(void)
{
	CDispPtr xmlDoc;
	struct CD {
		CDhStringA szTitle, szArtist, szCountry;
		double dblPrice;
	};

	try
	{
		/* Load cd_catalog.xml from file */
		dhCheck( dhCreateObject(L"MSXML.DOMDocument", NULL, &xmlDoc) );
		dhCheck( dhPutValue(xmlDoc, L".Async = %b", FALSE) );
		dhCheck( dhCallMethod(xmlDoc, L".Load(%S)", L"cd_catalog.xml") );

		FOR_EACH(xmlNode, xmlDoc, L".documentElement.childNodes")
		{
			CD CDItem = { 0 };

			dhGetValue(L"%s", &CDItem.szTitle,   xmlNode, L".selectSingleNode(%S).text", L"TITLE");
			dhGetValue(L"%s", &CDItem.szArtist,  xmlNode, L".selectSingleNode(%S).text", L"ARTIST");
			dhGetValue(L"%s", &CDItem.szCountry, xmlNode, L".selectSingleNode(%S).text", L"COUNTRY");
			dhGetValue(L"%e", &CDItem.dblPrice,  xmlNode, L".selectSingleNode(%S).text", L"PRICE");

			cout << "Title: "   << CDItem.szTitle   << endl <<
			        "Artist: "  << CDItem.szArtist  << endl <<
			        "Country: " << CDItem.szCountry << endl <<
			        "Price: $"  << fixed << setprecision(2) << CDItem.dblPrice << endl << endl;

		} NEXT_THROW(xmlNode);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * RSSRead:
 *   Demonstrates reading an rss feed using MSXML.
 *
 ============================================================================ */
void RSSRead(LPCTSTR szURL)
{
	CDispPtr xmlDoc;
	struct RSS {
		CDhStringA szTitle, szLink, szDescription;
	};

	try
	{
		/* Load the rss document from the URL */
		dhCheck( dhCreateObject(L"MSXML.DOMDocument", NULL, &xmlDoc) );
		dhCheck( dhPutValue(xmlDoc, L".Async = %b", FALSE) );
		dhCheck( dhCallMethod(xmlDoc, L".Load(%T)", szURL) );

		FOR_EACH1(xmlNode, xmlDoc, L".documentElement.getElementsByTagName(%S)", L"item")
		{
			RSS RSSItem = { 0 };

			dhGetValue(L"%s", &RSSItem.szTitle,       xmlNode, L".selectSingleNode(%S).text", L"title");
			dhGetValue(L"%s", &RSSItem.szLink,        xmlNode, L".selectSingleNode(%S).text", L"link");
			dhGetValue(L"%s", &RSSItem.szDescription, xmlNode, L".selectSingleNode(%S).text", L"description");

			cout << "Title: " << RSSItem.szTitle << endl <<
			        "Link: "  << RSSItem.szLink  << endl <<
			        "Description: " << setprecision(140) << RSSItem.szDescription << endl << endl;

		} NEXT_THROW(xmlNode);
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * CreateAndAppendNode:
 *   Helper function that wraps up the functions needed to create an element,
 * set its text, and add it to the DOM as a child of xmlParent.
 *
 ============================================================================ */
static HRESULT CreateAndAppendNode(IDispatch * xmlDoc, IDispatch * xmlParent,
                                   LPCWSTR szNodeName, LPCWSTR szNodeText)
{
	CDispPtr xmlNewNode;
	HRESULT hr;

	if (SUCCEEDED(hr = dhGetValue(L"%o", &xmlNewNode, xmlDoc, L".createElement(%S)", szNodeName)) &&
	    SUCCEEDED(hr = dhPutValue(xmlNewNode, L".text = %S", szNodeText)))
	{
		hr = dhCallMethod(xmlParent, L".appendChild(%o)", (IDispatch*) xmlNewNode);
	}

	return hr;
}


/* **************************************************************************
 * MergeXMLFragment:
 *   Helper function that creates an XML Document from a fragment and adds
 * it as a child of the xmlParent node.
 *
 ============================================================================ */
static HRESULT MergeXMLFragment(IDispatch * xmlParent, LPCTSTR szFragment)
{
	CDispPtr xmlDoc, xmlNode;
	HRESULT hr;

	/* Load the fragment in a DOMDocument */
	if (SUCCEEDED(hr = dhCreateObject(L"MSXML.DOMDocument", NULL, &xmlDoc)) &&
	    SUCCEEDED(hr = dhPutValue(xmlDoc, L".Async = %b", FALSE)) &&
	    SUCCEEDED(hr = dhCallMethod(xmlDoc, L".LoadXML(%T)", szFragment)))
	{
		/* Get the root element of the DOMDocument that holds our fragment */
		if (SUCCEEDED(hr = dhGetValue(L"%o", &xmlNode, xmlDoc, L".documentElement")))
		{
			/* Append it to the parent */
			hr = dhCallMethod(xmlParent, L".appendChild(%o)", (IDispatch*) xmlNode);
		}
	}

	return hr;
}



/* **************************************************************************
 * Xml used to demonstrate merging an xml fragment into an existing DOM.
 *
 ============================================================================ */
static const TCHAR * SZ_XML_FRAGMENT = TEXT(" \
	<CD> \
		<TITLE>Unchain my heart</TITLE> \
		<ARTIST>Joe Cocker</ARTIST> \
		<COUNTRY>USA</COUNTRY> \
		<COMPANY>EMI (Merged by DispHelper)</COMPANY> \
		<PRICE>8.20</PRICE> \
		<YEAR>1987</YEAR> \
	</CD>");


/* **************************************************************************
 * XMLWrite:
 *   Demonstrate various methods of altering and adding data to an XML document
 * using MSXML.
 *
 ============================================================================ */
void XMLWrite(void) 
{
	CDispPtr xmlDoc;

	try
	{
		/* Load cd_catalog.xml from file */
		dhCheck( dhCreateObject(L"MSXML.DOMDocument", NULL, &xmlDoc) );
		dhCheck( dhPutValue(xmlDoc, L".Async = %b", FALSE) );
		dhCheck( dhCallMethod(xmlDoc, L".Load(%S)", L"cd_catalog.xml") );

	/* -- Altering an XML record -- */

		/* Get the CD element whose child element 'TITLE' = 'Eros' */
		WITH1(xmlNode, xmlDoc, L".documentElement.selectSingleNode(%S)", L"CD[TITLE='Eros']")
		{
			/* Change its price and company */
			dhPutValue(xmlNode, L".selectSingleNode(%S).text = %S", L"PRICE", L"20.50");
			dhPutValue(xmlNode, L".selectSingleNode(%S).text = %S", L"COMPANY", L"BMG (Price by DispHelper)");

		} END_WITH(xmlNode);

	/* -- Adding a record - Method 1 - Clone an existing node -- */

		/* Clone first child node of the root element */
		WITH1(xmlNode, xmlDoc, L".documentElement.firstChild.cloneNode(%b)", TRUE)
		{
			/* Set its values */
			dhPutValue(xmlNode, L".selectSingleNode(%S).text = %S", L"TITLE",   L"DispHelper Theme Music");
			dhPutValue(xmlNode, L".selectSingleNode(%S).text = %S", L"ARTIST",  L"DispHelper Support Team");
			dhPutValue(xmlNode, L".selectSingleNode(%S).text = %S", L"COUNTRY", L"Solar System");
			/* TODO: Set the rest of the values here */

			/* Add the node as a child of the root node */
			dhCallMethod(xmlDoc, L".documentElement.appendChild(%o)", (IDispatch*) xmlNode);

		} END_WITH(xmlNode);

	/* -- Adding a record - Method 2 - Create and append nodes */

		/* Create an IXMLDOMElement */
		WITH1(xmlNode, xmlDoc, L".CreateElement(%S)", L"CD")
		{
			/* Add the child elements with our helper function */
			CreateAndAppendNode(xmlDoc, xmlNode, L"TITLE",   L"DispHelper Soundtrack");
			CreateAndAppendNode(xmlDoc, xmlNode, L"ARTIST",  L"DispHelper Development Department");
			CreateAndAppendNode(xmlDoc, xmlNode, L"COUNTRY", L"Global");
			CreateAndAppendNode(xmlDoc, xmlNode, L"COMPANY", L"DispHelper");
			CreateAndAppendNode(xmlDoc, xmlNode, L"PRICE",   L"12.75");
			CreateAndAppendNode(xmlDoc, xmlNode, L"YEAR",    L"2003");

			/* Append our new node to the original document */
			dhCallMethod(xmlDoc, L".documentElement.appendChild(%o)", (IDispatch*) xmlNode);

		} END_WITH(xmlNode);

	/* -- Adding a record - Method 3 - Merge an xml fragment */

		/* Get the root element so we can use it as the parent node of the merged xml */
		WITH(xmlNode, xmlDoc, L".documentElement")
		{
			/* Use helper function to merge xml */
			MergeXMLFragment(xmlNode, SZ_XML_FRAGMENT);

		} END_WITH(xmlNode);

	/* Finally, save the xml back to file */

		dhCallMethod(xmlDoc, L"Save(%S)", L"cd_catalog2.xml");
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

	cout << "Running DownloadWebPage sample..." << endl;
	DownloadWebPage(TEXT("http://www.google.com/ie"));

	cout << "Press ENTER to run RSSRead sample..." << endl;
	cin.get();
	RSSRead(TEXT("http://news.bbc.co.uk/rss/newsonline_world_edition/science/nature/rss091.xml"));

	cout << "Press ENTER to run XMLRead sample..." << endl;
	cin.get();
	XMLRead();

	cout << "Running XMLWrite sample..." << endl;
	XMLWrite();

	cout << "\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}




