#include "KJVCanOpener.h"
#include "ui_KJVCanOpener.h"

CKJVCanOpener::CKJVCanOpener(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CKJVCanOpener)
{
    ui->setupUi(this);
}

CKJVCanOpener::~CKJVCanOpener()
{
    delete ui;
}
