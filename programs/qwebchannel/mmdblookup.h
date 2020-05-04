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

#ifndef MMDBLOOKUP_H
#define MMDBLOOKUP_H

// ============================================================================

#include <QString>

// Forward Declarations
class CMyApplication;
struct MMDB_s;

// ============================================================================

class CMMDBLookup
{
public:
	CMMDBLookup();
	~CMMDBLookup();

	bool lookup(QString &strResultsJSON, const QString &strIPAddress, const QString &strPartialDataPath = QString());
	QString lastError() const;	// Returns error detail for last lookup

	static QString getMMDBPath() { return m_strMMDBPath; }

protected:
	friend class CMyApplication;
	static void setMMDBPath(const QString &strDBPath);

	QString getMetaDataDetail() const;

private:
#ifdef USING_MMDB
	MMDB_s *m_pMMDB;		// MaxMind Database File in use
#endif
	QString m_strLastError;
	static QString m_strMMDBPath;
};

// ============================================================================

#endif	// MMDBLOOKUP_H
