#pragma once

#include "afxdialogex.h"
#include "ml_network.h"
#include "dib.h"
#include <map>
#include <algorithm>


class serialise;

// traindlg dialog

class traindlg : public CDialogEx
{
	DECLARE_DYNAMIC(traindlg)

public:
	traindlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~traindlg();

	void setactive(const bool b);
	void onhint(const hint *p);

	bool read(const serialise *pS) {return true;}
	bool write(const serialise *pS) const {return true;}

// Dialog Data
	enum { IDD = IDD_TRAIN };
	CTreeCtrl m_SetTree;
	CTreeCtrl m_InputTree;
	CTreeCtrl m_OutputTree;
	CString m_csInput;
	CString m_csOutput;
	int m_nInitialiseType;
	CString m_csUserRandomFrom;
	CString m_csUserRandomTo;
	int m_nInfiniteEpochs;
	CString m_csEpochs;
	CSpinButtonCtrl m_EpochsSpin;
	CString m_csLearningRate;

	// enum <-> index
	static afml::activationfn::initialisetype getinitialisetype(const int n);
	static int getinitialisetypeindex(const afml::activationfn::initialisetype t);

	// set
	static HTREEITEM populatesettreectrl(CTreeCtrl *pSetTree);

	// i/o
	static HTREEITEM populateinputtreectrl(CTreeCtrl *pSetTree,CTreeCtrl *pInputTree);

	// generic
	static afml::trainsetitem *gettrainsetitem(CTreeCtrl *pSetTree,const HTREEITEM h);
	static bool gettreectrlchildindex(CTreeCtrl *pTree,const HTREEITEM hParent,const HTREEITEM hChild,int& n);
	static int gettreectrlchildcount(CTreeCtrl *pTree,const HTREEITEM hParent);
	static void validatenetworktrainingmismatch(bool& bMismatch,bool& bFixed);
	static bool loadimagetrainingdata(const serialise *pS,std::map<afml::trainsetitem::inputtype,std::shared_ptr<afml::inputtypeitem>>& m);
protected:
	bool m_bInitialised;
	std::pair<bool,afml::trainsetitem::inputtype> m_InputType;
	
	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnSetTreeChange(NMHDR* pNMHDR,LRESULT* pResult);
	afx_msg void OnInputTreeChange(NMHDR* pNMHDR,LRESULT* pResult);
	afx_msg void OnOutputTreeChange(NMHDR* pNMHDR,LRESULT* pResult);
	afx_msg void OnAdd(void);
	afx_msg void OnErase(void);
	afx_msg void OnLearningRateEditChange(void);
	afx_msg void OnLearningRateEditKillFocus(void);
	afx_msg void OnEpochsEditChange(void);
	afx_msg void OnEpochsEditKillFocus(void);
	afx_msg void OnInputEditChange(void);
	afx_msg void OnOutputEditChange(void);
	afx_msg void OnInitialiseFromEditChange(void);
	afx_msg void OnInitialiseToEditChange(void);
	afx_msg void OnInputEditKillFocus(void);
	afx_msg void OnOutputEditKillFocus(void);
	afx_msg void OnInitialiseFromEditKillFocus(void);
	afx_msg void OnInitialiseToEditKillFocus(void);
	afx_msg void OnInitialiseChange(void);
	afx_msg void OnInfiniteEpochs(void);
	DECLARE_MESSAGE_MAP()

	// set
	HTREEITEM pushbacksettreectrl(void);

	// i/o
	void populateinputoutput(void);
	HTREEITEM populateoutputtreectrl(void);
	void initfrominputtreectrl(void);
	void initfromoutputtreectrl(void);

	// generic
	void populatecontrol(void);
	void enabledisable(void);
	void validatefocus(const HWND hPreFocus);
	bool repopulate(void);
	static std::vector<std::shared_ptr<afml::trainsetitem>> getnetworktrainingmismatch(const afml::trainsetitem::inputtype t);
	static bool getlayerio(int *pI=nullptr,int *pO=nullptr);
	static bool gettrainio(int *pI=nullptr,int *pO=nullptr);
	static HTREEITEM gettreectrlchild(CTreeCtrl *pTree,const HTREEITEM hParent,const int n);
};
