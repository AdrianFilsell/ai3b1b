
#include "pch.h"
#include "winthread_ml.h"
#include "ai3b1b.h"
#include "serialise.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(mlthread, winthread)

mlthread::mlthread()
{
	m_bPaused=false;
	m_spPlayConfigPending = std::shared_ptr<CEvent>(new CEvent(FALSE,TRUE));
	m_spQueryPending = std::shared_ptr<CEvent>(new CEvent(FALSE,TRUE));
}

mlthread::~mlthread()
{
	stop(INFINITE);
}

void mlthread::stop(const DWORD dwTimeOut)
{
	winthread::stop(dwTimeOut);
	
	if(m_NotifyWnd.first)
	{
		if(m_NotifyWnd.second)
			m_NotifyWnd.first->DestroyWindow();
		m_NotifyWnd.first=nullptr;
		m_NotifyWnd.second=NULL;
	}

	m_spPlayConfigPending->ResetEvent();
	m_spQueryPending->ResetEvent();

	m_sp=nullptr;
	m_bPaused=false;
	m_vPendingQueryRequests.clear();
	m_vPendingPlayConfigRequests.clear();
	m_spPlayConfigResults=nullptr;
	m_spQueryResults=nullptr;;
	m_spN=nullptr;
	m_spTrain=nullptr;
}

void mlthread::start(std::shared_ptr<afml::inputtypeitem> sp)
{
	// create the network
	ASSERT(!isrunning());
	
	m_NotifyWnd.first = std::shared_ptr<mlthreadwnd>(new mlthreadwnd());
	m_NotifyWnd.first->CreateEx(0,AfxRegisterWndClass(0),_T(""),WS_POPUP,CRect(0,0,0,0),nullptr,0);
	m_NotifyWnd.second = m_NotifyWnd.first->GetSafeHwnd();
	m_sp=sp;
	m_spN=sp->createnetwork();
	m_spTrain=sp->createtraining(m_spN);

	CreateThread();
	SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
}

BOOL mlthread::InitInstance(void)
{
	const bool bParallel = true;
	const afthread::taskscheduler *pSched = bParallel ? theApp.getscheduler().get() : nullptr;

	const HANDLE arrEvents[3]={*m_pStop,*m_spPlayConfigPending.get(),*m_spQueryPending.get()};
	
	while(true)
	{
		// wait for work
		DWORD dwWait = WaitForMultipleObjects(3,arrEvents,FALSE,INFINITE);
		if(dwWait==WAIT_OBJECT_0+0)
			break;
		
		// requests
		std::vector<std::shared_ptr<mlthreadplayconfigrequest>> vPendingPlayConfigRequests;
		std::vector<std::shared_ptr<mlthreadqueryrequest>> vPendingQueryRequests;
		dwWait = WaitForSingleObject(*m_spPlayConfigPending.get(),0);
		if(dwWait==WAIT_OBJECT_0+0)
		{
			CSingleLock l(&m_CS,TRUE);
			vPendingPlayConfigRequests=m_vPendingPlayConfigRequests;
			m_vPendingPlayConfigRequests.clear();
			m_spPlayConfigPending->ResetEvent();
		}
		dwWait = WaitForSingleObject(*m_spQueryPending.get(),0);
		if(dwWait==WAIT_OBJECT_0+0)
		{
			CSingleLock l(&m_CS,TRUE);
			vPendingQueryRequests=m_vPendingQueryRequests;
			m_vPendingQueryRequests.clear();
			m_spQueryPending->ResetEvent();
		}
		processrequests(vPendingQueryRequests);
		processrequests(vPendingPlayConfigRequests);
	}
	
	return FALSE;
}

