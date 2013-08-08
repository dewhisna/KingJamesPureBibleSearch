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

#include "KJVNoteEditDlg.h"
#include "ui_KJVNoteEditDlg.h"

#include "SearchCompleter.h"
#include "PersistentSettings.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QMessageBox>
#include <QIcon>
#include <QPair>

// ============================================================================

namespace {
	//////////////////////////////////////////////////////////////////////
	// File-scoped constants
	//////////////////////////////////////////////////////////////////////

	// Key constants:
	// --------------

	// RestoreState:
	const QString constrRestoreStateGroup("RestoreState");
	const QString constrGeometryKey("Geometry");
	const QString constrWindowStateKey("WindowState");

}

// ============================================================================

CNoteKeywordModel::CNoteKeywordModel(const QStringList &lstSelectedKeywords, const QStringList &lstCompositeKeywords, QObject *pParent)
	:	QAbstractListModel(pParent)
{
	m_lstKeywordData.reserve(lstCompositeKeywords.size());
	for (int ndx = 0; ndx < lstCompositeKeywords.size(); ++ndx) {
		m_lstKeywordData.append(CNoteKeywordModelItemData(lstCompositeKeywords.at(ndx), lstSelectedKeywords.contains(lstCompositeKeywords.at(ndx), Qt::CaseInsensitive)));
	}
}

CNoteKeywordModel::~CNoteKeywordModel()
{

}

int CNoteKeywordModel::rowCount(const QModelIndex &zParent) const
{
	if (zParent.isValid())
		return 0;

	return m_lstKeywordData.size();
}

QVariant CNoteKeywordModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid()) return QVariant();
	if (index.row() < 0 || index.row() >= m_lstKeywordData.size()) return QVariant();

	if ((role == Qt::DisplayRole) || (role == Qt::EditRole))
		return m_lstKeywordData.at(index.row()).m_strKeyword;

	if (role == Qt::CheckStateRole)
		return (m_lstKeywordData.at(index.row()).m_bChecked ? Qt::Checked : Qt::Unchecked);

	return QVariant();
}

bool CNoteKeywordModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if ((index.row() >= 0) && (index.row() < m_lstKeywordData.size())) {
		switch (role) {
			case Qt::EditRole:
			case Qt::DisplayRole:
			{
				QString strDecompNewKeyword = CSearchStringListModel::decompose(value.toString());
				QString strDecompOldKeyword = CSearchStringListModel::decompose(m_lstKeywordData.at(index.row()).m_strKeyword);
				if (strDecompOldKeyword.compare(strDecompNewKeyword, Qt::CaseInsensitive) != 0) {
					m_lstKeywordData[index.row()].m_strKeyword = value.toString();
					emit dataChanged(index, index);
					emit changedNoteKeywords();
				}
				return true;
			}
			case Qt::CheckStateRole:
				if (m_lstKeywordData.at(index.row()).m_bChecked != value.toBool()) {
					m_lstKeywordData[index.row()].m_bChecked = value.toBool();
					emit dataChanged(index, index);
					emit changedNoteKeywords();
				}
				return true;
			default:
				break;
		}
	}

	return false;
}

QModelIndex CNoteKeywordModel::findKeyword(const QString &strKeyword) const
{
	QString strDecomposedKeyword = CSearchStringListModel::decompose(strKeyword);
	int ndxFound = -1;
	for (int ndx = 0; ndx < m_lstKeywordData.size(); ++ndx) {
		if (m_lstKeywordData.at(ndx).m_strKeyword.compare(strDecomposedKeyword, Qt::CaseInsensitive) == 0) {
			ndxFound = ndx;
			break;
		}
	}

	if (ndxFound == -1) return QModelIndex();
	return index(ndxFound);
}

Qt::ItemFlags CNoteKeywordModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

	return Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsUserCheckable;	// | Qt::ItemIsSelectable;
}

