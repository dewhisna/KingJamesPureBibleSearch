/****************************************************************************
**
** Copyright (C) 2016-2020 Donna Whisnant, a.k.a. Dewtronics.
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
// MaxMindDB GeoIP locate resolution
//

#include "mmdblookup.h"

#define DEBUG_MMDB_METADATA 0
#define DEBUG_MMDB_ENTRY_DUMP 0

#define MMDB_EMPTY_ENTRY_OK 1		// Set to 1 to enable non-existant MMDB entry to return with empty JSON rather than error

// ============================================================================

#ifdef USING_MMDB

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <maxminddb.h>

#endif

#include <errno.h>
#include <stdio.h>
#include <assert.h>

#include <QDateTime>
#include <QBuffer>
#include <QRegExp>
#include <QByteArray>

// ============================================================================

#if DEBUG_MMDB_ENTRY_DUMP && defined(USING_MMDB)

class CFileBuffer : public QBuffer
{
public:
	CFileBuffer()
		:	m_pFile(NULL)
	{
		cookie_io_functions_t myCookieFunctions =
					{ .read = fileRead,
					  .write = fileWrite,
					  .seek = fileSeek,
					  .close = fileClose
					};

		m_pFile = fopencookie(this, "wb", myCookieFunctions);
		if (m_pFile) {
			open(QIODevice::WriteOnly);
		}
	}

	~CFileBuffer()
	{
		if (m_pFile) {
			fclose(m_pFile);
		}
	}

	FILE *file() const { return m_pFile; }

protected:
	static ssize_t fileRead(void *cookie, char *buffer, size_t size)
	{
		Q_UNUSED(cookie);
		Q_UNUSED(buffer);
		Q_UNUSED(size);
		return 0;
	}

	static ssize_t fileWrite(void *cookie, const char *buffer, size_t size)
	{
		CFileBuffer *pBuffer = static_cast<CFileBuffer*>(cookie);
		assert(pBuffer != NULL);
		if (pBuffer) {
			return pBuffer->write(buffer, size);
		} else {
			return 0;
		}
	}

	static int fileSeek(void *cookie, off64_t *position, int whence)
	{
		Q_UNUSED(cookie);
		Q_UNUSED(position);
		Q_UNUSED(whence);
		return -1;
	}

	static int fileClose(void *cookie)
	{
		CFileBuffer *pBuffer = static_cast<CFileBuffer*>(cookie);
		assert(pBuffer != NULL);
		if (pBuffer) {
			pBuffer->m_pFile = NULL;
			pBuffer->close();
		} else {
			return EOF;
		}
		return 0;
	}

private:
	FILE *m_pFile;
};

#endif	// DEBUG_MMDB_ENTRY_DUMP && defined(USING_MMDB)

// ============================================================================

#ifdef USING_MMDB

static void print_indentation(QBuffer &buffer, int i)
{
	while (i--) buffer.putChar(' ');
}

static MMDB_entry_data_list_s *entry_data_list_to_JSON(QBuffer &buffer, MMDB_entry_data_list_s *entry_data_list, int indent, int *status)
{
	assert(entry_data_list != NULL);

	switch (entry_data_list->entry_data.type) {
		case MMDB_DATA_TYPE_MAP:
		{
			uint32_t size = entry_data_list->entry_data.data_size;

			print_indentation(buffer, indent);
			buffer.write("{\n");
			indent += 2;

			bool bFirst = true;
			for (entry_data_list = entry_data_list->next; size && entry_data_list; size--) {
				if (!bFirst) buffer.write(",\n");
				bFirst = false;

				if (MMDB_DATA_TYPE_UTF8_STRING !=
					entry_data_list->entry_data.type) {
					*status = MMDB_INVALID_DATA_ERROR;
					return NULL;
				}
				QString key(QByteArray(entry_data_list->entry_data.utf8_string, entry_data_list->entry_data.data_size));
				print_indentation(buffer, indent);
				buffer.write(QString("\"%1\": \n").arg(key).toUtf8());

				entry_data_list = entry_data_list_to_JSON(buffer, entry_data_list->next, indent + 2, status);
				if (MMDB_SUCCESS != *status) return NULL;
			}
			if (!bFirst) buffer.write("\n");

			indent -= 2;
			print_indentation(buffer, indent);
			buffer.write("}");
		}
		break;
	case MMDB_DATA_TYPE_ARRAY:
		{
			uint32_t size = entry_data_list->entry_data.data_size;

			print_indentation(buffer, indent);
			buffer.write("[\n");
			indent += 2;

			bool bFirst = true;
			for (entry_data_list = entry_data_list->next; size && entry_data_list; size--) {
				if (!bFirst) buffer.write(",\n");
				bFirst = false;

				entry_data_list = entry_data_list_to_JSON(buffer, entry_data_list, indent, status);
				if (MMDB_SUCCESS != *status) return NULL;
			}
			if (!bFirst) buffer.write("\n");

			indent -= 2;
			print_indentation(buffer, indent);
			buffer.write("]");
		}
		break;
	case MMDB_DATA_TYPE_UTF8_STRING:
		{
			QString string(QByteArray(entry_data_list->entry_data.utf8_string, entry_data_list->entry_data.data_size));
			print_indentation(buffer, indent);
			buffer.write(QString("\"%1\"").arg(string).toUtf8());
			entry_data_list = entry_data_list->next;
		}
		break;
	case MMDB_DATA_TYPE_BYTES:
		{
			QString hex_string;
			const uint8_t *pData = entry_data_list->entry_data.bytes;
			uint32_t nSize = entry_data_list->entry_data.data_size;
			while (nSize--) {
				hex_string += QString("%1").arg(*pData++, 2, 16, QChar('0')).toUpper();
			}
			print_indentation(buffer, indent);
			buffer.write(QString("\"%1\"").arg(hex_string).toUtf8());
			entry_data_list = entry_data_list->next;
		}
		break;
	case MMDB_DATA_TYPE_DOUBLE:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.double_value).toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	case MMDB_DATA_TYPE_FLOAT:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.float_value).toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	case MMDB_DATA_TYPE_UINT16:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.uint16).toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	case MMDB_DATA_TYPE_UINT32:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.uint32).toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	case MMDB_DATA_TYPE_BOOLEAN:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.boolean ? "true" : "false").toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	case MMDB_DATA_TYPE_UINT64:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.uint64).toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	case MMDB_DATA_TYPE_UINT128:
	{
		print_indentation(buffer, indent);
#if MMDB_UINT128_IS_BYTE_ARRAY
		QString hex_string;
		const uint8_t *pData = (const uint8_t *)entry_data_list->entry_data.uint128
		uint32_t nSize = 16;
		while (nSize--) {
			hex_string += QString("%1").arg(*pData++, 2, 16, QChar('0')).toUpper();
		}
		buffer.write(QString("0x%1").arg(hex_string).toUtf8());
#else
		uint64_t high = entry_data_list->entry_data.uint128 >> 64;
		uint64_t low = (uint64_t)entry_data_list->entry_data.uint128;
		buffer.write(QString("0x%1%2").arg(high, 16, 16, QChar('0')).arg(low, 16, 16, QChar('0')).toUtf8());
#endif
		entry_data_list = entry_data_list->next;
		break;
	}
	case MMDB_DATA_TYPE_INT32:
		print_indentation(buffer, indent);
		buffer.write(QString("%1").arg(entry_data_list->entry_data.int32).toUtf8());
		entry_data_list = entry_data_list->next;
		break;
	default:
		*status = MMDB_INVALID_DATA_ERROR;
		return NULL;
	}

	*status = MMDB_SUCCESS;
	return entry_data_list;
}

static int MMDB_entry_data_list_to_JSON(QBuffer &buffer, MMDB_entry_data_list_s *entry_data_list, int indent)
{
	int nStatus = MMDB_SUCCESS;
	entry_data_list_to_JSON(buffer, entry_data_list, indent, &nStatus);
	if ((nStatus == MMDB_SUCCESS) && (!buffer.data().isEmpty())) buffer.write("\n");
	return nStatus;
}

#endif	// USING_MMDB

// ============================================================================

QString CMMDBLookup::m_strMMDBPath;

void CMMDBLookup::setMMDBPath(const QString &strDBPath)
{
	m_strMMDBPath = strDBPath;
}

// ----------------------------------------------------------------------------

CMMDBLookup::CMMDBLookup()
{
#ifdef USING_MMDB
	if (m_strMMDBPath.isEmpty()) {
		m_strLastError = MMDB_strerror(MMDB_FILE_OPEN_ERROR);
	} else {
		m_pMMDB = new MMDB_s;

		int status = MMDB_open(m_strMMDBPath.toUtf8().data(), MMDB_MODE_MMAP, m_pMMDB);

		if (MMDB_SUCCESS != status) {
			m_strLastError = QString("Can't open %s - %s").arg(m_strMMDBPath).arg(MMDB_strerror(status));
			if (MMDB_IO_ERROR == status) {
				m_strLastError += QString(" : IO error: %s").arg(strerror(errno));		// Thread safe???
			}
			delete m_pMMDB;
			m_pMMDB = NULL;
		}

#if DEBUG_MMDB_METADATA
		qDebug("%s", getMetaDataDetail().toUtf8().data());
#endif

	}
#endif	// USING_MMDB
}

CMMDBLookup::~CMMDBLookup()
{
#ifdef USING_MMDB
	if (m_pMMDB) {
		MMDB_close(m_pMMDB);
		delete m_pMMDB;
		m_pMMDB = NULL;
	}
#endif
}

bool CMMDBLookup::lookup(QString &strResultsJSON, const QString &strIPAddress, const QString &strPartialDataPath)
{
	bool bRetVal = true;

#ifdef USING_MMDB
	if (!m_pMMDB) {
		m_strLastError = "No MMDB File Open";
		return false;
	}

	int nGAIerror;
	int nMMDBerror;
	MMDB_lookup_result_s lookupResult = MMDB_lookup_string(m_pMMDB, strIPAddress.toUtf8().data(), &nGAIerror, &nMMDBerror);

	if (nGAIerror) {
		m_strLastError = QString("Error from call to getaddrinfo for %1 - %2")
							.arg(strIPAddress).arg(gai_strerror(nGAIerror));
		return false;
	}

	if (MMDB_SUCCESS != nMMDBerror) {
		m_strLastError = QString("Got an error from the maxminddb library: %1").arg(MMDB_strerror(nMMDBerror));
		return false;
	}

	MMDB_entry_data_list_s *entry_data_list = NULL;

	if (lookupResult.found_entry) {
		int nStatus;
		if (!strPartialDataPath.isEmpty()) {
			MMDB_entry_data_s entry_data;
			const char *pPartialDataPath = strPartialDataPath.toUtf8().data();
			nStatus = MMDB_aget_value(&lookupResult.entry, &entry_data, &pPartialDataPath);
			if (MMDB_SUCCESS == nStatus) {
				if (entry_data.offset) {
					MMDB_entry_s entry = { .mmdb = m_pMMDB, .offset = entry_data.offset };
					nStatus = MMDB_get_entry_data_list(&entry, &entry_data_list);
				} else {
					bRetVal = false;
					m_strLastError = "No data was found at the partial lookup path specified";
				}
			}
		} else {
			nStatus = MMDB_get_entry_data_list(&lookupResult.entry, &entry_data_list);
		}
		if (MMDB_SUCCESS != nStatus) {
			bRetVal = false;
			m_strLastError = QString("Got an error looking up the entry data - %1").arg(MMDB_strerror(nStatus));
		} else if (NULL != entry_data_list) {
#if DEBUG_MMDB_ENTRY_DUMP
			CFileBuffer fileBuf;
			MMDB_dump_entry_data_list(fileBuf.file(), entry_data_list, 2);
			fclose(fileBuf.file());
			qDebug("MMDB Dump:\n%s", fileBuf.buffer().data());
#endif
			QBuffer bufJSON;
			bufJSON.open(QIODevice::WriteOnly);
			nStatus = MMDB_entry_data_list_to_JSON(bufJSON, entry_data_list, 0);
			if (MMDB_SUCCESS != nStatus) {
				bRetVal = false;
				m_strLastError = QString("Error generating JSON data - %1").arg(MMDB_strerror(nStatus));
			}
			bufJSON.close();
			strResultsJSON = bufJSON.data();
		}
	} else {
		m_strLastError = QString("Could not find an entry for this IP address (%1)").arg(strIPAddress);
#if MMDB_EMPTY_ENTRY_OK
		strResultsJSON = "{ }";
		return true;					// Return OK with empty results so we don't escalate
#else
		return false;
#endif
	}

	MMDB_free_entry_data_list(entry_data_list);
#else
	Q_UNUSED(strResultsJSON);
	Q_UNUSED(strIPAddress);
	Q_UNUSED(strPartialDataPath);
	bRetVal = false;
	m_strLastError = "No MMDB Support";
#endif

	return bRetVal;
}

QString CMMDBLookup::lastError() const
{
	return m_strLastError;
}

QString CMMDBLookup::getMetaDataDetail() const
{
	QString strMetaData;

#ifdef USING_MMDB

	QDateTime dtBuildEpoch = QDateTime::fromTime_t(m_pMMDB->metadata.build_epoch);
	QString strEpochDate = QString("%1 UTC").arg(dtBuildEpoch.toString("yyyy-MM-dd HH:mm:ss"));
	strMetaData = QString(	"  MMDB Database metadata\n"
							"    Node count:    %1\n"
							"    Record size:   %2 bits\n"
							"    IP version:    IPv%3\n"
							"    Binary format: %4.%5\n"
							"    Build epoch:   %6 (%7)\n"
							"    Type:          %8\n"
							"    Languages:     ")
							.arg(m_pMMDB->metadata.node_count)
							.arg(m_pMMDB->metadata.record_size)
							.arg(m_pMMDB->metadata.ip_version)
							.arg(m_pMMDB->metadata.binary_format_major_version)
							.arg(m_pMMDB->metadata.binary_format_minor_version)
							.arg(m_pMMDB->metadata.build_epoch)
							.arg(strEpochDate)
							.arg(m_pMMDB->metadata.database_type);

	for (size_t i = 0; i < m_pMMDB->metadata.languages.count; ++i) {
		if (i!=0) strMetaData += QChar(' ');
		strMetaData += QString("%1").arg(m_pMMDB->metadata.languages.names[i]);
	}
	strMetaData += QChar('\n');

	if (m_pMMDB->metadata.description.count) {
		strMetaData += "    Description:\n";
	}
	for (size_t i = 0; i < m_pMMDB->metadata.description.count; ++i) {
		strMetaData += QString("      %1:   %2\n")	.arg(m_pMMDB->metadata.description.descriptions[i]->language)
													.arg(m_pMMDB->metadata.description.descriptions[i]->description);
	}
	strMetaData += QChar('\n');

#endif

	return strMetaData;
}

// ============================================================================