void mlthread::processrequests(const std::vector<std::shared_ptr<mlthreadplayconfigrequest>>& v)
{
	if(v.size()==0)
		return;

	const bool bParallel = true;
	const afthread::taskscheduler *pSched = bParallel ? theApp.getscheduler().get() : nullptr;

	auto i = v.cbegin(),end=v.cend();
	for(;i!=end;++i)
	{
		switch((*i)->gettype())
		{
			case mlthreadplayconfigrequest::t_play:
			case mlthreadplayconfigrequest::t_step:
			{
				const bool bStep = (*i)->gettype() == mlthreadplayconfigrequest::t_step;

				if(m_bPaused && !bStep)
					break;
				
				m_spTrain->setepochs(1);
				m_spTrain->setinfiniteepochs(false);

				HANDLE arrEvents[3]={*m_pStop,*m_spPlayConfigPending.get(),*m_spQueryPending.get()};

				while(true)
				{					
					// Epoch
					const auto nEpochs = m_sp->getepochs();
					const bool bPendingEpoch = (nEpochs<0 || ( m_sp->getepoch() < nEpochs ));
					if( !bPendingEpoch )
					{
						m_bPaused=true;	// go into paused state
						break;
					}
					const double dLR = m_sp->getlearningrate();
					m_spTrain->setlearningrate(dLR);
					m_spTrain->train(pSched);
					m_sp->setepoch(m_sp->getepoch()+1);
					if(bStep)
						break;

					// should we stop
					DWORD dwWait = WaitForMultipleObjects(3,arrEvents,FALSE,0);
					if(dwWait==WAIT_OBJECT_0+0)
						break;
					dwWait = WaitForSingleObject(*m_spPlayConfigPending.get(),0);
					if(dwWait==WAIT_OBJECT_0+0)
						break;
					std::vector<std::shared_ptr<mlthreadqueryrequest>> vPendingQueryRequests;
					dwWait = WaitForSingleObject(*m_spQueryPending.get(),0);
					if(dwWait==WAIT_OBJECT_0+0)
					{
						CSingleLock l(&m_CS,TRUE);
						vPendingQueryRequests=m_vPendingQueryRequests;
						m_vPendingQueryRequests.clear();
						m_spQueryPending->ResetEvent();
					}
					processrequests(vPendingQueryRequests);
					// continue playing...
				}
			}
			break;
			case mlthreadplayconfigrequest::t_pause:m_bPaused=static_cast<mlthreadpauserequest*>((*i).get())->getpaused();break;
			default:ASSERT(false);
		}
		CSingleLock l(&m_CS,TRUE);
		if(!m_spPlayConfigResults)
			m_spPlayConfigResults=std::shared_ptr<mlthreadplayconfigresults>(new mlthreadplayconfigresults);
		m_spPlayConfigResults->push_back(*i);
		::PostMessage(m_NotifyWnd.second,WM_THREAD_PLAYCONFIG_RESULT,0,0);
	}
}

