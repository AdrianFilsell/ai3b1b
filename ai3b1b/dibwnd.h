#pragma once

#include "pch.h"
#include "dib.h"
#include <memory>

// dibwnd

class dibwnd : public CWnd
{
	DECLARE_DYNAMIC(dibwnd)

public:
	dibwnd();
	virtual ~dibwnd();

	CSize getwindowdim(void)const{return m_szWindow;}
	std::shared_ptr<const afdib::dib> get(void)const{return m_sp;}

	void clear(void);
	void set(std::shared_ptr<const afdib::dib> sp);
	void setscaletofit(const bool b);
protected:
	CBitmap m_OffscreenBmp;
	CSize m_szWindow;
	std::shared_ptr<const afdib::dib> m_sp;
	bool m_bScaleToFit;
	
	afx_msg void OnPaint(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()
};
