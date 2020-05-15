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

#include "DictionaryWidget.h"

#include "PersistentSettings.h"
#include "BusyCursor.h"

#include <QTextCursor>
#include <QTextCharFormat>
#include <QMenu>
#include <QAction>
#include <QEvent>
#include <QAbstractItemView>

#define DICTIONARY_COMPLETER_BUTTON_SIZE_Y 24

// ============================================================================

CDictionaryLineEdit::CDictionaryLineEdit(QWidget *pParent)
	:	CSingleLineTextEdit(DICTIONARY_COMPLETER_BUTTON_SIZE_Y, pParent),
		m_pCompleter(nullptr),
		m_bUpdateInProgress(false)
{

}

CDictionaryLineEdit::~CDictionaryLineEdit()
{

}

void CDictionaryLineEdit::initialize(CDictionaryDatabasePtr pDictionary)
{
	setAcceptRichText(false);
	setUndoRedoEnabled(false);		// TODO : If we ever address what to do with undo/redo, then re-enable this

	setTabChangesFocus(true);
	setWordWrapMode(QTextOption::NoWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setFixedHeight(sizeHint().height());
	setLineWrapMode(QTextEdit::NoWrap);

	m_dlyUpdateCompleter.setMinimumDelay(10);				// Arbitrary time, but I think it must be less than our textChanged delay or we may have issues
	connect(&m_dlyUpdateCompleter, SIGNAL(triggered()), this, SLOT(delayed_UpdatedCompleter()));

	connect(CPersistentSettings::instance(), SIGNAL(changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)), this, SLOT(en_changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM)));

	setDictionary(pDictionary);
}

void CDictionaryLineEdit::setDictionary(CDictionaryDatabasePtr pDictionary)
{
	assert(!pDictionary.isNull());
	m_pDictionaryDatabase = pDictionary;

	m_dlyUpdateCompleter.untrigger();
	if (m_pCompleter) {
		delete m_pCompleter;
		m_pCompleter = nullptr;
	}

	if (m_pDictionaryDatabase->descriptor().m_dtoFlags & DTO_Strongs) {
		m_pCompleter = new CStrongsDictionarySearchCompleter(m_pDictionaryDatabase, *this, this);
	} else {
		m_pCompleter = new SearchCompleter_t(m_pDictionaryDatabase, *this, this);
//		m_pCompleter->setCaseSensitivity(isCaseSensitive() ? Qt::CaseSensitive : Qt::CaseInsensitive);
		// TODO : ??? Add AccentSensitivity to completer ???
	}

	m_pCompleter->setCompletionFilterMode(CPersistentSettings::instance()->dictionaryCompleterFilterMode());

	connect(m_pCompleter, SIGNAL(activated(const QModelIndex &)), this, SLOT(insertCompletion(const QModelIndex &)));

	// Reset the word to trigger an update so widget will update definition with that of the new database:
	if (!toPlainText().isEmpty()) {
		setPlainText(toPlainText());
	}
}

void CDictionaryLineEdit::en_changedDictionaryCompleterFilterMode(CSearchCompleter::SEARCH_COMPLETION_FILTER_MODE_ENUM nMode)
{
	m_pCompleter->setCompletionFilterMode(nMode);
}

void CDictionaryLineEdit::en_cursorPositionChanged()
{
	// Not calling base class because we don't need to update the completer on the
	//		cursor move (unlike the Search Phrases do)
}

void CDictionaryLineEdit::setupCompleter(const QString &strText, bool bForce)
{
	QString strWord = toPlainText();

	bool bCompleterOpen = m_pCompleter->popup()->isVisible();
	if ((bForce) || (!strText.isEmpty()) || (bCompleterOpen && (strWord.length() > 2) && (textCursor().atEnd()))) {
		delayed_UpdatedCompleter();			// Do an immediate update of the completer so we have values below (it speeds up the initial completer calculations!)
		m_pCompleter->popup()->close();
		if ((bCompleterOpen) && (strWord.length() > 2) && (textCursor().atEnd())) bForce = true;				// Reshow completer if it was open already and we're changing words
		m_pCompleter->selectFirstMatchString();
	}

	if (bForce || (!strText.isEmpty() && (strWord.length() > 2))) {
		if (bForce || (!m_pDictionaryDatabase->wordExists(toPlainText().trimmed()))) {
			CBusyCursor iAmBusy(nullptr);
			m_pCompleter->complete();
		}
	}
}

