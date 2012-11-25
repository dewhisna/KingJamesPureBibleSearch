#ifndef KJVABOUTDLG_H
#define KJVABOUTDLG_H

#include <QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>

namespace Ui {
class CKJVAboutDlg;
}

class CKJVAboutDlg : public QDialog
{
	Q_OBJECT

public:
	explicit CKJVAboutDlg(QWidget *parent = 0);
	~CKJVAboutDlg();

private slots:
	void on_licenseDisplay();

// Data Private:
private:


// UI Private:
private:
	Ui::CKJVAboutDlg *ui;
	QGraphicsPixmapItem *m_pKJVCan;
	QGraphicsPixmapItem *m_pBethelChurch;
	QGraphicsTextItem *m_pAppTitle;
	QGraphicsTextItem *m_pBroughtToYouBy;
	QGraphicsTextItem *m_pBethelURL;
};

#endif // KJVABOUTDLG_H
