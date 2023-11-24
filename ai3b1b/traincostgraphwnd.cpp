// traincostgraphwnd.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "traincostgraphwnd.h"


// traincostgraphwnd

IMPLEMENT_DYNAMIC(traincostgraphwnd, CWnd)

traincostgraphwnd::traincostgraphwnd()
{
	m_szWindow=CSize(0,0);
	const std::pair<double,int> c(0,0);
	m_LocalMin=c;
	m_LocalMax=c;
	m_GlobalMin=c;
	m_GlobalMax=c;
	m_dInflatedLocalRangeScale=1.2;
	m_InflatedLocalRange=std::make_pair(0,0);
	m_nFirst=0;
	m_nLength=0;
}

traincostgraphwnd::~traincostgraphwnd()
{
}


BEGIN_MESSAGE_MAP(traincostgraphwnd, CWnd)
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()



// traincostgraphwnd message handlers
void traincostgraphwnd::OnSize(UINT nType, int cx, int cy)
{
	// base class
	CWnd::OnSize(nType, cx, cy);

	m_szWindow=CSize(cx,cy);
	m_v.resize(cx);

	CPaintDC dc(this);

	CDC *pDC = GetDC();
	if(m_OffscreenBmp.GetSafeHandle())
		m_OffscreenBmp.DeleteObject();
	const BOOL b = m_OffscreenBmp.CreateCompatibleBitmap(&dc,m_szWindow.cx,m_szWindow.cy);
	ReleaseDC(pDC);
}

BOOL traincostgraphwnd::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
}

void traincostgraphwnd::OnPaint(void)
{
	CPaintDC dc(this);

	CDC memDC;
	if(memDC.CreateCompatibleDC(&dc))
	{
		CRect rcUpdate;
		dc.GetClipBox(&rcUpdate);

		CBitmap *pOldBmp = memDC.SelectObject(&m_OffscreenBmp);

		memDC.FillSolidRect(&rcUpdate,RGB(255,255,255));

		if(m_szWindow.cy>1 && m_szWindow.cx>0 && m_nLength)
		{
			m_DiscreteSamples.resize(m_nLength);
			m_SolidPolygon.resize(m_nLength+3);

			auto i = m_DiscreteSamples.begin();
			auto j = m_SolidPolygon.begin();
			int n=m_nFirst,nItem=0;
			for(;n<int(m_v.size()) && nItem<m_nLength;++n,++i,++nItem,++j)
			{
				(*i).x=nItem;
				(*i).y=getdiscretelocal(m_v[n].first);
				(*j)=(*i);
			}
			for(n=0;nItem<m_nLength;++n,++i,++nItem,++j)
			{
				ASSERT(n<int(m_v.size()));
				(*i).x=nItem;
				(*i).y=getdiscretelocal(m_v[n].first);
				(*j)=(*i);
			}
			
			const bool bSolid = true;
			if(bSolid)
			{
				const COLORREF c=RGB(170,200,215);

				CBrush b;
				b.CreateSolidBrush(c);
				CBrush *pOldBrush = memDC.SelectObject(&b);

				CPen p(PS_SOLID,1,c);
				CPen *pOldPen=memDC.SelectObject(&p);

				m_SolidPolygon[m_nLength+0]=CPoint(m_SolidPolygon[m_nLength-1].x,m_szWindow.cy-1);
				m_SolidPolygon[m_nLength+1]=CPoint(0,m_szWindow.cy-1);
				m_SolidPolygon[m_nLength+2]=m_SolidPolygon[0];

				memDC.Polygon(&(m_SolidPolygon[0]),int(m_SolidPolygon.size()));

				memDC.SelectObject(pOldPen);
				memDC.SelectObject(pOldBrush);
			}
			
			const int nWidth=1;
			CPen p(PS_SOLID,nWidth,RGB(130,175,195));
			CPen *pOldPen=memDC.SelectObject(&p);
			memDC.Polyline(&m_DiscreteSamples[0],int(m_DiscreteSamples.size())); // polyline NOT inclusive
			POINT pts[2]={m_DiscreteSamples[m_DiscreteSamples.size()-1],m_DiscreteSamples[m_DiscreteSamples.size()-1]};
			pts[1].x++;
			memDC.Polyline(pts,2);
			memDC.SelectObject(pOldPen);
		
			{
				const int nMinY = getdiscretelocal(m_GlobalMin.first);

				const int nWidth=1;
				CPen p(PS_SOLID,nWidth,RGB(255,0,0));
				CPen *pOldPen=memDC.SelectObject(&p);
				POINT l[2]={CPoint(0,nMinY),CPoint(m_szWindow.cx,nMinY)}; // polyline NOT inclusive so use cx NOT cx - 1
				memDC.Polyline(l,2);
				memDC.SelectObject(pOldPen);
			}
		}

		BOOL b = dc.BitBlt(rcUpdate.left,rcUpdate.top,rcUpdate.Width(),rcUpdate.Height(),&memDC,rcUpdate.left,rcUpdate.top,SRCCOPY);

		memDC.SelectObject(pOldBmp);
	}
}

