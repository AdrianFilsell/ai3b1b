
#pragma once

#include "winthread.h"
#include "ml_network.h"
#include <afxmt.h>

class serialise;

#define WM_THREAD_PLAYCONFIG_RESULT			( WM_USER + 0x100 )
#define WM_THREAD_QUERY_RESULT				( WM_USER + 0x101 )

class mlthreadrequest
{
public:
	const void *getowner(void) const{return m_pOwner;}
protected:
	mlthreadrequest(const void *pOwner):m_pOwner(pOwner){}
	virtual ~mlthreadrequest(){}
	
	bool merge(std::shared_ptr<const mlthreadrequest> p) { return p && ( m_pOwner==p->getowner() ); }

	const void *m_pOwner;
};

class mlthreadplayconfigrequest : public mlthreadrequest
{
public:
	enum type {t_play=0x1,t_step=0x2,t_pause=0x4};

	type gettype(void) const{return m_Type;}
	
	virtual bool merge(std::shared_ptr<const mlthreadplayconfigrequest> p) { return mlthreadrequest::merge(p) && ( gettype() == p->gettype() ); }
protected:
	mlthreadplayconfigrequest(const void *pOwner,const type t):mlthreadrequest(pOwner),m_Type(t){}
	virtual ~mlthreadplayconfigrequest(){}

	type m_Type;
};

class mlthreadplayrequest : public mlthreadplayconfigrequest { public:mlthreadplayrequest(const void *pOwner):mlthreadplayconfigrequest(pOwner,t_play){} };
class mlthreadsteprequest : public mlthreadplayconfigrequest
{
public:
	mlthreadsteprequest(const void *pOwner):mlthreadplayconfigrequest(pOwner,t_step){}
	virtual bool merge(std::shared_ptr<const mlthreadplayconfigrequest> p) override { return false; }
};
class mlthreadpauserequest : public mlthreadplayconfigrequest
{
public:
	mlthreadpauserequest(const void *pOwner,const bool b):mlthreadplayconfigrequest(pOwner,t_pause),m_b(b){}
	bool getpaused(void)const{return m_b;}
protected:
	bool m_b;
};

class mlthreadqueryrequest : public mlthreadrequest
{
public:
	enum type {t_cost=0x1,t_dib=0x2,t_lerp_dib=0x4};

	type gettype(void) const{return m_Type;}
	
	virtual bool merge(std::shared_ptr<const mlthreadqueryrequest> p) { return mlthreadrequest::merge(p) && ( gettype() == p->gettype() ); }
protected:
	mlthreadqueryrequest(const void *pOwner,const type t):mlthreadrequest(pOwner),m_Type(t){}
	virtual ~mlthreadqueryrequest(){}

	type m_Type;
};

class mlthreadcostrequest : public mlthreadqueryrequest
{
public:
	mlthreadcostrequest(const void *pOwner):mlthreadqueryrequest(pOwner,t_cost){m_c=std::make_pair(0,0);}

	const std::pair<double,int>& getcost(void)const{return m_c;}
	void setcost(const std::pair<double,int>& c){m_c=c;}
protected:
	std::pair<double,int> m_c;
};

class mlthreaddibrequestcache
{
public:
	mlthreaddibrequestcache(){m_rcMIClip.left=0;m_rcMIClip.right=0;m_rcMIClip.top=0;m_rcMIClip.bottom=0;m_dMIID=-1;m_MIType=afml::trainsetitem::it_null;}
	std::shared_ptr<af::mxnxzmatrix<>> getmi(void)const{return m_spMI;}
	void setmi(std::shared_ptr<af::mxnxzmatrix<>> sp){m_spMI=sp;}
	std::shared_ptr<af::mxnxzmatrix<>> getmo(void)const{return m_spMO;}
	void setmo(std::shared_ptr<af::mxnxzmatrix<>> sp){m_spMO=sp;}
	std::shared_ptr<std::vector<af::mxnxzmatrix<>>> getactivation(void)const{return m_spActivation;}
	void setactivation(std::shared_ptr<std::vector<af::mxnxzmatrix<>>> sp){m_spActivation=sp;}
	std::shared_ptr<std::vector<af::mxnxzmatrix<>>> getz(void)const{return m_spZ;}
	void setz(std::shared_ptr<std::vector<af::mxnxzmatrix<>>> sp){m_spZ=sp;}
	mlthreaddibrequestcache& operator=(const mlthreaddibrequestcache& o){m_rcMIClip=o.m_rcMIClip;m_dMIID=o.m_dMIID;m_spMI=o.m_spMI;m_spMO=o.m_spMO;m_spActivation=o.m_spActivation;m_spZ=o.m_spZ;return *this;}
	void setmiid(const double d){m_dMIID=d;}
	double getmiid(void)const{return m_dMIID;}
	void setmiclip(const RECT& rc){m_rcMIClip=rc;}
	void setmitype(const afml::trainsetitem::inputtype it){m_MIType=it;}
	const RECT& getmiclip(void)const{return m_rcMIClip;}
	afml::trainsetitem::inputtype getmitype(void)const{return m_MIType;}
protected:
	std::shared_ptr<af::mxnxzmatrix<>> m_spMI;
	std::shared_ptr<af::mxnxzmatrix<>> m_spMO;
	std::shared_ptr<std::vector<af::mxnxzmatrix<>>> m_spActivation;
	std::shared_ptr<std::vector<af::mxnxzmatrix<>>> m_spZ;
	double m_dMIID;
	RECT m_rcMIClip;
	afml::trainsetitem::inputtype m_MIType;
};

