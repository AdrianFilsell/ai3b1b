#pragma once

#include "afxdialogex.h"
#include "resource.h"
#include "ai3b1b.h"
#include "traincostgraphwnd.h"

// traincostdlg dialog

class traincostdlg : public CDialogEx
{
	DECLARE_DYNAMIC(traincostdlg)

public:

	traincostdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~traincostdlg();

	std::shared_ptr<mlthreadqueryrequest> createrequest(void)const;

	void onhint(const hint *p);

// Dialog Data
	enum { IDD = IDD_TRAIN_COST };
	int m_nUpdate;
	CString m_csEpochs;
	CSpinButtonCtrl m_EpochsSpin;
	CString m_csMinValue;
	CString m_csMaxValue;
	CString m_csLastValue;
	CString m_csMinEpoch;
	CString m_csMaxEpoch;
	CString m_csLastEpoch;
	CString m_csLocalMin;
	CString m_csLocalMax;
protected:
	bool m_bInitialised;
	UINT_PTR m_nEpochTimer;

	std::shared_ptr<traincostgraphwnd> m_spGraph;
	
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnTimer(UINT_PTR nEvent);
	afx_msg void OnDestroy(void);
	afx_msg void OnEpochsEditChange(void);
	afx_msg void OnEpochsEditKillFocus(void);
	afx_msg void OnEpochsUpdateNow(void);
	afx_msg void OnEpochsUpdate(void);
	DECLARE_MESSAGE_MAP()

	// generic
	void enabledisable(void);
	void updateepochtimer(void);
	void killepochtimer(void);
	void postepochrequest(void);
	void updatecosts(const std::pair<double,double> inflatedlocalrange,const std::pair<double,int> globalmin,const std::pair<double,int> globalmax,const std::pair<double,int> last,const bool bUpdateData=true);
};
