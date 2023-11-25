#pragma once
#include "afxdialogex.h"
#include "ml_network.h"
#include <vector>
#include <map>


class serialise;

// layerdlg dialog

class layerdlg : public CDialogEx
{
	DECLARE_DYNAMIC(layerdlg)

	// layer selection
	class layerstg
	{
		public:
			bool isempty(void)const{return m_vIndices.size()==0;}
			int getoutput(void)const{return m_nOutput;}
			bool isoutputselected(void)const{return m_bOutput;}
			const std::vector<int>& getindices(void)const{return m_vIndices;}
			const std::vector<afml::layer<>*>& getlayers(void)const{return m_vLayers;}
		protected:
			layerstg(){m_bOutput=false;m_nOutput=-1;}
			bool m_bOutput;
			int m_nOutput;
			std::vector<int> m_vIndices;
			std::vector<afml::layer<>*> m_vLayers;
	};
	class sel:public layerstg { public: sel(layerdlg *p,const bool bRemoveHidden=false,const bool bRemoveOutput=false); };
	class movablesel:public layerstg { public: movablesel(const sel *p,const bool bUp); };
	class outputlayer:public layerstg { public: outputlayer(layerdlg *p); };

	// layer params
	class layerparams
	{
		public:
			layerparams(){}
			template <typename T> layerparams(const std::vector<T>& v);
			std::vector<int>& getperceptrons(void){return m_vPerceptrons; }
			std::vector<afml::activationfn>& getactivationfn(void){return m_vActFns;}
			std::vector<afml::activationnorm<>>& getactivationnorm(void){return m_vNormActFns;}
			std::vector<afml::gradientclip<>>& getgradientclip(void){return m_vGClips;}
			std::vector<std::shared_ptr<afml::layer<>>> createlayers(const int nInputPerceptrons) const;
			void swap(const int nDst,const int nSrc) {std::swap(m_vPerceptrons[nDst],m_vPerceptrons[nSrc]);std::swap(m_vActFns[nDst],m_vActFns[nSrc]);std::swap(m_vNormActFns[nDst],m_vNormActFns[nSrc]);std::swap(m_vGClips[nDst],m_vGClips[nSrc]);}
		protected:
			std::vector<int> m_vPerceptrons;
			std::vector<afml::activationfn> m_vActFns;
			std::vector<afml::activationnorm<>> m_vNormActFns;
			std::vector<afml::gradientclip<>> m_vGClips;
	};

public:
	layerdlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~layerdlg();

	void setactive(const bool b){}
	afml::trainsetitem::inputtype getinputtype(void) const;
	void setinputtype(const afml::trainsetitem::inputtype it);
	void onhint(const hint *p);

	bool read(const serialise *pS);
	bool write(const serialise *pS) const;

	// enum <-> index
	static afml::trainsetitem::inputtype getinputtype(const int n);
	static afml::activationfn::type getactivationfntype(const int n);
	static afml::activationnorm<>::type getactivationnormtype(const int n);
	static afml::gradientclip<>::type getgradientcliptype(const int n);
	static int getinputtypeindex(const afml::trainsetitem::inputtype t);
	static int getactivationfntypeindex(const afml::activationfn::type t);
	static int getactivationnormtypeindex(const afml::activationnorm<>::type t);
	static int getgradientcliptypeindex(const afml::gradientclip<>::type t);
	static CString getinputtypename(const afml::trainsetitem::inputtype t);

	// list ctrl
	static void setlistctrlcolumntext(CListCtrl *pList,const int n,const int nC,const CString& cs);

// Dialog Data
	enum { IDD = IDD_LAYER };
	CSpinButtonCtrl m_InputPerceptronsSpin;
	CSpinButtonCtrl m_PerceptronsSpin;
	CSpinButtonCtrl m_InputDimSpin;
	CListCtrl m_List;
	int m_nInputType;
	CString m_csInputPerceptrons;
	CString m_csInputUserPerceptrons;
	int m_nActivationFnType;
	CString m_csPerceptrons;
	int m_nActivationNormType;
	int m_nGradientClipType;
	CString m_csGradientClipThreshold;
	CString m_csInputDim;

protected:
	bool m_bInitialised;

	virtual void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override {}
	virtual void OnCancel() override {}

	// Generated message map functions
	afx_msg void OnInputTypeSelChange(void);
	afx_msg void OnInputPerceptronsEditKillFocus(void);
	afx_msg void OnInputPerceptronsEditChange(void);
	afx_msg void OnInputDimEditKillFocus(void);
	afx_msg void OnInputDimEditChange(void);
	afx_msg void OnPerceptronsEditKillFocus(void);
	afx_msg void OnPerceptronsEditChange(void);
	afx_msg void OnSelChange(NMHDR* pNMHDR,LRESULT* pResult);
	afx_msg void OnLayerUp(void);
	afx_msg void OnLayerDown(void);
	afx_msg void OnLayerAdd(void);
	afx_msg void OnLayerErase(void);
	afx_msg void OnActivationFnChange(void);
	afx_msg void OnActivationNormChange(void);
	afx_msg void OnGradientClipChange(void);
	afx_msg void OnGradientClipEditChange(void);
	afx_msg void OnGradientClipEditKillFocus(void);
	DECLARE_MESSAGE_MAP()
	
	// enum -> string
	CString getactivationfn(const afml::activationfn::type t) const;
	CString getactivationnorm(const afml::activationnorm<>::type t) const;
	CString getgradientclip(const afml::gradientclip<>::type t) const;

	// input
	CString getinputimagedim(void) const;
	CString getinputperceptrons(void) const;

	// layer
	void createdefaultlayers(const int nTypes);
	void movelayers(const bool bUp);

	// list ctrl
	void clearlistctrlsel(void);
	void populatelistctrl(void);
	void initfromlistctrl(void);
	void setlistctrllayer(const int n,const afml::layer<> *pL);
	
	// generic
	void validatefocus(const HWND hPreFocus);
	void enabledisable(void);
};
