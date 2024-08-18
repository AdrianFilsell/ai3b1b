// traincostgraphwnd.cpp : implementation file
//

#include "pch.h"
#include "dibwnd.h"
#include "ml_network.h"

// dibwnd

IMPLEMENT_DYNAMIC(dibwnd, CWnd)

dibwnd::dibwnd()
{
	m_bScaleToFit=true;
	m_szWindow=CSize(0,0);
}

dibwnd::~dibwnd()
{
}


BEGIN_MESSAGE_MAP(dibwnd, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// dibwnd message handlers
void dibwnd::OnSize(UINT nType, int cx, int cy)
{
	// base class
	CWnd::OnSize(nType, cx, cy);

	m_szWindow=CSize(cx,cy);

	CDC *pDC = GetDC();
	if(m_OffscreenBmp.GetSafeHandle())
		m_OffscreenBmp.DeleteObject();
	const BOOL b = m_OffscreenBmp.CreateCompatibleBitmap(pDC,m_szWindow.cx,m_szWindow.cy);
	ReleaseDC(pDC);
}

BOOL dibwnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

void dibwnd::OnPaint(void)
{
	CPaintDC dc(this);

	CDC memDC;
	if(memDC.CreateCompatibleDC(&dc))
	{
		CRect rcUpdate;
		dc.GetClipBox(&rcUpdate);

		CBitmap *pOldBmp = memDC.SelectObject(&m_OffscreenBmp);

		memDC.FillSolidRect(&rcUpdate,RGB(255,255,255));

		if(m_sp)
		{
			double dS=1;
			const bool bLetterBox = true;
			const double dSrcTLX=0,dSrcTLY=0,dSrcBRX=m_sp->getwidth(),dSrcBRY=m_sp->getheight();
			const double dDstTLX=0,dDstTLY=0,dDstBRX=m_szWindow.cx,dDstBRY=m_szWindow.cy;
			if(m_bScaleToFit)
				afml::traintsetimageitem::getrectscale(dSrcTLX,dSrcTLY,dSrcBRX,dSrcBRY,dDstTLX,dDstTLY,dDstBRX,dDstBRY,bLetterBox,dS);

			CRect rcDst(0,0,af::posround<double,int>(dSrcBRX*dS),af::posround<double,int>(dSrcBRY*dS));
			rcDst.OffsetRect(CPoint(m_szWindow.cx/2,m_szWindow.cy/2)-rcDst.CenterPoint());

			const CRect rcSrc(0,0,m_sp->getwidth(),m_sp->getheight());

			BITMAPINFO *pBmi = m_sp->createbitmapinfo();
			const int n = ::StretchDIBits(memDC.GetSafeHdc(),
										  rcDst.TopLeft().x,rcDst.TopLeft().y,rcDst.Width(),rcDst.Height(),
										  rcSrc.TopLeft().x,rcSrc.TopLeft().y,rcSrc.Width(),rcSrc.Height(),
										  m_sp->getscanline(0),pBmi,DIB_RGB_COLORS,SRCCOPY);
			m_sp->tidybmi(pBmi);
		}
		
		const BOOL b = dc.BitBlt(rcUpdate.left,rcUpdate.top,rcUpdate.Width(),rcUpdate.Height(),&memDC,rcUpdate.left,rcUpdate.top,SRCCOPY);

		memDC.SelectObject(pOldBmp);
	}
}

void dibwnd::clear(void)
{
	m_sp=nullptr;
		
	if(GetSafeHwnd())
		Invalidate();
}

void dibwnd::set(std::shared_ptr<const afdib::dib> sp)
{
	m_sp=sp;
	Invalidate();
}

void dibwnd::setscaletofit(const bool b)
{
	m_bScaleToFit=b;
	Invalidate();
}