class mlthreaddibrequestbasecustomdim
{
public:
	mlthreaddibrequestbasecustomdim(const SIZE *pSZ=nullptr,const bool bMemLimit=false,const int nMemLimit=0)
	{
		m_b=!!pSZ;
		if(m_b)
			m_sz=*pSZ;
		m_bMemLimit=bMemLimit;
		m_nMemLimit=nMemLimit;
	}
	bool get(void)const{return m_b;}
	const SIZE& getdim(void)const{return m_sz;}
	bool getlimit(void)const{return m_bMemLimit;}
	int getbytelimit(void)const{return m_nMemLimit;}
	mlthreaddibrequestbasecustomdim& operator =(const mlthreaddibrequestbasecustomdim& o){m_b=o.m_b;m_sz=o.m_sz;m_bMemLimit=o.m_bMemLimit;m_nMemLimit=o.m_nMemLimit;return *this;}
protected:
	bool m_b;
	SIZE m_sz;
	bool m_bMemLimit;
	int m_nMemLimit;
};

class mlthreaddibrequestbase : public mlthreadqueryrequest
{
public:
	const mlthreaddibrequestbasecustomdim& getcustomdim(void)const{return m_CustomDim;}
	std::shared_ptr<mlthreaddibrequestcache> getcache(void)const{return m_spCache;}
	std::shared_ptr<const afdib::dib> getdib(void)const{return m_sp;}
	void setdib(std::shared_ptr<const afdib::dib> sp){m_sp=sp;}
protected:
	mlthreaddibrequestbase(const void *pOwner,const type t,std::shared_ptr<mlthreaddibrequestcache> spCache=nullptr,const mlthreaddibrequestbasecustomdim customdim=mlthreaddibrequestbasecustomdim()):mlthreadqueryrequest(pOwner,t){m_spCache=spCache;m_CustomDim=customdim;}
	mlthreaddibrequestbasecustomdim m_CustomDim;
	std::shared_ptr<const afdib::dib> m_sp;
	std::shared_ptr<mlthreaddibrequestcache> m_spCache;
};

class mlthreaddibrequest : public mlthreaddibrequestbase
{
public:
	mlthreaddibrequest(const void *pOwner,const int nID,std::shared_ptr<mlthreaddibrequestcache> spCache=nullptr,const mlthreaddibrequestbasecustomdim customdim=mlthreaddibrequestbasecustomdim()):mlthreaddibrequestbase(pOwner,t_dib,spCache,customdim){m_nID=nID;}

	int getid(void)const{return m_nID;}

	virtual bool merge(std::shared_ptr<const mlthreadqueryrequest> p)override
	{
		if(!mlthreaddibrequestbase::merge(p))return false;
		auto pR = static_cast<const mlthreaddibrequest*>(p.get());
		if(m_nID == pR->m_nID)
		{
			m_CustomDim=pR->m_CustomDim;
			return true;
		}
		return false;
	}
protected:
	int m_nID;
};

class mlthreadlerpdibrequest : public mlthreaddibrequestbase
{
public:
	mlthreadlerpdibrequest(const void *pOwner,const int nFromID,const int nToID,const double dLerp,std::shared_ptr<mlthreaddibrequestcache> spCache=nullptr,const mlthreaddibrequestbasecustomdim customdim=mlthreaddibrequestbasecustomdim()):mlthreaddibrequestbase(pOwner,t_lerp_dib,spCache,customdim){m_nFromID=nFromID;m_nToID=nToID;m_dLerp=dLerp;}

