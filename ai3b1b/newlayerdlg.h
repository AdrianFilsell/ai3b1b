#pragma once
#include "afxdialogex.h"
#include "ml_network.h"


// newlayerdlg dialog

class newlayerdlg : public CDialogEx
{
	DECLARE_DYNAMIC(newlayerdlg)

public:
	newlayerdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~newlayerdlg();

// Dialog Data
	enum { IDD = IDD_NEWLAYER };
	CSpinButtonCtrl m_PerceptronsSpin;
	int m_nActivationFnType;
	CString m_csPerceptrons;
	int m_nActivationNormType;
	CString m_csGradientClipThreshold;
	int m_nGradientClipType;

protected:
	bool m_bInitialised;
	
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;

	// Generated message map functions
	afx_msg void OnPerceptronsEditKillFocus(void);
	afx_msg void OnPerceptronsEditChange(void);
	afx_msg void OnActivationFnChange(void);
	afx_msg void OnActivationNormChange(void);
	afx_msg void OnGradientClipChange(void);
	afx_msg void OnGradientClipEditChange(void);
	afx_msg void OnGradientClipEditKillFocus(void);
	DECLARE_MESSAGE_MAP()

	// generic
	void enabledisable(void);
};
