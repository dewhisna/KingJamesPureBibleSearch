#include "KJVPassageNavigatorDlg.h"
#include "ui_KJVPassageNavigatorDlg.h"

CKJVPassageNavigatorDlg::CKJVPassageNavigatorDlg(QWidget *parent) :
	QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
	m_pApplyButton(NULL),
	m_pModeButton(NULL),
	m_pResetButton(NULL),
	m_pOKButton(NULL),
	m_pCancelButton(NULL),
	ui(new Ui::CKJVPassageNavigatorDlg)
{
	ui->setupUi(this);

	m_pApplyButton = ui->buttonBox->addButton("&Apply Resolved to From Location", QDialogButtonBox::ApplyRole);
	connect(m_pApplyButton, SIGNAL(clicked()), this, SLOT(on_ApplyResolvedClicked()));

	m_pModeButton = ui->buttonBox->addButton("&Switch Mode", QDialogButtonBox::ActionRole);
	connect(m_pModeButton, SIGNAL(clicked()), this, SLOT(on_ModeClicked()));

	m_pResetButton = ui->buttonBox->addButton("&Reset", QDialogButtonBox::ResetRole);
	connect(m_pResetButton, SIGNAL(clicked()), this, SLOT(on_ResetClicked()));

	m_pOKButton = ui->buttonBox->addButton("&Goto", QDialogButtonBox::AcceptRole);

	m_pCancelButton = ui->buttonBox->addButton("&Cancel", QDialogButtonBox::RejectRole);

	// Setup initial mode to match widget:
	on_modeChanged(ui->widgetKJVPassageNavigator->isRelative());

	connect(ui->widgetKJVPassageNavigator, SIGNAL(modeChanged(bool)), this, SLOT(on_modeChanged(bool)));
}

CKJVPassageNavigatorDlg::~CKJVPassageNavigatorDlg()
{
	delete ui;
}

CRelIndex CKJVPassageNavigatorDlg::passage() const
{
	return ui->widgetKJVPassageNavigator->passage();
}

void CKJVPassageNavigatorDlg::setPassage(const CRelIndex &ndx)
{
	ui->widgetKJVPassageNavigator->setPassage(ndx);
}

CKJVPassageNavigator &CKJVPassageNavigatorDlg::navigator()
{
	return *(ui->widgetKJVPassageNavigator);
}

void CKJVPassageNavigatorDlg::on_modeChanged(bool bRelative)
{
	if (bRelative) {
		setWindowTitle("Passage Navigator - Relative Mode");
		m_pApplyButton->show();
		m_pModeButton->setText("&Switch to Absolute Mode");
	} else {
		setWindowTitle("Passage Navigator - Absolute Mode");
		m_pApplyButton->hide();
		m_pModeButton->setText("&Switch to Relative Mode");
	}
}

void CKJVPassageNavigatorDlg::on_ApplyResolvedClicked()
{
	// Reversing and swapping passage and startRef are symmetric:
	ui->widgetKJVPassageNavigator->startRelativeMode(ui->widgetKJVPassageNavigator->passage(), !ui->widgetKJVPassageNavigator->isReversed());
}

void CKJVPassageNavigatorDlg::on_ModeClicked()
{
	if (ui->widgetKJVPassageNavigator->isAbsolute()) {
		ui->widgetKJVPassageNavigator->startRelativeMode(ui->widgetKJVPassageNavigator->passage());
		ui->widgetKJVPassageNavigator->setPassage(CRelIndex());
	} else {
		ui->widgetKJVPassageNavigator->startAbsoluteMode(ui->widgetKJVPassageNavigator->passage());
	}
}

void CKJVPassageNavigatorDlg::on_ResetClicked()
{
	setPassage(CRelIndex());
}

