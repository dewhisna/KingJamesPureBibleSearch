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

#include "XML.h"

// ============================================================================

// --------------
// CXmlAttributes
// --------------

// index : Returns the index of the qualified name or -1 if it wasn't found:
int CXmlAttributes::index(const QString &qName) const
{
	for (int i = 0; i < attList.size(); ++i) {
		if (attList.at(i).qname == qName)
			return i;
	}
	return -1;
}

int CXmlAttributes::index(QLatin1String qName) const
{
	for (int i = 0; i < attList.size(); ++i) {
		if (attList.at(i).qname == qName)
			return i;
	}
	return -1;
}

int CXmlAttributes::index(const QString &uri, const QString &localPart) const
{
	for (int i = 0; i < attList.size(); ++i) {
		const Attribute &att = attList.at(i);
		if (att.uri == uri && att.localname == localPart)
			return i;
	}
	return -1;
}

QString CXmlAttributes::value(int index) const
{
	Q_ASSERT((index >= 0) && (index < count()));
	return attList.at(index).value;
}

QString CXmlAttributes::value(const QString &qName) const
{
	int i = index(qName);
	return ((i > -1) ? attList.at(i).value : QString());
}

QString CXmlAttributes::value(QLatin1String qName) const
{
	int i = index(qName);
	return ((i > -1) ? attList.at(i).value : QString());
}

QString CXmlAttributes::value(const QString &uri, const QString &localName) const
{
	int i = index(uri, localName);
	return ((i > -1) ? attList.at(i).value : QString());
}

void CXmlAttributes::append(const QString &qName, const QString &uri, const QString &localPart, const QString &value)
{
	Attribute att;
	att.qname = qName;
	att.uri = uri;
	att.localname = localPart;
	att.value = value;

	attList.append(att);
}

// ============================================================================

// ------------------
// CXmlDefaultHandler
// ------------------

// XmlContentHandler:
// ------------------
//	void CXmlDefaultHandler::setDocumentLocator(CXmlLocator *locator)

bool CXmlDefaultHandler::startDocument()
{
	return true;
}

bool CXmlDefaultHandler::endDocument()
{
	return true;
}

//	bool CXmlDefaultHandler::startPrefixMapping(const QString &prefix, const QString &uri)
//	bool CXmlDefaultHandler::endPrefixMapping(const QString &prefix)

bool CXmlDefaultHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const CXmlAttributes &atts)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(localName);
	Q_UNUSED(qName);
	Q_UNUSED(atts);
	return true;
}

bool CXmlDefaultHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	Q_UNUSED(namespaceURI);
	Q_UNUSED(localName);
	Q_UNUSED(qName);
	return true;
}

bool CXmlDefaultHandler::characters(const QString &ch)
{
	Q_UNUSED(ch);
	return true;
}

bool CXmlDefaultHandler::ignorableWhitespace(const QString &ch)
{
	Q_UNUSED(ch);
	return true;
}

bool CXmlDefaultHandler::processingInstruction(const QString &target, const QString &data)
{
	Q_UNUSED(target);
	Q_UNUSED(data);
	return true;
}

//	bool CXmlDefaultHandler::skippedEntity(const QString &name)

	// XmlErrorHandler:
	// ----------------
//	bool CXmlDefaultHandler::warning(const CXmlParseException &exception)

bool CXmlDefaultHandler::error(const CXmlParseException &exception)
{
	Q_UNUSED(exception);
	return true;
}

//	bool CXmlDefaultHandler::fatalError(const CXmlParseException &exception)

bool CXmlDefaultHandler::needMoreInput(const CXmlParseException &exception)
{
	Q_UNUSED(exception);
	return false;		// Must return false here to avoid infinite loop
}

	// XmlDTDHandler:
	// --------------