bool CNoteKeywordModel::insertRows(int row, int count, const QModelIndex &zParent)
{
	if ((count < 1) || (row < 0) || (row > rowCount(zParent))) return false;

	beginInsertRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstKeywordData.insert(row, CNoteKeywordModelItemData());

	endInsertRows();

	return true;
}

bool CNoteKeywordModel::removeRows(int row, int count, const QModelIndex &zParent)
{
	if ((count <= 0) || (row < 0) || ((row + count) > rowCount(zParent))) return false;

	beginRemoveRows(QModelIndex(), row, row + count - 1);

	for (int r = 0; r < count; ++r)
		m_lstKeywordData.removeAt(row);

	endRemoveRows();

	return true;
}

static bool ascendingLessThan(const QPair<CNoteKeywordModelItemData, int> &s1, const QPair<CNoteKeywordModelItemData, int> &s2)
{
	return (CSearchStringListModel::decompose(s1.first.m_strKeyword).compare(CSearchStringListModel::decompose(s2.first.m_strKeyword), Qt::CaseInsensitive) < 0);
}

static bool decendingLessThan(const QPair<CNoteKeywordModelItemData, int> &s1, const QPair<CNoteKeywordModelItemData, int> &s2)
{
	return (CSearchStringListModel::decompose(s2.first.m_strKeyword).compare(CSearchStringListModel::decompose(s1.first.m_strKeyword), Qt::CaseInsensitive) < 0);
}

void CNoteKeywordModel::sort(int /* column */, Qt::SortOrder order)
{
	emit layoutAboutToBeChanged();

	QList<QPair<CNoteKeywordModelItemData, int> > list;
	list.reserve(m_lstKeywordData.size());
	for (int i = 0; i < m_lstKeywordData.size(); ++i)
		list.append(QPair<CNoteKeywordModelItemData, int>(m_lstKeywordData.at(i), i));

	if (order == Qt::AscendingOrder)
		qSort(list.begin(), list.end(), ascendingLessThan);
	else
		qSort(list.begin(), list.end(), decendingLessThan);

	m_lstKeywordData.clear();
	m_lstKeywordData.reserve(list.size());
	QVector<int> forwarding(list.size());
	for (int i = 0; i < list.size(); ++i) {
		m_lstKeywordData.append(list.at(i).first);
		forwarding[list.at(i).second] = i;
	}

	QModelIndexList oldList = persistentIndexList();
	QModelIndexList newList;
	newList.reserve(oldList.size());
	for (int i = 0; i < oldList.size(); ++i)
		newList.append(index(forwarding.at(oldList.at(i).row()), 0));
	changePersistentIndexList(oldList, newList);

	emit layoutChanged();
}

const CNoteKeywordModelItemDataList &CNoteKeywordModel::itemList() const
{
	return m_lstKeywordData;
}

void CNoteKeywordModel::setItemList(const CNoteKeywordModelItemDataList &aList)
{
	emit beginResetModel();
	m_lstKeywordData = aList;
	emit endResetModel();
}

QStringList CNoteKeywordModel::selectedKeywordList() const
{
	QStringList lstKeywords;
	lstKeywords.reserve(m_lstKeywordData.size());

	for (int ndx = 0; ndx < m_lstKeywordData.size(); ++ndx) {
		if (m_lstKeywordData.at(ndx).m_bChecked)
			lstKeywords.append(m_lstKeywordData.at(ndx).m_strKeyword);
	}

	return lstKeywords;
}

/*
Qt::DropActions CNoteKeywordModel::supportedDropActions() const
{

}
*/

// ============================================================================

QAction *CKJVNoteEditDlg::m_pActionUserNoteEditor = NULL;

QAction *CKJVNoteEditDlg::actionUserNoteEditor()
{
	assert(m_pActionUserNoteEditor != NULL);
	return m_pActionUserNoteEditor;
}

void CKJVNoteEditDlg::setActionUserNoteEditor(QAction *pAction)
{
	assert(m_pActionUserNoteEditor == NULL);
	m_pActionUserNoteEditor = pAction;
}