void mlthread::processrequests(const std::vector<std::shared_ptr<mlthreadqueryrequest>>& v)
{
	if(v.size()==0)
		return;

	const bool bParallel = true;
	const afthread::taskscheduler *pSched = bParallel ? theApp.getscheduler().get() : nullptr;

	auto i = v.cbegin(),end=v.cend();
	for(;i!=end;++i)
	{
		switch((*i)->gettype())
		{
			case mlthreadqueryrequest::t_cost:
			{
				mlthreadcostrequest *pReq = static_cast<mlthreadcostrequest*>((*i).get());
				ASSERT(m_sp->getgraddescenttype()==afml::networktraining<>::gdt_batch);
				m_spTrain->getactivation(pSched,af::vectorrange<true,true>(0,(*m_spTrain->getactivation())[m_spTrain->getactivation()->size()-1].getz()-1));
				pReq->setcost(std::make_pair(m_spTrain->getcost(),m_sp->getepoch()));
			}
			break;
			case mlthreadqueryrequest::t_dib:
			{				
				mlthreaddibrequest *pReq = static_cast<mlthreaddibrequest*>((*i).get());
				int nZOffset = 0;
				auto i = m_sp->gettraining().cbegin(),end=m_sp->gettraining().cend();
				for(;i!=end;++i)
					if((*i)->gettype() & afml::trainsetitem::it_image)
					{
						afml::traintsetimageitem *pImageItem=static_cast<afml::traintsetimageitem*>((*i).get());
						if(pImageItem->getid() == pReq->getid())
						{
							processdibrequest(pSched,pReq,pImageItem,nZOffset);
							break;
						}
						switch(pImageItem->gettype())
						{
							case afml::trainsetitem::it_image_i_id_o_b8g8r8:++nZOffset;break;
							case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:nZOffset += pImageItem->getdibdim().cx * pImageItem->getdibdim().cy;break;
							default:ASSERT(false);
						}
					}
			}
			break;
			case mlthreadqueryrequest::t_lerp_dib:
			{
				mlthreadlerpdibrequest *pReq = static_cast<mlthreadlerpdibrequest*>((*i).get());

				afml::traintsetimageitem *pFromImageItem=nullptr;
				afml::traintsetimageitem *pToImageItem=nullptr;
				auto i = m_sp->gettraining().cbegin(),end=m_sp->gettraining().cend();
				for(;i!=end;++i)
					if((*i)->gettype() & afml::trainsetitem::it_image)
					{
						afml::traintsetimageitem* pIItem=static_cast<afml::traintsetimageitem*>((*i).get());
						if(pIItem->getid() == pReq->getfromid())
						{
							pFromImageItem=pIItem;
							if(pToImageItem)
								break;
						}
						if(pIItem->getid() == pReq->gettoid())
						{
							pToImageItem=pIItem;
							if(pFromImageItem)
								break;
						}
					}

				if(pFromImageItem && pToImageItem)
					processlerpdibrequest(pSched,pReq,pFromImageItem,pToImageItem);
			}
			break;
			default:ASSERT(false);
		}
		CSingleLock l(&m_CS,TRUE);
		if(!m_spQueryResults)
			m_spQueryResults=std::shared_ptr<mlthreadqueryresults>(new mlthreadqueryresults);
		m_spQueryResults->push_back(*i);
		::PostMessage(m_NotifyWnd.second,WM_THREAD_QUERY_RESULT,0,0);
	}
}

void mlthread::getdibscanlinechunk(mlthreaddibrequestbase *pReq,const afml::trainsetitem::inputtype it,const SIZE& szImageItemDim,SIZE& szDib,int& nScanlineChunk)const
{
	// dim
	bool bCustomDim = pReq->getcustomdim().get();
	switch(it)
	{
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:bCustomDim=false;break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:break;
		default:ASSERT(false);
	}
	if(bCustomDim)
		szDib = pReq->getcustomdim().getdim();
	else
		szDib = szImageItemDim;

	// scanline chunk
	if(bCustomDim && pReq->getcustomdim().getlimit())
	{
		// bytes for 1 scanline
		const int nScanlines = 1;

		// input matrix
		const int nInput = (/* rows */3 * /* cols */1 * /* Z */szDib.cx*nScanlines);
		
		// output matrix
		const int nBytesPerPixel = 3;
		const int nOutput = (/* rows */nBytesPerPixel * /* cols */1 * /* Z */szDib.cx*nScanlines);

		// layers
		int nPerceptrons = 0;
		std::shared_ptr<const afml::network<>> spN = m_spTrain->getnetwork();
		auto iSrc = spN->getlayers()->cbegin(), end = spN->getlayers()->cend();
		for(;iSrc!=end;++iSrc)
			nPerceptrons += (*iSrc)->getperceptrons();
		const int nLayer = (2*nPerceptrons) * 1 * (szDib.cx*nScanlines);

		// total
		const int nTotal = nInput + nOutput + nLayer;

		// scanlines per mem chunk
		nScanlineChunk = std::max<int>(1,af::posround<double,int>(pReq->getcustomdim().getbytelimit() / ( szDib.cx * ( 6 + ( 2.0 * nPerceptrons ) ) )));
	}
	else
		nScanlineChunk=szDib.cy;
}