	int getfromid(void)const{return m_nFromID;}
	int gettoid(void)const{return m_nToID;}
	double getlerp(void)const{return m_dLerp;}

	virtual bool merge(std::shared_ptr<const mlthreadqueryrequest> p)override
	{
		if(!mlthreaddibrequestbase::merge(p))return false;
		auto pR = static_cast<const mlthreadlerpdibrequest*>(p.get());
		if(m_nFromID == pR->m_nFromID && m_nToID == pR->m_nToID)
		{
			m_CustomDim=pR->m_CustomDim;
			m_dLerp = pR->m_dLerp; // update to latest query
			return true;
		}
		return false;
	}
protected:
	int m_nFromID;
	int m_nToID;
	double m_dLerp;
};

class mlthreadwnd : public CWnd
{
	DECLARE_DYNAMIC(mlthreadwnd)

public:
	mlthreadwnd();
	virtual ~mlthreadwnd();
protected:
	afx_msg LRESULT OnThreadQueryResult(WPARAM,LPARAM);
	afx_msg LRESULT OnThreadPlayConfigResult(WPARAM,LPARAM);
	afx_msg void OnTimer(UINT);
	DECLARE_MESSAGE_MAP()
};

class mlthreadplayconfigresults
{
public:
	mlthreadplayconfigresults(){m_nTypes=0;}
	bool isempty(void)const{return gettypes()==0;}
	int gettypes(void)const{return m_nTypes;}
	const std::vector<std::shared_ptr<const mlthreadplayconfigrequest>>& getresults(void)const{return m_vResults;}
	void push_back(std::shared_ptr<const mlthreadplayconfigrequest> sp){if(sp){m_vResults.push_back(sp);m_nTypes|=sp->gettype();}}
	void push_back(const std::vector<std::shared_ptr<mlthreadplayconfigrequest>>& v){auto i=v.cbegin(),end=v.cend();for(;i!=end;++i)push_back(*i);}
protected:
	int m_nTypes;
	std::vector<std::shared_ptr<const mlthreadplayconfigrequest>> m_vResults;
};

class mlthreadqueryresults
{
public:
	mlthreadqueryresults(){m_nTypes=0;}
	bool isempty(void)const{return gettypes()==0;}
	int gettypes(void)const{return m_nTypes;}
	const std::vector<std::shared_ptr<const mlthreadqueryrequest>>& getresults(void)const{return m_vResults;}
	void push_back(std::shared_ptr<const mlthreadqueryrequest> sp){if(sp){m_vResults.push_back(sp);m_nTypes|=sp->gettype();}}
	void push_back(const std::vector<std::shared_ptr<mlthreadqueryrequest>>& v){auto i=v.cbegin(),end=v.cend();for(;i!=end;++i)push_back(*i);}
protected:
	int m_nTypes;
	std::vector<std::shared_ptr<const mlthreadqueryrequest>> m_vResults;
};

class mlthread : public winthread
{

DECLARE_DYNAMIC(mlthread)

public:
	mlthread();
	virtual ~mlthread();

	int getpaused(void)const{return m_bPaused;}
	
	void start(std::shared_ptr<afml::inputtypeitem> sp);
	void stop(const DWORD dwTimeOut=INFINITE);

	std::shared_ptr<const afml::network<>> getnetwork(void)const{return m_spN;}
	std::shared_ptr<const afml::networktraining<>> gettrain(void)const{return m_spTrain;}

