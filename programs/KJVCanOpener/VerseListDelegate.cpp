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

#include "VerseListDelegate.h"
#include "PhraseEdit.h"
#include "Highlighter.h"
#include "ToolTipEdit.h"
#include "KJVCanOpener.h"
#include "PersistentSettings.h"
#include <QModelIndex>
#include <QApplication>
#include <QStyle>
#include <QStyleOptionViewItemV4>
#include <QSize>
#include <QRect>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QToolTip>
#include <QWhatsThis>

CVerseListDelegate::CVerseListDelegate(CVerseListModel &model, QObject *parent)
	:	QStyledItemDelegate(parent),
		m_model(model)
{
}

void CVerseListDelegate::SetDocumentText(QTextDocument &doc, const QModelIndex &index, bool bDoingSizeHint) const
{
	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());

	doc.setDefaultFont(CPersistentSettings::instance()->fontSearchResults());

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n");

//	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n");
	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n");

	if (ndxRel.verse() != 0) {
		const CVerseListItem &item(index.data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());

		CPhraseNavigator navigator(m_model.bibleDatabase(), doc);
		if (!bDoingSizeHint) {
			navigator.setDocumentToVerse(item.getIndex());
			CSearchResultHighlighter highlighter(item.phraseTags());
			navigator.doHighlighting(highlighter);
		} else {
			navigator.setDocumentToVerse(item.getIndex(), false, true);		// If not doing highlighting, no need to add anchors (improves search results rendering for size hints)
		}
	} else if (ndxRel.chapter() != 0) {
		int nVerses = m_model.GetVerseCount(ndxRel.book(), ndxRel.chapter());
		int nResults = m_model.GetResultsCount(ndxRel.book(), ndxRel.chapter());
		if ((nResults) || (nVerses)) {
			strHTML += QString("<p>{%1} (%2) %3</p>\n").arg(nVerses).arg(nResults).arg(Qt::escape(index.data().toString()));
		} else {
			strHTML += QString("<p>%1</p>\n").arg(Qt::escape(index.data().toString()));
		}
		strHTML += "</body></html>";
		doc.setHtml(strHTML);
	} else {
		int nVerses = m_model.GetVerseCount(ndxRel.book());
		int nResults = m_model.GetResultsCount(ndxRel.book());
		if ((nResults) || (nVerses)) {
			strHTML += QString("<p>{%1} (%2) <b>%3</b></p>\n").arg(nVerses).arg(nResults).arg(Qt::escape(index.data().toString()));
		} else {
			strHTML += QString("<p><b>%1</b></p>\n").arg(Qt::escape(index.data().toString()));
		}
		strHTML += "</body></html>";
		doc.setHtml(strHTML);
	}
}

int CVerseListDelegate::indentationForIndex(const QModelIndex &index) const
{
	QTreeView *pView = parentView();
	assert(pView != NULL);

	int nLevel = 0;
	if (pView->rootIsDecorated()) ++nLevel;
	for (QModelIndex ndxParent = index.parent(); ndxParent.isValid(); ndxParent = ndxParent.parent()) {
		++nLevel;
	}

	// Return number of pixels instead of level count:
	return (nLevel * pView->indentation());
}

void CVerseListDelegate::paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 optionV4 = option;
	QStyle* style = (optionV4.widget ? optionV4.widget->style() : QApplication::style());

	initStyleOption(&optionV4, index);

	QRect textRect;
	if (parentView()) {
		textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4, parentView());
	} else {
		textRect = optionV4.rect;
	}

	switch (m_model.displayMode()) {
		case CVerseListModel::VDME_HEADING:
			style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter, optionV4.widget);
			break;
		case CVerseListModel::VDME_RICHTEXT:
			{
				setVerseListPalette(&optionV4.palette);

				if (optionV4.state & QStyle::State_Selected) {
//					painter->fillRect(optionV4.rect, optionV4.palette.highlight());
					painter->fillRect(textRect, optionV4.palette.highlight());
				}
				if (optionV4.state & QStyle::State_HasFocus) {
					qDrawShadeRect(painter, textRect, optionV4.palette, false);
				}

				CRelIndex ndxRel(index.internalId());
				assert(ndxRel.isSet());
				if ((index.row() != 0) && (ndxRel.verse() != 0)) {
					// Ideally we would just draw the line on the top of all entries except for
					//		when index.row() == 0. However, there seems to be a one row overlap
					//		in the rectangles from one cell to the next (QTreeView bug?) and the
					//		qDrawShadeRect for the HasFocus above causes the lines to disappear
					//		when moving the cursor in the up direction and reappear in the
					//		down direction.  So, we'll just draw them on both top and bottom.
					//		It still looks OK, but does have a funky doubling effect when
					//		moving up and down.
					// ... update...  Adding 1 to the top row fixes the drawing issue and we
					//		can now just draw it on the rows we want...
					qDrawShadeLine(painter, textRect.left(), textRect.top()+1, textRect.right(), textRect.top()+1, /*, textRect.topLeft(), textRect.topRight() */ optionV4.palette);
				}

//				QStyleOption branchOption;
//				branchOption.rect = style->subElementRect(QStyle::SE_TreeViewDisclosureItem, &optionV4, parentView());
//				branchOption.palette = optionV4.palette;
//				branchOption.state = 0;
//
//				if (optionV4.state & QStyle::State_Children)
//					branchOption.state |= QStyle::State_Children;
//
//				if (optionV4.state & QStyle::State_Open)
//					branchOption.state |= QStyle::State_Open;
//
//				style->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, parentView());


				// draw the background:
//				style->drawPrimitive(QStyle::PE_PanelItemViewItem, &optionV4, painter, parentView());
//				if (optionV4.state & QStyle::State_Selected)
//					painter->fillRect(optionV4.rect, optionV4.palette.highlight());

				QTextDocument doc;

				doc.setTextWidth(textRect.width());
				SetDocumentText(doc, index, false);
				painter->save();
				painter->translate(textRect.topLeft());
				doc.drawContents(painter, textRect.translated(-textRect.topLeft()));
				painter->restore();
			}
			break;
		default:
			// CVerseListModel::VDME_VERYPLAIN:
			// CVerseListModel::VDME_COMPLETE:
			// default:
			style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter, optionV4.widget);
			break;
	}
}

