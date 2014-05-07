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


/* NEWS FLASH: Microsoft is retiring the MSSoap toolkit in favor of .NET.
 * Please consider using PocketSoap instead.
 *
 * Note: If you get an error "Loading of the WSDL file failed, ..." when 
 * running these samples please download the WSDL file and update the path
 * to the local copy in the call to mssoapinit(). There seems to be an error
 * in the MSXML http client that sometimes causes this error.
 */


/* --
soap.cpp:
  Demonstrates using the MSSoap toolkit to utilise several web services.
Web services demonstrated include Google search, spell checker and cache viewer.

  The population web service sample demonstrates how to deal with a returned
safe array.

IMPORTANT NOTE - READ FIRST:
  To limit excessive use, Google requires that those who wish to use the Google Web
service acquire a unique license key (which is free to obtain). This license key
is used to limit individuals to no more than 1,000 calls to the Google Web
service per day.

  I have left the license key in below. You must ONLY use this license key to run
these samples. If you wish to develop these samples or any other code that uses the 
Google web service you MUST register to obtain your own key.

  If others have failed to do this or stolen the key, the provided key may no
longer work and you will have to acquire your own key to run these samples.

  PLEASE only use the key to run these samples a couple of times. If you need to run
the samples more please acquire your own key.

  The Google web api home page, where you can acquire a key, is at
http://www.google.com/apis/

MSSOAP NOTES:
  These samples use the high-level functionality of the MSSoap toolkit. This is a
three step process.
  1. Create the MSSoap.SoapClient object.
  2. Load the WSDL file for the web service. WSDL stands for web services
description language and describes the methods and types exposed by the web service.
  3. Call the web service methods which have been added to the COM object in step 2.

  Generally you should download the wsdl file to disk and load it from there for
greater speed and efficiency. If you are using multiple calls to a web service you
should only load the WSDL file once.

  A large listing of web services can be found at http://www.xmethods.com/

  If you require the MSSoap toolkit search for 'soap' at the Windows Download Centre or try
http://www.microsoft.com/downloads/details.aspx?familyid=c943c0dd-ceec-4088-9753-86f052ec8450
 -- */


#include "disphelper.h"
#include <stdio.h>
#include <wchar.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
using namespace std;

/* DO NOT USE THIS KEY EXCEPT TO RUN THESE SAMPLES A LIMITED NUMBER OF TIMES */
const WCHAR * szGoogleKey = L"pre9s/xQFHKRSutekE8K5bS7p0nD/+T4";


/* **************************************************************************
 * GoogleSpellChecker:
 *   Demonstrates using Google's spell checking service.
 *
 ============================================================================ */
