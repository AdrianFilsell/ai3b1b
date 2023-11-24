// trainimagewnd.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "trainimagewnd.h"
#include "ai3b1bDlg.h"


// trainimagewnd

IMPLEMENT_DYNAMIC(trainimagewnd, CWnd)

trainimagewnd::trainimagewnd()
{

}

trainimagewnd::~trainimagewnd()
{
}


BEGIN_MESSAGE_MAP(trainimagewnd, CWnd)
	ON_WM_CLOSE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// trainimagewnd message handlers


void trainimagewnd::OnClose()
{
	theApp.closeimageui();
}

int trainimagewnd::OnCreate(LPCREATESTRUCT p)
{
	const int n = CWnd::OnCreate(p);
	
	m_spDlg=std::shared_ptr<trainimagedlg>(new trainimagedlg);
	m_spDlg->Create(trainimagedlg::IDD,this);

	return n;
}

void trainimagewnd::OnSize(UINT nType,int cx,int cy)
{
	CWnd::OnSize(nType,cx,cy);
}

BOOL trainimagewnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}
