
// ai3b1bDlg.h : header file
//

#pragma once

#include "tabctrl.h"


// Cai3b1bDlg dialog
class Cai3b1bDlg : public CDialogEx
{
// Construction
public:
	Cai3b1bDlg(CWnd* pParent = nullptr);	// standard constructor

	const tabctrl *gettabctrl(void)const{return &m_TabCtrl;}

	void onhint(const hint *p);
	virtual INT_PTR DoModal() override;

// Dialog Data
	enum { IDD = IDD_AI3B1B_DIALOG };
	int m_nPanelUI;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	tabctrl m_TabCtrl;

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}
	
	// Generated message map functions
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose(void);
	afx_msg void OnLoad();
	afx_msg void OnSave();
	afx_msg void OnPanelUI();
	DECLARE_MESSAGE_MAP()

	// generic
	void enabledisable(void);
};
