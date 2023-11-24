// traincostwnd.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "traincostwnd.h"
#include "ai3b1bDlg.h"


// trainpanelwnd

IMPLEMENT_DYNAMIC(traincostwnd, CWnd)

traincostwnd::traincostwnd()
{

}

traincostwnd::~traincostwnd()
{
}


BEGIN_MESSAGE_MAP(traincostwnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// traincostwnd message handlers


void traincostwnd::OnClose()
{
	theApp.closecostui();
}

int traincostwnd::OnCreate(LPCREATESTRUCT p)
{
	const int n = CWnd::OnCreate(p);
	
	m_spDlg=std::shared_ptr<traincostdlg>(new traincostdlg);
	m_spDlg->Create(traincostdlg::IDD,this);

	return n;
}

void traincostwnd::OnSize(UINT nType,int cx,int cy)
{
	CWnd::OnSize(nType,cx,cy);
}

BOOL traincostwnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}