void CDictionaryLineEdit::processPendingUpdateCompleter()
{
	if (m_dlyUpdateCompleter.isTriggered()) delayed_UpdatedCompleter();
}

void CDictionaryLineEdit::UpdateCompleter()
{
	if (CPersistentSettings::instance()->dictionaryActivationDelay() == -1) {
		// Immediate activation if the dictionary activation delay is disabled:
		delayed_UpdatedCompleter();
	} else {
		m_dlyUpdateCompleter.trigger();
	}
}

void CDictionaryLineEdit::delayed_UpdatedCompleter()
{
	m_dlyUpdateCompleter.untrigger();

	m_pCompleter->setFilterMatchString();
	m_pCompleter->setWordsFromPhrase();

	if (updateInProgress()) return;
	CDoUpdate doUpdate(this);

	QTextCursor cursor(textCursor());
	cursor.beginEditBlock();

	QTextCharFormat fmt = cursor.charFormat();
	fmt.setFontStrikeOut(false);
	fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);

	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.setCharFormat(fmt);

	if (!m_pDictionaryDatabase->wordExists(toPlainText().trimmed())) {
		fmt.setFontStrikeOut(true);
		fmt.setUnderlineColor(QColor(255,0,0));
		fmt.setUnderlineStyle(QTextCharFormat::WaveUnderline);
		cursor.setCharFormat(fmt);
	}

	cursor.endEditBlock();
}

void CDictionaryLineEdit::insertCompletion(const QModelIndex &index)
{
	insertCompletion(m_pCompleter->completionModel()->data(index, Qt::DisplayRole).toString());
}

void CDictionaryLineEdit::insertCompletion(const QString &strWord)
{
	QTextCursor cursor(textCursor());
	cursor.beginEditBlock();
	cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	cursor.insertText(strWord);
	cursor.endEditBlock();
}

// ============================================================================

