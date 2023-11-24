
#include "pch.h"
#include "ml_network.h"
#include "jpeg.h"
#include "serialise.h"

namespace afml
{

bool trainsetitem::read(const serialise *pS)
{
	// version
	int nVersion;
	if(!pS->read<>(nVersion))
		return false;
			
	// members
	if(nVersion > 0)
	{
		if(!pS->read<inputtype,int>(m_Type))
			return false;
	}

	return true;
}

bool trainsetitem::write(const serialise *pS) const
{
	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;

	// members
	if(!pS->write<inputtype,int>(m_Type))
		return false;

	return true;
}

std::shared_ptr<afdib::dib> traintsetimageitem::getdib(const af::mxnxzmatrix<>& m,const int nZOffset,const inputtype it,const SIZE& szDibDim,const RECT& rcClip,std::shared_ptr<afdib::dib> spFrom)
{
	if(m.isempty())
		return nullptr;

	std::shared_ptr<afdib::dib> sp = spFrom;
	if(!sp)
		sp=std::shared_ptr<afdib::dib>(new afdib::dib);
	if(!sp->create(szDibDim.cx,szDibDim.cy,afdib::dib::pt_b8g8r8))
		return spFrom;

	switch(it)
	{
		case it_image_i_id_o_b8g8r8:
		{
			// placed in centre of image (nDim x nDim)
			for(int nY=rcClip.top,nR=0;nY<rcClip.bottom;++nY)
			{
				unsigned char *pScanline = sp->getscanline(nY);
				for(int nX=rcClip.left;nX<rcClip.right;++nX)
					for(int nC=0;nC<3;++nC,++nR)
						pScanline[(nX*3)+nC]=af::posround<double,unsigned char>(255.0*m.get(nR,0,nZOffset));
			}
		}
		break;
		case it_image_i_id_xy_o_b8g8r8:
		{
			// scaled so largest dim is <= nDim
			for(int nY=rcClip.top,nZ=0;nY<rcClip.bottom;++nY)
			{
				unsigned char *pScanline = sp->getscanline(nY);
				for(int nX=rcClip.left;nX<rcClip.right;++nX,++nZ)
					for(int nC=0;nC<3;++nC)
						pScanline[(nX*3)+nC]=af::posround<double,unsigned char>(255.0*m.get(nC,0,nZOffset+nZ));
			}
		}
		break;
		default:return nullptr;
	}
	
	return sp;
}

bool traintsetimageitem::setdim(const int nDim)
{
	if(isempty())
		return false;
	return set(m_csPath,nDim,m_Type) && setid(getid());
}

std::shared_ptr<afdib::dib> traintsetimageitem::loadtrnsdib(LPCTSTR lpsz,const int nDim,const inputtype it)
{
	// attempt jpeg load
	std::shared_ptr<afdib::dib> spDib=afdib::jpeg::load8bpp(lpsz);
	if(!spDib)
		return false;
	if(spDib->getpixeltype()!=afdib::dib::pt_b8g8r8)
		return false;
		
	// dst to src matrix row,col, ( T1 * S ) * T2
	double m00=1,m11=1,m20=0,m21=0;
	std::shared_ptr<afdib::dib> spTrnsDib(new afdib::dib);
	int nBytesPerPixel = spDib->getbytesperpixel();
	std::vector<char> bkgndcolour(nBytesPerPixel,0);
	switch(it)
	{
		case it_image_i_id_o_b8g8r8:
		{
			// place in centre of image (nDim x nDim)
			double dSrcTLX=0,dSrcTLY=0,dSrcBRX=spDib->getwidth(),dSrcBRY=spDib->getheight();
			double dDstTLX=0,dDstTLY=0,dDstBRX=nDim,dDstBRY=nDim;
			spTrnsDib->create(nDim,nDim,spDib->getpixeltype());

			// scale
			double dS;
			const bool bLetterBox = true;
			getrectscale(dSrcTLX,dSrcTLY,dSrcBRX,dSrcBRY,dDstTLX,dDstTLY,dDstBRX,dDstBRY,bLetterBox,dS);
			const bool bScaleUp = false;
			const bool bScale = ( dS < 1 ) || bScaleUp;
			dS = bScale ? dS : 1;
			const double dScaledSrcTLX=dS*dSrcTLX,dScaledSrcTLY=dS*dSrcTLY,dScaledSrcBRX=dS*dSrcBRX,dScaledSrcBRY=dS*dSrcBRY;
									
			// translate
			const double dT2X=(dSrcBRX-dSrcTLX)/2.0;
			const double dT2Y=(dSrcBRY-dSrcTLY)/2.0;
			const double dT1X=(dDstBRX-dDstTLX)/-2.0;
			const double dT1Y=(dDstBRY-dDstTLY)/-2.0;
			
			// dst to src matrix row,col, ( T1 * S ) * T2
			m00 = 1/dS;
			m11 = 1/dS;
			m20 = (dT1X * (1/dS)) + dT2X;
			m21 = (dT1Y * (1/dS)) + dT2Y;
		}
		break;
		case it_image_i_id_xy_o_b8g8r8:
		{
			// scale so largest dim is <= nDim
			double dSrcTLX=0,dSrcTLY=0,dSrcBRX=spDib->getwidth(),dSrcBRY=spDib->getheight();

			// scale
			const double dMaxDim = std::max<double>(dSrcBRX,dSrcBRY);
			double dS=dMaxDim>nDim?nDim/dMaxDim:1;
			const bool bScaleUp = false;
			const bool bScale = ( dS < 1 ) || bScaleUp;
			dS = bScale ? dS : 1;
			double dScaledSrcTLX=dS*dSrcTLX,dScaledSrcTLY=dS*dSrcTLY,dScaledSrcBRX=dS*dSrcBRX,dScaledSrcBRY=dS*dSrcBRY;
			spTrnsDib->create(int(dScaledSrcBRX-dScaledSrcTLX),int(dScaledSrcBRY-dScaledSrcTLY),spDib->getpixeltype());

			// translate
			const double dT2X=(dSrcBRX-dSrcTLX)/2.0;
			const double dT2Y=(dSrcBRY-dSrcTLY)/2.0;
			const double dT1X=(dScaledSrcBRX-dScaledSrcTLX)/-2.0;
			const double dT1Y=(dScaledSrcBRY-dScaledSrcTLY)/-2.0;
			
			// dst to src matrix row,col, ( T1 * S ) * T2
			m00 = 1/dS;
			m11 = 1/dS;
			m20 = (dT1X * (1/dS)) + dT2X;
			m21 = (dT1Y * (1/dS)) + dT2Y;
		}
		break;
		default:return false;
	}
	for(int nY=0;nY<spTrnsDib->getheight();++nY)
	{
		unsigned char *pDstRow = spTrnsDib->getscanline(nY);

		const double dContinuousY = nY+0.5;
		const double dSrcY = (dContinuousY*m11)+m21;
		const int nSrcY=static_cast<int>(dSrcY);
		const unsigned char *pSrcRow = spDib->getscanline(nSrcY);

		// could check y extents here and flood scanline with background colour...
		for(int nX=0;nX<spTrnsDib->getwidth();++nX)
		{
			const double dContinuousX = nX+0.5;
			const double dSrcX = (dContinuousX*m00)+m20;
			
			const int nSrcX=static_cast<int>(dSrcX);
			if((nSrcX<0||nSrcX>=spDib->getwidth())||
			   (nSrcY<0||nSrcY>=spDib->getheight()))
			{
				// just use a background colour
				for(int nChannel=0;nChannel<nBytesPerPixel;++nChannel)
					pDstRow[(nX*nBytesPerPixel)+nChannel]=bkgndcolour[nChannel];
				continue;
			}

			for(int nChannel=0;nChannel<nBytesPerPixel;++nChannel)
				pDstRow[(nX*nBytesPerPixel)+nChannel]=pSrcRow[(nSrcX*nBytesPerPixel)+nChannel];
		}
	}
	return spTrnsDib;
}

traintsetimageitem::reloadtype traintsetimageitem::reload(LPCTSTR lpsz,const int nDim,const inputtype it)
{
	// attempt jpeg load
	std::shared_ptr<afdib::dib> spTrnsDib=loadtrnsdib(lpsz,nDim,it);
	if(!spTrnsDib)
		return rt_file_err;
	if(m_pt!=spTrnsDib->getpixeltype() ||
	   m_szDibDim.cx!=spTrnsDib->getwidth() ||
	   m_szDibDim.cy!=spTrnsDib->getheight() ||
	   m_nDim!=nDim)
		return rt_mismatch_err;
	const bool b = set(lpsz,nDim,it,spTrnsDib);
	ASSERT(b);
	return rt_ok;
}

bool traintsetimageitem::set(LPCTSTR lpsz,const int nDim,const inputtype it)
{
	// attempt jpeg load
	std::shared_ptr<afdib::dib> spTrnsDib=loadtrnsdib(lpsz,nDim,it);
	return set(lpsz,nDim,it,spTrnsDib);
}

bool traintsetimageitem::set(LPCTSTR lpsz,const int nDim,const inputtype it,std::shared_ptr<afdib::dib> spTrnsDib)
{
	if(!spTrnsDib)
		return false;
	const int nBytesPerPixel = spTrnsDib->getbytesperpixel();

	// setup io
	switch(it)
	{
		case it_image_i_id_o_b8g8r8:
		{
			// n pixels

			// z[0]: id -> pixel[0], ..., pixel[n-2], pixel[n-1]
			m_Input.setrowscols(1,1);
			m_Input.setrowcol(0,0,m_nID);
			m_Output.setrowscols(spTrnsDib->getheight()*spTrnsDib->getwidth()*nBytesPerPixel,1);
			for(int nY=0,nR=0;nY<spTrnsDib->getheight();++nY)
			{
				const unsigned char *pRow = spTrnsDib->getscanline(nY);
				for(int nX=0;nX<spTrnsDib->getwidth();++nX)
					for(int nChannel=0;nChannel<nBytesPerPixel;++nChannel,++nR)
						m_Output.setrowcol(nR,0,pRow[(nX*nBytesPerPixel)+nChannel]/255.0);
			}
		}
		break;
		case it_image_i_id_xy_o_b8g8r8:
		{
			// n pixels

			// z[0]: id + xy -> pixel[0]
			// ...
			// z[n-2]: id + xy -> pixel[n-2]
			// z[n-1]: id + xy -> pixel[n-1]
			m_Input.setrowscols(3,1,spTrnsDib->getheight()*spTrnsDib->getwidth());
			m_Output.setrowscols(nBytesPerPixel,1,spTrnsDib->getheight()*spTrnsDib->getwidth());
			for(int nY=0,nZ=0;nY<spTrnsDib->getheight();++nY)
			{
				const double dNormY = spTrnsDib->getheight() > 1 ? nY/double(spTrnsDib->getheight()-1):1;		// left edge of pixel needs to be pixel centres
				const unsigned char *pRow = spTrnsDib->getscanline(nY);
				for(int nX=0;nX<spTrnsDib->getwidth();++nX,++nZ)
				{
					const double dNormX = spTrnsDib->getwidth() > 1 ? nX/double(spTrnsDib->getwidth()-1):1;		// left edge of pixel needs to be pixel centres
					m_Input.setrowcol(0,0,m_nID,nZ);
					m_Input.setrowcol(1,0,dNormX,nZ);
					m_Input.setrowcol(2,0,dNormY,nZ);
					for(int nChannel=0;nChannel<nBytesPerPixel;++nChannel)
						m_Output.setrowcol(nChannel,0,pRow[(nX*nBytesPerPixel)+nChannel]/255.0,nZ);
				}
			}
		}
		break;
		default:return false;
	}

	m_csPath=lpsz;
	m_pt=spTrnsDib->getpixeltype();
	m_szDibDim.cx=spTrnsDib->getwidth();
	m_szDibDim.cy=spTrnsDib->getheight();
	m_nDim=nDim;
	m_Type=it;
	return true;
}

bool traintsetimageitem::setid(const int nID)
{
	// setup io
	switch(m_Type)
	{
		case it_image_i_id_o_b8g8r8:
		{
			// n pixels

			// z[0]: id -> pixel[0], ..., pixel[n-2], pixel[n-1]
			m_Input.setrowcol(0,0,nID);
		}
		break;
		case it_image_i_id_xy_o_b8g8r8:
		{
			// n pixels

			// z[0]: id + xy -> pixel[0]
			// ...
			// z[n-2]: id + xy -> pixel[n-2]
			// z[n-1]: id + xy -> pixel[n-1]
			for(int nZ=0;nZ<m_Input.getz();++nZ)
				m_Input.setrowcol(0,0,nID,nZ);
		}
		break;
		default:return false;
	}
	m_nID=nID;
	return true;
}

CString traintsetimageitem::getfname(void) const
{
	wchar_t _drive[_MAX_DRIVE], _dir[_MAX_DIR], _fname[_MAX_FNAME], _ext[_MAX_EXT];
	_wsplitpath_s( m_csPath, _drive, _MAX_DRIVE, _dir, _MAX_DIR, _fname, _MAX_FNAME, _ext, _MAX_EXT );
	return CString(_fname)+CString(_ext);
}

void traintsetimageitem::getrectscale(const double dSrcTLX,const double dSrcTLY,const double dSrcBRX,const double dSrcBRY,
									  const double dDstTLX,const double dDstTLY,const double dDstBRX,const double dDstBRY,
									  const bool bLetterBox,double& dS)
{
	const double dDstWidth = dDstBRX-dDstTLX;
	const double dDstHeight = dDstBRY-dDstTLY;
	const double dSrcWidth = dSrcBRX-dSrcTLX;
	const double dSrcHeight = dSrcBRY-dSrcTLY;
	const double x = dDstWidth / dSrcWidth, y = dDstHeight / dSrcHeight;	
	if( bLetterBox )
	{
		// make src as large as possible while keeping both sides of src within dst
		const double min = x < y ? x : y;
		dS=min;
	}
	else
	{
		// make src as large as possible while keeping smallest side of src within dst
		const double max = x < y ? y : x;
		dS=max;
	}
}

bool traintsetimageitem::read(const serialise *pS)
{
	// base class
	if(!trainsetitem::read(pS))
		return false;

	// version
	int nVersion;
	if(!pS->read<>(nVersion))
		return false;
			
	// members
	if(nVersion > 0)
	{
		CString cs;
		if(!pS->read<>(cs))
			return false;;
		m_csPath=cs;
		if(!pS->read<afdib::dib::pixeltype,int>(m_pt))return false;
		if(!pS->read<>(m_szDibDim))return false;
		if(!pS->read<>(m_nID))return false;
		if(!pS->read<>(m_nDim))return false;
	}

	return true;
}

bool traintsetimageitem::write(const serialise *pS) const
{
	// base class
	if(!trainsetitem::write(pS))
		return false;

	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;
			
	// members
	if(!pS->write<>(m_csPath))return false;
	if(!pS->write<afdib::dib::pixeltype,int>(m_pt))return false;
	if(!pS->write<>(m_szDibDim))return false;
	if(!pS->write<>(m_nID))return false;
	if(!pS->write<>(m_nDim))return false;

	return true;
}

bool traintsetuseritem::read(const serialise *pS)
{
	// base class
	if(!trainsetitem::read(pS))
		return false;

	// version
	int nVersion;
	if(!pS->read<>(nVersion))
		return false;
	
	// members
	if(nVersion > 0)
	{
		if(!m_Input.read(pS) || !m_Output.read(pS))
			return false;
	}

	return true;
}

bool traintsetuseritem::write(const serialise *pS) const
{
	// base class
	if(!trainsetitem::write(pS))
		return false;
		
	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;

	// members
	if(!m_Input.write(pS,true) || !m_Output.write(pS,true))
		return false;

	return true;
}

bool inputtypeitem::read(const serialise *pS)
{
	// version
	int nVersion=0;
	if(!pS->read<>(nVersion))
		return false;

	// members
	if(nVersion > 0)
	{
		{
			int nSize;
			if(!pS->read<>(nSize))
				return false;
			std::vector<std::shared_ptr<trainsetitem>> v;
			for(int n=0;n<nSize;++n)
			{
				trainsetitem::inputtype it;
				if(!pS->read<trainsetitem::inputtype,int>(it))
					return false;
				std::shared_ptr<trainsetitem> sp;
				switch(it)
				{
					case trainsetitem::it_user:sp=std::shared_ptr<trainsetitem>(new traintsetuseritem);break;
					case trainsetitem::it_image_i_id_o_b8g8r8:
					case trainsetitem::it_image_i_id_xy_o_b8g8r8:sp=std::shared_ptr<trainsetitem>(new traintsetimageitem);break;
					default:ASSERT(false);return false;
				}
				if(!sp || !sp->read(pS))
					return false;
				v.push_back(sp);
			}
			vTraining=v;
		}
		{
			int nSize;
			if(!pS->read<>(nSize))
				return false;
			std::vector<std::shared_ptr<layer<>>> v;
			layer<> *pPrev=nullptr;
			for(int n=0;n<nSize;++n)
			{
				gradientclip<>::type gt;
				double dGT;
				if(!pS->read<gradientclip<>::type,int>(gt))return false;
				if(!pS->read<>(dGT))return false;
				activationfn::type afnt;
				if(!pS->read<activationfn::type,int>(afnt))return false;
				activationnorm<>::type ant;
				double dL,dU;
				if(!pS->read<activationnorm<>::type,int>(ant))return false;
				if(!pS->read<>(dL))return false;
				if(!pS->read<>(dU))return false;
				af::mxnxzmatrix<> bkwndweight,bias;
				bkwndweight.read(pS);
				bias.read(pS);
				std::shared_ptr<layer<>> sp(new layer<>(bkwndweight,bias,afnt,activationnorm<>(ant,dU,dL),gradientclip<>(gt,dGT)));
				sp->setprev(pPrev);
				if(pPrev)
					pPrev->setnext(sp.get());
				v.push_back(sp);
				pPrev=sp.get();
			}
			vLayers=v;
		}
		if(!pS->read<>(dLearningRate))return false;
		if(!pS->read<>(dTermCost))return false;
		if(!pS->read<>(bTermCostCheck))return false;
		if(!pS->read<>(bValidateFP))return false;
		if(!pS->read<networktraining<>::gradientdescenttype,int>(gdt))return false;
		if(!pS->read<>(nMiniBatch))return false;
		if(!pS->read<trainsetitem::inputtype,int>(it))return false;
		if(!pS->read<activationfn::initialisetype,int>(initType))return false;
		if(!pS->read<>(userRandomRange.first))return false;
		if(!pS->read<>(userRandomRange.second))return false;
		if(!pS->read<>(nEpochs))return false;
		if(!pS->read<>(nEpoch))return false;
		if(!pS->read<>(minepoch.first))return false;
		if(!pS->read<>(minepoch.second))return false;
		if(!pS->read<>(maxepoch.first))return false;
		if(!pS->read<>(maxepoch.second))return false;
		if(!pS->read<>(bTrainingInitialised))return false;
		if(!pS->read<>(nImageDim))return false;
	}
	
	return true;
}

bool inputtypeitem::write(const serialise *pS) const
{
	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;
	
	// members
	if(!pS->write<size_t,int>(vTraining.size()))
		return false;
	{
		auto i = vTraining.cbegin(),end=vTraining.cend();
		for(;i!=end;++i)
		{
			if(!pS->write<trainsetitem::inputtype,int>((*i)->gettype()))return false;
			if(!(*i)->write(pS))
				return false;
		}
	}
	if(!pS->write<size_t,int>(vLayers.size()))
		return false;
	{
		auto i = vLayers.cbegin(),end=vLayers.cend();
		for(;i!=end;++i)
		{
			if(!pS->write<gradientclip<>::type,int>((*i)->getgradientclip().gettype()))return false;
			if(!pS->write<>((*i)->getgradientclip().getthreshold()))return false;
			if(!pS->write<activationfn::type,int>((*i)->getactivationfn().gettype()))return false;
			if(!pS->write<activationnorm<>::type,int>((*i)->getactivationnorm().gettype()))return false;
			if(!pS->write<>((*i)->getactivationnorm().getlower()))return false;
			if(!pS->write<>((*i)->getactivationnorm().getupper()))return false;
			(*i)->getbkwdweight().write(pS,nEpoch>0);
			(*i)->getbias().write(pS,nEpoch>0);
		}
	}
	if(!pS->write<>(dLearningRate))return false;
	if(!pS->write<>(dTermCost))return false;
	if(!pS->write<>(bTermCostCheck))return false;
	if(!pS->write<>(bValidateFP))return false;
	if(!pS->write<networktraining<>::gradientdescenttype,int>(gdt))return false;
	if(!pS->write<>(nMiniBatch))return false;
	if(!pS->write<trainsetitem::inputtype,int>(it))return false;
	if(!pS->write<activationfn::initialisetype,int>(initType))return false;
	if(!pS->write<>(userRandomRange.first))return false;
	if(!pS->write<>(userRandomRange.second))return false;
	if(!pS->write<>(nEpochs))return false;
	if(!pS->write<>(nEpoch))return false;
	if(!pS->write<>(minepoch.first))return false;
	if(!pS->write<>(minepoch.second))return false;
	if(!pS->write<>(maxepoch.first))return false;
	if(!pS->write<>(maxepoch.second))return false;
	if(!pS->write<>(bTrainingInitialised))return false;
	if(!pS->write<>(nImageDim))return false;

	return true;
}

}
