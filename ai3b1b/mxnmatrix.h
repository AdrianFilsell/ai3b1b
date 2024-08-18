#pragma once

// Created by Adrian Filsell 2018
// Copyright 2018 Adrian Filsell. All rights reserved.

#include "core.h"

class serialise;

namespace af
{

// T - matrix data type
template<typename T=double> class mxnxzmatrix
{
public:

	// Constructor/Destructor
	mxnxzmatrix() { m_nRows=0; m_nCols = 0; m_nZ = 0; }
	mxnxzmatrix(const int nR,const int nC,const int nZ=1):mxnxzmatrix() { setrowscols(nR,nC,nZ); }
	mxnxzmatrix(const int nR,const int nC,const T d,const int nZ=1):mxnxzmatrix(nR,nC,nZ) { for(int nZ=0;nZ<getz();++nZ) set(d,nZ); }
	mxnxzmatrix(const mxnxzmatrix& other) { *this=other; }
	mxnxzmatrix(mxnxzmatrix&& other):mxnxzmatrix() {*this=other;}
	~mxnxzmatrix() {}

	// Accessors
	__forceinline bool isempty(void) const{return getrows()==0||getcols()==0||getz()==0;}
	__forceinline bool isextentsequal(const mxnxzmatrix& o,const bool bZ=false) const{return getrows()==o.getrows() && getcols()==o.getcols() && (!bZ||getz()==o.getz());}
	__forceinline bool isextentsequal(const int nRows,const int nCols,const int nZ=1) const{return getrows()==nRows && getcols()==nCols && getz()==nZ;}
	__forceinline bool isvalidrow(const int n)const{return n>=0&&n<getrows();}
	__forceinline bool isvalidcol(const int n)const{return n>=0&&n<getcols();}
	__forceinline bool isvalidz(const int n)const{return n>=0&&n<getz();}
	__forceinline int getrows(void)const{return m_nRows;}
	__forceinline int getcols(void)const{return m_nCols;}
	__forceinline int getz(void)const{return m_nZ;}
	__forceinline const T* get(const int nZ=0) const { return &(m_v[0]) + (getrows()*getcols()*nZ); }
	__forceinline const T* getrow(const int nR,const int nZ=0) const { return (get(nZ) + (nR * m_nCols)); }
	__forceinline const T get(const int nR,const int nC,const int nZ=0) const { return getrow(nR,nZ)[nC]; }
	__forceinline T getsum(const int nZ=0)const{auto i = m_v.cbegin()+(getrows()*getcols()*nZ); return std::accumulate(i,i+(getrows()*getcols()),T(0));}
	__forceinline T getmean(const int nZ=0)const{return getsum(nZ)/T(getrows()*getcols());}
	__forceinline T getmax(const int nZ=0)const{auto i = m_v.cbegin()+(getrows()*getcols()*nZ); return *std::max_element(i,i+(getrows()*getcols()));}
	__forceinline T getmin(const int nZ=0)const{auto i = m_v.cbegin()+(getrows()*getcols()*nZ); return *std::min_element(i,i+(getrows()*getcols()));}
	__forceinline void getminmax(T& dMin,T& dMax,const int nZ)const
	{
		auto i = m_v.cbegin()+(getrows()*getcols()*nZ); 
		dMin=*i;dMax=*i;
		std::for_each(i+1,i+(getrows()*getcols()),[&](const T v){if(v<dMin)dMin=v;else if(v>dMax)dMax=v;});
	}
	__forceinline T getstddev(const vectorrange<true,true>&r,const bool bApplyBesselCorrection,T& dMean,const int nZ=0)const
	{
		// if we are dealing with a subset/sample of the population we may want to use bessel's correction
		const int nRC = getrows()*getcols();
		dMean = getmean(nZ);
		T d=0;
		auto i = m_v.cbegin()+(getrows()*getcols()*nZ); 
		std::for_each(i+r.getfrom(),i+r.getinclusiveto()+1,[&](const T v){d+=(v-dMean)*(v-dMean);});
		d=(bApplyBesselCorrection&&r.issubset(vectorrange<true,true>(0,nRC-1))) ? d/(nRC-1) : d/(nRC);
		return d;
	}
	__forceinline void getmaxmagnitude(T& d,int& nMR,int& nMC,const int nZ=0)const
	{
		T dAbs=0;int n = (getrows()*getcols()*nZ);
		for (int nR = 0; nR < m_nRows ; ++nR)
			for (int nC = 0; nC < m_nCols ; ++nC,++n)
			{
				const T dSign = (m_v[n]<0) ? -1 : 1;
				if((m_v[n]*dSign)>dAbs)
				{
					d=m_v[n];
					dAbs=m_v[n]*dSign;
					nMR=nR;
					nMC=nC;
				}
			}
	}
	__forceinline mxnxzmatrix<T>& gettranspose(mxnxzmatrix<T>& m,const int nZ=0) const
	{
		if(isempty())
		{
			m.clear();
			return m;
		}
		m.setrowscols(getcols(),getrows(),1);
		const T *p=get(nZ);
		for(int nRow=0;nRow<m_nRows;++nRow,p+=m_nCols)
			for(int nCol=0;nCol<m_nCols;++nCol)
				m.getrow(nCol)[nRow]=p[nCol];
		return m;
	}
	mxnxzmatrix<T> gettranspose(void) const {mxnxzmatrix<T> m=*this;gettranspose(m);return m;}
	mxnxzmatrix<T>& copy(mxnxzmatrix<T>& dst,const int nDstZ=0,const int nZ=0) const
	{
		T *pDst=dst.get(nDstZ);
		const T *p=get(nZ);
		for (int n = 0; n < m_nRows * m_nCols; ++n)pDst[n] = p[n];
		return dst;
	}
	mxnxzmatrix<T>& copy_hadamard_sub_hadamard_mul(mxnxzmatrix<T>& dst,const mxnxzmatrix<T>& a,const mxnxzmatrix<T>& b,const int nDstZ=0,const int nAZ=0,const int nBZ=0,const int nZ=0) const
	{
		// specialised composite
		T *pDst=dst.get(nDstZ);
		const T *p=get(nZ);
		const T *pA=a.get(nAZ);
		const T *pB=b.get(nBZ);
		for (int n = 0; n < m_nRows * m_nCols; ++n)pDst[n] = ( p[n] - pA[n] ) * pB[n];
		return dst;
	}
	bool isequal(const mxnxzmatrix<T>& other,const int nOtherZ,const int nZ=0)const{if(!isextentsequal(other)) return false;const T *pA=getrow(0,nZ);const T *pB=other.getrow(0,nOtherZ);for(int n = 0;n<getrows()*getcols();++n)if(pA[n]!=pB[n])return false;return true;}
	bool fpvalid(const bool bDenormalNormal = false,const int nZ=0) const { const T* p=get(nZ); for (int n = 0; n < m_nRows * m_nCols; ++n) if (!af::fpvalid<T>(p[n],bDenormalNormal)) return false; return true; }
	bool read(const serialise *pS)
	{
		// version
		int nVersion=0;
		if(!pS->read<>(nVersion))return false;
		
		// members
		if(!pS->read<>(m_nRows))return false;
		if(!pS->read<>(m_nCols))return false;
		if(!pS->read<>(m_nZ))return false;
		bool bEntries=true;
		if(!pS->read<>(bEntries))return false;
		if(!bEntries)
			m_v.resize(m_nRows*m_nCols*m_nZ);
		else
		if(!pS->read<T>(m_v))
			return false;
				
		return true;
	}
	bool write(const serialise *pS,const bool bEntries) const
	{
		// version
		const int nVersion = 1;
		if(!pS->write<>(nVersion))return false;
		
		// members
		if(!pS->write<>(m_nRows))return false;
		if(!pS->write<>(m_nCols))return false;
		if(!pS->write<>(m_nZ))return false;
		if(!pS->write<>(bEntries))return false;
		if(bEntries && !pS->write<T>(m_v))
			return false;
				
		return true;
	}
			
	// Modifiers
	__forceinline void push_back(const mxnxzmatrix& o)
	{
		if(o.isempty())return;
		m_v.insert(m_v.begin()+(m_nRows*m_nCols*m_nZ),o.m_v.cbegin(),o.m_v.cbegin()+(o.getrows()*o.getcols()*o.getz()));
		m_nCols=o.getcols();
		m_nRows=o.getrows();
		m_nZ+=o.m_nZ;
	}
	__forceinline void erase(const int nZ)
	{
		if(isempty())return;
		if(!isvalidz(nZ))return;
		auto dst = m_v.begin()+(getrows()*getcols()*nZ);
		auto src = dst + (getrows()*getcols());
		auto end = m_v.cend();
		for(;src!=end;++src,++dst)
			*dst=*src;
		--m_nZ;
	}
	__forceinline void setrowscols(const mxnxzmatrix& o){setrowscols(o.getrows(),o.getcols(),o.getz());}
	__forceinline void setrowscols(const int nR, const int nC,const int nZ=1) { if(isextentsequal(nR,nC,nZ))return;if (nR<1 || nC<1 || nZ<1) { return; } m_nRows = nR; m_nCols = nC; m_nZ = nZ; if(static_cast<int>(m_v.size())<(m_nRows*m_nCols*m_nZ)) m_v.resize(m_nRows*m_nCols*m_nZ); }
	__forceinline void setrowcol(const int nR, const int nC,const T d,const int nZ=0) { getrow(nR,nZ)[nC]=d; }
	__forceinline void clear(void) {m_v.clear();m_nRows=0;m_nCols=0;m_nZ=0;}
	__forceinline void rand(const T dFrom,const T dTo,const int nZ=0)
	{
		auto i = m_v.begin()+(getrows()*getcols()*nZ);
		auto end = i+(getrows()*getcols());
		for(;i!=end;++i)
			*i = af::rand<T>(dFrom,dTo);
	}
	__forceinline void normalrand(const std::normal_distribution<T>& nd,const int nZ=0)
	{
		std::default_random_engine generator;
		std::normal_distribution<T> distribution(nd);
		auto i = m_v.begin()+(getrows()*getcols()*nZ);
		auto end = i+(getrows()*getcols());
		for(;i!=end;++i)
			*i = distribution(generator);
	}
	__forceinline void swapz(const int nFromZ,const int nToZ)
	{
		const int nFrom_From = nFromZ*getrows()*getcols();
		const int nTo_From = nToZ*getrows()*getcols();
		for(int n=0;n<(getrows()*getcols());++n)
			std::swap<T>(m_v[nFrom_From+n],m_v[nTo_From+n]);
	}
	__forceinline void set(const T d,const int nZ=0)
	{
		auto i = m_v.begin()+(getrows()*getcols()*nZ);
		auto end = i+(getrows()*getcols());
		for(;i!=end;++i)
			*i = d;
	}
	__forceinline T* get(const int nZ=0) { return &(m_v[0]) + (getrows()*getcols()*nZ); }
	__forceinline T* getrow(const int nR,const int nZ=0) { return (get(nZ) + (nR * m_nCols)); }
	__forceinline void normalise(const T dUpper, const T dLower,const int nZ=0)
	{
		T dMin,dMax;
		getminmax(dMin,dMax,nZ);
		const T dUpperLower=dUpper-dLower;
		const T dMaxMin=dMax-dMin;
		auto i = m_v.begin()+(getrows()*getcols()*nZ);
		auto end = i+(getrows()*getcols());
		for(;i!=end;++i)
			*i = (((*i-dMin)/dMaxMin)*dUpperLower)+dLower;
	}
	__forceinline void normalise(const T dMin, const T dMax, const T dUpper, const T dLower,const int nZ=0)
	{
		const T dUpperLower=dUpper-dLower;
		const T dMaxMin=dMax-dMin;
		auto i = m_v.begin()+(getrows()*getcols()*nZ);
		auto end = i+(getrows()*getcols());
		for(;i!=end;++i)
			*i = (((*i-dMin)/dMaxMin)*dUpperLower)+dLower;
	}
	__forceinline void standardise(const int nZ=0)
	{
		T dMean;
		standardise(dMean,getstddev(vectorrange<true,true>(0,getrows()*getcols()-1),false,dMean,nZ),nZ);
	}
	__forceinline void standardise(const T dMean,const T dStdDev,const int nZ=0)
	{
		auto i = m_v.begin()+(getrows()*getcols()*nZ);
		auto end = i+(getrows()*getcols());
		if(dStdDev)
			for(;i!=end;++i)
				*i=(*i-dMean)/dStdDev;
	}
	__forceinline void sumz(mxnxzmatrix<T>& dst,const int nDstZ,const vectorrange<true,true>& rZs) const
	{
		T *pDst = dst.get(nDstZ);
		{
			const T *pSrc = get(rZs.getfrom());
			for(int n=0;n<getrows()*getcols();++n)
				pDst[n]=pSrc[n];
		}
		for(int nZ=rZs.getfrom()+1;nZ<=rZs.getinclusiveto();++nZ)
		{
			const T *pSrc = get(nZ);
			for(int n=0;n<getrows()*getcols();++n)
				pDst[n]+=pSrc[n];
		}
	}
	__forceinline void sumz(void)
	{
		T *pDst = get();
		for(int nZ=1;nZ<m_nZ;++nZ)
		{
			const T *pSrc = get(nZ);
			for(int n=0;n<getrows()*getcols();++n)
				pDst[n]+=pSrc[n];
		}
	}
	__forceinline void mul(const double d,const int nZ=0)
	{
		T *pDst = get(nZ);
		for(int n=0;n<getrows()*getcols();++n)
			pDst[n]*=d;
	}
		
	// Operators
	mxnxzmatrix& operator =(const mxnxzmatrix& other){ if(other.isempty()){clear();return *this;}if(!isextentsequal(other,true)) setrowscols(other);m_v=other.m_v; return *this;}
	mxnxzmatrix& operator =(mxnxzmatrix&& other){m_v=std::move(other.m_v);m_nZ=other.m_nZ;m_nRows=other.m_nRows;m_nCols=other.m_nCols;other.m_nZ=0;other.m_nCols=0;other.m_nRows=0;return *this;}
	mxnxzmatrix& mul(const mxnxzmatrix& a,const mxnxzmatrix& b,const int nAZ=0,const int nBZ=0,const int nZ=0)
	{
		const int nCommon=a.getcols();
		const T *pARow = a.get(nAZ);
		T *pDstRow = get(nZ);
		for(int nDstRow=0;nDstRow<getrows();++nDstRow,pARow+=a.getcols(),pDstRow+=getcols())
			for(int nDstCol=0;nDstCol<getcols();++nDstCol)
			{
				// sum all the multiplications between elements in nDstRow(th) row in a and nDstCol(th) column in b
				T d = 0;
				const T *pB = b.getrow(0,nBZ)+nDstCol;
				for(int n = 0;n<nCommon;++n,pB+=b.getcols())
					d+=pARow[n]*pB[0];
				pDstRow[nDstCol]=d;
			}
		return *this;
	};
	mxnxzmatrix& mul_hadamard_add(const mxnxzmatrix& a,const mxnxzmatrix& b,const mxnxzmatrix& c,const int nAZ=0,const int nBZ=0,const int nCZ=0,const int nZ=0)
	{
		// specialised composite
		const int nCommon=a.getcols();
		const T *pARow = a.get(nAZ);
		T *pDstRow = get(nZ);
		const T *pCRow = c.get(nCZ);
		for(int nDstRow=0;nDstRow<getrows();++nDstRow,pARow+=a.getcols(),pDstRow+=getcols(),pCRow+=getcols())
			for(int nDstCol=0;nDstCol<getcols();++nDstCol)
			{
				// sum all the multiplications between elements in nDstRow(th) row in a and nDstCol(th) column in b
				T d = 0;
				const T *pB = b.getrow(0,nBZ)+nDstCol;
				for(int n = 0;n<nCommon;++n,pB+=b.getcols())
					d+=pARow[n]*pB[0];
				pDstRow[nDstCol]=d+pCRow[nDstCol];
			}
		return *this;
	};
	mxnxzmatrix& mul_aT(const mxnxzmatrix& a,const mxnxzmatrix& b,const int nAZ=0,const int nBZ=0,const int nZ=0)
	{
		const int nCommon=a.getrows();
		const T *pA = a.get(nAZ);
		T *pDstRow = get(nZ);
		for(int nDstRow=0;nDstRow<getrows();++nDstRow,pDstRow+=getcols())
			for(int nDstCol=0;nDstCol<getcols();++nDstCol)
			{
				// sum all the multiplications between elements in nDstRow(th) row in a and nDstCol(th) column in b - remember transpose a
				T d = 0;
				const T *pB = b.getrow(0,nBZ)+nDstCol;
				const T *pARow = pA;
				for(int n = 0;n<nCommon;++n,pB+=b.getcols(),pARow+=a.getcols())
					d+=pARow[nDstRow]*pB[0];
				pDstRow[nDstCol]=d;
			}
		return *this;
	};
	mxnxzmatrix& mul_aT_hadamard_mul(const mxnxzmatrix& a,const mxnxzmatrix& b,const mxnxzmatrix& c,const int nAZ=0,const int nBZ=0,const int nCZ=0,const int nZ=0)
	{
		// specialised composite
		const int nCommon=a.getrows();
		const T *pA = a.get(nAZ);
		T *pDstRow = get(nZ);
		const T *pC = c.get(nCZ);
		for(int nDstRow=0;nDstRow<getrows();++nDstRow,pDstRow+=getcols(),pC+=getcols())
			for(int nDstCol=0;nDstCol<getcols();++nDstCol)
			{
				// sum all the multiplications between elements in nDstRow(th) row in a and nDstCol(th) column in b - remember transpose a
				T d = 0;
				const T *pB = b.getrow(0,nBZ)+nDstCol;
				const T *pARow = pA;
				for(int n = 0;n<nCommon;++n,pB+=b.getcols(),pARow+=a.getcols())
					d+=pARow[nDstRow]*pB[0];
				pDstRow[nDstCol]=d*pC[nDstCol];
			}
		return *this;
	};
	mxnxzmatrix& hadamard_mul(const mxnxzmatrix& other,const int nOtherZ=0,const int nZ=0)
	{
		const T *pOther=other.get(nOtherZ);
		T *p=get(nZ);
		for (int n = 0; n < m_nRows * m_nCols; ++n)p[n] *= pOther[n];
		return *this;
	}
	mxnxzmatrix& hadamard_sub(const mxnxzmatrix& other,const int nOtherZ=0,const int nZ=0)
	{
		const T *pOther=other.get(nOtherZ);
		T *p=get(nZ);
		for (int n = 0; n < m_nRows * m_nCols; ++n)p[n] -= pOther[n];
		return *this;
	}
	mxnxzmatrix& hadamard_add(const mxnxzmatrix& other,const int nOtherZ=0,const int nZ=0)
	{
		const T *pOther=other.get(nOtherZ);
		T *p=get(nZ);
		for (int n = 0; n < m_nRows * m_nCols; ++n)p[n] += pOther[n];
		return *this;
	}
	mxnxzmatrix& hadamard_add_subsq(const mxnxzmatrix& a,const mxnxzmatrix& b,const int nAZ=0,const int nBZ=0,const int nZ=0)
	{
		// specialised composite
		const T *pA=a.get(nAZ);
		const T *pB=b.get(nBZ);
		T *p=get(nZ);
		for (int n = 0; n < m_nRows * m_nCols; ++n)
		{
			const T d = pA[n]-pB[n];
			p[n] += (d*d);
		}
		return *this;
	}
		
	// Static
	static const T tol;

protected:

	// Data
	int m_nRows;
	int m_nCols;
	int m_nZ;
	std::vector<T> m_v;
};

template <typename T> const T mxnxzmatrix<T>::tol = T(1e-6);

// T - matrix data type
template<typename T=double> class mxnxzmatrixval
{
public:
	mxnxzmatrixval(const T d):m_d(d){}
	virtual void set(mxnxzmatrix<T>& m,const int nZ=0) const { m.set(m_d,nZ); }
	mxnxzmatrixval& operator =(const mxnxzmatrixval& o){m_d=o.m_d;}
protected:
	T m_d;
};
template<typename T=double> class mxnxzmatrix_rand_val : public mxnxzmatrixval<T>
{
public:
	mxnxzmatrix_rand_val(const T dFrom,const T dTo):mxnxzmatrixval<T>(dFrom),m_dTo(dTo){}
	mxnxzmatrix_rand_val(const std::normal_distribution<T>& n):mxnxzmatrixval<T>(0),m_spN(new std::normal_distribution<T>(n)){}
	virtual void set(mxnxzmatrix<T>& m,const int nZ=0) const override { if(m_spN) m.normalrand(*m_spN,nZ); else m.rand(mxnxzmatrixval<T>::m_d,m_dTo,nZ); }
	mxnxzmatrix_rand_val& operator =(const mxnxzmatrix_rand_val& o){mxnxzmatrixval<T>::operator =(o);m_spN=o.m_spN;m_dTo=o.m_dTo;}
protected:
	T m_dTo;
	std::shared_ptr<const std::normal_distribution<T>> m_spN;
};

}
