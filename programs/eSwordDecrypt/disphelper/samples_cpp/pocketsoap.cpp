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
pocketsoap.cpp:
  Demonstrates using the PocketSoap toolkit to utilise several web services.
Play the 'Who wants to be a millionaire' quiz with the help of a web service.
  PocketSoap is available from http://www.pocketsoap.com/
 -- */


#include "disphelper.h"
#include <iostream>
#include <string>
#include <iomanip>
using namespace std;


/* **************************************************************************
 * Temperature:
 *   Demonstrates using XMethod's Temperature service to get the temperature for
 * a specific US zip code.
 * http://xmethods.net/ve2/ViewListing.po;?key=uuid:477CEED8-1EDD-89FA-1070-6C2DBE1685F8
 *
 ============================================================================ */
void Temperature(LPCTSTR szZipCode)
{
	CDispPtr psEnvelope, psTransport;
	CDhStringW szEnvelope;
	double dblTemperature;

	try
	{
		/* Create a PocketSoap envelope */
		dhCheck( dhCreateObject(L"PocketSOAP.Envelope.2", NULL, &psEnvelope) );

		/* Set the methodName and namespace URI */
		dhCheck( dhCallMethod(psEnvelope, L".SetMethod(%S, %S)",
		       	           L"getTemp", L"urn:xmethods-Temperature") );

		/* Create a new parameter */
		dhCheck( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %T)", L"zipcode", szZipCode) );

		/* Create and set up the transport object */
		dhCheck( dhCreateObject(L"PocketSoap.HTTPTransport.2", NULL, &psTransport) );
		dhCheck( dhPutValue(psTransport, L".SOAPAction = %S", L"") );

		/* Serialize the envelope into an xml string and send it to the end point */
		dhCheck( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
		dhCheck( dhCallMethod(psTransport, L".Send(%S, %S)",
		        	L"http://services.xmethods.net:80/soap/servlet/rpcrouter", (LPWSTR) szEnvelope) );

		/* Parse the returned xml back into the COM object model */
		dhCheck( dhCallMethod(psEnvelope, L".Parse(%o)", (IDispatch*) psTransport) );

		/* Extract the return result */
		dhCheck( dhGetValue(L"%e", &dblTemperature, psEnvelope, L".Parameters.Item(%d).Value", 0) );

		cout << "The temperature for zipcode " << szZipCode << " is "
		     << fixed << setprecision(2) << dblTemperature << " degrees fahrenheit." << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * ExchangeRate:
 *   Demonstrates using XMethod's Exchange Rate service to get the exchange
 * rate between the currencies of two countries.
 * http://xmethods.net/ve2/ViewListing.po;?key=uuid:D784C184-99B2-DA25-ED45-3665D11A12E5
 *
 ============================================================================ */
void ExchangeRate(LPCTSTR szCountry1, LPCTSTR szCountry2)
{
	CDispPtr psEnvelope, psTransport;
	CDhStringW szEnvelope;
	double dblRate;

	try
	{
		dhCheck( dhCreateObject(L"PocketSOAP.Envelope.2", NULL, &psEnvelope) );
		dhCheck( dhCallMethod(psEnvelope, L".SetMethod(%S, %S)",
		                          L"getRate", L"urn:xmethods-CurrencyExchange") );

		dhCheck( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %T)", L"country1", szCountry1) );
		dhCheck( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %T)", L"country2", szCountry2) );

		dhCheck( dhCreateObject(L"PocketSoap.HTTPTransport.2", NULL, &psTransport) );
		dhCheck( dhPutValue(psTransport, L".SOAPAction = %S", L"") );

		dhCheck( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
		dhCheck( dhCallMethod(psTransport, L".Send(%S, %S)",
		               L"http://services.xmethods.net:80/soap", (LPWSTR) szEnvelope) );

		dhCheck( dhCallMethod(psEnvelope, L".Parse(%o)", (IDispatch*) psTransport) );

		dhCheck( dhGetValue(L"%e", &dblRate, psEnvelope, L".Parameters.Item(%d).Value", 0) );

		cout << "The exchange rate between " << szCountry1 << " and " << szCountry2 << " is "
		     << fixed << setprecision(2) << dblRate << endl;
	}
	catch (string errstr)
	{
		cerr << "Fatal error details:" << endl << errstr << endl;
	}
}


/* **************************************************************************
 * Millionaire:
 *   Demonstrates using Konrad Wulf's Millionaire Quiz service.
 * http://xmethods.net/ve2/ViewListing.po?key=uuid:0D5D2C03-91DD-EFEA-4167-3554ED3F6FDD
 *
 * This sample first gets the question with alternate answers and then when
 * the user is ready uses a different method to get the correct answer.
 * For added realism we should add 'Sleep(240000)' between question and answer.
 *
 ============================================================================ */
void Millionaire(void)
{
	CDispPtr psEnvelope, psTransport;
	CDhStringW szEnvelope;
	struct MillionaireQuestion {
		CDhStringA szQuestion, szAnswerA, szAnswerB, szAnswerC, szAnswerD, szCorrectAnswer;
		int id, level;
	} Question = { 0 };

	try
	{
		dhCheck( dhCreateObject(L"PocketSOAP.Envelope.2", NULL, &psEnvelope) );
		dhCheck( dhCallMethod(psEnvelope, L".SetMethod(%S, %S)",
		                          L"getRandomQuestion", L"http://tempuri.org/QuizService") );

		dhCheck( dhCreateObject(L"PocketSoap.HTTPTransport.2", NULL, &psTransport) );
		dhCheck( dhPutValue(psTransport, L".SOAPAction = %S", L"") );

		dhCheck( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
		dhCheck( dhCallMethod(psTransport, L".Send(%S, %S)",
		               L"http://java.rus.uni-stuttgart.de:80/quiz/quiz", (LPWSTR) szEnvelope) );

		dhCheck( dhCallMethod(psEnvelope, L".Parse(%o)", (IDispatch*) psTransport) );

		WITH1(psNodes, psEnvelope, L".Parameters.item(%d).Nodes", 0)
		{
			dhGetValue(L"%s", &Question.szQuestion, psNodes, L".ItemByName(%S).Value", L"question");
			dhGetValue(L"%s", &Question.szAnswerA,  psNodes, L".ItemByName(%S).Value", L"answerA");
			dhGetValue(L"%s", &Question.szAnswerB,  psNodes, L".ItemByName(%S).Value", L"answerB");
			dhGetValue(L"%s", &Question.szAnswerC,  psNodes, L".ItemByName(%S).Value", L"answerC");
			dhGetValue(L"%s", &Question.szAnswerD,  psNodes, L".ItemByName(%S).Value", L"answerD");
			dhGetValue(L"%d", &Question.id,         psNodes, L".ItemByName(%S).Value", L"id");
			dhGetValue(L"%d", &Question.level,      psNodes, L".ItemByName(%S).Value", L"difficultyLevel");

		} END_WITH_THROW(psNodes);

		cout << "Question No. " << Question.id << " - $" << Question.level << endl
		     << Question.szQuestion         << endl
		     << "A: " << Question.szAnswerA << endl
		     << "B: " << Question.szAnswerB << endl
		     << "C: " << Question.szAnswerC << endl
		     << "D: " << Question.szAnswerD << endl << endl;

		/* Now get the answer */
		cout << "Press ENTER to get the answer..." << endl;
		cin.get();

		dhCheck( dhCallMethod(psEnvelope, L".Parameters.Clear") );

		dhCheck( dhPutValue(psEnvelope, L".MethodName = %S", L"getCorrectAnswerForQuestionById") );
		dhCheck( dhCallMethod(psEnvelope, L".Parameters.Create(%S, %d)", L"id", Question.id) );

		dhCheck( dhGetValue(L"%S", &szEnvelope, psEnvelope, L".serialize") );
		dhCheck( dhCallMethod(psTransport, L".Send(%S, %S)",
		               L"http://java.rus.uni-stuttgart.de:80/quiz/quiz", (LPWSTR) szEnvelope) );

		dhCheck( dhCallMethod(psEnvelope, L".Parse(%o)", (IDispatch*) psTransport) );

		dhCheck( dhGetValue(L"%S", &Question.szCorrectAnswer, psEnvelope, L".Parameters.Item(%d).Value", 0) );

		cout << "The answer is " << Question.szCorrectAnswer << endl;
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

	cout << "\nRunning Temperature sample...\n" << endl;
	Temperature(TEXT("94107"));

	cout << "\nRunning ExchangeRate sample...\n" << endl;
	ExchangeRate(TEXT("Fiji"), TEXT("Egypt"));

	cout << "\nRunning Millionaire sample...\n" << endl;
	Millionaire();

	cout << "\nPress ENTER to exit..." << endl;
	cin.get();

	return 0;
}




