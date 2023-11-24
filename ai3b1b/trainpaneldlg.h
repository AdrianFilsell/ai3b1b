#pragma once

#include "afxdialogex.h"
#include "resource.h"
#include "ai3b1b.h"

// trainpaneldlg dialog

class trainpaneldlg : public CDialogEx
{
	DECLARE_DYNAMIC(trainpaneldlg)

public:

	trainpaneldlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~trainpaneldlg();

	void onhint(const hint *p);

// Dialog Data
	enum { IDD = IDD_TRAIN_PANEL };
	CBitmapButton m_Play;
	CBitmapButton m_Pause;
	CBitmapButton m_Step;
	CBitmapButton m_Clear;
	int m_nCost;
	int m_nImage;
	int m_nEpoch;
protected:
	UINT_PTR m_nEpochTimer;
	UINT m_nEpochTimerInterval;
	
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnPlay(void);
	afx_msg void OnPause(void);
	afx_msg void OnClear(void);
	afx_msg void OnStep(void);
	afx_msg void OnImage(void);
	afx_msg void OnCost(void);
	afx_msg void OnTimer(UINT_PTR nEvent);
	DECLARE_MESSAGE_MAP()

	// generic
	void enabledisable(void);
	void updateepochtimer(void);
	void updateepoch(void);
};
