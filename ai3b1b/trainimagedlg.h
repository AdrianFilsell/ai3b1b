#pragma once

#include "afxdialogex.h"
#include "resource.h"
#include "ai3b1b.h"
#include "dibwnd.h"

// trainimagedlg dialog

class trainimagedlg : public CDialogEx
{
	DECLARE_DYNAMIC(trainimagedlg)

public:

	enum dibtype {dt_input,dt_output,dt_lerp};

	trainimagedlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~trainimagedlg();

	void onhint(const hint *p);

	// enum <-> index
	static dibtype getdibtype(const int n);
	static int getdibtypeindex(const dibtype t);

// Dialog Data
	enum { IDD = IDD_TRAIN_IMAGE };
	CComboBox m_InputCombo;
	CComboBox m_LerpCombo;
	CString m_csLerp;
	CSpinButtonCtrl m_LerpSpin;
	int m_nLerpSlider;
	CSliderCtrl m_LerpSlider;
	int m_nUpdate;
	CString m_csEpochs;
	CSpinButtonCtrl m_EpochsSpin;
	int m_nDibSrc;
	int m_nLerpDst;
	int m_nSmooth;
	int m_nScaleToFit;
protected:
	bool m_bInitialised;
	UINT_PTR m_nEpochTimer;
	
	std::shared_ptr<dibwnd> m_spDibWnd;
	std::shared_ptr<mlthreaddibrequestcache> m_spReqCache;
				
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnTimer(UINT_PTR nEvent);
	afx_msg void OnHScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar);
	afx_msg void OnInputChanged(void);
	afx_msg void OnSmooth(void);
	afx_msg void OnScaleToFit(void);
	afx_msg void OnLerpEditChange(void);
	afx_msg void OnLerpEditKillFocus(void);
	afx_msg void OnEpochsEditChange(void);
	afx_msg void OnEpochsEditKillFocus(void);
	afx_msg void OnEpochsUpdateNow(void);
	afx_msg void OnEpochsUpdate(void);
	afx_msg void OnThumbInputChanged(void);
	afx_msg void OnLerpDstChanged(void);
	afx_msg void OnExport(void);
	afx_msg void OnLerpAnimate(void);
	DECLARE_MESSAGE_MAP()

	// dib
	void cleardib(const dibtype t){if(m_spDibWnd && getdibtype(m_nDibSrc)==t)m_spDibWnd->clear();}
	void setdib(std::shared_ptr<const afdib::dib> sp,const dibtype t){if(m_spDibWnd && getdibtype(m_nDibSrc)==t)m_spDibWnd->set(sp);}
	void postdibrequest(const dibtype t);

	// generic
	void updateepochtimer(void);
	void killepochtimer(void);
	void enabledisable(void);
	std::shared_ptr<const afdib::dib> getinputdib(void)const;
	void populatetrainingitemcombos(void);
	SIZE *getcustomdim(const dibtype t,const bool bInputDim,SIZE& sz)const;
	std::shared_ptr<mlthreadqueryrequest> createrequest(const void *pOwner,const dibtype t,const mlthreaddibrequestbasecustomdim customdim=mlthreaddibrequestbasecustomdim())const;
	SIZE getoutputdim(const SIZE& sz)const;
	SIZE getlerpdim(const SIZE& szFrom,const SIZE& szTo,const double dLerp)const;
	static bool populatetrainingitemcombo(CComboBox *p);
	static const afml::traintsetimageitem *gettrainingitem(const CComboBox *p,const int n);
};
