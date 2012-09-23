#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include <QMainWindow>

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CKJVCanOpener(QWidget *parent = 0);
    ~CKJVCanOpener();
    
private:
    Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
