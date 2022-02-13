/****************************************************************************
**
** Copyright (C) 2013-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#include <QString>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QStringList>

#ifdef QT_WIDGETS_LIB
#include <QWidget>
#include <QCompleter>
#include <QTextEdit>
#endif

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

	CSearchStringListModel(QObject *parent = nullptr)
		:	QAbstractListModel(parent)
	{

	}

	virtual ~CSearchStringListModel()
	{

	}

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override = 0;

	virtual QVariant data(const QModelIndex &index, int role) const override = 0;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override = 0;

	virtual QString soundEx(const QString &strDecomposedWord, bool bCache = true) const = 0;		// Return and/or calculate soundEx for the specified Word
	virtual QString cursorWord() const = 0;

	virtual const TBasicWordList &basicWordsList() const { return m_lstBasicWords; }
	virtual bool isDynamicModel() const = 0;									// Returns true if the model's BasicWords list changes dynamically via setWordsFromPhrase() like our regular SearchPhrases or if it loads at construction and is static like the Dictionary

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
	CSearchParsedPhraseListModel(const CParsedPhrase &parsedPhrase, QObject *parent = nullptr);
	virtual ~CSearchParsedPhraseListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	virtual QString soundEx(const QString &strDecomposedWord, bool bCache = true) const override;
	virtual QString cursorWord() const override;

	virtual bool isDynamicModel() const override { return true; }

public slots:
	virtual void setWordsFromPhrase(bool bForceUpdate = false) override;

private:
	Q_DISABLE_COPY(CSearchParsedPhraseListModel)
	const CParsedPhrase &m_parsedPhrase;
	int m_nCursorWord;						// Last word index of phrase cursor was on
};

// ============================================================================

#ifdef QT_WIDGETS_LIB

class CSearchDictionaryListModel : public CSearchStringListModel
{
	Q_OBJECT

public:
	CSearchDictionaryListModel(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QObject *parent = nullptr);
	virtual ~CSearchDictionaryListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	virtual QString soundEx(const QString &strDecomposedWord, bool bCache = true) const override;
	virtual QString cursorWord() const override;

	virtual bool isDynamicModel() const override { return false; }

public slots:
	virtual void setWordsFromPhrase(bool bForceUpdate = false) override;

private:
	Q_DISABLE_COPY(CSearchDictionaryListModel)
	CDictionaryDatabasePtr m_pDictionaryDatabase;
	const QTextEdit &m_editorWord;
};

// ----------------------------------------------------------------------------

class CSearchStrongsDictionaryListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CSearchStrongsDictionaryListModel(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QObject *parent = nullptr);
	virtual ~CSearchStrongsDictionaryListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;

	virtual QString cursorWord() const;

private:
	Q_DISABLE_COPY(CSearchStrongsDictionaryListModel)
	CDictionaryDatabasePtr m_pDictionaryDatabase;
	const QTextEdit &m_editorWord;
};

#endif

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


	CSoundExSearchCompleterFilter(CSearchStringListModel *pSearchStringListModel, QObject *parent = nullptr);
	~CSoundExSearchCompleterFilter();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	virtual bool hasChildren(const QModelIndex & parent = QModelIndex()) const override;
	virtual QModelIndex parent(const QModelIndex & index) const override;
	virtual QModelIndex	index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;

	virtual QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
	virtual QModelIndex mapToSource(const QModelIndex &proxyIndex) const;

	QVariant data(const QModelIndex &index, int role) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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


#ifndef QT_WIDGETS_LIB

class CSearchCompleter
{
public:
	enum SEARCH_COMPLETION_FILTER_MODE_ENUM {
		SCFME_NORMAL = 0,
		SCFME_UNFILTERED = 1,
		SCFME_SOUNDEX = 2
	};
};

#else

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
	CSearchCompleter(QWidget *parentWidget);
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

protected:
	SEARCH_COMPLETION_FILTER_MODE_ENUM m_nCompletionFilterMode;
	QString m_strFilterMatchString;

private:
	CSearchStringListModel *m_pSearchStringListModel;
	CSoundExSearchCompleterFilter *m_pSoundExFilterModel;
};

// ============================================================================

#if QT_VERSION < 0x050000

// Forward declares:
class CParsedPhrase;

// CComposingCompleter -- Needed to fix a bug in Qt 4.8.x QCompleter whereby
//		inputMethod events get redirected to the popup, but don't come back
//		to the editor because inputContext()->setFocusWidget() never gets
//		called again for the editor:
class CComposingCompleter : public CSearchCompleter
{
	Q_OBJECT

public:
	CComposingCompleter(const CParsedPhrase &parsedPhrase, QWidget *parentWidget)
		:	CSearchCompleter(parsedPhrase, parentWidget)
	{

	}

	CComposingCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QWidget *parentWidget)
		:	CSearchCompleter(pDictionary, editorWord, parentWidget)
	{

	}

	CComposingCompleter(QWidget *parentWidget)
		:	CSearchCompleter(parentWidget)
	{

	}

	~CComposingCompleter()
	{

	}

	virtual bool eventFilter(QObject *obj, QEvent *ev) override;
};

typedef CComposingCompleter SearchCompleter_t;

#else

typedef CSearchCompleter SearchCompleter_t;

#endif

// ============================================================================

class CStrongsDictionarySearchCompleter : public SearchCompleter_t
{
	Q_OBJECT

public:
	CStrongsDictionarySearchCompleter(CDictionaryDatabasePtr pDictionary, const QTextEdit &editorWord, QWidget *parentWidget);
	virtual ~CStrongsDictionarySearchCompleter() { }

	using CSearchCompleter::completionFilterMode;
	virtual void setCompletionFilterMode(SEARCH_COMPLETION_FILTER_MODE_ENUM nCompletionFilterMode) override { m_nCompletionFilterMode = nCompletionFilterMode; }

	using CSearchCompleter::filterMatchString;

	virtual void selectFirstMatchString() override;

public slots:
	virtual void setFilterMatchString() override;
	virtual void setWordsFromPhrase(bool bForceUpdate = false) override;

private:
	CSearchStrongsDictionaryListModel *m_pStrongsListModel;
};

#endif

// ============================================================================

#endif	// SEARCH_COMPLETER_H
