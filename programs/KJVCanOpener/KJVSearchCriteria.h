#ifndef KJVSEARCHCRITERIA_H
#define KJVSEARCHCRITERIA_H

#include <QWidget>

namespace Ui {
class CKJVSearchCriteria;
}

class CKJVSearchCriteria : public QWidget
{
	Q_OBJECT
	
public:
	explicit CKJVSearchCriteria(QWidget *parent = 0);
	~CKJVSearchCriteria();
	
private:
	Ui::CKJVSearchCriteria *ui;
};

#endif // KJVSEARCHCRITERIA_H
