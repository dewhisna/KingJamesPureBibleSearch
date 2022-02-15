/****************************************************************************
**
** Copyright (C) 2013-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef SEARCH_COMPLETER_LIST_MODEL_H
#define SEARCH_COMPLETER_LIST_MODEL_H

// ============================================================================

#include "dbstruct.h"

#include <QString>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractListModel>
#include <QList>
#include <QMap>
#include <QStringList>
#include <functional>

// Forward Declarations:
class QTextEdit;
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

typedef std::function<QString ()> TEditorWordCallback;


class CSearchDictionaryListModel : public CSearchStringListModel
{
	Q_OBJECT

public:
	CSearchDictionaryListModel(CDictionaryDatabasePtr pDictionary, TEditorWordCallback pFuncEditorWord, QObject *parent = nullptr);
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
	TEditorWordCallback m_pFuncEditorWord;
};

// ----------------------------------------------------------------------------

class CSearchStrongsDictionaryListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CSearchStrongsDictionaryListModel(CDictionaryDatabasePtr pDictionary, TEditorWordCallback pFuncEditorWord, QObject *parent = nullptr);
	virtual ~CSearchStrongsDictionaryListModel();

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex &index, int role) const override;

	virtual QString cursorWord() const;

private:
	Q_DISABLE_COPY(CSearchStrongsDictionaryListModel)
	CDictionaryDatabasePtr m_pDictionaryDatabase;
	TEditorWordCallback m_pFuncEditorWord;
};

// ============================================================================

class CSoundExSearchCompleterFilter : public QAbstractItemModel
{
	Q_OBJECT

public:
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

#endif	// SEARCH_COMPLETER_LIST_MODEL_H

