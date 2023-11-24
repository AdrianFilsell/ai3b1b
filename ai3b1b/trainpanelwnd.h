#pragma once

#include "trainpaneldlg.h"
#include <memory>

// trainpanelwnd

class trainpanelwnd : public CWnd
{
	DECLARE_DYNAMIC(trainpanelwnd)

public:
	trainpanelwnd();
	virtual ~trainpanelwnd();

	trainpaneldlg *getdlg(void) { return m_spDlg.get(); }
protected:
	std::shared_ptr<trainpaneldlg> m_spDlg;

	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT p);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};
