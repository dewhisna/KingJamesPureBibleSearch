/****************************************************************************
**
** Copyright (C) 2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "../KJVCanOpener/XML.h"

#include <QXmlAttributes>
#include <QXmlDefaultHandler>
#include <QXmlInputSource>
#include <QXmlSimpleReader>

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include <iostream>

// ============================================================================

class COldXmlHandler : public QXmlDefaultHandler
{
public:
	COldXmlHandler(QFile &fileOut)
		:	m_fileOut(fileOut)
	{ }

	// XmlContentHandler:
	// ------------------
//	virtual void setDocumentLocator(QXmlLocator *locator) override;

	virtual bool startDocument() override
	{
		m_fileOut.write(QString("startDocument\n").toUtf8().data());
		return true;
	}

	virtual bool endDocument() override
	{
		m_fileOut.write(QString("endDocument\n").toUtf8().data());
		return true;
	}

//	virtual bool startPrefixMapping(const QString &prefix, const QString &uri) override;
//	virtual bool endPrefixMapping(const QString &prefix) override;

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override
	{
		m_fileOut.write(QString("startElement: %1, %2, %3\n").arg(namespaceURI, localName, qName).toUtf8().data());
		if (atts.count() != 0) {
			m_fileOut.write(QString("atts:\n").toUtf8().data());
			for (int i = 0; i < atts.count(); ++i) {
				m_fileOut.write(QString("  %1:q=\"%2\"\n    u=\"%3\"\n    l=\"%4\"\n    v=\"%5\"\n").arg(i)
								.arg(atts.qName(i), atts.uri(i), atts.localName(i), atts.value(i)).toUtf8().data());
			}
		} else {
			m_fileOut.write(QString("atts: <none>\n").toUtf8().data());
		}
		return true;
	}

	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override
	{
		m_fileOut.write(QString("endElement: %1, %2, %3\n").arg(namespaceURI, localName, qName).toUtf8().data());
		return true;
	}

	virtual bool characters(const QString &ch) override
	{
		m_fileOut.write(QString("characters: \"%1\"\n").arg(ch).toUtf8().data());
		return true;
	}

	virtual bool ignorableWhitespace(const QString &ch) override
	{
		m_fileOut.write(QString("whitespace: \"%1\"\n").arg(ch).toUtf8().data());
		return true;
	}

	virtual bool processingInstruction(const QString &target, const QString &data) override
	{
		m_fileOut.write(QString("procInst: %1,%2\n").arg(target, data).toUtf8().data());
		return true;
	}

//	virtual bool skippedEntity(const QString &name) override;

	// XmlErrorHandler:
	// ----------------
	virtual bool error(const QXmlParseException &exception) override
	{
		m_fileOut.write(QString("error: \"%1\", Line: %2, Column: %3, PID: \"%4\", SID: \"%5\"\n")
						.arg(exception.message())
						.arg(exception.lineNumber())
						.arg(exception.columnNumber())
						.arg(exception.publicId(), exception.systemId()).toUtf8().data());
		return true;
	}

	virtual bool fatalError(const QXmlParseException &exception) override
	{
		m_fileOut.write(QString("fatal: \"%1\", Line: %2, Column: %3, PID: \"%4\", SID: \"%5\"\n")
						.arg(exception.message())
						.arg(exception.lineNumber())
						.arg(exception.columnNumber())
						.arg(exception.publicId(), exception.systemId()).toUtf8().data());
		return true;
	}

	virtual bool warning(const QXmlParseException &exception) override
	{
		m_fileOut.write(QString("warning: \"%1\", Line: %2, Column: %3, PID: \"%4\", SID: \"%5\"\n")
						.arg(exception.message())
						.arg(exception.lineNumber())
						.arg(exception.columnNumber())
						.arg(exception.publicId(), exception.systemId()).toUtf8().data());
		return true;
	}


	// XmlDTDHandler:
	// --------------
//	virtual bool notationDecl(const QString &name, const QString &publicId, const QString &systemId) override;
//	virtual bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName) override;

	// XmlEntityResolver:
	// ------------------
//	virtual bool resolveEntity(const QString &publicId, const QString &systemId, QXmlInputSource *&ret) override;

	// XmlLexicalHandler:
	// ------------------
	virtual bool startDTD(const QString &name, const QString &publicId, const QString &systemId) override
	{
		m_fileOut.write(QString("startDTD: %1,%2,%3\n").arg(name, publicId, systemId).toUtf8().data());
		return true;
	}

	virtual bool endDTD() override
	{
		m_fileOut.write(QString("endDTD\n").toUtf8().data());
		return true;
	}

//	virtual bool startEntity(const QString &name) override;
//	virtual bool endEntity(const QString &name) override;

	virtual bool startCDATA() override
	{
		m_fileOut.write(QString("startCDATA\n").toUtf8().data());
		return true;
	}

	virtual bool endCDATA() override
	{
		m_fileOut.write(QString("endCDATA\n").toUtf8().data());
		return true;
	}

	virtual bool comment(const QString &ch) override
	{
		m_fileOut.write(QString("comment: \"%1\"\n").arg(ch).toUtf8().data());
		return true;
	}

	// XmlDeclHandler:
	// ---------------
//	virtual bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value) override;
//	virtual bool internalEntityDecl(const QString &name, const QString &value) override;
//	virtual bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId) override;

	// Common:
	// -------
	virtual QString errorString() const override
	{
		return "query-error-string";
	}

private:
	QFile &m_fileOut;
};

// ----------------------------------------------------------------------------

class CNewXmlHandler : public CXmlDefaultHandler
{
public:
	CNewXmlHandler(QFile &fileOut)
		:	m_fileOut(fileOut)
	{ }

	// XmlContentHandler:
	// ------------------
//	virtual void setDocumentLocator(CXmlLocator *locator) override;

	virtual bool startDocument() override
	{
		m_fileOut.write(QString("startDocument\n").toUtf8().data());
		return true;
	}

	virtual bool endDocument() override
	{
		m_fileOut.write(QString("endDocument\n").toUtf8().data());
		return true;
	}

//	virtual bool startPrefixMapping(const QString &prefix, const QString &uri) override;
//	virtual bool endPrefixMapping(const QString &prefix) override;

	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const CXmlAttributes &atts) override
	{
		m_fileOut.write(QString("startElement: %1, %2, %3\n").arg(namespaceURI, localName, qName).toUtf8().data());
		if (atts.count() != 0) {
			m_fileOut.write(QString("atts:\n").toUtf8().data());
			for (int i = 0; i < atts.count(); ++i) {
				m_fileOut.write(QString("  %1:q=\"%2\"\n    u=\"%3\"\n    l=\"%4\"\n    v=\"%5\"\n").arg(i)
								.arg(atts.qName(i), atts.uri(i), atts.localName(i), atts.value(i)).toUtf8().data());
			}
		} else {
			m_fileOut.write(QString("atts: <none>\n").toUtf8().data());
		}
		return true;
	}

	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override
	{
		m_fileOut.write(QString("endElement: %1, %2, %3\n").arg(namespaceURI, localName, qName).toUtf8().data());
		return true;
	}

	virtual bool characters(const QString &ch) override
	{
		m_fileOut.write(QString("characters: \"%1\"\n").arg(ch).toUtf8().data());
		return true;
	}

	virtual bool ignorableWhitespace(const QString &ch) override
	{
		m_fileOut.write(QString("whitespace: \"%1\"\n").arg(ch).toUtf8().data());
		return true;
	}

	virtual bool processingInstruction(const QString &target, const QString &data) override
	{
		m_fileOut.write(QString("procInst: %1,%2\n").arg(target, data).toUtf8().data());
		return true;
	}

//	virtual bool skippedEntity(const QString &name) override;

	// XmlErrorHandler:
	// ----------------
//	virtual bool warning(const CXmlParseException &exception) override;

	virtual bool error(const CXmlParseException &exception) override
	{
		m_fileOut.write(QString("error: \"%1\", Line: %2, Column: %3, Code: %4\n")
						.arg(exception.message())
						.arg(exception.lineNumber())
						.arg(exception.columnNumber())
						.arg(exception.error()).toUtf8().data());
		return true;
	}

//	virtual bool fatalError(const CXmlParseException &exception) override;

	virtual bool needMoreInput(const CXmlParseException &exception) override
	{
		m_fileOut.write(QString("needMoreInput: \"%1\", Line: %2, Column: %3, Code: %4\n")
						.arg(exception.message())
						.arg(exception.lineNumber())
						.arg(exception.columnNumber())
						.arg(exception.error()).toUtf8().data());
		return false;
	}


	// XmlDTDHandler:
	// --------------
//	virtual bool notationDecl(const QString &name, const QString &publicId, const QString &systemId) override;
//	virtual bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName) override;

	// XmlEntityResolver:
	// ------------------
//	virtual bool resolveEntity(const QString &publicId, const QString &systemId, CXmlInputSource *&ret) override;

	// XmlLexicalHandler:
	// ------------------
	virtual bool startDTD(const QString &name, const QString &publicId, const QString &systemId) override
	{
		m_fileOut.write(QString("startDTD: %1,%2,%3\n").arg(name, publicId, systemId).toUtf8().data());
		return true;
	}

	virtual bool endDTD() override
	{
		m_fileOut.write(QString("endDTD\n").toUtf8().data());
		return true;
	}

//	virtual bool startEntity(const QString &name) override;
//	virtual bool endEntity(const QString &name) override;

	virtual bool startCDATA() override
	{
		m_fileOut.write(QString("startCDATA\n").toUtf8().data());
		return true;
	}

	virtual bool endCDATA() override
	{
		m_fileOut.write(QString("endCDATA\n").toUtf8().data());
		return true;
	}

	virtual bool comment(const QString &ch) override
	{
		m_fileOut.write(QString("comment: \"%1\"\n").arg(ch).toUtf8().data());
		return true;
	}

	// XmlDeclHandler:
	// ---------------
//	virtual bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value) override;
//	virtual bool internalEntityDecl(const QString &name, const QString &value) override;
//	virtual bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId) override;

	// Common:
	// -------
	virtual QString errorString() const override
	{
		return "query-error-string";
	}

private:
	QFile &m_fileOut;
};


// ============================================================================

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	int nArgsFound = 0;
	bool bUnknownOption = false;
	QString strXmlPath;
	QString strOut1Path;
	QString strOut2Path;

	for (int ndx = 1; ndx < argc; ++ndx) {
		QString strArg = QString::fromUtf8(argv[ndx]);
		if (!strArg.startsWith("-")) {
			++nArgsFound;
			if (nArgsFound == 1) {
				strXmlPath = strArg;
			} else if (nArgsFound == 2) {
				strOut1Path = strArg;
			} else if (nArgsFound == 3) {
				strOut2Path = strArg;
			}
		} else {
			bUnknownOption = true;
		}
	}

	if ((nArgsFound != 3) ||
		(strXmlPath.isEmpty()) || (strOut1Path.isEmpty()) || (strOut2Path.isEmpty()) ||
		(bUnknownOption)) {
		std::cerr << "xmltest\n";
		std::cerr << QString("Usage: %1 [options] <XMLFileInput> <OldParseOutput> <NewParseOutput>\n\n").arg(argv[0]).toUtf8().data();
		std::cerr << "Where:\n";
		std::cerr << "    <XMLFileInput> = Source XML File to Parse\n";
		std::cerr << "    <OldParseOutput> = Output File to Write Old Parse Results\n";
		std::cerr << "    <NewParseOutput> = Output File to Write New Parse Results\n";
		std::cerr << "\n";
		return -1;
	}

	QFile fileXMLOld(strXmlPath);
	QFile fileXMLNew(strXmlPath);

	if (!fileXMLOld.open(QIODevice::ReadOnly) || !fileXMLNew.open(QIODevice::ReadOnly)) {
		std::cerr << QString("\n\n*** Failed to open XML File \"%1\" for reading.\n").arg(strXmlPath).toUtf8().data();
		return -2;
	}

	QFile fileOut1(strOut1Path);
	if (fileOut1.exists()) {
		if (QMessageBox::question(nullptr, "File Exists", QString("Warning: File \"%1\" exists.  Overwrite!?").arg(strOut1Path)) != QMessageBox::Yes) {
			return -3;
		}
	}
	QFile fileOut2(strOut2Path);
	if (fileOut2.exists()) {
		if (QMessageBox::question(nullptr, "File Exists", QString("Warning: File \"%1\" exists.  Overwrite!?").arg(strOut2Path)) != QMessageBox::Yes) {
			return -3;
		}
	}

	if (!fileOut1.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Out1 File \"%1\" for writing.\n").arg(strOut1Path).toUtf8().data();
		return -4;
	}
	if (!fileOut2.open(QIODevice::WriteOnly)) {
		std::cerr << QString("\n\n*** Failed to open Out2 File \"%1\" for writing.\n").arg(strOut2Path).toUtf8().data();
		return -4;
	}

	QXmlInputSource xmlInput(&fileXMLOld);
	QXmlSimpleReader xmlOldReader;
	COldXmlHandler xmlOldHandler(fileOut1);
	xmlOldReader.setContentHandler(&xmlOldHandler);
	xmlOldReader.setErrorHandler(&xmlOldHandler);
	xmlOldReader.setDTDHandler(&xmlOldHandler);
	xmlOldReader.setEntityResolver(&xmlOldHandler);
	xmlOldReader.setLexicalHandler(&xmlOldHandler);
	xmlOldReader.setDeclHandler(&xmlOldHandler);

	CXmlReader xmlNewReader(&fileXMLNew);
	CNewXmlHandler xmlNewHandler(fileOut2);
	xmlNewReader.setXmlHandler(&xmlNewHandler);

	bool bOldParseResult = xmlOldReader.parse(xmlInput);
	fileOut1.write(QString("parse: %1\n").arg(bOldParseResult ? "true" : "false").toUtf8().data());
	fileOut1.close();
	fileXMLOld.close();

	bool bNewParseResult = xmlNewReader.parse();
	fileOut2.write(QString("parse: %1\n").arg(bNewParseResult ? "true" : "false").toUtf8().data());
	fileOut2.close();
	fileXMLNew.close();

	return 0;
}

// ============================================================================