QSize CVerseListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 optionV4 = option;
	QStyle* style = (optionV4.widget ? optionV4.widget->style() : QApplication::style());

	initStyleOption(&optionV4, index);

//	QSize szHint = QStyledItemDelegate::sizeHint(optionV4, index);

	if (m_model.displayMode() == CVerseListModel::VDME_RICHTEXT) {
		QTextDocument doc;

		QTreeView *pTree = parentView();
		if (pTree) {
			CRelIndex ndxRel(index.internalId());
			assert(ndxRel.isSet());
			int nWidth = pTree->viewport()->width() - indentationForIndex(index);

//			int nWidth = pTree->viewport()->width() - style->subElementRect(QStyle::SE_TreeViewDisclosureItem, &optionV4, parentView()).width();

			doc.setTextWidth(nWidth);
			SetDocumentText(doc, index, true);
		} else {
			if (optionV4.rect.isValid()) {
				doc.setTextWidth(optionV4.rect.width());
				SetDocumentText(doc, index, true);
			} else {
				SetDocumentText(doc, index, true);
				doc.setTextWidth(doc.idealWidth());
			}
		}

		return doc.size().toSize();
	}

	// CVerseListModel::VDME_HEADING:
	// CVerseListModel::VDME_VERYPLAIN:
	// CVerseListModel::VDME_COMPLETE:
	// default:

// Note: Do NOT call to index->data SizeHintRole here when using the CReflowDelegate or
//	you'll end up in a circular loop with calculating/recalculating the layout over and
//	over again:
//
//	QVariant value = index.data(Qt::SizeHintRole);
//	if (value.isValid())
//		return value.toSize();

	return style->sizeFromContents(QStyle::CT_ItemViewItem, &optionV4, QSize(), optionV4.widget);
}

bool CVerseListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	Q_UNUSED(option);

	CSearchResultsTreeView *pSearchResultsView = static_cast<CSearchResultsTreeView*>(view);

	if (!event || !view || !pSearchResultsView)
		return false;

	switch (event->type()) {
		case QEvent::ToolTip:
		{
			if (pSearchResultsView->isActive() && pSearchResultsView->haveDetails()) {
//				QVariant tooltip = index.data(Qt::ToolTipRole);
//				if (tooltip.canConvert<QString>()) {
//	//					QToolTip::showText(event->globalPos(), tooltip.toString(), view);
//					CToolTipEdit::showText(event->globalPos(), tooltip.toString(), view, view->rect());
//					return true;
//				}

				QToolTip::showText(event->globalPos(), tr("Press %1 to see Phrase Details").arg(QKeySequence(Qt::CTRL + Qt::Key_D).toString(QKeySequence::NativeText)), view);
				return true;
			}
			break;
		}
		case QEvent::QueryWhatsThis:
			if (index.data(Qt::WhatsThisRole).isValid())
				return true;
			break;
		case QEvent::WhatsThis:
		{
			QVariant whatsthis = index.data(Qt::WhatsThisRole);
			if (whatsthis.canConvert<QString>()) {
				QWhatsThis::showText(event->globalPos(), whatsthis.toString(), view);
				return true;
			}
			break;
		}
		default:
			break;
	}

	return false;
}

