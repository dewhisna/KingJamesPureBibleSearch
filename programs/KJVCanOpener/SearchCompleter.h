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
#include <QList>
#include <QMap>
#include <QStringList>
#include <QTextEdit>

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

	CSearchStringListModel(QObject *parent = NULL)
		:	QAbstractListModel(parent)
	{

	}

	virtual ~CSearchStringListModel()
	{

	}

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const = 0;

	virtual QVariant data(const QModelIndex &index, int role) const = 0;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) = 0;

	virtual QString soundEx(const QString &strDecomposedWord, bool bCache = true) const = 0;		// Return and/or calculate soundEx for the specified Word
	virtual QString cursorWord() const = 0;

	static QString decompose(const QString &strWord, bool bRemoveHyphens);		// Word decompose() function to breakdown and remove accents from words for searching purposes
	static QString deApostrHyphen(const QString &strWord, bool bRemoveHyphens);	// Decompose Apostrophes and Hyphens so matches work correctly and yet rendered text can have the rich set.  (decompose already does this too)
	static QString deApostrophe(const QString &strWord, bool bRemove = false);	// Decompose/remove Apostrophes only
	static QString deHyphen(const QString &strWord, bool bRemove = false);		// Decompose/remove Hyphens only

	virtual const TBasicWordList &basicWordsList() const { return m_lstBasicWords; }
	virtual bool isDynamicModel() const = 0;			// Returns true if the model's BasicWords list changes dynamically via setWordsFromPhrase() like our regular SearchPhrases or if it loads at construction and is static like the Dictionary

signals:
	void modelChanged();

public slots:
	virtual void setWordsFromPhrase(bool bForceUpdate = false) = 0;

protected:
	TBasicWordList m_lstBasicWords;

private:
	Q_DISABLE_COPY(CSearchStringListModel)
};

// ============================================================================

class CSearchParsedPhraseListModel : public CSearchStringListModel
{
	Q_OBJECT

public:
	CSearchParsedPhraseListModel(const CParsedPhrase &parsedPhrase, QObject *parent = NULL);
	virtual ~CSearchParsedPhraseListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual QString soundEx(const QString &strDecomposedWord, bool bCache = true) const;
	virtual QString cursorWord() const;

	virtual bool isDynamicModel() const { return true; }

public slots:
	virtual void setWordsFromPhrase(bool bForceUpdate = false);

private:
	Q_DISABLE_COPY(CSearchParsedPhraseListModel)
	const CParsedPhrase &m_parsedPhrase;
	int m_nCursorWord;						// Last word index of phrase cursor was on
};

// ============================================================================

class CSearchDictionaryListModel : public CSearchStringListModel
{
	Q_OBJECT

public:
	CSearchDictionaryListModel(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QObject *parent = NULL);
	virtual ~CSearchDictionaryListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	virtual QString soundEx(const QString &strDecomposedWord, bool bCache = true) const;
	virtual QString cursorWord() const;

	virtual bool isDynamicModel() const { return false; }

public slots:
	virtual void setWordsFromPhrase(bool bForceUpdate = false);

private:
	Q_DISABLE_COPY(CSearchDictionaryListModel)
	CDictionaryDatabasePtr m_pDictionaryDatabase;
	const QTextEdit &m_editorWord;
};

// ============================================================================

class CSoundExSearchCompleterFilter : public QAbstractItemModel
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


	CSoundExSearchCompleterFilter(CSearchStringListModel *pSearchStringListModel, QObject *parent = NULL);
	~CSoundExSearchCompleterFilter();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex & index) const;
	virtual QModelIndex	index(int row, int column, const QModelIndex & parent = QModelIndex()) const;

	virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
	virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

	void setFilterFixedString(const QString &strPattern);
	inline const QString &filterFixedString() const { return m_strFilterFixedString; }

	QModelIndex firstMatchStringIndex(bool bComposed = false) const;			// bComposed = False for Composed index, True for Decomposed index

	bool soundExEnabled() const { return m_bSoundExEnabled; }
	void setSoundExEnabled(bool bEnabled) { m_bSoundExEnabled = bEnabled; }

	static SOUNDEX_LANGUAGES_ENUM languageValue(const QString &strLanguage);
	static QString soundEx(const QString &strWordIn, SOUNDEX_LANGUAGES_ENUM nLanguage = SELE_ENGLISH, int nLength = 4, SOUNDEX_OPTION_MODE_ENUM nOption = SEOME_ENHANCED);

public slots:
	void en_modelChanged();

protected slots:
	void en_dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

protected:
	void updateModel(bool bResetModel = true);			// bResetModel flag is used to disable reset when setting filter text when SoundEx is disabled, in which case the SoundEx doesn't need to drive a model reset

private:
	QMap<QString, QList<int> > m_mapSoundEx;			// SoundEx lookup of SoundEx words to indexes of source words, set in en_modelChanged() when our base SearchStringListModel changes
	bool m_bSoundExEnabled;								// True when we are resolving SoundEx expressions, False for pass-through
	QString m_strFilterFixedString;
	int m_nFirstComposedMatchStringIndex;
	int m_nFirstDecomposedMatchStringIndex;
	QList<int> m_lstMatchedIndexes;						// Matched indexes in proxied SearchStringListModel that match the FixedString Filter
	CSearchStringListModel *m_pSearchStringListModel;
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
	CSearchCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QWidget *parentWidget);
	virtual ~CSearchCompleter();

	virtual CSearchStringListModel *searchStringListModel() { return m_pSearchStringListModel; }
	virtual CSoundExSearchCompleterFilter *soundExFilterModel() { return m_pSoundExFilterModel; }

	virtual SEARCH_COMPLETION_FILTER_MODE_ENUM completionFilterMode() const { return m_nCompletionFilterMode; }
	virtual void setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode);

	virtual QString filterMatchString() const { return m_strFilterMatchString; }

	virtual void selectFirstMatchString();

public slots:
	virtual void setFilterMatchString();
	virtual void setWordsFromPhrase(bool bForceUpdate = false);

private:
	SEARCH_COMPLETION_FILTER_MODE_ENUM m_nCompletionFilterMode;
	CSearchStringListModel *m_pSearchStringListModel;
	CSoundExSearchCompleterFilter *m_pSoundExFilterModel;
	QString m_strFilterMatchString;
};

// ============================================================================

#endif	// SEARCH_COMPLETER_H
