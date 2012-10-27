#ifndef KJVPASSAGENAVIGATORDLG_H
#define KJVPASSAGENAVIGATORDLG_H

#include "dbstruct.h"

#include <QDialog>

namespace Ui {
class CKJVPassageNavigatorDlg;
}

class CKJVPassageNavigatorDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CKJVPassageNavigatorDlg(QWidget *parent = 0);
	~CKJVPassageNavigatorDlg();

	CRelIndex passage() const;
	void setPassage(const CRelIndex &ndx);

private:
	Ui::CKJVPassageNavigatorDlg *ui;
};

#endif // KJVPASSAGENAVIGATORDLG_H