// ============================================================================

CKJVNoteEditDlg::CKJVNoteEditDlg(CBibleDatabasePtr pBibleDatabase, QWidget *parent)
	:	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
		ui(new Ui::CKJVNoteEditDlg),
		m_pBackgroundColorButton(NULL),
		m_pRichTextEdit(NULL),
		m_pDeleteNoteButton(NULL),
		m_pKeywordModel(NULL),
		m_pBibleDatabase(pBibleDatabase),
		m_bDoingUpdate(false),
		m_bIsDirty(false)
{
	assert(pBibleDatabase != NULL);

	ui->setupUi(this);

	// --------------------------------------------------------------

	int ndx;
	int nRow;
	int nCol;
	int nRowSpan;
	int nColSpan;

	// --------------------------------------------------------------

	//	Swapout the textEdit from the layout with a QwwRichTextEdit:

	ndx = ui->gridLayoutMain->indexOf(ui->textEdit);
	assert(ndx != -1);
	if (ndx == -1) return;
	ui->gridLayoutMain->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pRichTextEdit = new QwwRichTextEdit(this);
	m_pRichTextEdit->setObjectName(QString::fromUtf8("textEdit"));
	m_pRichTextEdit->setMouseTracking(true);
//	m_pRichTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//	m_pRichTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pRichTextEdit->setTabChangesFocus(false);
	m_pRichTextEdit->setTextInteractionFlags(Qt::TextSelectableByKeyboard|Qt::TextSelectableByMouse|Qt::TextEditable);

	delete ui->textEdit;
	ui->textEdit = NULL;
	ui->gridLayoutMain->addWidget(m_pRichTextEdit, nRow, nCol, nRowSpan, nColSpan);

	// --------------------------------------------------------------

	//	Swapout the buttonBackgroundColor from the layout with a QwwColorButton:

//	ndx = ui->horizontalLayout->indexOf(ui->buttonBackgroundColor);
//	assert(ndx != -1);
//	if (ndx == -1) return;

	ndx = ui->gridLayoutControls->indexOf(ui->buttonBackgroundColor);
	assert(ndx != -1);
	if (ndx == -1) return;
	ui->gridLayoutControls->getItemPosition(ndx, &nRow, &nCol, &nRowSpan, &nColSpan);

	m_pBackgroundColorButton = new QwwColorButton(this);
	m_pBackgroundColorButton->setObjectName(QString::fromUtf8("buttonBackgroundColor"));
	m_pBackgroundColorButton->setShowName(false);			// Must do this before setting our real text
	m_pBackgroundColorButton->setText(tr("Note Background Color"));
	m_pBackgroundColorButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

	delete ui->buttonBackgroundColor;
	ui->buttonBackgroundColor = NULL;
//	ui->horizontalLayout->insertWidget(ndx, m_pBackgroundColorButton);
	ui->gridLayoutControls->addWidget(m_pBackgroundColorButton, nRow, nCol, nRowSpan, nColSpan);

	// --------------------------------------------------------------

	// Reinsert it in the correct TabOrder:
	QWidget::setTabOrder(m_pRichTextEdit, ui->buttonBox);
	QWidget::setTabOrder(ui->buttonBox, ui->editNoteLocation);
	QWidget::setTabOrder(ui->editNoteLocation, m_pBackgroundColorButton);
	QWidget::setTabOrder(m_pBackgroundColorButton, ui->lblKeywords);
	QWidget::setTabOrder(ui->lblKeywords, ui->comboKeywords);

	// --------------------------------------------------------------

	// Setup Dialog buttons:

	assert(ui->buttonBox->button(QDialogButtonBox::Ok) != NULL);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(":res/ok_blue-24.png"));
	assert(ui->buttonBox->button(QDialogButtonBox::Cancel) != NULL);
	ui->buttonBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon(":res/cancel-24.png"));
	m_pDeleteNoteButton = ui->buttonBox->addButton(tr("Delete Note"), QDialogButtonBox::ActionRole);
	m_pDeleteNoteButton->setIcon(QIcon(":res/deletered1-24.png"));
	connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(en_ButtonClicked(QAbstractButton*)));

	// --------------------------------------------------------------

	connect(m_pRichTextEdit, SIGNAL(textChanged()), this, SLOT(en_textChanged()));
	connect(m_pBackgroundColorButton, SIGNAL(colorPicked(const QColor &)), this, SLOT(en_BackgroundColorPicked(const QColor &)));

	CNoteKeywordModelListView *pKeywordView = new CNoteKeywordModelListView(ui->comboKeywords);
	pKeywordView->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->comboKeywords->setView(pKeywordView);
	connect(pKeywordView, SIGNAL(currentKeywordChanged(const QString &)), this, SLOT(en_keywordCurrentIndexChanged(const QString &)));
