#include "VerseListDelegate.h"
#include "PhraseEdit.h"
#include "Highlighter.h"
#include "ToolTipEdit.h"

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
#include <QTreeView>

CVerseListDelegate::CVerseListDelegate(CVerseListModel &model, QObject *parent)
	:	QStyledItemDelegate(parent),
		m_model(model)
{
}

void CVerseListDelegate::SetDocumentText(QTextDocument &doc, const QModelIndex &index) const
{
	CRelIndex ndxRel(index.internalId());
	assert(ndxRel.isSet());

	QString strHTML = QString("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><style type=\"text/css\">\nbody, p, li { white-space: pre-wrap; font-family:\"Times New Roman\", Times, serif; font-size:12pt; }\n.book { font-size:24pt; font-weight:bold; }\n.chapter { font-size:18pt; font-weight:bold; }\n</style></head><body>\n");

	if (ndxRel.verse() != 0) {
		const CVerseListItem &item(index.data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());

		CPhraseNavigator navigator(doc);
		CSearchResultHighlighter highlighter(item.phraseTags());

		navigator.setDocumentToVerse(item.getIndex() /*, (index.row() != 0) */ );
		navigator.doHighlighting(highlighter);
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

void CVerseListDelegate::paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 optionV4 = option;
	QStyle* style = (optionV4.widget ? optionV4.widget->style() : QApplication::style());

	initStyleOption(&optionV4, index);

	QRect textRect;
	if(parentView()) {
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
				if (ndxRel.verse() != 0) {
					// Ideally we would just draw the line on the top of all entries except for
					//		when index.row() == 0. However, there seems to be a one row overlap
					//		in the rectangles from one cell to the next (QTreeView bug?) and the
					//		qDrawShadeRect for the HasFocus above causes the lines to disappear
					//		when moving the cursor in the up direction and reappear in the
					//		down direction.  So, we'll just draw them on both top and bottom.
					//		It still looks OK, but does have a funky doubling effect when
					//		moving up and down.
					qDrawShadeLine(painter, textRect.topLeft(), textRect.topRight(), optionV4.palette);
					qDrawShadeLine(painter, textRect.bottomLeft(), textRect.bottomRight(), optionV4.palette);
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
				SetDocumentText(doc, index);

				doc.setTextWidth(textRect.width());
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
		SetDocumentText(doc, index);

		if(parentView()) {
			CRelIndex ndxRel(index.internalId());
			assert(ndxRel.isSet());
			QTreeView *pTree = static_cast<QTreeView *>(parentView());
			int nIndentation = pTree->indentation();
			int nWidth = parentView()->width() - nIndentation;

//			int nWidth = parentView()->width() - style->subElementRect(QStyle::SE_TreeViewDisclosureItem, &optionV4, parentView()).width();

			switch (m_model.treeMode()) {
				case CVerseListModel::VTME_LIST:
					break;
				case CVerseListModel::VTME_TREE_BOOKS:
					if (ndxRel.verse() != 0) nWidth -= nIndentation;
					nWidth -= nIndentation;
					break;
				case CVerseListModel::VTME_TREE_CHAPTERS:
					if (ndxRel.verse() != 0) nWidth -= nIndentation;
					if (ndxRel.chapter() != 0) nWidth -= nIndentation;
					nWidth -= nIndentation;
					break;
			}
			doc.setTextWidth(nWidth);
		} else {
			if (optionV4.rect.isValid()) {
				doc.setTextWidth(optionV4.rect.width());
			} else {
				doc.setTextWidth(doc.idealWidth());
			}
		}

		return doc.size().toSize();
	}

	// CVerseListModel::VDME_HEADING:
	// CVerseListModel::VDME_VERYPLAIN:
	// CVerseListModel::VDME_COMPLETE:
	// default:
	QVariant value = index.data(Qt::SizeHintRole);
	if (value.isValid())
		return qvariant_cast<QSize>(value);
	return style->sizeFromContents(QStyle::CT_ItemViewItem, &optionV4, QSize(), optionV4.widget);
}

bool CVerseListDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	Q_UNUSED(option);

	if (!event || !view)
		return false;

	switch (event->type()) {
		case QEvent::ToolTip:
		{
			if (!m_model.hasExceededDisplayLimit()) {
				QVariant tooltip = index.data(Qt::ToolTipRole);
				if (tooltip.canConvert<QString>()) {
//					QToolTip::showText(event->globalPos(), tooltip.toString(), view);
					CToolTipEdit::showText(event->globalPos(), tooltip.toString(), view);
					return true;
				}
			} else {
				QToolTip::showText(event->globalPos(), "Too many search results to display in this mode!!\nTry Switching to View References Only mode.", view);
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

