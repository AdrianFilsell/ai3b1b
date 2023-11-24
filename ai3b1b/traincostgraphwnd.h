#pragma once

// traincostgraphwnd

class traincostgraphwnd : public CWnd
{
	DECLARE_DYNAMIC(traincostgraphwnd)

public:
	traincostgraphwnd();
	virtual ~traincostgraphwnd();

	int getlength(void)const{return m_nLength;}
	bool getgloballast(std::pair<double,int>& l)const{if(!m_nLength)return false;l=m_v[(m_nFirst+m_nLength-1)%m_v.size()];return true;}
	std::pair<double,int> getlocalmin(void)const{return m_LocalMin;}
	std::pair<double,int> getlocalmax(void)const{return m_LocalMax;}
	std::pair<double,int> getglobalmin(void)const{return m_GlobalMin;}
	std::pair<double,int> getglobalmax(void)const{return m_GlobalMax;}
	std::pair<double,double> getinflatedlocalrange(void)const{return m_InflatedLocalRange;}

	void set(const std::pair<double,int> minepoch,const std::pair<double,int> maxepoch);
	void push_back(const std::pair<double,int> d);
	void clear(void);
protected:
	CBitmap m_OffscreenBmp;
	CSize m_szWindow;
	std::pair<double,int> m_LocalMin;		// min within m_v
	std::pair<double,int> m_LocalMax;		// max within m_v
	std::pair<double,int> m_GlobalMin;		// all time min
	std::pair<double,int> m_GlobalMax;		// all time max
	double m_dInflatedLocalRangeScale;
	std::pair<double,double> m_InflatedLocalRange;
	std::vector<std::pair<double,int>> m_v;		// circulare buffer
	int m_nFirst;								// circulare buffer - first non empty position
	int m_nLength;								// circulare buffer - non empty positions
	std::vector<POINT> m_DiscreteSamples;
	std::vector<POINT> m_SolidPolygon;
	
	afx_msg void OnPaint(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()

	void shuffleleft(void);
	void updateinflatedlocalrange(void);
	int getdiscretelocal(const double dLocal)const;
};
