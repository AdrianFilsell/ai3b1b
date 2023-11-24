#pragma once

#include <memory>
#include "layerdlg.h"
#include "traindlg.h"

// tabctrl

class tabctrl : public CTabCtrl
{
	DECLARE_DYNAMIC(tabctrl)

public:
	tabctrl();
	virtual ~tabctrl();

	// accessors	
	layerdlg *getlayerdlg(void)const{return m_spLayerDlg.get();}
	traindlg *gettraindlg(void)const{return m_spTrainDlg.get();}

	// overrides
	virtual void PreSubclassWindow(void);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg BOOL OnSelChange(NMHDR* pNMHDR, LRESULT* pResult);

	std::shared_ptr<layerdlg> m_spLayerDlg;
	std::shared_ptr<traindlg> m_spTrainDlg;

	void showhide(void);
	void updatelayout(void);
	CRect getchildctrlrect(void);
	void commoninit(void);
	void createdlgs(void);
};