//	connect(ui->comboKeywords, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(en_keywordCurrentIndexChanged(const QString &)));
	connect(ui->comboKeywords, SIGNAL(enterPressed()), this, SLOT(en_keywordEntered()));

	m_pRichTextEdit->setFocus();
}

CKJVNoteEditDlg::~CKJVNoteEditDlg()
{
	delete ui;
}

// ============================================================================

void CKJVNoteEditDlg::writeSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	settings.setValue(constrGeometryKey, saveGeometry());
	settings.endGroup();
}

void CKJVNoteEditDlg::readSettings(QSettings &settings, const QString &prefix)
{
	// RestoreState:
	settings.beginGroup(groupCombine(prefix, constrRestoreStateGroup));
	restoreGeometry(settings.value(constrGeometryKey).toByteArray());
	settings.endGroup();
}

// ============================================================================

void CKJVNoteEditDlg::setLocationIndex(const CRelIndex &ndxLocation)
{
	assert(m_pRichTextEdit != NULL);
	assert(g_pUserNotesDatabase != NULL);

	m_ndxLocation = ndxLocation;
	m_ndxLocation.setWord(0);		// Work with whole verses only
	ui->editNoteLocation->setText(m_pBibleDatabase->PassageReferenceText(m_ndxLocation));

	m_UserNote = g_pUserNotesDatabase->noteFor(m_ndxLocation);

	m_bDoingUpdate = true;

	m_pBackgroundColorButton->setCurrentColor(m_UserNote.backgroundColor());
	setBackgroundColorPreview();

	// Setup Keywords:
	m_pKeywordModel = new CNoteKeywordModel(m_UserNote.keywordList(), g_pUserNotesDatabase->compositeKeywordList(), ui->comboKeywords);		// Parent it to the comboBox so that it will get auto-deleted when we set a new model
	m_pKeywordModel->sort(0);
	ui->comboKeywords->setInsertPolicy(QComboBox::NoInsert);		// No auto-insert as we need to parse and decide how to insert it
	ui->comboKeywords->setModel(m_pKeywordModel);
	ui->comboKeywords->clearEditText();
	setKeywordListPreview();

	connect(m_pKeywordModel, SIGNAL(changedNoteKeywords()), this, SLOT(en_keywordListChanged()));

	m_pRichTextEdit->setHtml(m_UserNote.text());
	m_bIsDirty = false;

	m_bDoingUpdate = false;
}

// ============================================================================

void CKJVNoteEditDlg::accept()
{
	assert(m_pRichTextEdit != NULL);
	assert(g_pUserNotesDatabase != NULL);

	m_UserNote.setText(m_pRichTextEdit->toHtml());
	m_UserNote.setIsVisible(true);			// Make note visible when they are explicitly setting it
	g_pUserNotesDatabase->setNoteFor(m_ndxLocation, m_UserNote);
	m_bIsDirty = false;
	QDialog::accept();
}

