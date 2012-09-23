#include <QtGui/QApplication>
#include "KJVCanOpener.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CKJVCanOpener w;
    w.show();
    
    return a.exec();
}
