/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#include <QtGui>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "finddialog.h"
#include "findreplacedialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_findDialog = new FindDialog(this);
    m_findDialog->setModal(false);
    m_findDialog->setTextEdit(ui->textEdit);

    m_findReplaceDialog = new FindReplaceDialog(this);
    m_findReplaceDialog->setModal(false);
    m_findReplaceDialog->setTextEdit(ui->textEdit);

    ui->textEdit->setText(
            "Here's some text\nYou can use it to find\n"
            "with the Find/Replace dialog\n"
            "and do some tests. :)"
            );

    createActions();

    readSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createActions() {
    connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(findDialog()));
    connect(ui->actionReplace, SIGNAL(triggered()), this, SLOT(findReplaceDialog()));

    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));

    connect(ui->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    connect(ui->actionFindNext, SIGNAL(triggered()), m_findDialog, SLOT(findNext()));
    connect(ui->actionFindPrevious, SIGNAL(triggered()), m_findDialog, SLOT(findPrev()));
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::about()
{
      QMessageBox::about(this, tr("About Find/Replace Example"),
            tr("Simple application showing Find/Replace Dialog.<br><br>") +
                + "Author: <a href=\"http://www.lorenzobettini.it\">Lorenzo Bettini</a><br><br>");
}

void MainWindow::findDialog() {
    m_findDialog->show();
}

void MainWindow::findReplaceDialog() {
    m_findReplaceDialog->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::writeSettings() {
    QSettings settings("QtFindReplaceDialog", "QtFindReplaceDialogExample");
    m_findDialog->writeSettings(settings);
    m_findReplaceDialog->writeSettings(settings);
}

void MainWindow::readSettings() {
    QSettings settings("QtFindReplaceDialog", "QtFindReplaceDialogExample");
    m_findDialog->readSettings(settings);
    m_findReplaceDialog->readSettings(settings);
}
