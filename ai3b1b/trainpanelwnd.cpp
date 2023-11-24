// trainpanelwnd.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "trainpanelwnd.h"
#include "ai3b1bDlg.h"


// trainpanelwnd

IMPLEMENT_DYNAMIC(trainpanelwnd, CWnd)

trainpanelwnd::trainpanelwnd()
{

}

trainpanelwnd::~trainpanelwnd()
{
}


BEGIN_MESSAGE_MAP(trainpanelwnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// trainpanelwnd message handlers


void trainpanelwnd::OnClose()
{
	theApp.closepanelui();
}

int trainpanelwnd::OnCreate(LPCREATESTRUCT p)
{
	const int n = CWnd::OnCreate(p);
	
	m_spDlg=std::shared_ptr<trainpaneldlg>(new trainpaneldlg);
	m_spDlg->Create(trainpaneldlg::IDD,this);

	return n;
}

void trainpanelwnd::OnSize(UINT nType,int cx,int cy)
{
	CWnd::OnSize(nType,cx,cy);

	if(false && m_spDlg && m_spDlg->GetSafeHwnd())
	{
		CRect rcDlg;
		GetClientRect(rcDlg);
		::SetWindowPos(m_spDlg ->GetSafeHwnd(),NULL,rcDlg.left,rcDlg.top,rcDlg.Width(),rcDlg.Height(),SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);
	}
}

BOOL trainpanelwnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}
