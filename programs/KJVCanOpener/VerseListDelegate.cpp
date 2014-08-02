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
#include "UserNotesDatabase.h"
#include "ScriptureDocument.h"
#include "myApplication.h"

#include <QModelIndex>
#include <QApplication>
#include <QStyle>
#include <QStyleOptionViewItemV4>
#include <QPalette>
#include <QSize>
#include <QRect>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>
#include <QToolTip>
#include <QWhatsThis>
#if QT_VERSION >= 0x050000
#include <qdrawutil.h>
#endif

CVerseListDelegate::CVerseListDelegate(CVerseListModel &model, QObject *parent)
	:	QStyledItemDelegate(parent),
		m_model(model)
{
}

void CVerseListDelegate::SetDocumentText(const QStyleOptionViewItemV4 &option, QTextDocument &doc, const QModelIndex &index, bool bDoingSizeHint) const
{
	assert(index.isValid());

	QTreeView *pView = parentView();
	assert(pView != NULL);
	bool bViewHasFocus = pView->hasFocus();

	Q_UNUSED(bDoingSizeHint);

	doc.setDefaultFont(m_model.font());
	doc.setDefaultStyleSheet(QString("body, p, li, book, chapter { background-color:%1; color:%2; }")
									.arg(option.palette.color(((((option.state & QStyle::State_HasFocus) || (option.state & QStyle::State_Selected)) && bViewHasFocus) ?  QPalette::Active : QPalette::Inactive), ((option.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Base)).name())
									.arg(option.palette.color(((((option.state & QStyle::State_HasFocus) || (option.state & QStyle::State_Selected)) && bViewHasFocus) ?  QPalette::Active : QPalette::Inactive), ((option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text)).name()));



//	doc.setDefaultStyleSheet(QString("body, p, li { background-color:%1; color:%2; white-space: pre-wrap; font-size:medium; }\n.book { background-color:%1; color:%2; font-size:xx-large; font-weight:bold; }\n.chapter { background-color:%1; color:%2; font-size:x-large; font-weight:bold; }")
//			.arg(option.palette.color(QPalette::Active, ((option.state & QStyle::State_Selected) ? QPalette::Highlight : QPalette::Base)).name())
//			.arg(option.palette.color(QPalette::Active, ((option.state & QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text)).name()));

	CScriptureTextHtmlBuilder scriptureHTML;

//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n"));
//	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:medium; }\n.book { font-size:xx-large; font-weight:bold; }\n.chapter { font-size:x-large; font-weight:bold; }\n</style></head><body>\n"));
	scriptureHTML.appendRawText(QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
										"<html><head><style type=\"text/css\">\n"
										"body, p, li { white-space: pre-wrap; font-size:medium; }\n"
										".book { font-size:xx-large; font-weight:bold; }\n"
										".chapter { font-size:x-large; font-weight:bold; }\n"
										".subtitle { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".category { font-size:medium; font-weight:normal; }\n"
										".superscription { font-size:medium; font-weight:normal; font-style:italic; }\n"
										".colophon { font-size:medium; font-weight:normal; font-style:italic; }\n"
										"</style></head><body>\n"));

	scriptureHTML.appendRawText(option.text);
	scriptureHTML.appendRawText("</body></html>");
	doc.setHtml(scriptureHTML.getResult());
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

// The follow is broken on Windows for the startDrag pixel preview but otherwise seems to work
//		everywhere else (even in Windows).  Seems to be a style difference with what is generated
//		for the drag prevew from that of the TreeView itself.  But, the optionV4.rect should
//		always be correct, so we'll just use it...
//
//	if (parentView()) {
//		textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4, parentView());
//	} else {
//		textRect = optionV4.rect;
//	}

	textRect = optionV4.rect;

	switch (m_model.displayMode()) {
		case CVerseListModel::VDME_HEADING:
//			style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter, optionV4.widget);
//			break;
		case CVerseListModel::VDME_RICHTEXT:
			{
				// draw the background:
//				style->drawPrimitive(QStyle::PE_PanelItemViewItem, &optionV4, painter, parentView());
//				if (optionV4.state & QStyle::State_Selected)
//					painter->fillRect(optionV4.rect, optionV4.palette.highlight());

				QTextDocument doc;

				doc.setTextWidth(textRect.width());
				SetDocumentText(optionV4, doc, index, false);
				painter->save();
				painter->translate(textRect.topLeft());
				doc.drawContents(painter, textRect.translated(-textRect.topLeft()));
				painter->restore();

				if (optionV4.state & QStyle::State_HasFocus) {
					qDrawShadeRect(painter, textRect, optionV4.palette, false);
				}

				CRelIndex ndxRel(CVerseListModel::toVerseIndex(index)->relIndex());
				if ((ndxRel.isSet()) &&
					(((index.row() != 0) && ((ndxRel.verse() != 0) || ((ndxRel.verse() == 0) && (ndxRel.word() != 0)))) || (m_model.viewMode() == CVerseListModel::VVME_USERNOTES))) {
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
					int nLineWidth = ((m_model.viewMode() == CVerseListModel::VVME_USERNOTES) ? 4 : 1);
					qDrawShadeLine(painter, textRect.left(), textRect.top()+1, textRect.right(), textRect.top()+1, /*, textRect.topLeft(), textRect.topRight() */ optionV4.palette, true, nLineWidth);
				}

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

	// Note: Without the extra -1 on the setTextWidth calculations below, occassionally QTextDocument will
	//			miscalculate the wrapping for our rectangle size.  For example, in Times New Roman font at
	//			12.00 pnt, a search for "ears to hear" and looking at result Luke 14:35, then when the
	//			TreeView width is just so, the last word will wrap to the next line, but the sizeHint won't
	//			be correct to have that next line.  This -1 will cause it to calculate an extra line for
	//			those.  There is an occassional "extra space case", like will be seen in Matthew 11:15 on
	//			the same search.  But extra space is better than missing text...  It may be due to the
	//			proportional font and the colored search results markup.  Hmmm...

	if ((m_model.displayMode() == CVerseListModel::VDME_RICHTEXT) ||
		(m_model.displayMode() == CVerseListModel::VDME_HEADING)) {
		QTextDocument doc;

		QTreeView *pTree = parentView();
		if (pTree) {
			int nWidth = pTree->viewport()->width() - indentationForIndex(index) /* - 1 */ ;

//			int nWidth = pTree->viewport()->width() - style->subElementRect(QStyle::SE_TreeViewDisclosureItem, &optionV4, parentView()).width();

			doc.setTextWidth(nWidth);
			SetDocumentText(optionV4, doc, index, true);
		} else {
			if (optionV4.rect.isValid()) {
				doc.setTextWidth(optionV4.rect.width() /* -1 */ );
				SetDocumentText(optionV4, doc, index, true);
			} else {
				SetDocumentText(optionV4, doc, index, true);
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

QString CVerseListDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
	Q_UNUSED(locale);
	return value.toString();
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

#ifdef DEBUG_SEARCH_RESULTS_NODE_TOOLTIPS
			QVariant tooltip = index.data(Qt::ToolTipRole);
			if (tooltip.canConvert<QString>()) {
				QToolTip::showText(event->globalPos(), tooltip.toString(), view);
				return true;
			}
#else
			if ((pSearchResultsView->viewMode() == CVerseListModel::VVME_SEARCH_RESULTS) ||
				(pSearchResultsView->viewMode() == CVerseListModel::VVME_SEARCH_RESULTS_EXCLUDED)) {
				if (pSearchResultsView->isActive() && pSearchResultsView->haveDetails() && (!CTipEdit::tipEditIsPinned(pSearchResultsView->parentCanOpener()))) {
//					QVariant tooltip = index.data(Qt::ToolTipRole);
//					if (tooltip.canConvert<QString>()) {
//	//						QToolTip::showText(event->globalPos(), tooltip.toString(), view);
//						CToolTipEdit::showText(event->globalPos(), tooltip.toString(), view, view->rect());
//						return true;
//					}

					QToolTip::showText(event->globalPos(), tr("Press %1 to see Phrase Details", "MainMenu").arg(QKeySequence(Qt::CTRL + Qt::Key_D).toString(QKeySequence::NativeText)), view);
					return true;
				}
			} else if (pSearchResultsView->viewMode() == CVerseListModel::VVME_HIGHLIGHTERS) {
				QToolTip::showText(event->globalPos(), tr("To Edit Highlighted Phrase Associations: Select verses to move,\n"
														  "drag them to the desired highlighter, and drop them.", "MainMenu"), view);
				return true;
			}
#endif

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