void mlthread::getdibscanlinechunks(const int nY,const int nScanlineChunk,int& nChunks,int& nTrailingChunk)const
{
	nChunks=nScanlineChunk > 0 ? nY/nScanlineChunk : 0;
	nTrailingChunk = nY-(nScanlineChunk*nChunks);
}

void mlthread::processdibrequest(const afthread::taskscheduler *pSched,mlthreaddibrequest *pReq,const afml::traintsetimageitem *pImageItem,const int nZOffset)const
{
	SIZE szDib;
	int nScanlineChunk;
	getdibscanlinechunk(pReq,pImageItem->gettype(),pImageItem->getdibdim(),szDib,nScanlineChunk);

	int nChunks, nTrailingChunk;
	getdibscanlinechunks(szDib.cy,nScanlineChunk,nChunks,nTrailingChunk);
	int nTotalChunks=nChunks+(nTrailingChunk?1:0);

	bool bCustom = true;
	switch(pImageItem->gettype())
	{
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			bCustom = false;
			m_spTrain->getactivation(pSched,af::vectorrange<true,true>(nZOffset,nZOffset));
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			bCustom = !(nTotalChunks==1 && CSize(szDib)==pImageItem->getdibdim());
			if(!bCustom)
				m_spTrain->getactivation(pSched,af::vectorrange<true,true>(nZOffset,nZOffset+(pImageItem->getdibdim().cx * pImageItem->getdibdim().cy)-1));
		}
		break;
		default:ASSERT(false);return;
	}
	if(!bCustom)
	{
		pReq->setdib(pImageItem->getdib((*m_spTrain->getactivation())[m_spTrain->getactivation()->size()-1],nZOffset,pImageItem->gettype(),pImageItem->getdibdim(),CRect(CPoint(0,0),pImageItem->getdibdim())));
		return;
	}

	std::shared_ptr<mlthreaddibrequestcache> spCache = pReq->getcache();
	
	const int nBytesPerPixel = 3;
	std::shared_ptr<afdib::dib> spDib; 
	std::shared_ptr<af::mxnxzmatrix<>> spMI=spCache?spCache->getmi():nullptr,spMO=spCache?spCache->getmo():nullptr;
	std::shared_ptr<std::vector<af::mxnxzmatrix<>>> spActivation=spCache?spCache->getactivation():nullptr,spZ=spCache?spCache->getz():nullptr;
	if(!spMI || spCache->getmitype()!=pImageItem->gettype())
		spMI=std::shared_ptr<af::mxnxzmatrix<>>(new af::mxnxzmatrix<>);
	if(!spMO)
		spMO=std::shared_ptr<af::mxnxzmatrix<>>(new af::mxnxzmatrix<>);
	for(int nChunk=0;nChunk<nTotalChunks;++nChunk)
	{
		const bool bLast = nChunk==(nTotalChunks-1);
		const int nScanlines = (bLast && nTrailingChunk)?nTrailingChunk:nScanlineChunk;

		const int nFromY = nChunk * nScanlineChunk;
		const CRect rcClip(0,nFromY,szDib.cx,nScanlines+nFromY);

		spMI->setrowscols(3,1,szDib.cx * nScanlines);
		spMO->setrowscols(nBytesPerPixel,1,szDib.cx * nScanlines);

		if(!spCache || spCache->getmiid() != pReq->getid() || CRect(spCache->getmiclip()) != rcClip || spMI!=spCache->getmi())
			for(int nY=0,nZ=0;nY<nScanlines;++nY)
			{
				const int nScanline = nFromY + nY;
				const double dNormY = szDib.cy > 1 ? nScanline/double(szDib.cy-1):1;
				for(int nX=0;nX<szDib.cx;++nX,++nZ)
				{
					const double dNormX = szDib.cx > 1 ? nX/double(szDib.cx-1):1;		// left edge of pixel needs to be pixel centres
					spMI->setrowcol(0,0,pReq->getid(),nZ);
					spMI->setrowcol(1,0,dNormX,nZ);
					spMI->setrowcol(2,0,dNormY,nZ);
				}
			}
		
		std::shared_ptr<const afml::network<>> spN = m_spTrain->getnetwork();
		m_spTrain->allocactivation(spN,*spMI,*spMO,spActivation,spZ,spMI->getz());
		m_spTrain->getactivation(pSched,spN.get(),*spMI,af::vectorrange<true,true>(0,spMI->getz()-1),spActivation.get(),spZ.get());
		
		int nZOffset = 0;
		spDib=pImageItem->getdib((*spActivation)[spActivation->size()-1],nZOffset,pImageItem->gettype(),szDib,rcClip,spDib);
		
		if(spCache)
			spCache->setmiclip(rcClip);
	}
	if(spCache)
	{
		spCache->setmi(spMI);
		spCache->setmo(spMO);
		spCache->setactivation(spActivation);
		spCache->setz(spZ);
		spCache->setmiid(pReq->getid());
		spCache->setmitype(pImageItem->gettype());
	}
	pReq->setdib(spDib);
}