//	bool CXmlDefaultHandler::notationDecl(const QString &name, const QString &publicId, const QString &systemId)
//	bool CXmlDefaultHandler::unparsedEntityDecl(const QString &name, const QString &publicId, const QString &systemId, const QString &notationName)

	// XmlEntityResolver:
	// ------------------
//	bool CXmlDefaultHandler::resolveEntity(const QString &publicId, const QString &systemId, CXmlInputSource *&ret)

	// XmlLexicalHandler:
	// ------------------
bool CXmlDefaultHandler::startDTD(const QString &name, const QString &publicId, const QString &systemId)
{
	Q_UNUSED(name);
	Q_UNUSED(publicId);
	Q_UNUSED(systemId);
	return true;
}

bool CXmlDefaultHandler::endDTD()
{
	return true;
}

//	bool CXmlDefaultHandler::startEntity(const QString &name)
//	bool CXmlDefaultHandler::endEntity(const QString &name)

bool CXmlDefaultHandler::startCDATA()
{
	return true;
}

bool CXmlDefaultHandler::endCDATA()
{
	return true;
}

bool CXmlDefaultHandler::comment(const QString &ch)
{
	Q_UNUSED(ch);
	return true;
}

	// XmlDeclHandler:
	// ---------------
//	bool CXmlDefaultHandler::attributeDecl(const QString &eName, const QString &aName, const QString &type, const QString &valueDefault, const QString &value)
//	bool CXmlDefaultHandler::internalEntityDecl(const QString &name, const QString &value)
//	bool CXmlDefaultHandler::externalEntityDecl(const QString &name, const QString &publicId, const QString &systemId)

	// Common:
	// -------
QString CXmlDefaultHandler::errorString() const
{
	return QString::fromLatin1("error triggered by consumer");
}

// ============================================================================

// ----------
// CXmlReader
// ----------

CXmlReader::CXmlReader()
	:	m_bSeparateWhitespace(false),
		m_pHandler(&m_defaultHandler)
{
}

CXmlReader::CXmlReader(QIODevice *device)
	:	m_bSeparateWhitespace(false),
		m_stream(device),
		m_pHandler(&m_defaultHandler)
{
}

CXmlReader::CXmlReader(const QByteArray &data)
	:	m_bSeparateWhitespace(false),
		m_stream(data),
		m_pHandler(&m_defaultHandler)
{
}

CXmlReader::CXmlReader(const QString &data)
	:	m_bSeparateWhitespace(false),
		m_stream(data),
		m_pHandler(&m_defaultHandler)
{
}

CXmlReader::CXmlReader(const char *data)
	:	m_bSeparateWhitespace(false),
		m_stream(data),
		m_pHandler(&m_defaultHandler)
{
}

CXmlReader::~CXmlReader()
{
}

// ----------------------------------------------------------------------------