CDictionaryWidget::CDictionaryWidget(CDictionaryDatabasePtr pDictionary, const QString &strLanguage, QWidget *parent)
	:	QWidget(parent),
		m_pDictionaryDatabase(pDictionary),
		m_strLanguage(strLanguage),
		m_bDoingPopup(false),
		m_pEditMenuDictionary(nullptr),
		m_pEditMenuDictWord(nullptr),
		m_pActionDictDatabasesList(nullptr),
		m_bDoingUpdate(false),
		m_bIgnoreNextWordChange(false),
		m_bHaveURLLastWord(false)
{
	assert(!m_pDictionaryDatabase.isNull());

	ui.setupUi(this);

	QAction *pAction;

	// ------------------------------------------------------------------------

	m_pEditMenuDictWord = new QMenu(tr("&Edit", "MainMenu"), ui.editDictionaryWord);
	m_pEditMenuDictWord->setStatusTip(tr("Dictionary Word Editor Operations", "MainMenu"));

/*
	TODO : If we ever address what to do with undo/redo, then put this code back in:

	pAction = m_pEditMenuDictWord->addAction(tr("&Undo", "MainMenu"), ui.editDictionaryWord, SLOT(undo()), QKeySequence(Qt::CTRL + Qt::Key_Z));
	pAction->setStatusTip(tr("Undo last operation to the Dictionary Word Editor", "MainMenu"));
	pAction->setEnabled(false);
	connect(ui.editDictionaryWord, SIGNAL(undoAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));
	pAction = m_pEditMenuDictWord->addAction(tr("&Redo", "MainMenu"), ui.editDictionaryWord, SLOT(redo()), QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
	pAction->setStatusTip(tr("Redo last operation on the Dictionary Word Editor", "MainMenu"));
	pAction->setEnabled(false);
	connect(ui.editDictionaryWord, SIGNAL(redoAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));
	m_pEditMenuDictWord->addSeparator();
*/
	pAction = m_pEditMenuDictWord->addAction(tr("Cu&t", "MainMenu"), ui.editDictionaryWord, SLOT(cut()), QKeySequence(Qt::CTRL + Qt::Key_X));
	pAction->setStatusTip(tr("Cut selected text from the Dictionary Word Editor to the clipboard", "MainMenu"));
	pAction->setEnabled(false);
	connect(ui.editDictionaryWord, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
//	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));
	pAction = m_pEditMenuDictWord->addAction(tr("&Copy", "MainMenu"), ui.editDictionaryWord, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	pAction->setStatusTip(tr("Copy selected text from the Dictionary Word Editor to the clipboard", "MainMenu"));
	pAction->setEnabled(false);
	connect(ui.editDictionaryWord, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
//	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));
	pAction = m_pEditMenuDictWord->addAction(tr("&Paste", "MainMenu"), ui.editDictionaryWord, SLOT(paste()), QKeySequence(Qt::CTRL + Qt::Key_V));
	pAction->setStatusTip(tr("Paste text on clipboard into the Dictionary Word Editor", "MainMenu"));
	pAction->setEnabled(true);
//	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));
	pAction = m_pEditMenuDictWord->addAction(tr("&Delete", "MainMenu"), ui.editDictionaryWord, SLOT(clear()), QKeySequence(Qt::Key_Delete));
	pAction->setStatusTip(tr("Delete selected text from the Dictionary Word Editor", "MainMenu"));
	pAction->setEnabled(false);
	connect(ui.editDictionaryWord, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
//	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));
	m_pEditMenuDictWord->addSeparator();
	pAction = m_pEditMenuDictWord->addAction(tr("Select &All", "MainMenu"), ui.editDictionaryWord, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	pAction->setStatusTip(tr("Select All Text in the Dictionary Word Editor", "MainMenu"));
	pAction->setEnabled(true);
//	connect(pAction, SIGNAL(triggered()), ui.editDictionaryWord, SLOT(setFocus()));

	// ------------------------------------------------------------------------

	m_pEditMenuDictionary = new QMenu(tr("&Edit", "MainMenu"), ui.definitionBrowser);
	m_pEditMenuDictionary->setStatusTip(tr("Dictionary Definition Text Edit Operations", "MainMenu"));

	pAction = m_pEditMenuDictionary->addAction(tr("&Copy", "MainMenu"), ui.definitionBrowser, SLOT(copy()), QKeySequence(Qt::CTRL + Qt::Key_C));
	pAction->setStatusTip(tr("Copy selected text from the Dictionary Definition to the clipboard", "MainMenu"));
	pAction->setEnabled(false);
	connect(ui.definitionBrowser, SIGNAL(copyAvailable(bool)), pAction, SLOT(setEnabled(bool)));
	connect(pAction, SIGNAL(triggered()), ui.definitionBrowser, SLOT(setFocus()));
	m_pEditMenuDictionary->addSeparator();
	pAction = m_pEditMenuDictionary->addAction(tr("Select &All", "MainMenu"), ui.definitionBrowser, SLOT(selectAll()), QKeySequence(Qt::CTRL + Qt::Key_A));
	pAction->setStatusTip(tr("Select All Text in the Dictionary Definition", "MainMenu"));
	pAction->setEnabled(true);
	connect(pAction, SIGNAL(triggered()), ui.definitionBrowser, SLOT(setFocus()));
	// ----
	m_pEditMenuDictionary->addSeparator();
	// ----
	m_pActionDictDatabasesList = new QAction(QIcon(":/res/Apps-accessories-dictionary-icon-128.png"), tr("Select &Dictionary", "MainMenu"), this);
	m_pActionDictDatabasesList->setStatusTip(tr("Select dictionary to use in this Search Window", "MainMenu"));
	m_pActionDictDatabasesList->setToolTip(tr("Select dictionary to use", "MainMenu"));
	// Setup the submenu:
	m_pActionDictDatabasesList->setMenu(new QMenu);			// The action will take ownership via setOverrideMenuAction()
	en_updateDictionaryDatabasesList();
	//	Do the update via a QueuedConnection so that KJVCanOpeners coming/going during opening other search windows
	//	that have to open new Dictionary Databases won't crash if the menu that was triggering it gets yanked out from under it:
	connect(TDictionaryDatabaseList::instance(), SIGNAL(changedDictionaryDatabaseList()), this, SLOT(en_updateDictionaryDatabasesList()), Qt::QueuedConnection);
	m_pEditMenuDictionary->addAction(m_pActionDictDatabasesList);

	// ------------------------------------------------------------------------

	ui.definitionBrowser->installEventFilter(this);
	ui.editDictionaryWord->installEventFilter(this);

	// ------------------------------------------------------------------------

	ui.editDictionaryWord->initialize(m_pDictionaryDatabase);

	ui.definitionBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.definitionBrowser, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_definitionBrowserContextMenuRequested(const QPoint &)));

	ui.editDictionaryWord->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.editDictionaryWord, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(en_editDictionaryWordContextMenuRequested(const QPoint &)));

	setDictionaryActivationDelay(CPersistentSettings::instance()->dictionaryActivationDelay());
	connect(ui.editDictionaryWord, SIGNAL(textChanged()), &m_dlyTextChanged, SLOT(trigger()));
	connect(&m_dlyTextChanged, SIGNAL(triggered()), this, SLOT(en_wordChanged()));
	connect(CPersistentSettings::instance(), SIGNAL(changedDictionaryActivationDelay(int)), this, SLOT(setDictionaryActivationDelay(int)));

	setFont(CPersistentSettings::instance()->fontDictionary());
	setTextBrightness(CPersistentSettings::instance()->invertTextBrightness(), CPersistentSettings::instance()->textBrightness());

	connect(CPersistentSettings::instance(), SIGNAL(fontChangedDictionary(const QFont &)), this, SLOT(setFont(const QFont &)));
	connect(CPersistentSettings::instance(), SIGNAL(changedTextBrightness(bool, int)), this, SLOT(setTextBrightness(bool, int)));

	connect(ui.buttonClearWord, SIGNAL(clicked()), ui.editDictionaryWord, SLOT(clear()));
	connect(ui.buttonClearWord, SIGNAL(clicked()), ui.editDictionaryWord, SLOT(setFocus()));

	connect(ui.definitionBrowser, SIGNAL(anchorClicked(const QUrl &)), this, SLOT(en_anchorClicked(const QUrl &)));
	connect(ui.buttonHistoryBack, SIGNAL(clicked()), ui.definitionBrowser, SLOT(backward()));
	connect(ui.buttonHistoryBack, SIGNAL(clicked()), ui.editDictionaryWord, SLOT(setFocus()));
	connect(ui.definitionBrowser, SIGNAL(backwardAvailable(bool)), ui.buttonHistoryBack, SLOT(setEnabled(bool)));
	connect(ui.buttonHistoryForward, SIGNAL(clicked()), ui.definitionBrowser, SLOT(forward()));
	connect(ui.buttonHistoryForward, SIGNAL(clicked()), ui.editDictionaryWord, SLOT(setFocus()));
	connect(ui.definitionBrowser, SIGNAL(forwardAvailable(bool)), ui.buttonHistoryForward, SLOT(setEnabled(bool)));
	connect(ui.definitionBrowser, SIGNAL(sourceChanged(const QUrl &)), this, SLOT(en_sourceChanged(const QUrl &)));
}

CDictionaryWidget::~CDictionaryWidget()
{

}

bool CDictionaryWidget::eventFilter(QObject *pObject, QEvent *pEvent)
{
	assert(pEvent != nullptr);

	if ((pObject == ui.definitionBrowser) && (pEvent->type() == QEvent::FocusIn)) {
		emit activatedDictionary(false);
	} else if ((pObject == ui.editDictionaryWord) && (pEvent->type() == QEvent::FocusIn)) {
		emit activatedDictionary(true);
	}

	return QWidget::eventFilter(pObject, pEvent);
}

void CDictionaryWidget::setWord(const QString &strWord, bool bIsTracking)
{
	if ((bIsTracking) && (m_pDictionaryDatabase->flags() & DTO_DisableTracking)) return;

	ui.editDictionaryWord->processPendingUpdateCompleter();
	ui.editDictionaryWord->insertCompletion(strWord);
}

void CDictionaryWidget::en_wordChanged()
{
	QString strWord = ui.editDictionaryWord->toPlainText().trimmed();

	QUrl strURLLastWord = ui.definitionBrowser->source();

	if (m_bIgnoreNextWordChange) {
		m_bIgnoreNextWordChange = false;
		m_bDoingUpdate = true;
		// Restore last word:
		if (strURLLastWord.hasFragment()) setWord(strURLLastWord.fragment(), false);
		m_bDoingUpdate = false;
		return;
	}

	// Don't set the word again, if it's already set.  This prevents reloading the
	//		same thing and losing the user's place in the definition, particularly
	//		if we are using the ignoreNextWordChange guard above for when the user
	//		click on a particular passage.  Unless the dictionary being used has
	//		changed, in which case we need to trigger an update to display the
	//		definition for the new dictionary:
	if (m_bHaveURLLastWord &&
		strURLLastWord.hasFragment() && (strURLLastWord.fragment() == strWord) &&
		(m_pDictionaryDatabase->compatibilityUUID() == m_strLastWordDictionaryUUID)) return;

	m_strLastWordDictionaryUUID = m_pDictionaryDatabase->compatibilityUUID();
	ui.definitionBrowser->setHtml(m_pDictionaryDatabase->definition(strWord));
	if (m_pDictionaryDatabase->wordExists(strWord)) {
		m_bDoingUpdate = true;
		ui.definitionBrowser->setSource(QUrl(QString("#%1").arg(strWord)));
		m_bHaveURLLastWord = true;
		m_bDoingUpdate = false;
	} else {
		m_bHaveURLLastWord = false;
	}
}

void CDictionaryWidget::en_sourceChanged(const QUrl &src)
{
	if (m_bDoingUpdate) return;

	QString strURL = src.toString();		// Internal URLs are in the form of "#nnnnnnnn" as anchors
	int nPos = strURL.indexOf('#');
	if (nPos > -1) setWord(strURL.mid(nPos+1), false);
}

void CDictionaryWidget::en_anchorClicked(const QUrl &link)
{
	// Incoming URL Format:		dict/Web-1828://Word
	//							dict/Web-1913://Word
	//							bible://Reference		-> converted to bible/://Reference in ReadDB so that parsing works!!
	//							strong://TextIndex		-> converted to strong/://TextIndex in ReadDB so that parsing works!!

	QString strAnchor = link.toString();

	// Convert "dict/Web-1828" or "dict/Web-1913" to:  dict://Word
	int ndxColon = strAnchor.indexOf(':');
	if (ndxColon == -1) return;
	int ndxSlash = strAnchor.left(ndxColon).indexOf('/');
	if (ndxSlash != -1) strAnchor.remove(ndxSlash, ndxColon-ndxSlash);

	int ndxDblSlash = strAnchor.lastIndexOf("//");
	QString strValue = strAnchor.mid(ndxDblSlash+2);

	QUrl urlResolved(strAnchor);

	// Scheme = "dict" or "bible" or "strong"
	// Host = Word/Reference (Note: Reference decode doesn't work correctly as "host", use strValue)

	if (urlResolved.scheme().compare("dict", Qt::CaseInsensitive) == 0) {
		if (ndxDblSlash != -1) {
			setWord(strValue, false);
		} else {
			setWord(urlResolved.host(), false);
		}
	} else if (urlResolved.scheme().compare("strong", Qt::CaseInsensitive) == 0) {
#ifndef ENABLE_ONLY_LOADED_DICTIONARY_DATABASES
		if (!(m_pDictionaryDatabase->descriptor().m_dtoFlags & DTO_Strongs)) {
			CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::locateAndLoadStrongsDictionary(QString(), m_pDictionaryDatabase->language(), this);
			if (!pDictDatabase.isNull()) {
				m_pDictionaryDatabase = pDictDatabase;
				ui.editDictionaryWord->setDictionary(pDictDatabase);
			}
		}
#endif
		if (ndxDblSlash != -1) {
			setWord(strValue.toUpper(), false);
		} else {
			setWord(urlResolved.host().toUpper(), false);
		}
	} else if (urlResolved.scheme().compare("bible", Qt::CaseInsensitive) == 0) {
		if (ndxDblSlash != -1) {
			if (!(m_pDictionaryDatabase->flags() & DTO_DisableTracking)) {
				m_bIgnoreNextWordChange = true;					// Ignore automatic word selection from the new passage, as it's annoying
			}
			emit gotoPassageReference(strValue);
		}
	}
}

void CDictionaryWidget::en_definitionBrowserContextMenuRequested(const QPoint &pos)
{
	bool bPopupSave = m_bDoingPopup;
	m_bDoingPopup = true;

	assert(m_pEditMenuDictionary != nullptr);
#ifndef USE_ASYNC_DIALOGS
	m_pEditMenuDictionary->exec(ui.definitionBrowser->viewport()->mapToGlobal(pos));
#else
	m_pEditMenuDictionary->popup(ui.definitionBrowser->viewport()->mapToGlobal(pos));
#endif

	m_bDoingPopup = bPopupSave;
}

void CDictionaryWidget::en_editDictionaryWordContextMenuRequested(const QPoint &pos)
{
	bool bPopupSave = m_bDoingPopup;
	m_bDoingPopup = true;

	assert(m_pEditMenuDictWord != nullptr);
#ifndef USE_ASYNC_DIALOGS
	m_pEditMenuDictWord->exec(ui.editDictionaryWord->viewport()->mapToGlobal(pos));
#else
	m_pEditMenuDictWord->popup(ui.editDictionaryWord->viewport()->mapToGlobal(pos));
#endif

	m_bDoingPopup = bPopupSave;
}

void CDictionaryWidget::setFont(const QFont& aFont)
{
	ui.definitionBrowser->document()->setDefaultFont(aFont);
}

void CDictionaryWidget::setTextBrightness(bool bInvert, int nBrightness)
{
	setStyleSheet(QString("CDictionaryLineEdit, QTextBrowser { background-color:%1; color:%2; }")
								   .arg(CPersistentSettings::textBackgroundColor(bInvert, nBrightness).name())
								   .arg(CPersistentSettings::textForegroundColor(bInvert, nBrightness).name()));
}

void CDictionaryWidget::en_updateDictionaryDatabasesList()
{
	assert(m_pActionDictDatabasesList != nullptr);
	assert(m_pActionDictDatabasesList->menu() != nullptr);

	if (!m_pActionGroupDictDatabasesList.isNull()) delete m_pActionGroupDictDatabasesList;
	m_pActionGroupDictDatabasesList = new QActionGroup(this);
	m_pActionGroupDictDatabasesList->setExclusive(true);

#ifdef ENABLE_ONLY_LOADED_DICTIONARY_DATABASES
	for (int ndx = 0; ndx < TDictionaryDatabaseList::instance()->size(); ++ndx) {
		if (TDictionaryDatabaseList::instance()->at(ndx).isNull()) continue;
		if ((!m_strLanguage.isEmpty()) && (TDictionaryDatabaseList::instance()->at(ndx)->language().compare(m_strLanguage, Qt::CaseInsensitive) != 0) &&
			(!(TDictionaryDatabaseList::instance()->at(ndx)->flags() & DTO_IgnoreLang))) continue;
		QAction *pAction = new QAction(TDictionaryDatabaseList::instance()->at(ndx)->description(), m_pActionGroupDictDatabasesList);
		pAction->setData(TDictionaryDatabaseList::instance()->at(ndx)->compatibilityUUID());
		pAction->setCheckable(true);
		if (TDictionaryDatabaseList::instance()->at(ndx)->compatibilityUUID().compare(m_pDictionaryDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) {
			pAction->setChecked(true);
		}
		m_pActionDictDatabasesList->menu()->addAction(pAction);
	}
#else
	const QList<TDictionaryDescriptor> &lstAvailableDictDescs = TDictionaryDatabaseList::availableDictionaryDatabases();
	for (int ndx = 0; ndx < lstAvailableDictDescs.size(); ++ndx) {
		CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(lstAvailableDictDescs.at(ndx).m_strUUID);

		if (!pDictDatabase.isNull()) {
			if ((m_strLanguage.isEmpty()) || (pDictDatabase->language().compare(m_strLanguage, Qt::CaseInsensitive) == 0) ||
				(pDictDatabase->flags() & DTO_IgnoreLang)) {
				QAction *pAction = new QAction(pDictDatabase->description(), m_pActionGroupDictDatabasesList);
				pAction->setData(pDictDatabase->compatibilityUUID());
				pAction->setCheckable(true);
				if (pDictDatabase->compatibilityUUID().compare(m_pDictionaryDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) {
					pAction->setChecked(true);
				}
				m_pActionDictDatabasesList->menu()->addAction(pAction);
			}
		} else {
			TDictionaryDescriptor dctDesc = lstAvailableDictDescs.at(ndx);
			assert(dctDesc.isValid());
			if ((m_strLanguage.isEmpty()) || (dctDesc.m_strLanguage.compare(m_strLanguage, Qt::CaseInsensitive) == 0) ||
				(dctDesc.m_dtoFlags & DTO_IgnoreLang)) {
				QAction *pAction = new QAction(dctDesc.m_strDBDesc, m_pActionGroupDictDatabasesList);
				pAction->setData(dctDesc.m_strUUID);
				pAction->setCheckable(true);
				if (dctDesc.m_strUUID.compare(m_pDictionaryDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) {
					pAction->setChecked(true);
				}
				m_pActionDictDatabasesList->menu()->addAction(pAction);
			}
		}
	}
#endif

	connect(m_pActionGroupDictDatabasesList.data(), SIGNAL(triggered(QAction*)), this, SLOT(en_selectDictionary(QAction*)));
}

void CDictionaryWidget::en_selectDictionary(QAction *pAction)
{
	assert(pAction != nullptr);

	if (pAction != nullptr) {
		QString strUUID = pAction->data().toString();
		if (strUUID.compare(m_pDictionaryDatabase->compatibilityUUID(), Qt::CaseInsensitive) == 0) return;		// Ignore if "switching" to same database

		CDictionaryDatabasePtr pDictDatabase = TDictionaryDatabaseList::instance()->atUUID(strUUID);
#ifndef ENABLE_ONLY_LOADED_DICTIONARY_DATABASES
		if (pDictDatabase.isNull()) {
			pDictDatabase = TDictionaryDatabaseList::instance()->loadDictionaryDatabase(strUUID, false, this);
			if (pDictDatabase.isNull()) return;
		}
#else
		assert(!pDictDatabase.isNull());
#endif

		m_pDictionaryDatabase = pDictDatabase;
		ui.editDictionaryWord->setDictionary(pDictDatabase);
	}
}

// ============================================================================
