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

CVerseListDelegate::CVerseListDelegate(CVerseListModel &model, QObject *parent)
	:	QStyledItemDelegate(parent),
		m_model(model)
{
}

void CVerseListDelegate::paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const CVerseListItem &item(index.data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());

	QStyleOptionViewItemV4 optionV4 = option;
	QStyle* style = (optionV4.widget ? optionV4.widget->style() : QApplication::style());

	initStyleOption(&optionV4, index);

	QRect textRect;
	if(parentView()) {
		textRect = style->subElementRect(QStyle::SE_ItemViewItemText, &optionV4, parentView());
	} else {
		textRect = option.rect;
	}

	switch (m_model.displayMode()) {
		case CVerseListModel::VDME_HEADING:
			style->drawControl(QStyle::CE_ItemViewItem, &optionV4, painter, optionV4.widget);
			break;
		case CVerseListModel::VDME_RICHTEXT:
			{
				setVerseListPalette(&optionV4.palette);

				// draw the background:
				style->drawPrimitive(QStyle::PE_PanelItemViewItem, &optionV4, painter, parentView());

				QTextDocument doc;
				CPhraseNavigator navigator(doc);
				CSearchResultHighlighter highlighter(item.phraseTags());

				navigator.setDocumentToVerse(item.getIndex(), (index.row() != 0));
				navigator.doHighlighting(highlighter);

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
	initStyleOption(&optionV4, index);
	QStyle* style = (optionV4.widget ? optionV4.widget->style() : QApplication::style());

	if (m_model.displayMode() == CVerseListModel::VDME_RICHTEXT) {
		const CVerseListItem &item(index.data(CVerseListModel::VERSE_ENTRY_ROLE).value<CVerseListItem>());

		QTextDocument doc;
		CPhraseNavigator navigator(doc);
		CSearchResultHighlighter highlighter(item.phraseTags());

		navigator.setDocumentToVerse(item.getIndex(), (index.row() != 0));
		navigator.doHighlighting(highlighter);

		if (optionV4.rect.isValid()) {
			doc.setTextWidth(optionV4.rect.width());
		} else {
			doc.setTextWidth(doc.idealWidth());
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

