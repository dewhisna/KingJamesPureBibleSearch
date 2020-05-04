/****************************************************************************
**
** Copyright (C) 2014-2020 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QString>
#include <QPair>
#include <QList>
#include <QMap>
#include <QTranslator>
#include <QLocale>
#include <QPointer>

// ============================================================================

class CTranslator : public QObject
{
	Q_OBJECT

private:
	CTranslator(const QString &strLangName = QString(), const QString &strTranslationFilename = QString(), QObject *pParent = NULL);
	friend class CTranslatorList;				// Only creatable by our container

public:
	virtual ~CTranslator();

	QString langName() const { return m_strLangName; }			// Name from our resource filename (as indexed by CTranslatorList)
	QString name(bool bLangOnly) const;							// Resolved Locale name.  If bLangOnly is True, this will automatically be truncated to the first "_" (language part only)
	QString nativeLanguageName() const;							// Translated name for translator in currently selected local (via TR)

	QTranslator &translatorApp() { return m_translatorApp; }
	QTranslator &translatorwwWidgets() { return m_translatorwwWidgets; }
	QTranslator &translatorQt() { return m_translatorQt; }
	QLocale &locale() { return m_locale; }
	bool isLoaded() const { return m_bLoaded; }

private:
	QString m_strLangName;
	QTranslator m_translatorApp;
	QTranslator m_translatorwwWidgets;
	QTranslator m_translatorQt;
	QLocale m_locale;
	bool m_bLoaded;
};

typedef QPointer<CTranslator> TTranslatorPtr;

// ============================================================================

class CTranslatorList : public QObject
{
	Q_OBJECT

private:				// Enforce Singleton:
	CTranslatorList(QObject *pParent = NULL);

public:
	virtual ~CTranslatorList();
	static CTranslatorList *instance();

	bool setApplicationLanguage(const QString &strLangName = QString());			// Select desired language and set on application (empty string means system locale language)
	TTranslatorPtr currentApplicationLanguage() const { return m_pCurrentTranslator; }

	typedef QPair<QString, QString> TLanguageName;	// Pairs containing LanguageID (en, fr, etc) and NativeLanguageName (English, French, etc)
	QList<TLanguageName> languageList() const;		// Returns list of supported language name pairs
	TTranslatorPtr translator(const QString &strLangName = QString()) const;		// Return Translator object for specified language name or the system locale one if empty name

private:
	typedef QMap<QString, TTranslatorPtr> TTranslatorMap;

	TTranslatorPtr m_pSystemLocaleTranslator;		// Locator for the system locale.  May or may not exist in our map
	TTranslatorMap m_mapTranslators;				// Map of language id (en, fr, es, etc) to the QTranslator object that supports it

	TTranslatorPtr m_pCurrentTranslator;			// Points to the current Translator in use (set in application)
};

// ============================================================================

// Globals:

extern QString g_strTranslationsPath;
extern QString g_strTranslationFilenamePrefix;

// ============================================================================

#endif // TRANSLATOR_H