void GoogleSpellChecker(LPCTSTR szWords)
{
	CDispPtr SoapClient;
	CDhStringA szSuggestions;

	cout << "Checking spelling of '" << szWords << "'" << endl;

	try
	{
		dhCheck( dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient) );
		/* dhCheck( dhPutValue(SoapClient, L".ClientProperty(%S) = %b", L"ServerHTTPRequest", TRUE) ); */
		dhCheck( dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://api.google.com/GoogleSearch.wsdl") );
		dhCheck( dhGetValue(L"%s", &szSuggestions, SoapClient, L".doSpellingSuggestion(%S, %T)", szGoogleKey , szWords) );

		cout << "Suggestion(s): " << szSuggestions << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * StockQuote:
 *   Demonstrates using XMethod's delayed stock quote service to receive
 * a stock price (delayed by 20 minutes). Note: Uses port 9090.
 * See http://services.xmethods.net/ve2/ViewListing.po;?key=uuid:889A05A5-5C03-AD9B-D456-0E54A527EDEE
 *
 ============================================================================ */
void StockQuote(LPCTSTR szStockSymbol)
{
	CDispPtr SoapClient;
	double dblStockPrice;

	try
	{
		dhCheck( dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient) );
		dhCheck( dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://services.xmethods.net/soap/urn:xmethods-delayed-quotes.wsdl") );
		dhCheck( dhGetValue(L"%e", &dblStockPrice, SoapClient, L".getQuote(%T)", szStockSymbol) );

		cout << "Stock Price(" << szStockSymbol << "):" 
		     << fixed << setprecision(2) << dblStockPrice << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * GoogleCacheViewer:
 *   Demonstrates using the Google web api to get the cached copy of a page
 * and display it in Internet Explorer.
 *
 ============================================================================ */
void GoogleCacheViewer(LPCTSTR szWords)
{
	CDispPtr SoapClient, ieApp;
	CDhStringB szCache;
	BOOL bIsBusy;

	try
	{
		dhCheck( dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient) );
		/* dhCheck( dhPutValue(SoapClient, L".ClientProperty(%S) = %b", L"ServerHTTPRequest", TRUE) ); */
		dhCheck( dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://api.google.com/GoogleSearch.wsdl") );
		dhCheck( dhGetValue(L"%B", &szCache, SoapClient, L".doGetCachedPage(%S, %T)", szGoogleKey , szWords) );
		
		dhCheck( dhCreateObject(L"InternetExplorer.Application", NULL, &ieApp) );
		dhCheck( dhPutValue(ieApp, L".Visible = %b", TRUE) );
		dhCheck( dhCallMethod(ieApp, L".Navigate(%S)", L"about:blank") );

		/* Wait for blank page to load */
		while (dhCheck(dhGetValue(L"%b", &bIsBusy, ieApp, L".Busy")) && bIsBusy) Sleep(150);

		/* Note that the text in szCache is in UTF8 format.
		 * We should convert it to unicode and pass that in. However, for
		 * simplicity we will treat it as ansi(%s). This will produce incorrect
		 * results for pages that contain non-ascii characters. */
		dhCheck( dhCallMethod(ieApp, L".Document.Write(%s)", (LPSTR) (BSTR) szCache) );
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * Population:
 *   Demonstrates using the population web service provided by AbudantTech.
 * See http://www.xmethods.net/ve2/ViewListing.po;?key=uuid:16C429C7-5724-8335-4ABB-B459F7C55371
 *
 *   This demonstrates handling the return of more complex data. If szCountry
 * is NULL we request a list of supported countries using getCountries(). This
 * method returns an array of strings. MSSoap handles this by returning a safe
 * array of BSTRs in the returned VARIANT which we then print out.
 *
 *   If szCountry is not NULL we call getPopulation(szCountry). The return
 * value of this method is a complex type containing three members(similar to a
 * C struct). MSSoap handles this by returning an IXMLDOMNodeList COM object.
 * In this sample we enumerate the nodes(which are IXMLDOMNode objects)
 * of the list and print out their name/value pairs.
 *
 *   For more information see the documentation for safe array and the
 * IXMLDOMNodeList interface in MSDN.
 *
 ============================================================================ */
void Population(LPCTSTR szCountry)
{
	CDispPtr SoapClient, xmlNodeList;
	VARIANT vtCountries;

	try
	{
		dhCheck( dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient) );
		dhCheck( dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://www.abundanttech.com/webservices/population/population.wsdl") );

		if (!szCountry)
		{
			dhCheck( dhGetValue(L"%v", &vtCountries, SoapClient, L".getCountries()") );

			if (vtCountries.vt == (VT_ARRAY | VT_BSTR))
			{
				BSTR * pbstr;
				UINT i;

				cout << "Listing supported countries...\n" << endl;

				if (SUCCEEDED(SafeArrayAccessData(vtCountries.parray, (void **) &pbstr)))
				{
					for (i=0;i < vtCountries.parray->rgsabound[0].cElements;i++)
					{
						wprintf(L"%s\n", pbstr[i]);
					}

					SafeArrayUnaccessData(vtCountries.parray);
				}
			}
			else
			{
				cout << "## Unexpected data type returned!" << endl;
			}

			VariantClear(&vtCountries);
		}
		else
		{
			dhCheck( dhGetValue(L"%o", &xmlNodeList, SoapClient, L".getPopulation(%T)", szCountry) );

			FOR_EACH(xmlNode, xmlNodeList, NULL)
			{
				CDhStringA szNodeName, szNodeText;

				dhGetValue(L"%s", &szNodeName, xmlNode, L".nodeName");
				cout << szNodeName << ": ";

				dhGetValue(L"%s", &szNodeText, xmlNode, L".text");
				cout << szNodeText << endl;

			} NEXT_THROW(xmlNode);
		}
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * GoogleSearch:
 *   Demonstrates using the Google web api to perform a web search. This sample
 * dumps the result to a file, 'search.htm' in basic html format.
 *
 *   The result of doGoogleSearch is returned as a IXMLDOMNodeList. We use some
 * trickery to get the result as an IXMLDOMNode. This allows us to use the 
 * selectSingleNode to get members by name.
 *
 *   We then enumerate all the child nodes of the resultElements node
 * which are the individual search results and and print out their contents.
 *
 *   It should be noted that while I have elected to get members by name it is
 * far more efficient(although not as robust) to get them by index. For example
 * you can get the documentFiltering member with:
 * dhGetValue(L"%b", &bDocumentFiltering, xmlNodeList, L".item(%d).text", 1);
 * See http://www.codeproject.com/asp/googleapisinasp.asp
 *
 *   This is a sample only, if you simply wish to convert the results
 * to a html page or other text format it is far cleaner and more efficient to
 * use an XSL transform. Please see http://momche.net/res/google/ for a
 * sample.
 *
 ============================================================================ */
void GoogleSearch(LPCTSTR szQuery, UINT iFirstResult)
{
	CDispPtr SoapClient, xmlNodeList, xmlParentNode;

	struct { /* Structure to hold the returned search meta information */
		BOOL bDocumentFiltering, bEstimateIsExact;
		int cEstTotalResults, nEndIndex, nStartIndex;
		double dblSearchTime;
		CDhStringA szSearchTips, szSearchComments, szSearchQuery;
	} GoogleSearch = { 0 };

	struct SearchItem { /* Structure to hold an individual search result */
		CDhStringA szSnippet, szHostName, szURL, szCachedSize;
		CDhStringA szTitle, szSummary, szDirectoryTitle;
		BOOL bRelatedInformation;
	};

	try
	{
		ofstream out("search.htm", ios::out | ios::trunc);

		dhCheck( dhCreateObject(L"MSSoap.SoapClient", NULL, &SoapClient) );
		/* dhCheck( dhPutValue(SoapClient, L".ClientProperty(%S) = %b", L"ServerHTTPRequest", TRUE) ); */
		dhCheck( dhCallMethod(SoapClient, L".mssoapinit(%S)", L"http://api.google.com/GoogleSearch.wsdl") );

		/* Do the search and get the result as an IXMLDOMNodeList */
		dhCheck( dhGetValue(L"%o", &xmlNodeList, SoapClient, L".doGoogleSearch(%S, %T, %d, %d, %b, %S, %b, %S, %S, %S)",
				szGoogleKey, szQuery, iFirstResult, 10, TRUE, L"", FALSE, L"", L"", L"") );

		/* Turn the IXMLDOMNodeList into an IXMLDOMNode so we can use selectSingleNode */
		dhCheck( dhGetValue(L"%o", &xmlParentNode, xmlNodeList, L".item(%d).parentNode", 0) );

		/* Now get the returned information. */
		dhGetValue(L"%b", &GoogleSearch.bDocumentFiltering, xmlParentNode, L".selectSingleNode(%S).text", L"documentFiltering");
		dhGetValue(L"%b", &GoogleSearch.bEstimateIsExact,   xmlParentNode, L".selectSingleNode(%S).text", L"estimateIsExact");
		dhGetValue(L"%d", &GoogleSearch.nEndIndex,          xmlParentNode, L".selectSingleNode(%S).text", L"endIndex");
		dhGetValue(L"%d", &GoogleSearch.nStartIndex,        xmlParentNode, L".selectSingleNode(%S).text", L"startIndex");
		dhGetValue(L"%e", &GoogleSearch.dblSearchTime,      xmlParentNode, L".selectSingleNode(%S).text", L"searchTime");
		dhGetValue(L"%s", &GoogleSearch.szSearchTips,       xmlParentNode, L".selectSingleNode(%S).text", L"searchTips");
		dhGetValue(L"%s", &GoogleSearch.szSearchComments,   xmlParentNode, L".selectSingleNode(%S).text", L"searchComments");
		dhGetValue(L"%s", &GoogleSearch.szSearchQuery,      xmlParentNode, L".selectSingleNode(%S).text", L"searchQuery");
		dhGetValue(L"%d", &GoogleSearch.cEstTotalResults,   xmlParentNode, L".selectSingleNode(%S).text", L"estimatedTotalResultsCount");

		out << "<html><body>" << endl
		    << "Searched the web for <u>" << GoogleSearch.szSearchQuery << "</u>.<br />"                           << endl
	            << "Results " << GoogleSearch.nStartIndex << " to " << GoogleSearch.nEndIndex << " of "
		    << (GoogleSearch.bEstimateIsExact ? "exactly " : "about ") << GoogleSearch.cEstTotalResults << ".<br />" << endl
		    << "Search took " << fixed << setprecision(3) << GoogleSearch.dblSearchTime << " seconds.<br />"       << endl
		    << "Results are " << (GoogleSearch.bDocumentFiltering ? "" : "not ") << "filtered.<br />"                 << endl
		    << "Search Tips: " << GoogleSearch.szSearchTips << "<br />"                                            << endl
		    << "Search Comments: " << GoogleSearch.szSearchComments << "<br />"                                    << endl;


		/* Enumerate the childNodes of the resultElements node which are the individual results */
		FOR_EACH1(xmlResult, xmlParentNode, L".selectSingleNode(%S).childNodes", L"resultElements")
		{
			int nNodeType;
			dhCheck( dhGetValue(L"%d", &nNodeType, xmlResult, L".nodeType") );

			if (nNodeType == 1) /* NODE_ELEMENT - Ignore comment and text nodes. */
			{
				SearchItem Result = { 0 };

				dhGetValue(L"%s", &Result.szCachedSize,        xmlResult, L".selectSingleNode(%S).text", L"cachedSize");
				dhGetValue(L"%s", &Result.szSnippet,           xmlResult, L".selectSingleNode(%S).text", L"snippet");
				dhGetValue(L"%s", &Result.szHostName,          xmlResult, L".selectSingleNode(%S).text", L"hostName");
				dhGetValue(L"%s", &Result.szURL,               xmlResult, L".selectSingleNode(%S).text", L"URL");
				dhGetValue(L"%s", &Result.szTitle,             xmlResult, L".selectSingleNode(%S).text", L"title");
				dhGetValue(L"%s", &Result.szSummary,           xmlResult, L".selectSingleNode(%S).text", L"summary");
				dhGetValue(L"%s", &Result.szDirectoryTitle,    xmlResult, L".selectSingleNode(%S).text", L"directoryTitle");
				dhGetValue(L"%b", &Result.bRelatedInformation, xmlResult, L".selectSingleNode(%S).text", L"relatedInformationPresent");

				out << "\n\n<br /><b>Title:</b> " << Result.szTitle          << "<br />" << endl
				    << "<b>Description:</b> "     << Result.szSummary        << "<br />" << endl
				    << "<b>Snippet:</b> "         << Result.szSnippet        << "<br />" << endl
				    << "<b>Directory Title:</b> " << Result.szDirectoryTitle << "<br />" << endl
				    << "<b>Host Name:</b> "       << Result.szHostName       << "<br />" << endl
				    << "<b>Similar Pages Available:</b> " << (Result.bRelatedInformation ? "True" : "False") << "<br />" << endl
				    << "<a href=\"" << Result.szURL << "\">" << Result.szURL << "</a> - "
				    << Result.szCachedSize << "<br />" << endl;
			}

		} NEXT_THROW(xmlResult);

		out << "</body></html>" << endl;
		out.close();
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

	cout << "Running Google Spell Checking sample..." << endl;
	GoogleSpellChecker(TEXT("Thiis is a teest of Googel spelll checking."));

	cout << "\nRunning Stock Quote sample..." << endl;
	StockQuote(TEXT("YHOO"));
	StockQuote(TEXT("TIVO"));

	cout << "\nRunning Google Cache Viewer sample..." << endl;
	GoogleCacheViewer(TEXT("http://sourceforge.net"));

	cout << "\nRunning Population sample..." << endl;
	Population(TEXT("United States"));
	Population(TEXT("Western Sahara"));
	Population(NULL); /* List supported countries. */

	cout << "\nRunning Google Search sample(output to search.htm)..." << endl;
	GoogleSearch(TEXT("test"), 500);

	cout << "\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}




