#pragma once

#include "afxdialogex.h"
#include "resource.h"
#include "ml_network.h"

// imageexportdimdlg dialog

class imageexportdimdlg : public CDialogEx
{
	DECLARE_DYNAMIC(imageexportdimdlg)

public:

	enum dimtype {dt_input,dt_output,dt_custom};

	imageexportdimdlg(CWnd* pParent,const CSize& szInput,const CSize& szOutput);   // standard constructor
	virtual ~imageexportdimdlg();

	CString getwidth(const dimtype dt)const;
	CString getheight(const dimtype dt)const;

	// enum <-> index
	static dimtype getdimtype(const int n);
	static int getdimtypeindex(const dimtype t);

// Dialog Data
	enum { IDD = IDD_EXPORT_IMAGE_DIM };
	CComboBox m_Combo;
	int m_nType;
	int m_nAspect;
	CString m_csWidth;
	CString m_csHeight;
	CSpinButtonCtrl m_WidthSpin;
	CSpinButtonCtrl m_HeightSpin;
protected:
	CSize m_szInput;
	CSize m_szOutput;
	CSize m_szCustom;
	double m_dAspect;
	bool m_bInitialised;

	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;

	// Generated message map functions
	afx_msg void OnTypeChanged(void);
	afx_msg void OnWidthEditChange(void);
	afx_msg void OnWidthEditKillFocus(void);
	afx_msg void OnHeightEditChange(void);
	afx_msg void OnHeightEditKillFocus(void);
	DECLARE_MESSAGE_MAP()

	void enabledisable(void);
};
