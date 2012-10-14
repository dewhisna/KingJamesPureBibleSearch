#ifndef KJVCANOPENER_H
#define KJVCANOPENER_H

#include <QMainWindow>

#include "dbstruct.h"

namespace Ui {
class CKJVCanOpener;
}

class CKJVCanOpener : public QMainWindow
{
	Q_OBJECT

public:
	explicit CKJVCanOpener(QWidget *parent = 0);
	~CKJVCanOpener();

	void Initialize(uint32_t nInitialIndex = MakeIndex(1,1,1,1));	// Default initial location is the first word of Genesis 1:1)

// UI Private:
private:
	Ui::CKJVCanOpener *ui;
};

#endif // KJVCANOPENER_H
