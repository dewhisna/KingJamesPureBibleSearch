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

//
// Based loosely on the deprecated Qt XML SAX parser
//

#ifndef XML_H_
#define XML_H_

#include <QString>
#include <QXmlStreamReader>

#include <utility>

// ============================================================================

// Forward Declarations:
class QIODevice;
class QByteArray;

//class CXmlNamespaceSupport;
class CXmlAttributes;
class CXmlHandler;
class CXmlDefaultHandler;
//class CXmlInputSource;
class CXmlParseException;

class CXmlReader;

// ----------------------------------------------------------------------------

class CXmlParseException
{
public:
    explicit CXmlParseException(QXmlStreamReader::Error nError = QXmlStreamReader::NoError,
								const QString &strMessage = QString(),
								int nColumn = -1, int nLine = -1)
		:	m_nError(nError),
			m_strMessage(strMessage),
			m_nColumn(nColumn),
			m_nLine(nLine)
	{ }
    CXmlParseException(const CXmlParseException &other) = default;
	~CXmlParseException() { }

	QXmlStreamReader::Error error() const { return m_nError; }
	QString message() const { return m_strMessage; }
	int columnNumber() const { return m_nColumn; }
	int lineNumber() const { return m_nLine; }

private:
	QXmlStreamReader::Error m_nError = QXmlStreamReader::NoError;
	QString m_strMessage;
	int m_nColumn = 0;
	int m_nLine = 0;
};

// ----------------------------------------------------------------------------

class CXmlAttributes
{
public:
	CXmlAttributes() {}
	CXmlAttributes(const CXmlAttributes &) = default;
	CXmlAttributes(CXmlAttributes &&) noexcept = default;
	CXmlAttributes &operator=(const CXmlAttributes &) = default;
	CXmlAttributes &operator=(CXmlAttributes &&) noexcept = default;

	virtual ~CXmlAttributes() {}

	void swap(CXmlAttributes &other) noexcept
	{
		std::swap(attList, other.attList);
	}

