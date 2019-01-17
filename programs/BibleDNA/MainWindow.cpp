//
// Bible DNA Study - MainWindow Implementation
//

#include "MainWindow.h"
#include "ui_MainWindow.h"

// ============================================================================

CMainWindow::CMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::CMainWindow)
{
	ui->setupUi(this);
}

CMainWindow::~CMainWindow()
{
	delete ui;
}

// ============================================================================

