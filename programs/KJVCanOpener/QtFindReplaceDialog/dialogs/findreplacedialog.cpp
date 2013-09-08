/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#include "findreplacedialog.h"

FindReplaceDialog::FindReplaceDialog(QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint)
{
	ui.setupUi(this);
}

FindReplaceDialog::~FindReplaceDialog()
{

}

void FindReplaceDialog::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui.retranslateUi(this);
		break;
	default:
		break;
	}
}

void FindReplaceDialog::setTextEdit(QTextEdit *textEdit) {
	ui.findReplaceForm->setTextEdit(textEdit);
}

void FindReplaceDialog::writeSettings(QSettings &settings, const QString &prefix) {
	ui.findReplaceForm->writeSettings(settings, prefix);
}

void FindReplaceDialog::readSettings(QSettings &settings, const QString &prefix) {
	ui.findReplaceForm->readSettings(settings, prefix);
}

void FindReplaceDialog::setTextToFind(const QString &strText)
{
	ui.findReplaceForm->setTextToFind(strText);
}

void FindReplaceDialog::find()
{
	ui.findReplaceForm->find();
}

void FindReplaceDialog::findNext() {
	ui.findReplaceForm->findNext();
}

void FindReplaceDialog::findPrev() {
	ui.findReplaceForm->findPrev();
}
