#pragma once

#include "traincostdlg.h"
#include <memory>

// traincostwnd

class traincostwnd : public CWnd
{
	DECLARE_DYNAMIC(traincostwnd)

public:
	traincostwnd();
	virtual ~traincostwnd();

	traincostdlg *getdlg(void) { return m_spDlg.get(); }
protected:
	std::shared_ptr<traincostdlg> m_spDlg;

	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT p);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};
