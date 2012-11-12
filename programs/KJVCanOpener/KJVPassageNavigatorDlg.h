#ifndef KJVPASSAGENAVIGATORDLG_H
#define KJVPASSAGENAVIGATORDLG_H

#include "dbstruct.h"

#include <QDialog>
#include <QAbstractButton>
#include <QPushButton>

namespace Ui {
class CKJVPassageNavigatorDlg;
}

class CKJVPassageNavigatorDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CKJVPassageNavigatorDlg(QWidget *parent = 0);
	virtual ~CKJVPassageNavigatorDlg();

	CRelIndex passage() const;
	void setPassage(const CRelIndex &ndx);

private slots:
	void on_modeChanged(bool bRelative);
	void on_ApplyResolvedClicked();
	void on_ModeClicked();
	void on_ResetClicked();

private:
	QPushButton *m_pApplyButton;		// Apply is to apply the resolved reference to the start reference for relative mode
	QPushButton *m_pModeButton;			// Mode button is to switch Relative/Absolute modes
	QPushButton *m_pResetButton;		// Clear passage offset
	QPushButton *m_pOKButton;			// Goto passage button
	QPushButton *m_pCancelButton;		// Abort
	Ui::CKJVPassageNavigatorDlg *ui;
};

#endif // KJVPASSAGENAVIGATORDLG_H