bool CXmlReader::parse()
{
	Q_ASSERT(m_pHandler != nullptr);
	bool bRetVal = true;
	bool bDone = false;

	while (!bDone) {
		while (!m_stream.atEnd()) {
			QXmlStreamReader::TokenType nToken = m_stream.readNext();
			switch (nToken) {
				case QXmlStreamReader::NoToken:
					// We must be in a waiting for more input state:
					Q_ASSERT(m_stream.hasError());
					Q_ASSERT(m_stream.atEnd());
					Q_ASSERT(m_stream.error() == QXmlStreamReader::PrematureEndOfDocumentError);
					break;
				case QXmlStreamReader::Invalid:
					Q_ASSERT(m_stream.hasError());
					Q_ASSERT(m_stream.atEnd());				// Error should trigger atEnd to abort processing
					break;
				case QXmlStreamReader::StartDocument:
					if (!m_pHandler->startDocument()) {
						m_stream.raiseError(m_pHandler->errorString());
					}
					break;
				case QXmlStreamReader::EndDocument:
					if (!m_pHandler->endDocument()) {
						m_stream.raiseError(m_pHandler->errorString());
					}
					break;
				case QXmlStreamReader::StartElement:
				{
					CXmlAttributes atts;
					QXmlStreamAttributes srcAtts = m_stream.attributes();
					for (auto const &itr : srcAtts) {
						atts.append(itr.qualifiedName().toString(),
									itr.namespaceUri().toString(),
									itr.name().toString(),
									itr.value().toString());
					}
					if (!m_pHandler->startElement(	m_stream.namespaceUri().toString(),
													m_stream.name().toString(),
													m_stream.qualifiedName().toString(),
													atts)) {
						m_stream.raiseError(m_pHandler->errorString());
					}
					break;
				}
				case QXmlStreamReader::EndElement:
					if (!m_pHandler->endElement(m_stream.namespaceUri().toString(),
												m_stream.name().toString(),
												m_stream.qualifiedName().toString())) {
						m_stream.raiseError(m_pHandler->errorString());
					}
					break;
				case QXmlStreamReader::Characters:
				{
					bool bGood = true;
					if (m_stream.isCDATA() && !m_pHandler->startCDATA()) {
						m_stream.raiseError(m_pHandler->errorString());
						bGood = false;
					}
					if (m_stream.isWhitespace() && separateWhitespace()) {
						if (bGood && !m_pHandler->ignorableWhitespace(m_stream.text().toString())) {
							m_stream.raiseError(m_pHandler->errorString());
							bGood = false;
						}
					} else {
						if (bGood && !m_pHandler->characters(m_stream.text().toString())) {
							m_stream.raiseError(m_pHandler->errorString());
							bGood = false;
						}
					}
					if (bGood && m_stream.isCDATA() && !m_pHandler->endCDATA()) {
						m_stream.raiseError(m_pHandler->errorString());
						bGood = false;
					}
					break;
				}
				case QXmlStreamReader::Comment:
					if (!m_pHandler->comment(m_stream.text().toString())) {
						m_stream.raiseError(m_pHandler->errorString());
					}
					break;
				case QXmlStreamReader::DTD:
					if (!m_pHandler->startDTD(	m_stream.dtdName().toString(),
												m_stream.dtdPublicId().toString(),
												m_stream.dtdSystemId().toString())) {
						m_stream.raiseError(m_pHandler->errorString());
					} else {
						// TODO : Implement XmlDTDHandler and XmlDeclHandler
						if (!m_pHandler->endDTD()) {
							m_stream.raiseError(m_pHandler->errorString());
						}
					}
					break;
				case QXmlStreamReader::EntityReference:
					// TODO : Implement XmlEntityResolver
					break;
				case QXmlStreamReader::ProcessingInstruction:
					if (!m_pHandler->processingInstruction(m_stream.processingInstructionTarget().toString(),
															m_stream.processingInstructionData().toString())) {
						m_stream.raiseError(m_pHandler->errorString());
					}
					break;
			}
		}

		if (m_stream.hasError()) {
			Q_ASSERT(m_stream.error() != QXmlStreamReader::NoError);
			if (m_stream.error() == QXmlStreamReader::PrematureEndOfDocumentError) {
				// Report PrematureEnd to callback and allow it to add more document text:
				CXmlParseException xmlNeedMoreInput(m_stream.error(), m_stream.errorString(), m_stream.columnNumber(), m_stream.lineNumber());
				if (!m_pHandler->needMoreInput(xmlNeedMoreInput)) {
					bDone = true;
					bRetVal = false;
				}
			} else {
				bDone = true;		// All other errors are non-recoverable [as per QXmlStreamReader docs and Qt Source code]
				bRetVal = false;
				CXmlParseException xmlError(m_stream.error(), m_stream.errorString(), m_stream.columnNumber(), m_stream.lineNumber());
				m_pHandler->error(xmlError);
			}
		} else {
			bDone = true;		// atEnd without error means we're done
		}
	}

	return bRetVal;
}

// ============================================================================