	void push_back(std::shared_ptr<mlthreadplayconfigrequest> p,const bool bWait=false)
	{
		int nPreResultsSize=0;
		CSingleLock l(&m_CS,TRUE);
		if(bWait)
			nPreResultsSize=m_spPlayConfigResults?static_cast<int>(m_spPlayConfigResults->getresults().size()):0;

		const bool bMerged = (!bWait && merge(p));
		if(!bMerged)
			m_vPendingPlayConfigRequests.push_back(p);
		m_spPlayConfigPending->SetEvent();
		l.Unlock();

		while(bWait)
		{
			const int nWait = 10;
			::Sleep(nWait);
			{
				CSingleLock l(&m_CS,TRUE);
				if(m_spPlayConfigResults && m_spPlayConfigResults->gettypes()&p->gettype())
					for(int n = static_cast<int>(m_spPlayConfigResults->getresults().size()-1);n>=0 && (n+1)>nPreResultsSize;--n)
						if(m_spPlayConfigResults->getresults()[n]==p)
							return;
			}
		}
	}
	void push_back(std::shared_ptr<mlthreadqueryrequest> p,const bool bWait=false)
	{
		int nPreResultsSize=0;
		CSingleLock l(&m_CS,TRUE);
		if(bWait)
			nPreResultsSize=m_spQueryResults?static_cast<int>(m_spQueryResults->getresults().size()):0;

		const bool bMerged = (!bWait && merge(p));
		if(!bMerged)
			m_vPendingQueryRequests.push_back(p);
		m_spQueryPending->SetEvent();
		l.Unlock();

		while(bWait)
		{
			const int nWait = 10;
			::Sleep(nWait);
			{
				CSingleLock l(&m_CS,TRUE);
				if(m_spQueryResults && m_spQueryResults->gettypes()&p->gettype())
					for(int n = static_cast<int>(m_spQueryResults->getresults().size()-1);n>=0 && (n+1)>nPreResultsSize;--n)
						if(m_spQueryResults->getresults()[n]==p)
							return;
			}
		}
	}
	std::shared_ptr<const mlthreadqueryresults> flushqueryresults(void)
	{
		CSingleLock l(&m_CS,TRUE);
		std::shared_ptr<mlthreadqueryresults> sp=m_spQueryResults;
		m_spQueryResults=nullptr;
		return sp;
	}
	std::shared_ptr<const mlthreadplayconfigresults> flushplayconfigresults(void)
	{
		CSingleLock l(&m_CS,TRUE);
		std::shared_ptr<mlthreadplayconfigresults> sp = m_spPlayConfigResults;
		m_spPlayConfigResults=nullptr;
		return sp;
	}

	virtual BOOL mlthread::InitInstance(void)override;
protected:
	bool m_bPaused;
	std::shared_ptr<afml::inputtypeitem> m_sp;
	std::shared_ptr<afml::network<>> m_spN;
	std::shared_ptr<afml::networktraining<>> m_spTrain;
	std::vector<std::shared_ptr<mlthreadplayconfigrequest>> m_vPendingPlayConfigRequests;
	std::vector<std::shared_ptr<mlthreadqueryrequest>> m_vPendingQueryRequests;
	std::shared_ptr<mlthreadplayconfigresults> m_spPlayConfigResults;
	std::shared_ptr<mlthreadqueryresults> m_spQueryResults;
	CCriticalSection m_CS;
	std::shared_ptr<CEvent> m_spPlayConfigPending;
	std::shared_ptr<CEvent> m_spQueryPending;
	std::pair<std::shared_ptr<mlthreadwnd>,HWND> m_NotifyWnd;

	bool merge(std::shared_ptr<mlthreadplayconfigrequest> p)
	{
		auto i = m_vPendingPlayConfigRequests.rbegin(),end=m_vPendingPlayConfigRequests.rend();
		for(;i!=end;++i)
			if((*i)->merge(p))
				return true;
		return false;
	}
	bool merge(std::shared_ptr<mlthreadqueryrequest> p)
	{
		auto i = m_vPendingQueryRequests.rbegin(),end=m_vPendingQueryRequests.rend();
		for(;i!=end;++i)
			if((*i)->merge(p))
				return true;
		return false;
	}

	void processrequests(const std::vector<std::shared_ptr<mlthreadplayconfigrequest>>& v);
	void processrequests(const std::vector<std::shared_ptr<mlthreadqueryrequest>>& v);
	void processdibrequest(const afthread::taskscheduler *pSched,mlthreaddibrequest *pReq,const afml::traintsetimageitem *pImageItem,const int nZOffset)const;
	void processlerpdibrequest(const afthread::taskscheduler *pSched,mlthreadlerpdibrequest *pReq,const afml::traintsetimageitem *pFromImageItem,const afml::traintsetimageitem *pToImageItem)const;
	void getdibscanlinechunk(mlthreaddibrequestbase *pReq,const afml::trainsetitem::inputtype it,const SIZE& szImageItemDim,SIZE& szDib,int& nScanlineChunk)const;
	void getdibscanlinechunks(const int nY,const int nScanlineChunk,int& nChunks,int& nTrailingChunk)const;
};
