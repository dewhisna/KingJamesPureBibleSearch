/****************************************************************************
**
** Copyright (C) 2013 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SEARCH_COMPLETER_H
#define SEARCH_COMPLETER_H

#include "dbstruct.h"

#include <QWidget>
#include <QString>
#include <QVariant>
#include <QModelIndex>
#include <QCompleter>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>

// ============================================================================

// Forward Declarations:
class CParsedPhrase;

// ============================================================================

class CSearchStringListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum SEARCH_STRING_DATA_ROLES_ENUM {
		SOUNDEX_ENTRY_ROLE = Qt::UserRole + 0				// SoundEx completion
	};

	CSearchStringListModel(const CParsedPhrase &parsedPhrase, QObject *parent = NULL);
	virtual ~CSearchStringListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

	static QString decompose(const QString &strWord);			// Word decompose() function to breakdown and remove accents from words for searching purposes

public slots:
	void setWordsFromPhrase();

private:
	Q_DISABLE_COPY(CSearchStringListModel)
	const CParsedPhrase &m_parsedPhrase;
};

// ============================================================================

class CSoundExSearchCompleterFilter : public QSortFilterProxyModel
{
	Q_OBJECT

public:

	enum SOUNDEX_OPTION_MODE_ENUM {
		SEOME_CLASSIC = 0,			// Classic SoundEx
		SEOME_ENHANCED = 1,			// Enhanced SoundEx, but Not Census Codes
		SEOME_CENSUS_NORMAL = 2,	// Normal Census Codes (Used in all censuses including 1920)
		SEOME_CENSUS_SPECIAL = 3	// Special Census Codes (Used intermittently in 1880, *, 1900, 1910) *1890 destroyed in a fire
	};


	enum SOUNDEX_LANGUAGES_ENUM {
		SELE_UNKNOWN = 0,
		SELE_ENGLISH = 1,
		SELE_FRENCH = 2,
		SELE_SPANISH = 3,
		SELE_GERMAN = 4
	};


	CSoundExSearchCompleterFilter(QObject *parent = NULL);
	~CSoundExSearchCompleterFilter();

	static SOUNDEX_LANGUAGES_ENUM languageValue(const QString &strLanguage);
	static QString soundEx(const QString &strWordIn, SOUNDEX_LANGUAGES_ENUM nLanguage = SELE_ENGLISH, int nLength = 4, SOUNDEX_OPTION_MODE_ENUM nOption = SEOME_ENHANCED);

};

// ============================================================================

class CSearchCompleter : public QCompleter
{
	Q_OBJECT

public:
	enum SEARCH_COMPLETION_FILTER_MODE_ENUM {
		SCFME_NORMAL = 0,
		SCFME_UNFILTERED = 1,
		SCFME_SOUNDEX = 2
	};

	CSearchCompleter(const CParsedPhrase &parsedPhrase, QWidget *parentWidget);
	virtual ~CSearchCompleter();

	virtual CSearchStringListModel *searchStringListModel() { return m_pSearchStringListModel; }
	virtual CSoundExSearchCompleterFilter *soundExFilterModel() { return m_pSoundExFilterModel; }

	virtual void setCompletionPrefix(const QString &prefix);

	virtual SEARCH_COMPLETION_FILTER_MODE_ENUM completionFilterMode() const { return m_nCompletionFilterMode; }
	virtual void setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode);

private:
	SEARCH_COMPLETION_FILTER_MODE_ENUM m_nCompletionFilterMode;
	CSearchStringListModel *m_pSearchStringListModel;
	CSoundExSearchCompleterFilter *m_pSoundExFilterModel;
};

// ============================================================================

#endif	// SEARCH_COMPLETER_H