	int index(const QString &qName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	int index(QLatin1String qName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	int index(const QString &uri, const QString &localPart, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	inline int length() const { return attList.count(); }
	inline int count() const { return attList.count(); }
	QString localName(int index) const { return attList.at(index).localname; }
	QString qName(int index) const { return attList.at(index).qname; }
	QString uri(int index) const { return attList.at(index).uri; }
	QString value(int index) const;
	QString value(const QString &qName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	QString value(QLatin1String qName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
	QString value(const QString &uri, const QString &localName, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

	void clear() { attList.clear(); }
	void append(const QString &qName, const QString &uri, const QString &localPart, const QString &value);

private:
	struct Attribute {
		QString qname, uri, localname, value;
	};
	friend class QTypeInfo<Attribute>;
	typedef QList<Attribute> AttributeList;
	AttributeList attList;
};

// ----------------------------------------------------------------------------

class CXmlHandler
{
public:
	virtual ~CXmlHandler() {}

	// XmlContentHandler:
	// ------------------
//	virtual void setDocumentLocator(CXmlLocator *locator) = 0;
	virtual bool startDocument() = 0;
	virtual bool endDocument() = 0;
//	virtual bool startPrefixMapping(const QString &prefix, const QString &uri) = 0;
//	virtual bool endPrefixMapping(const QString &prefix) = 0;
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const CXmlAttributes &atts) = 0;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) = 0;
	virtual bool characters(const QString &ch) = 0;
	virtual bool ignorableWhitespace(const QString &ch) = 0;
	virtual bool processingInstruction(const QString &target, const QString &data) = 0;
//	virtual bool skippedEntity(const QString &name) = 0;

	// XmlErrorHandler:
	// ----------------
//	virtual bool warning(const CXmlParseException &exception) = 0;
	virtual bool error(const CXmlParseException &exception) = 0;
//	virtual bool fatalError(const CXmlParseException &exception) = 0;
	virtual bool needMoreInput(const CXmlParseException &exception) = 0;	// Called only for QXmlStreamReader::PrematureEndOfDocumentError, callback returns true to continue/retry or false to abort

	// XmlDTDHandler:
	// --------------
//	virtual bool notationDecl(const QString &name, const QString &publicId, const QString &systemId) = 0;
//	virtual bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName) = 0;

	// XmlEntityResolver:
	// ------------------
//	virtual bool resolveEntity(const QString &publicId, const QString &systemId, CXmlInputSource *&ret) = 0;

	// XmlLexicalHandler:
	// ------------------
	virtual bool startDTD(const QString &name, const QString &publicId, const QString &systemId) = 0;
	virtual bool endDTD() = 0;
//	virtual bool startEntity(const QString &name) = 0;
//	virtual bool endEntity(const QString &name) = 0;
	virtual bool startCDATA() = 0;
	virtual bool endCDATA() = 0;
	virtual bool comment(const QString &ch) = 0;

	// XmlDeclHandler:
	// ---------------
//	virtual bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value) = 0;
//	virtual bool internalEntityDecl(const QString &name, const QString &value) = 0;
//	virtual bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId) = 0;

	// Common:
	// -------
	virtual QString errorString() const = 0;
};

class CXmlDefaultHandler : public CXmlHandler
{
public:
	CXmlDefaultHandler() {}
	virtual ~CXmlDefaultHandler() {}

	// XmlContentHandler:
	// ------------------
//	virtual void setDocumentLocator(CXmlLocator *locator) override;
	virtual bool startDocument() override;
	virtual bool endDocument() override;
//	virtual bool startPrefixMapping(const QString &prefix, const QString &uri) override;
//	virtual bool endPrefixMapping(const QString &prefix) override;
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const CXmlAttributes &atts) override;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
	virtual bool characters(const QString &ch) override;
	virtual bool ignorableWhitespace(const QString &ch) override;
	virtual bool processingInstruction(const QString &target, const QString &data) override;
//	virtual bool skippedEntity(const QString &name) override;

	// XmlErrorHandler:
	// ----------------
//	virtual bool warning(const CXmlParseException &exception) override;
	virtual bool error(const CXmlParseException &exception) override;
//	virtual bool fatalError(const CXmlParseException &exception) override;
	virtual bool needMoreInput(const CXmlParseException &exception) override;

	// XmlDTDHandler:
	// --------------
//	virtual bool notationDecl(const QString &name, const QString &publicId, const QString &systemId) override;
//	virtual bool unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName) override;

	// XmlEntityResolver:
	// ------------------
//	virtual bool resolveEntity(const QString &publicId, const QString &systemId, CXmlInputSource *&ret) override;

	// XmlLexicalHandler:
	// ------------------
	virtual bool startDTD(const QString &name, const QString &publicId, const QString &systemId) override;
	virtual bool endDTD() override;
//	virtual bool startEntity(const QString &name) override;
//	virtual bool endEntity(const QString &name) override;
	virtual bool startCDATA() override;
	virtual bool endCDATA() override;
	virtual bool comment(const QString &ch) override;

	// XmlDeclHandler:
	// ---------------
//	virtual bool attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value) override;
//	virtual bool internalEntityDecl(const QString &name, const QString &value) override;
//	virtual bool externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId) override;

	// Common:
	// -------
	virtual QString errorString() const override;
};

// ============================================================================

class CXmlReader {
public:
	CXmlReader();
    explicit CXmlReader(QIODevice *device);
    explicit CXmlReader(const QByteArray &data);
    explicit CXmlReader(const QString &data);
    explicit CXmlReader(const char *data);
    virtual ~CXmlReader();

	void setDevice(QIODevice *device) { m_stream.setDevice(device); }
	QIODevice *device() const { return m_stream.device(); }
	void addData(const QByteArray &data) { m_stream.addData(data); }
	void addData(const QString &data) { m_stream.addData(data); }
	void addData(const char *data) { m_stream.addData(data); }
	void clear() { m_stream.clear(); }

	void setXmlHandler(CXmlHandler *pHandler) { m_pHandler = pHandler; }
	CXmlHandler *xmlHandler() const { return m_pHandler; }

	bool separateWhitespace() const { return m_bSeparateWhitespace; }
	void setSeparateWhitespace(bool bSeparateWhitespace) { m_bSeparateWhitespace = bSeparateWhitespace; }

	virtual bool parse();

private:
	bool m_bSeparateWhitespace;				// If true, will pass whitespace-only output to ignorableWhitespace() instead of characters()
	QXmlStreamReader m_stream;
	CXmlHandler *m_pHandler;
	CXmlDefaultHandler m_defaultHandler;
};

// ============================================================================

#endif	// XML_H_