void mlthread::processlerpdibrequest(const afthread::taskscheduler *pSched,mlthreadlerpdibrequest *pReq,const afml::traintsetimageitem *pFromImageItem,const afml::traintsetimageitem *pToImageItem)const
{
	ASSERT(pFromImageItem->gettype()==pToImageItem->gettype());
	const double dLerpID = af::getlerp<int,double>(pReq->getfromid(),pReq->gettoid(),pReq->getlerp());

	std::shared_ptr<mlthreaddibrequestcache> spCache = pReq->getcache();

	const int nBytesPerPixel = 3;
	std::shared_ptr<afdib::dib> spDib;
	std::shared_ptr<af::mxnxzmatrix<>> spMI=spCache?spCache->getmi():nullptr,spMO=spCache?spCache->getmo():nullptr;
	std::shared_ptr<std::vector<af::mxnxzmatrix<>>> spActivation=spCache?spCache->getactivation():nullptr,spZ=spCache?spCache->getz():nullptr;
	if(!spMI || spCache->getmitype()!=pFromImageItem->gettype())
		spMI=std::shared_ptr<af::mxnxzmatrix<>>(new af::mxnxzmatrix<>);
	if(!spMO)
		spMO=std::shared_ptr<af::mxnxzmatrix<>>(new af::mxnxzmatrix<>);
	switch(pFromImageItem->gettype())
	{
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			const int nBytesPerPixel = 3;
			spMI->setrowscols(1,1);
			spMI->setrowcol(0,0,dLerpID);
			spMO->setrowscols(pFromImageItem->getdibdim().cx * pFromImageItem->getdibdim().cy * nBytesPerPixel,1);

			std::shared_ptr<const afml::network<>> spN = m_spTrain->getnetwork();
			m_spTrain->allocactivation(spN,*spMI,*spMO,spActivation,spZ,spMI->getz());
			m_spTrain->getactivation(pSched,spN.get(),*spMI,af::vectorrange<true,true>(0,spMI->getz()-1),spActivation.get(),spZ.get());

			int nZOffset = 0;
			spDib=pFromImageItem->getdib((*spActivation)[spActivation->size()-1],nZOffset,pFromImageItem->gettype(),pFromImageItem->getdibdim(),CRect(CPoint(0,0),pFromImageItem->getdibdim()));
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			SIZE szLerp;
			const double dLerpWidth = af::getlerp<int,double>(pFromImageItem->getdibdim().cx,pToImageItem->getdibdim().cx,pReq->getlerp());
			const double dLerpHeight = af::getlerp<int,double>(pFromImageItem->getdibdim().cy,pToImageItem->getdibdim().cy,pReq->getlerp());
			szLerp.cx = af::posround<double,int>(dLerpWidth);
			szLerp.cy = af::posround<double,int>(dLerpHeight);

			SIZE szDib;
			int nScanlineChunk;
			getdibscanlinechunk(pReq,pFromImageItem->gettype(),szLerp,szDib,nScanlineChunk);

			int nChunks, nTrailingChunk;
			getdibscanlinechunks(szDib.cy,nScanlineChunk,nChunks,nTrailingChunk);
			int nTotalChunks=nChunks+(nTrailingChunk?1:0);

			for(int nChunk=0;nChunk<nTotalChunks;++nChunk)
			{
				const bool bLast = nChunk==(nTotalChunks-1);
				const int nScanlines = (bLast && nTrailingChunk)?nTrailingChunk:nScanlineChunk;

				const int nFromY = nChunk * nScanlineChunk;
				const CRect rcClip(0,nFromY,szDib.cx,nScanlines+nFromY);

				spMI->setrowscols(3,1,szDib.cx * nScanlines);
				spMO->setrowscols(nBytesPerPixel,1,szDib.cx * nScanlines);

				if(!spCache || spCache->getmiid() != dLerpID || CRect(spCache->getmiclip()) != rcClip || spMI!=spCache->getmi())
					for(int nY=0,nZ=0;nY<nScanlines;++nY)
					{
						const int nScanline = nFromY + nY;
						const double dNormY = szDib.cy > 1 ? nScanline/double(szDib.cy-1):1;
						for(int nX=0;nX<szDib.cx;++nX,++nZ)
						{
							const double dNormX = szDib.cx > 1 ? nX/double(szDib.cx-1):1;		// left edge of pixel needs to be pixel centres
							spMI->setrowcol(0,0,dLerpID,nZ);
							spMI->setrowcol(1,0,dNormX,nZ);
							spMI->setrowcol(2,0,dNormY,nZ);
						}
					}

				std::shared_ptr<const afml::network<>> spN = m_spTrain->getnetwork();
				m_spTrain->allocactivation(spN,*spMI,*spMO,spActivation,spZ,spMI->getz());
				m_spTrain->getactivation(pSched,spN.get(),*spMI,af::vectorrange<true,true>(0,spMI->getz()-1),spActivation.get(),spZ.get());
				
				int nZOffset = 0;
				spDib=pFromImageItem->getdib((*spActivation)[spActivation->size()-1],nZOffset,pFromImageItem->gettype(),szDib,rcClip,spDib);

				if(spCache)
					spCache->setmiclip(rcClip);
			}
		}
		break;
		default:ASSERT(false);return;
	}
	if(spCache)
	{
		spCache->setmi(spMI);
		spCache->setmo(spMO);
		spCache->setactivation(spActivation);
		spCache->setz(spZ);
		spCache->setmiid(dLerpID);
		spCache->setmitype(pFromImageItem->gettype());
	}
	pReq->setdib(spDib);
}








IMPLEMENT_DYNAMIC(mlthreadwnd, CWnd)

mlthreadwnd::mlthreadwnd()
{
}

mlthreadwnd::~mlthreadwnd()
{
}


BEGIN_MESSAGE_MAP(mlthreadwnd, CWnd)
	ON_MESSAGE(WM_THREAD_QUERY_RESULT,OnThreadQueryResult)
	ON_MESSAGE(WM_THREAD_PLAYCONFIG_RESULT,OnThreadPlayConfigResult)
END_MESSAGE_MAP()


LRESULT mlthreadwnd::OnThreadQueryResult(WPARAM,LPARAM)
{
	theApp.flushthreadqueryresults();
	return 0;
}

LRESULT mlthreadwnd::OnThreadPlayConfigResult(WPARAM,LPARAM)
{
	theApp.flushthreadplayconfigresults();
	return 0;
}
