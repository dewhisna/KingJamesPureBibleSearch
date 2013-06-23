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

#include "SearchCompleter.h"
#include "PhraseEdit.h"


// ============================================================================


CSearchStringListModel::CSearchStringListModel(const CParsedPhrase &parsedPhrase, QObject *parent)
	:	QAbstractListModel(parent),
		m_parsedPhrase(parsedPhrase)
{

}

CSearchStringListModel::~CSearchStringListModel()
{

}

int CSearchStringListModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;

	return m_parsedPhrase.nextWordsList().size();
}

QVariant CSearchStringListModel::data(const QModelIndex &index, int role) const
{
	if ((index.row() < 0) || (index.row() >= m_parsedPhrase.nextWordsList().size()))
		return QVariant();

	if (role == Qt::DisplayRole)
		return m_parsedPhrase.nextWordsList().at(index.row()).word();

	if (role == Qt::EditRole)
		return m_parsedPhrase.nextWordsList().at(index.row()).decomposedWord();

	return QVariant();
}

bool CSearchStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	Q_UNUSED(index);
	Q_UNUSED(value);
	Q_UNUSED(role);

	return false;
}

void CSearchStringListModel::setWordsFromPhrase()
{
	emit beginResetModel();

//	m_ParsedPhrase.nextWordsList();

	emit endResetModel();
}

QString CSearchStringListModel::decompose(const QString &strWord)
{
	QString strDecomposed = strWord.normalized(QString::NormalizationForm_KD);

	strDecomposed.replace(QChar(0x00C6), "Ae");				// U+00C6	&#198;		AE character
	strDecomposed.replace(QChar(0x00E6), "ae");				// U+00E6	&#230;		ae character
	strDecomposed.replace(QChar(0x0132), "IJ");				// U+0132	&#306;		IJ character
	strDecomposed.replace(QChar(0x0133), "ij");				// U+0133	&#307;		ij character
	strDecomposed.replace(QChar(0x0152), "Oe");				// U+0152	&#338;		OE character
	strDecomposed.replace(QChar(0x0153), "oe");				// U+0153	&#339;		oe character

	// There are two possible ways to remove accent marks:
	//
	//		1) strDecomposed.remove(QRegExp("[^a-zA-Z\\s]"));
	//
	//		2) Remove characters of class "Mark" (QChar::Mark_NonSpacing,
	//				QChar::Mark_SpacingCombining, QChar::Mark_Enclosing),
	//				which can be done by checking isMark()
	//

	for (int nPos = strDecomposed.size()-1; nPos >= 0; --nPos) {
		if (strDecomposed.at(nPos).isMark()) strDecomposed.remove(nPos, 1);
	}

	return strDecomposed;
}

// ============================================================================


CSearchCompleter::CSearchCompleter(CSearchStringListModel *model, QWidget *parentWidget)
	:	QCompleter(model, parentWidget)
{

}

CSearchCompleter::~CSearchCompleter()
{

}

