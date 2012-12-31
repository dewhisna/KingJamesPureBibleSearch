/****************************************************************************
**
** Copyright (C) 2012 Donna Whisnant, a.k.a. Dewtronics.
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

#ifndef KJVSEARCHCRITERIA_H
#define KJVSEARCHCRITERIA_H

#include <QWidget>

namespace Ui {
class CKJVSearchCriteria;
}

class CKJVSearchCriteria : public QWidget
{
	Q_OBJECT

public:
	explicit CKJVSearchCriteria(QWidget *parent = 0);
	~CKJVSearchCriteria();

	enum SEARCH_SCOPE_MODE_ENUM {
		SSME_WHOLE_BIBLE = 0,
		SSME_TESTAMENT = 1,
		SSME_BOOK = 2,
		SSME_CHAPTER = 3,
		SSME_VERSE = 4
	};

	SEARCH_SCOPE_MODE_ENUM searchScopeMode() const { return m_nSearchScopeMode; }

signals:
	void changedSearchScopeMode(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);
	void addSearchPhraseClicked();
	void copySearchPhraseSummary();

public slots:
	void enableCopySearchPhraseSummary(bool bEnable);
	void setSearchScopeMode(CKJVSearchCriteria::SEARCH_SCOPE_MODE_ENUM mode);

private slots:
	void on_changeSearchScopeMode(int ndx);

// Data Private:
private:
	SEARCH_SCOPE_MODE_ENUM m_nSearchScopeMode;

// UI Private:
private:
	bool m_bDoingUpdate;		// True if combo boxes, etc, are being updated and change notifications should be ignored

#define begin_update()							\
			bool bUpdateSave = m_bDoingUpdate;	\
			m_bDoingUpdate = true;
#define end_update()							\
			m_bDoingUpdate = bUpdateSave;

	Ui::CKJVSearchCriteria *ui;
};

#endif // KJVSEARCHCRITERIA_H
