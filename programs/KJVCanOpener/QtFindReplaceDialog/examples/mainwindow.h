/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class FindDialog;
class FindReplaceDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event);

    void createActions();

    void writeSettings();
    void readSettings();

private slots:
    void about();

    void findDialog();
    void findReplaceDialog();

private:
    Ui::MainWindow *ui;

    FindDialog *m_findDialog;
    FindReplaceDialog *m_findReplaceDialog;
};

#endif // MAINWINDOW_H
