// tabctrl.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "tabctrl.h"


// tabctrl

IMPLEMENT_DYNAMIC(tabctrl, CTabCtrl)

tabctrl::tabctrl()
{
}

tabctrl::~tabctrl()
{
}


BEGIN_MESSAGE_MAP(tabctrl, CTabCtrl)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT_EX(TCN_SELCHANGE,tabctrl::OnSelChange)
END_MESSAGE_MAP()



// tabctrl message handlers


void tabctrl::OnSize(UINT nType, int cx, int cy)
{
	// base class
	CTabCtrl::OnSize(nType, cx, cy);
	
	// layout
	updatelayout();
}

int tabctrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// base class
	const int nRetVal = CTabCtrl::OnCreate(lpCreateStruct);

	// init
	commoninit();

	return nRetVal;
}

BOOL tabctrl::OnEraseBkgnd(CDC *pDC)
{
	// child dialogs
	if(m_spLayerDlg && m_spLayerDlg->GetSafeHwnd())
	{
		CRect rcCtrl;
		m_spLayerDlg->GetWindowRect(rcCtrl);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcCtrl,2);
		pDC->ExcludeClipRect(rcCtrl);
	}
	if(m_spTrainDlg && m_spTrainDlg->GetSafeHwnd())
	{
		CRect rcCtrl;
		m_spTrainDlg ->GetWindowRect(rcCtrl);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcCtrl,2);
		pDC->ExcludeClipRect(rcCtrl);
	}
	
	// base class
	return CTabCtrl::OnEraseBkgnd( pDC );
}

BOOL tabctrl::OnSelChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	// show hide
	if(pResult)
		*pResult = 0;
	showhide();

	return FALSE;
}

void tabctrl::PreSubclassWindow(void)
{
	// Call the base class
	CTabCtrl::PreSubclassWindow();

	// init
	commoninit();
}

void tabctrl::commoninit(void)
{
	// needs to be a control parent because of child sub dialogs
	SetWindowLong(m_hWnd, GWL_EXSTYLE, GetWindowLong(m_hWnd, GWL_EXSTYLE)|WS_EX_CONTROLPARENT);

	// add tabs
	InsertItem(1,CString("Layers"));
	InsertItem(2,CString("Training"));
	
	// create the dlgs
	createdlgs();
	
	// layout
	updatelayout();
	
	// selected tab
	const int nTab = 0;
	SetCurSel(nTab);
	OnSelChange(NULL,NULL);
}

void tabctrl::createdlgs()
{
	m_spLayerDlg = std::shared_ptr<layerdlg>(new layerdlg(this));
	m_spTrainDlg = std::shared_ptr<traindlg>(new traindlg(this));
	m_spLayerDlg->Create(layerdlg::IDD,this);
	m_spTrainDlg->Create(traindlg::IDD,this);
}

void tabctrl::updatelayout(void)
{
	// child dialogs
	CRect rcListCtrl = getchildctrlrect();
	if(m_spLayerDlg && m_spLayerDlg->GetSafeHwnd())
		::SetWindowPos(m_spLayerDlg->GetSafeHwnd(),NULL,rcListCtrl.left,rcListCtrl.top,rcListCtrl.Width(),rcListCtrl.Height(),SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);
	if(m_spTrainDlg && m_spTrainDlg->GetSafeHwnd())
		::SetWindowPos(m_spTrainDlg->GetSafeHwnd(),NULL,rcListCtrl.left,rcListCtrl.top,rcListCtrl.Width(),rcListCtrl.Height(),SWP_DRAWFRAME|SWP_NOACTIVATE|SWP_NOCOPYBITS|SWP_NOZORDER);
}

CRect tabctrl::getchildctrlrect(void)
{
	CRect rcRetVal;
	rcRetVal.SetRectEmpty();
	if(m_hWnd)
	{
		GetWindowRect(rcRetVal);
		AdjustRect(FALSE,&rcRetVal);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcRetVal,2);
	}
	return rcRetVal;
}

void tabctrl::showhide(void)
{
	// current selection
	const int nSel = GetCurSel();
	switch( nSel )
	{
		case 0:
		{
			if(m_spLayerDlg && m_spLayerDlg->GetSafeHwnd())
			{
				m_spLayerDlg->setactive(true);
				m_spLayerDlg->ShowWindow(SW_SHOWNA);
			}
			if(m_spTrainDlg && m_spTrainDlg->GetSafeHwnd())
			{
				m_spTrainDlg->ShowWindow(SW_HIDE);
				m_spTrainDlg->setactive(false);
			}
		}
		break;
		case 1:
		{
			if(m_spLayerDlg && m_spLayerDlg->GetSafeHwnd())
			{
				m_spLayerDlg->ShowWindow(SW_HIDE);
				m_spLayerDlg->setactive(false);
			}
			if(m_spTrainDlg && m_spTrainDlg->GetSafeHwnd())
			{
				m_spTrainDlg->setactive(true);
				m_spTrainDlg->ShowWindow(SW_SHOWNA);
			}
		}
		break;
		default:ASSERT( FALSE );
	}
}
