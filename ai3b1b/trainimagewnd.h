#pragma once

#include "trainimagedlg.h"
#include <memory>

// trainimagewnd

class trainimagewnd : public CWnd
{
	DECLARE_DYNAMIC(trainimagewnd)

public:
	trainimagewnd();
	virtual ~trainimagewnd();

	trainimagedlg *getdlg(void) { return m_spDlg.get(); }
protected:
	std::shared_ptr<trainimagedlg> m_spDlg;

	afx_msg void OnClose();
	afx_msg int OnCreate(LPCREATESTRUCT p);
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};
