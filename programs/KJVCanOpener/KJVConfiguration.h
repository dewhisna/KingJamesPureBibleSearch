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

#ifndef KJVCONFIGURATION_H
#define KJVCONFIGURATION_H

#include "dbstruct.h"

#include <QWidget>
#include <QDialog>
#include <QDialogButtonBox>
#include <QwwConfigWidget>
#include <QFont>
#include <QPushButton>

// ============================================================================

// Forward Declarations:
class CSearchResultsTreeView;
class CScriptureBrowser;
class CParsedPhrase;

// ============================================================================

namespace Ui {
	class CKJVTextFormatConfig;
}

class CKJVTextFormatConfig : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVTextFormatConfig(CBibleDatabasePtr pBibleDatabase, QWidget *parent = 0);						// Database for the preview
	~CKJVTextFormatConfig();

	void saveSettings();					// Writes changes back to system

	bool isDirty() const { return m_bIsDirty; }

signals:
	void dataChanged();

private slots:
	void en_ScriptureBrowserFontChanged(const QFont &font);
	void en_SearchResultsFontChanged(const QFont &font);
	void en_ScriptureBrowserFontSizeChanged(double nFontSize);
	void en_SearchResultsFontSizeChanged(double nFontSize);

	void en_InvertTextBrightnessChanged(bool bInvert);
	void en_TextBrightnessChanged(int nBrightness);
	void en_AdjustDialogElementBrightness(bool bAdjust);

	void en_WordsOfJesusColorPicked(const QColor &color);
	void en_SearchResultsColorPicked(const QColor &color);
	void en_CursorTrackerColorPicked(const QColor &color);

	void navigateToDemoText();
	void setPreview();

// Private Data:
private:
//	CBibleDatabasePtr m_pBibleDatabase;
	QFont m_fntScriptureBrowser;
	QFont m_fntSearchResults;
	bool m_bInvertTextBrightness;
	int m_nTextBrightness;
	bool m_bAdjustDialogElementBrightness;

// UI Private:
private:
	CSearchResultsTreeView *m_pSearchResultsTreeView;
	CScriptureBrowser *m_pScriptureBrowser;
	bool m_bIsDirty;

	static class QwwColorButton *toQwwColorButton(QPushButton *pButton) { return reinterpret_cast<class QwwColorButton *>(pButton); }
	Ui::CKJVTextFormatConfig *ui;
};

// ============================================================================

class CKJVConfiguration : public QwwConfigWidget
{
	Q_OBJECT

public:
	CKJVConfiguration(CBibleDatabasePtr pBibleDatabase, QWidget *parent = NULL);
	virtual ~CKJVConfiguration();

	void saveSettings();					// Writes changes back to system
	bool isDirty() const { return m_pTextFormatConfig->isDirty(); }

signals:
	void dataChanged();

private:
	CKJVTextFormatConfig *m_pTextFormatConfig;
};

// ============================================================================

class CKJVConfigurationDialog : public QDialog
{
	Q_OBJECT

public:
	CKJVConfigurationDialog(CBibleDatabasePtr pBibleDatabase, QWidget *parent = NULL);
	virtual ~CKJVConfigurationDialog();

public slots:
	virtual void en_dataChanged();
	virtual void accept();
	virtual void reject();
	virtual void apply();

private:
	CKJVConfiguration *m_pConfiguration;
	QDialogButtonBox *m_pButtonBox;
};

// ============================================================================

#endif	// KJVCONFIGURATION_H