void traincostgraphwnd::clear(void)
{
	const std::pair<double,int> c(0,0);
	m_LocalMin=c;
	m_LocalMax=c;
	m_GlobalMin=c;
	m_GlobalMax=c;
	m_dInflatedLocalRangeScale=1.2;
	m_InflatedLocalRange=std::make_pair(0,0);
	m_nFirst=0;
	m_nLength=0;
	
	if(GetSafeHwnd())
		Invalidate();
}

void traincostgraphwnd::set(const std::pair<double,int> minepoch,const std::pair<double,int> maxepoch)
{
	ASSERT(m_nLength==0);
	
	m_GlobalMin=minepoch;
	m_GlobalMax=maxepoch;
}

void traincostgraphwnd::push_back(const std::pair<double,int> c)
{
	if(m_nLength==m_szWindow.cx)
		shuffleleft();

	const int nFirstFreePos = (m_nFirst+m_nLength++)%int(m_v.size());
	m_v[nFirstFreePos]=c;
	
	bool bLocalMutated = false;
	if(c.first<m_LocalMin.first || m_LocalMin.second==0)
	{
		m_LocalMin=c;
		bLocalMutated = true;
	}
	if(c.first<m_GlobalMin.first)
		m_GlobalMin=c;
	if(c.first>m_LocalMax.first || m_LocalMax.second==0)
	{
		m_LocalMax=c;
		bLocalMutated = true;
	}
	if(c.first>m_GlobalMax.first)
		m_GlobalMax=c;
	if(bLocalMutated)
		updateinflatedlocalrange();
	Invalidate();
}

void traincostgraphwnd::shuffleleft(void)
{
	const bool bValidate = (m_nLength == 1) || !(m_v[m_nFirst].first>m_LocalMin.first && m_v[m_nFirst].first<m_LocalMax.first);
	m_nFirst = (m_nFirst+1)%int(m_v.size());
	--m_nLength;
	ASSERT(m_nLength>=0);
	if(!bValidate)
		return;

	const std::pair<double,int> c(0,0);
	m_LocalMin=c;
	m_LocalMax=c;
	if(m_nLength)
	{
		m_LocalMin=m_v[m_nFirst];
		m_LocalMax=m_v[m_nFirst];

		int n=m_nFirst+1,nItem=1;
		for(;n<int(m_v.size()) && nItem<m_nLength;++n,++nItem)
		{
			if(m_v[n].first<m_LocalMin.first)
				m_LocalMin=m_v[n];
			else
			if(m_v[n].first>m_LocalMax.first)
				m_LocalMax=m_v[n];
		}
		for(n=0;nItem<m_nLength;++n,++nItem)
		{
			ASSERT(n<int(m_v.size()));
			if(m_v[n].first<m_LocalMin.first)
				m_LocalMin=m_v[n];
			else
			if(m_v[n].first>m_LocalMax.first)
				m_LocalMax=m_v[n];
		}
	}
	updateinflatedlocalrange();
}

void traincostgraphwnd::updateinflatedlocalrange(void)
{
	const double dRange = m_LocalMax.first-m_LocalMin.first;
	const double dInflatedRange = dRange*m_dInflatedLocalRangeScale;

	const double dMax = m_LocalMax.first + ( 0.5 * ( dInflatedRange - dRange ) );
	const double dMin = dMax - dInflatedRange;

	m_InflatedLocalRange=std::make_pair(dMin,dMax);
}

int traincostgraphwnd::getdiscretelocal(const double d)const
{
	// m_InflatedLocalRange.first maps to m_szWindow.cy-1
	// m_InflatedLocalRange.second maps to 0
	if(d<m_InflatedLocalRange.first)
		return m_szWindow.cy-1;
	if(d>m_InflatedLocalRange.second)
		return 0;
	
	const double dInflatedLocalRange = m_InflatedLocalRange.second-m_InflatedLocalRange.first;
	if(dInflatedLocalRange<=0)
		return m_szWindow.cy-1;
	const double dNorm = 1 - ( (d-m_InflatedLocalRange.first)/dInflatedLocalRange );
	ASSERT(dNorm>=0 && dNorm<=1.0);
	const int n = static_cast<int>( 0.5 + ( dNorm * (m_szWindow.cy - 1) ) );
	return n;
}
