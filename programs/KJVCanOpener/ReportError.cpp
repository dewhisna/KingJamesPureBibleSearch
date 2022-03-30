/****************************************************************************
**
** Copyright (C) 2014-2022 Donna Whisnant, a.k.a. Dewtronics.
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

#include "ReportError.h"

#include <QDebug>

#if defined(IS_CONSOLE_APP)
#include <iostream>
#endif

#if !defined(IS_CONSOLE_APP) && defined(USE_ASYNC_DIALOGS)
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QPointer>
#include <QEventLoop>
#include <QTimer>
#endif

// ============================================================================

#if !defined(IS_CONSOLE_APP) && defined(USE_ASYNC_DIALOGS)

static void asyncShowNewMessageBox(QWidget *parent,
	QMessageBox::Icon icon,
	const QString& title, const QString& text,
	QMessageBox::StandardButtons buttons,
	QMessageBox::StandardButton defaultButton,
	std::function<void (QMessageBox::StandardButton nResult)> fnCompletion)
{
	QPointer<QMessageBox> pMsgBox = new QMessageBox(icon, title, text, QMessageBox::NoButton, parent);
	QDialogButtonBox *buttonBox = pMsgBox->findChild<QDialogButtonBox*>();
	Q_ASSERT(buttonBox != 0);

	uint mask = QMessageBox::FirstButton;
	while (mask <= QMessageBox::LastButton) {
		uint sb = buttons & mask;
		mask <<= 1;
		if (!sb)
			continue;
		QPushButton *button = pMsgBox->addButton((QMessageBox::StandardButton)sb);
		// Choose the first accept role as the default
		if (pMsgBox->defaultButton())
			continue;
		if ((defaultButton == QMessageBox::NoButton && buttonBox->buttonRole(button) == QDialogButtonBox::AcceptRole)
			|| (defaultButton != QMessageBox::NoButton && sb == uint(defaultButton)))
			pMsgBox->setDefaultButton(button);
	}

	pMsgBox->setAttribute(Qt::WA_DeleteOnClose, false);
	pMsgBox->setAttribute(Qt::WA_ShowModal, true);
	pMsgBox->setResult(0);

	QMessageBox::connect(
		pMsgBox, &QMessageBox::finished,
		[parent, pMsgBox, fnCompletion](int nResult)
		{
			Q_UNUSED(nResult);
			QMessageBox::StandardButton nRetVal = QMessageBox::NoButton;
			Q_ASSERT(!pMsgBox.isNull());
			if (pMsgBox) {
				nRetVal = pMsgBox->standardButton(pMsgBox->clickedButton());
			}
			if (fnCompletion) fnCompletion(nRetVal);
			if (pMsgBox) {
				pMsgBox->deleteLater();
			}
#if QT_VERSION >= 0x050400		// Functor calls was introduced in Qt 5.4
			if (parent) {
				QTimer::singleShot(10, [parent]()->void { parent->activateWindow();  parent->setFocus(); });
			}
#else
			Q_UNUSED(parent);
#endif
		}
	);

	pMsgBox->show();
}

#endif

// ============================================================================

#ifndef IS_CONSOLE_APP

#ifndef USE_ASYNC_DIALOGS
QMessageBox::StandardButton
#else
void
#endif
				displayWarning(QWidget *pParent, const QString &strTitle, const QString &strText,
								QMessageBox::StandardButtons nButtons,
								QMessageBox::StandardButton nDefaultButton,
								std::function<void (QMessageBox::StandardButton nResult)> fnCompletion)
{
#if defined(EMSCRIPTEN) && !defined(Q_OS_WASM)
	qDebug("warning: %s: %s", strTitle.toUtf8().data(), strText.toUtf8().data());
	Q_UNUSED(pParent);
	Q_UNUSED(nButtons);
	if (nDefaultButton != QMessageBox::NoButton) return nDefaultButton;
	return QMessageBox::Ok;
#else

#ifndef USE_ASYNC_DIALOGS
	QMessageBox::StandardButton nRetVal = QMessageBox::warning(pParent, strTitle, strText, nButtons, nDefaultButton);
	if (fnCompletion) fnCompletion(nRetVal);
	return nRetVal;
#else
	asyncShowNewMessageBox(pParent, QMessageBox::Warning, strTitle, strText, nButtons, nDefaultButton, fnCompletion);
#endif

#endif
}

#else

void displayWarning(void *pParent, const QString &strTitle, const QString &strText)
{
	std::cerr << QString::fromUtf8("warning: %1: %2\n").arg(strTitle).arg(strText).toUtf8().data();
	Q_UNUSED(pParent);
}

#endif


// ----------------------------------------------------------------------------

#ifndef IS_CONSOLE_APP

#ifndef USE_ASYNC_DIALOGS
QMessageBox::StandardButton
#else
void
#endif
				displayInformation(QWidget *pParent, const QString &strTitle, const QString &strText,
									QMessageBox::StandardButtons nButtons,
									QMessageBox::StandardButton nDefaultButton,
									std::function<void (QMessageBox::StandardButton nResult)> fnCompletion)
{
#if defined(EMSCRIPTEN) && !defined(Q_OS_WASM)
	qDebug("information: %s: %s", strTitle.toUtf8().data(), strText.toUtf8().data());
	Q_UNUSED(pParent);
	Q_UNUSED(nButtons);
	if (nDefaultButton != QMessageBox::NoButton) return nDefaultButton;
	return QMessageBox::Ok;
#else

#ifndef USE_ASYNC_DIALOGS
	QMessageBox::StandardButton nRetVal = QMessageBox::information(pParent, strTitle, strText, nButtons, nDefaultButton);
	if (fnCompletion) fnCompletion(nRetVal);
	return nRetVal;
#else
	asyncShowNewMessageBox(pParent, QMessageBox::Information, strTitle, strText, nButtons, nDefaultButton, fnCompletion);
#endif

#endif
}

#else

void displayInformation(void *pParent, const QString &strTitle, const QString &strText)
{
	std::cerr << QString::fromUtf8("information: %1: %2\n").arg(strTitle).arg(strText).toUtf8().data();
	Q_UNUSED(pParent);
}

#endif

// ============================================================================