void CKJVNoteEditDlg::reject()
{
	if (m_bIsDirty) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("You have made changes to this note.  Do you wish to discard them??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
	}

	QDialog::reject();
}

// ============================================================================

void CKJVNoteEditDlg::en_textChanged()
{
	if (m_bDoingUpdate) return;

	m_bIsDirty = true;
}

// ============================================================================

void CKJVNoteEditDlg::setBackgroundColorPreview()
{
	setStyleSheet(QString("QwwRichTextEdit { background-color:%1; }\n").arg(m_UserNote.backgroundColor().name()));
}

void CKJVNoteEditDlg::en_BackgroundColorPicked(const QColor &color)
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	m_UserNote.setBackgroundColor(color);
	setBackgroundColorPreview();
	m_bIsDirty = true;

	m_bDoingUpdate = false;
}

// ============================================================================

void CKJVNoteEditDlg::en_ButtonClicked(QAbstractButton *button)
{
	assert(button != NULL);
	assert(g_pUserNotesDatabase != NULL);

	if (button == m_pDeleteNoteButton) {
		int nResult = QMessageBox::warning(this, windowTitle(), tr("Are you sure you want to completely delete this note??"),
																	(QMessageBox::Ok | QMessageBox::Cancel), QMessageBox::Cancel);
		if (nResult != QMessageBox::Ok) return;
		m_bIsDirty = false;
		g_pUserNotesDatabase->removeNoteFor(m_ndxLocation);
		QDialog::accept();
	}
}

// ============================================================================

void CKJVNoteEditDlg::en_keywordEntered()
{
	if (m_bDoingUpdate) return;

	m_bDoingUpdate = true;

	bool bDataChanged = false;
	QStringList lstNewKeywords = ui->comboKeywords->currentText().split(QChar(','), QString::SkipEmptyParts);
	for (int ndx = 0; ndx < lstNewKeywords.size(); ++ndx) {
		QString strNewKeyword = lstNewKeywords.at(ndx).trimmed();

		if (!strNewKeyword.isEmpty()) {
			QModelIndex index = m_pKeywordModel->findKeyword(strNewKeyword);
			if (!index.isValid()) {
				int ndx = m_pKeywordModel->rowCount();
				bool bSuccess;
				bSuccess = m_pKeywordModel->insertRow(ndx);
				assert(bSuccess);
				QModelIndex index = m_pKeywordModel->index(ndx);
				bSuccess = m_pKeywordModel->setData(index, strNewKeyword, Qt::EditRole);
				assert(bSuccess);
				bSuccess = m_pKeywordModel->setData(index, Qt::Checked, Qt::CheckStateRole);
				assert(bSuccess);
				bDataChanged = true;
			} else {
				if (!index.data(Qt::CheckStateRole).toBool()) {
					m_pKeywordModel->setData(index, Qt::Checked, Qt::CheckStateRole);
					bDataChanged = true;
				}
			}
		}
	}
	if (bDataChanged) {
		m_pKeywordModel->sort(0);
		m_UserNote.setKeywordList(m_pKeywordModel->selectedKeywordList());
		setKeywordListPreview();
		m_bIsDirty = true;
	}

	ui->comboKeywords->setEditText(QString());

	m_bDoingUpdate = false;
}

void CKJVNoteEditDlg::en_keywordListChanged()
{
	if (m_bDoingUpdate) return;
	m_bDoingUpdate = true;

	m_UserNote.setKeywordList(m_pKeywordModel->selectedKeywordList());
	setKeywordListPreview();
	m_bIsDirty = true;

	m_bDoingUpdate = false;
}

void CKJVNoteEditDlg::setKeywordListPreview()
{
	ui->lblKeywordsListPreview->setText(m_pKeywordModel->selectedKeywordList().join(", "));
}

void CKJVNoteEditDlg::en_keywordCurrentIndexChanged(const QString &text)
{
	ui->comboKeywords->setEditText(text);
	ui->comboKeywords->hidePopup();
}

// ============================================================================
