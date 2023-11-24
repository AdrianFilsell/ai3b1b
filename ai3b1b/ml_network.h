#pragma once

// multi layer perceptron network

#include "core.h"
#include "mxnmatrix.h"
#include "thread_taskscheduler.h"
#include "dib.h"
#include <algorithm>

namespace afthread { class taskscheduler; }
class serialise;

namespace afml
{

template<typename T> class networktraining;

template <typename T=double> class relu
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { return FOD ? ( x > 0 ? 1 : 0 ) : ( std::max<T>(0, x) ); /*fn: max(0,x) OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
};

template <typename T=double> class leaky_relu
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { return FOD ? ( x > 0 ? 1 : 0.01 ) : ( x > 0 ? x : x*0.01 ); /*fn: x > 0 ? x : 0.01x OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
};

template <typename T=double> class sigmoid
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { const T d = ( 1.0 / (1.0 + ::pow(af::gete<T>(), -x)) ); return FOD ? ( d*(1.0-d)) : d; /*fn: 1 / ( 1 + pow(e,-x) ) OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
};

template <typename T=double> class tanh
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { const T d = ( 2.0 / ( 1.0 + ::pow(af::gete<T>(), -2.0*x) ) ) - 1.0;return FOD ? ( 1.0 - (d*d) ) : d; /*fn: ( 2 / ( 1 + pow(e,-2x) ) ) - 1 OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
};

template <typename T=double> class normtanh
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { return FOD ? 0.5 * tanh<T>::fn<FOD>(x) : 0.5 * ( tanh<T>::fn<FOD>(x) + 1.0 ); /*fn: 0.5 * ( tanh(x) + 1 ) OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
};

template <typename T=double> class silu
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { const T d = sigmoid<T>::fn<false>(x); return FOD ? (x*sigmoid<T>::fn<true>(x)+d) : (x*d); /*fn: x*sigmoid(x) OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
};

template <typename T=double> class gelu
{
public:
	template <bool FOD> static __forceinline T fn(const T x) { const T d = cdf(x); return FOD ? (d+(pdf(x)*x)) : x*d; /*fn: x*cdf(x) OR fn'*/ }
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR]);	}
protected:
	static __forceinline T erf(const T x)
	{
		// error function
		return std::erf(x);
	}
	static __forceinline T cdf(const T x)
	{
		// cumulative distribution function of the standard normal distribution
		// ( 1 + erf(x/sqrt(2)) ) / 2
		return 0.5 * ( 1 + erf(x/::sqrt(2.0)) );
	}
	static __forceinline T pdf(const T x)
	{
		// probability density function
		return std::exp(-0.5 * ::pow(x, 2.0)) / ::sqrt(af::getpi_2<T>());
	}
};

template <typename T=double> class softplus
{
public:
	template <bool FOD> static __forceinline T fn(const T x,const T dBeta=1,const T dThreshold=20)
	{
		// { const T d = ::pow(af::gete<T>(),x); return FOD ? (d/(1.0+d)) : ::log(1+d); /*fn: ln(1+e^x) OR fn'*/ }
		// but... lets introduce some numerical stability using beta and threshold
		// fn: (1/dBeta) * ln(1+e^(dBeta*x))
		//	 : if ( x * beta ) > threshold revert to linear function
		if(FOD)
		{
			if( dBeta == 0 || ( (dBeta*x) > dThreshold ) || ( (dBeta*x) < -dThreshold ) )
				return 1;
			const T d = ::pow(af::gete<T>(),x*dBeta);
			return d/(1.0+d);
		}
		else
		{
			if( dBeta == 0 || (dBeta*x) > dThreshold )
				return x;
			const T d = ::pow(af::gete<T>(),x*dBeta);
			if( (dBeta*x) < -dThreshold )
				return d;
			return ::log(1+d)/dBeta; /*fn: ln(1+e^x) OR fn'*/
		}
	}
	template <bool FOD> static __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& src,const T dBeta=1,const T dThreshold=20,const int nDstZ=0,const int nSrcZ=0) { T* pDstRow = dst.get(nDstZ);const T* pSrcRow = src.get(nSrcZ);for (int nR = 0; nR < src.getrows(); ++nR)pDstRow[nR] = fn<FOD>(pSrcRow[nR],dBeta,dThreshold);	}
};

template <typename T=double> class gradientclip
{
public:
	enum type {t_null,						// do not clip
			   t_threshold,					// clip with respect to a threshold
			  };
	gradientclip(const type t=t_null,const T d=3){m_Type=t;m_dThreshold=d; /* typically within range 1-5 */}
	gradientclip(const gradientclip& o){*this=o;}
	type gettype(void)const{return m_Type;}
	T getthreshold(void)const{return m_dThreshold;}
	__forceinline void fn(af::mxnxzmatrix<T>& w,af::mxnxzmatrix<T>& b,const int nWZ=0,const int nBZ=0) const
	{
		switch(m_Type)
		{
			case t_null:break;
			case t_threshold:
			{
				T dS = 1, dB, dW;
				int nBR,nBC,nWR,nWC;
				b.getmaxmagnitude(dB,nBR,nBC,nBZ);
				w.getmaxmagnitude(dW,nWR,nWC,nWZ);
				const bool bB = (dB>=-m_dThreshold && dB<=m_dThreshold);
				const bool bW = (dW>=-m_dThreshold && dW<=m_dThreshold);
				if(!bB&&!bW)
				{
					const T dAbsB = (dB<0)?-dB:dB;
					const T dAbsW = (dW<0)?-dW:dW;
					dS = (dAbsW<dAbsB) ? m_dThreshold/dAbsB : m_dThreshold/dAbsW;
				}
				else
				if(!bB)
				{
					const T dAbsB = (dB<0)?-dB:dB;
					dS = m_dThreshold/dAbsB;
				}
				else
				if(!bW)
				{
					const T dAbsW = (dW<0)?-dW:dW;
					dS = m_dThreshold/dAbsW;
				}
				if(dS==1)
					return;
				w.mul(dS,nWZ);
				b.mul(dS,nBZ);
			}
			break;
			default:return;
		}
	}
	void setthreshold(const T d){m_dThreshold=d;}
	gradientclip& operator =(const gradientclip& o){m_Type=o.m_Type;m_dThreshold=o.m_dThreshold;return *this;}
	bool operator ==(const gradientclip& o)const{if(m_Type!=o.m_Type)return false;if(m_dThreshold!=o.m_dThreshold)return false;return true;}
protected:
	type m_Type;
	T m_dThreshold;
};

class activationfn
{
public:
	enum type {t_sigmoid,				// sigmoid

			   t_tanh,					// tanh [-1,1], superseded sigmoid
			   t_normtanh,				// tanh [1,0], normalised to range [0,1]
			   
			   t_relu,					// rectified linear unit, more effective in training deep neural networks BUT can result in dead units
			   t_leaky_relu,			// leaky rectified linear unit, allows a small positive gradient when unit is not active
			   t_silu,					// sigmoid linear unit, smooth approximation to relu
			   t_softplus,				// smooth linear unit, another smooth approximation to relu
			   t_gelu,					// gaussian error linear unit, another smooth approximation to relu
			  };
	enum initialisetype {it_auto_normal,	// auto calculated normal distribution of rand values
						 it_auto_random,	// auto calculated rand values
						 it_user_random		// auto calculated rand values from user supplied range
						};
	activationfn(const type t=t_normtanh){m_Type=t;}
	activationfn(const activationfn& o){*this=o;}
	type gettype(void)const{return m_Type;}
	template <typename T=double> __forceinline void fn(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& z,const int nDstZ=0,const int nZZ=0) const
	{
		switch(m_Type)
		{
			case t_tanh:tanh<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_normtanh:normtanh<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_sigmoid:sigmoid<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_silu:silu<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_relu:relu<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_softplus:softplus<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_leaky_relu:leaky_relu<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			case t_gelu:gelu<T>::fn<false>(dst,z,nDstZ,nZZ);break;
			default:return;
		}
	}
	template <typename T=double> __forceinline af::mxnxzmatrix_rand_val<T> getweightbiasrand(const initialisetype t,const int nIn,const int nOut,const std::pair<T,T>& userrange) const
	{
		// randomise
		switch(t)
		{
			case it_user_random:
			{
				// user range rand numbers
				return af::mxnxzmatrix_rand_val<T>(userrange.first,userrange.second);
			}
			break;
		}
		switch(m_Type)
		{
			case t_gelu:
			case t_silu:
			case t_softplus:
			case t_leaky_relu:
			case t_relu:
			{
				switch(t)
				{
					case it_auto_normal:
					{
						// He initialization is specifically designed for the Rectified Linear Unit (ReLU) activation function.
						// ReLU is an unbounded positive activation function, and He initialization addresses the issue of "dying ReLU" by providing appropriate scaling to the weights.
						std::normal_distribution<T> n(0.0, sqrt(2.0 / nIn));
						return af::mxnxzmatrix_rand_val<T>(n);
					}
					break;
					case  it_auto_random:
					{
						// small positive rand numbers
						return af::mxnxzmatrix_rand_val<T>(0.1,0.5);
					}
					break;
					default:return af::mxnxzmatrix_rand_val<T>(0.1,0.5);
				}
			}
			break;
			case t_tanh:
			case t_normtanh:
			case t_sigmoid:
			{
				switch(t)
				{
					case it_auto_normal:
					{
						// Xavier initialization is generally considered suitable for activation functions like sigmoid and tanh, which are centered around zero.
						// It helps to ensure that the activations neither vanish nor explode during forward propagation.
						std::normal_distribution<T> n(0.0, sqrt(2.0 / (nIn+nOut)));
						return af::mxnxzmatrix_rand_val<T>(n);
					}
					break;
					case  it_auto_random:
					{
						// small rand numbers around 0
						return af::mxnxzmatrix_rand_val<T>(-0.5,0.5);
					}
					break;
					default:return af::mxnxzmatrix_rand_val<T>(-0.5,0.5);
				}
			}
			break;
			default:return af::mxnxzmatrix_rand_val<T>(0,1);break; // just randomise [0,1]
		}
	}
	template <typename T=double> __forceinline af::mxnxzmatrix<T>& getfodz(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& z,const int nDstZ=0,const int nZZ=0) const
	{
		switch(m_Type)
		{
			case t_normtanh:normtanh<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_tanh:tanh<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_sigmoid:sigmoid<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_silu:silu<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_relu:relu<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_softplus:softplus<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_leaky_relu:leaky_relu<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			case t_gelu:gelu<T>::fn<true>(dst,z,nDstZ,nZZ);break;
			default:dst.clear();break;
		}
		return dst;
	}
	activationfn& operator =(const activationfn& o){m_Type=o.m_Type;return *this;}
	bool operator ==(const activationfn& o)const{return m_Type==o.m_Type;}
protected:
	type m_Type;
};

template <typename T=double> class activationnorm
{
public:
	enum type {t_null,						// do not mutate activation values
			   t_minmax,					// normalise activation values
			   t_standardise,				// standardise activation values, if this method of activation value normalisation is required but the dataset does not have a normal or something close to a normal distribution t_minmax would be better
			  };
	activationnorm(const type t=t_null,const T dUpper=1,const T dLower=0){m_Type=t;m_dUpper=dUpper;m_dLower=dLower;}
	activationnorm(const activationnorm& o){*this=o;}
	type gettype(void)const{return m_Type;}
	T getlower(void)const{return m_dLower;}
	T getupper(void)const{return m_dUpper;}
	template <typename T=double> __forceinline void fn(af::mxnxzmatrix<T>& dst,const int nDstZ=0) const
	{
		switch(m_Type)
		{
			case t_null:break;
			case t_minmax:dst.normalise(m_dUpper,m_dLower,nDstZ);break;
			case t_standardise:dst.standardise(nDstZ);break;
			default:return;
		}
	}
	activationnorm& operator =(const activationnorm& o){m_Type=o.m_Type;m_dLower=o.m_dLower;m_dUpper=o.m_dUpper;return *this;}
	bool operator ==(const activationnorm& o)const{if(m_Type!=o.m_Type)return false;if(m_dLower!=o.m_dLower)return false;if(m_dUpper!=o.m_dUpper)return false; return true;}
protected:
	type m_Type;
	T m_dLower;
	T m_dUpper;
};

template <typename T=double> class layerdeltascratch
{
public:
	af::mxnxzmatrix<T> zFOD; // we pass these as params rather than creating them on locally so we can reuse their memory instead of alloc/dealloc
};

template <typename T=double> class layer
{
public:
	enum successtype {st_success=0x1,st_error=0x2,st_fpinvalid_error=0x4,
					  st_fail=(st_error|st_fpinvalid_error)};
	layer(const int nPerceptrons, const int nPrevPerceptrons,
		  const activationfn actfn=activationfn::t_normtanh,
		  const activationnorm<T> actn=activationnorm<T>(),
		  const gradientclip<T> gc=gradientclip<T>()):m_GradClip(gc),m_ActFn(actfn),m_ActNorm(actn)
	{
		m_pPrev=nullptr;
		m_pNext=nullptr;
		m_BkwdWeight.setrowscols(nPerceptrons,nPrevPerceptrons);
		m_Bias.setrowscols(nPerceptrons,1);
	}
	layer(const af::mxnxzmatrix<>& bkwndweight,const af::mxnxzmatrix<>& bias,
		  const activationfn actfn=activationfn::t_normtanh,
		  const activationnorm<T> actn=activationnorm<T>(),
		  const gradientclip<T> gc=gradientclip<T>()):m_GradClip(gc),m_ActFn(actfn),m_ActNorm(actn)
	{
		m_pPrev=nullptr;
		m_pNext=nullptr;
		m_BkwdWeight=bkwndweight;
		m_Bias=bias;
	}
	virtual ~layer() {}
	const layer<T>* getnext(void) const {return m_pNext;}
	const layer<T>* getprev(void) const {return m_pPrev;}
	int getparams(void) const { const int n = m_Bias.getrows() /*biases*/ + ( m_BkwdWeight.getcols() * m_Bias.getrows() );/*weights*/ return n; }
	int getperceptrons(void)const{return getbkwdweight().getrows();}
	int getprevperceptrons(void)const{return getbkwdweight().getcols();}
	const activationfn& getactivationfn(void)const{return m_ActFn;}
	const activationnorm<T>& getactivationnorm(void)const{return m_ActNorm;}
	__forceinline const af::mxnxzmatrix<T>& getbias(void) const { return m_Bias; }
	__forceinline const af::mxnxzmatrix<T>& getbkwdweight(void) const { return m_BkwdWeight; }
	const gradientclip<T>& getgradientclip(void)const {return m_GradClip;}
	__forceinline af::mxnxzmatrix_rand_val<T> getweightbiasrand(const activationfn::initialisetype t,const std::pair<T,T>& userrange) const
	{
		return m_ActFn.getweightbiasrand(t,m_BkwdWeight.getcols()/*prev perceptrons/input*/,getperceptrons(),userrange);
	}
	__forceinline successtype getactivation(af::mxnxzmatrix<T>& a,af::mxnxzmatrix<T>& z,const af::mxnxzmatrix<T>& prev,const int nZ=0,const int nPrevZ=0) const
	{
		// Z = ( W * prev.A ) + B
		// A = afn( Z )
		z.mul_hadamard_add(m_BkwdWeight,prev,m_Bias,0,nPrevZ,0,nZ);			// multiply weight and previous activation then addbias
		//z.mul(m_BkwdWeight,prev,0,nPrevZ,nZ).hadamard_add(m_Bias,0,nZ);	// multiply weight and previous activation then addbias
		m_ActFn.fn(a,z,nZ,nZ);												// activation
		m_ActNorm.fn(a,nZ);													// normalised activation
		return st_success;
	}
	__forceinline af::mxnxzmatrix<T>& getfodz(af::mxnxzmatrix<T>& dst,const af::mxnxzmatrix<T>& z,const int nDstZ=0,const int nZ=0) const{return m_ActFn.getfodz(dst,z,nDstZ,nZ); }
	__forceinline successtype getdelta(layerdeltascratch<T>& scratch,af::mxnxzmatrix<T>& w,af::mxnxzmatrix<T>& b,const af::mxnxzmatrix<T> *pNextB,const af::mxnxzmatrix<T> *pA,const af::mxnxzmatrix<T> *pPrevA,const af::mxnxzmatrix<T> *pZ,const af::mxnxzmatrix<T>& input,const af::mxnxzmatrix<T>& output,
										  const int nZ=0,const int nIOZ=0) const
	{
		// determine error terms which will allow us to
		// determine partial derivatives of weights/biases with respect to the cost
		// which will show us the gradient of the cost function
		// which are the negative of the deltas we require
	
		// typically i,j	-> ith layer jth perceptron
		// typically i,j,k	-> ith layer jth perceptron, kth perceptron in (i-1)th layer

		// last layer i = ( layers - 1 )
		// first intra/hidden layer i = 0 and took its input from training set item, this will have no prev

		// alloc
		scratch.zFOD.setrowscols(pA->getrows(),pA->getcols());
		
		// first order derivative
		getfodz(scratch.zFOD,*pZ,0,nZ);

		// case specific
		if(getnext())
		{
			// not last layer

			// error term e(i) = ( w(i+1)T . e(i+1) ) H(.) afn.fod(z(i))
			//					 transposed weights from next layer * error terms from next layer ( we calculated these the last time )
			//					 multiplied in a elementwise hadamard style with afn.fod(z(i))
			// 
			//					 we know error vector e(i+1), applying transpose of w(i+1) will move error backward giving a measure of error at ith layers output and
			//					 the hadamard operator will move error backward again to ith layers input
			b.mul_aT(getnext()->getbkwdweight(),*pNextB,0,nZ,nZ).hadamard_mul(scratch.zFOD,0,nZ);	// error term, use transposed 'a' term
			//b.mul_aT_hadamard_mul(getnext()->getbkwdweight(),*pNextB,scratch.zFOD,0,nZ,0,nZ);		// error term, use transposed 'a' term
		}
		else
		{
			// output/last layer

			// error term e(i,j) = dC/da(i,j) * afn.fod(z(i,j))
			// given cost c = 1/2 * ( y(i,j) - a(i,j) )^2 for the last layer, where 'y' are the desired values and 'a' are activation/output values
			//				= 2 * 1/2 * ( y(i,j) - a(i,j) ) * -1 -> chain rule
			//				= ( a(i,j) - y(i,j) )
			pA->copy_hadamard_sub_hadamard_mul(b,output,scratch.zFOD,nZ,nIOZ,0,nZ);
			//pA->copy(b,nZ,nZ).hadamard_sub(output,nIOZ,nZ);							// partial derivative
			//b.hadamard_mul(scratch.zFOD,0,nZ);										// error term
		}

		// partial derivative of bias dC/db(j) is just the error term e(j) - which is already in b

		// partial derivative of weights dC/dw(l,j,k) is a(l-1,k) * e(l,j) i.e. a(l-1) H(.) e(l)T
		const T *pPrevARow = getprev() ? pPrevA->get(nZ) : input.get(nIOZ);
		T *pWRow=w.get(nZ);
		const T *pE = b.get(nZ);
		for(int nRow=0;nRow<w.getrows();++nRow,pWRow+=w.getcols())
			for(int nCol=0;nCol<w.getcols();++nCol)
				pWRow[nCol]=pPrevARow[nCol]*pE[nRow];
		return st_success;
	}
	__forceinline void hadamard_sub(const af::mxnxzmatrix<double>& w,const af::mxnxzmatrix<double>& b,const int nWZ=0,const int nBZ=0)
	{
		m_BkwdWeight.hadamard_sub(w,nWZ);
		m_Bias.hadamard_sub(b,nBZ);
	}
	__forceinline void setbkwdweight(const af::mxnxzmatrixval<T>& d){ d.set(m_BkwdWeight); }
	__forceinline void setbkwdweight(const af::mxnxzmatrix<T>& m){ m_BkwdWeight=m; }
	__forceinline void setbias(const af::mxnxzmatrixval<T>& d){ d.set(m_Bias); }
	__forceinline void setbias(const af::mxnxzmatrix<T>& m){ m_Bias=m; }
	void setnext(const layer<T>* p){m_pNext=p;}
	void setprev(const layer<T>* p){m_pPrev=p;}
protected:
	// constructor
	layer(const gradientclip<T>& g,const activationfn& a,const activationnorm<T>& n):m_GradClip(g),m_ActFn(a),m_ActNorm(n){m_pPrev=nullptr;m_pNext=nullptr;}
	
	// architecture
	const layer<T> *m_pPrev;					// prev layer
	const layer<T> *m_pNext;					// next layer

	// param
	const gradientclip<T> m_GradClip;			// gradient clipping
	af::mxnxzmatrix<T> m_BkwdWeight;			// perceptron bkwd weights
												// entry in jth row and kth column is the connection weight between a layers jth perceptron and its previous layers kth perceptron
												// number of rows is the number of this layers perceptrons
												// number of columns is the number of previous layers perceptrons
												// a row is all the connections going to a perceptron in this layer
												// a column is all the connections leaving a perceptron in the previous layer
	af::mxnxzmatrix<T> m_Bias;					// perceptron biases in a column vector, when is a perceptron meaningfully activated

	// activation
	const activationfn m_ActFn;					// non linear activation function
	const activationnorm<T> m_ActNorm;			// activation value normalisation
};

template <typename T=double> class network
{
	// for large majority of problems one hidden layer is sufficient
	// 1-2 for simple
	// 3-5 for deep learning
	// for large majority of problems the mean of input/output neurons is sufficient
public:
	network(){m_spLayers=std::shared_ptr<std::vector<std::shared_ptr<layer<T>>>>(new std::vector<std::shared_ptr<layer<T>>>);}
	virtual ~network() {}
	bool isempty(void)const{return (!m_spLayers || !(*m_spLayers).size());}
	std::shared_ptr<const std::vector<std::shared_ptr<layer<T>>>> getlayers(void) const { return m_spLayers; }
	int getparams(void)const
	{
		int n = 0;
		if( m_spLayers )
		{
			const std::vector<std::shared_ptr<layer<T>>>& v = *m_spLayers;
			auto i = v.cbegin(), end = v.cend();
			for(; i != end; ++i)
				n += (*i)->getparams();
		}
		return n;
	}
	int getperceptrons(const bool bIncludeOutput)const
	{
		int n = 0;
		if( m_spLayers )
		{
			const std::vector<std::shared_ptr<layer<T>>>& v = *m_spLayers;
			auto i = v.cbegin(), end = v.cend();
			if(!bIncludeOutput)
				--end;
			for(; i != end; ++i)
				n += (*i)->getperceptrons();
		}
		return n;
	}
	void push_back(std::shared_ptr<layer<T>> p)
	{
		std::vector<std::shared_ptr<layer<T>>>& v = *m_spLayers;
		if(v.size())
		{
			v[v.size()-1]->setnext(p.get());
			p->setprev(v[v.size()-1].get());
		}
		m_spLayers->push_back(p);
	}
	__forceinline layer<T>::template successtype getactivation(std::vector<af::mxnxzmatrix<T>> *pA,std::vector<af::mxnxzmatrix<T>> *pZ,const af::mxnxzmatrix<T>& input,const int nZ=0,const int nInputZ=0) const
	{
		// forward propagate input through network
		auto iL = m_spLayers->begin(), end = m_spLayers->end();
		auto iA = pA->begin();
		auto iZ = pZ->begin();
		(*iL)->getactivation(*iA,*iZ,input,nZ,nInputZ);
		const af::mxnxzmatrix<T> *pPrevA = &(*iA);
		for(++iL,++iA,++iZ;iL!=end;++iL,++iA,++iZ)
		{
			const layer<T>::successtype st = (*iL)->getactivation(*iA,*iZ,*pPrevA,nZ,nZ);
			if(st&layer<T>::st_fail)return st;
			pPrevA = &(*iA);
		}
		return layer<T>::st_success;
	}
	__forceinline layer<T>::template successtype getdelta(layerdeltascratch<T>& scratch,std::vector<af::mxnxzmatrix<T>> *pW,std::vector<af::mxnxzmatrix<T>> *pB,const std::vector<af::mxnxzmatrix<T>> *pA,const std::vector<af::mxnxzmatrix<T>> *pZ,const int nZ,const af::mxnxzmatrix<T>& input,const af::mxnxzmatrix<T>& output,const int nIOZ) const
	{
		// back propagate cost of above training set
		auto iL=m_spLayers->crbegin(),end=m_spLayers->crend();
		auto iB=pB->rbegin();
		auto iW=pW->rbegin();
		auto iA = pA->crbegin();
		auto iZ = pZ->crbegin();
		const af::mxnxzmatrix<T> *pNextB = nullptr;
		if(m_spLayers->size()>1)
		{
			for(--end;iL!=end;++iL,++iB,++iW,++iA,++iZ)
			{
				const layer<T>::successtype st = (*iL)->getdelta(scratch,*iW,*iB,pNextB,&(*iA),&(*(iA+1)),&(*iZ),input,output,nZ,nIOZ); // layers with a prev layer
				if(st&layer<T>::st_fail)return st;
				pNextB = &(*iB);
			}
		}
		return (*iL)->getdelta(scratch,*iW,*iB,pNextB,&(*iA),nullptr,&(*iZ),input,output,nZ,nIOZ); // no prev layer
	}
	__forceinline void hadamard_sub(const int nL,const af::mxnxzmatrix<double>& w,const af::mxnxzmatrix<double>& b,const int nWZ=0,const int nBZ=0) const
	{
		(*m_spLayers)[nL]->hadamard_sub(w,b,nWZ,nBZ);
	}
protected:
	std::shared_ptr<std::vector<std::shared_ptr<layer<T>>>> m_spLayers;
};

template <typename T=double> class networktraining
{
public:
	enum gradientdescenttype {gdt_stochastic,				// process a training set item, update network, repeat
							  gdt_batch,					// process all training set items, update network with averages
							  gdt_minibatch};				// process subset of all training set items, update network with averages, repeat
	networktraining(std::shared_ptr<const network<T>> spN,const af::mxnxzmatrix<T>& input,const af::mxnxzmatrix<T>& output);
	virtual ~networktraining(){}
	std::shared_ptr<const network<T>> getnetwork(void)const{return m_spNetwork;}
	const af::mxnxzmatrix<T>& getinput(void)const{return m_Input;}
	const af::mxnxzmatrix<T>& getoutput(void)const{return m_Output;}
	std::shared_ptr<const std::vector<af::mxnxzmatrix<T>>> getactivation(void)const{return m_spActivation;}
	static void allocactivation( std::shared_ptr<const network<T>> spNetwork, const af::mxnxzmatrix<T>& mI, const af::mxnxzmatrix<T>& mO, std::shared_ptr<std::vector<af::mxnxzmatrix<T>>>& spA, std::shared_ptr<std::vector<af::mxnxzmatrix<T>>>& spZ, const int nZ=1 )
	{
		if( spNetwork && !spNetwork->isempty() && mI.getz() == mO.getz() && mI.getz() > 0 )
		{
			if(!spA || spA->size() != spNetwork->getlayers()->size())
			{
				spA = std::shared_ptr<std::vector<af::mxnxzmatrix<T>>>(new std::vector<af::mxnxzmatrix<T>>);
				spA->resize(static_cast<int>(spNetwork->getlayers()->size()));
			}
			if(!spZ || spZ->size() != spNetwork->getlayers()->size())
			{
				spZ = std::shared_ptr<std::vector<af::mxnxzmatrix<T>>>(new std::vector<af::mxnxzmatrix<T>>);
				spZ->resize(static_cast<int>(spNetwork->getlayers()->size()));
			}
			auto iSrc = spNetwork->getlayers()->cbegin(), end = spNetwork->getlayers()->cend();
			auto iDstA = spA->begin();
			auto iDstZ = spZ->begin();
			for(;iSrc!=end;++iSrc,++iDstA,++iDstZ)
			{
				(*iDstA).setrowscols((*iSrc)->getperceptrons(),1,nZ);
				(*iDstZ).setrowscols((*iSrc)->getperceptrons(),1,nZ);
			}
		}
	}
	double getcost(void)const;
	
	void getactivation(const afthread::taskscheduler *pScheduler,const af::vectorrange<true,true>& r);
	void setepochs(const int n){m_nEpochs=n;m_bInfiniteEpochs=false;}
	void setinfiniteepochs(const bool b){m_bInfiniteEpochs=b;}
	void settermcost(const T d){m_dTermCost=d;}
	void settermcostcheck(const bool b){m_bTermCostCheck=b;}
	void setvalidatefp(const bool b){m_bValidateFP=b;}
	void setgradientdescenttype(const gradientdescenttype t){m_gdt=t;}
	void setminibatchsize(const int n){m_nMiniBatch=n;}
	void setlearningrate(const T d){m_dLearningRate=d;}
	layer<T>::template successtype train(const afthread::taskscheduler *pScheduler);

	static void getactivation(const afthread::taskscheduler *pScheduler,const network<T> *pN,const af::mxnxzmatrix<T>& input,const af::vectorrange<true,true>& r,std::vector<af::mxnxzmatrix<T>> *pA,std::vector<af::mxnxzmatrix<T>> *pZ);
protected:
	networktraining(std::shared_ptr<const network<T>> spN)
	{
		// init
		m_spNetwork=spN;
		m_nEpochs = 50000;
		m_bInfiniteEpochs = false;
		m_gdt = gdt_batch;
		m_nMiniBatch = 0;
		m_dLearningRate = 0.01;
		m_dTermCost = 1e-6;
		m_bTermCostCheck = true;
		m_bValidateFP = true;
	}
	std::shared_ptr<const network<T>> m_spNetwork;
	af::mxnxzmatrix<T> m_Input;				// input values to be propogated through the network in a column vector
	af::mxnxzmatrix<T> m_Output;			// output values in a column vector, how do we want the network to perform
	std::shared_ptr<std::vector<af::mxnxzmatrix<T>>> m_spActivation;
	std::shared_ptr<std::vector<af::mxnxzmatrix<T>>> m_spZ;
	std::shared_ptr<std::vector<af::mxnxzmatrix<T>>> m_spWeightDelta;
	std::shared_ptr<std::vector<af::mxnxzmatrix<T>>> m_spBiasDelta;
	gradientdescenttype m_gdt;
	int m_nMiniBatch;
	T m_dLearningRate;
	T m_dTermCost;
	bool m_bTermCostCheck;
	int m_nEpochs;
	bool m_bInfiniteEpochs;
	bool m_bValidateFP;
	layer<T>::template successtype train_stochastic(const afthread::taskscheduler *pScheduler);
	layer<T>::template successtype train_batch(const afthread::taskscheduler *pScheduler);
	layer<T>::template successtype train_minibatch(const afthread::taskscheduler *pScheduler);
	void shuffle(std::default_random_engine& rng,const af::vectorrange<true,true>& r,std::shared_ptr<std::vector<int>> sp)
	{
		if(static_cast<int>(sp->size())<r.getsize())
			sp->resize(r.getsize());
		auto i = sp->begin();
		for(int n=0;n<r.getsize();++n,++i)
			*i=n+r.getfrom();

		af::vectorrange<true,true> v(0,r.getsize()-1);
		std::shuffle(v.getstdbegin(sp.get()),v.getstdend(sp.get()),rng);

		{
			auto i = sp->cbegin();
			for(int n=0;n<r.getsize();++n,++i)
			{
				m_Input.swapz(r.getfrom()+n,*i);
				m_Output.swapz(r.getfrom()+n,*i);
			}
		}
	}
	void allocweightbias( const int nZ )
	{
		std::shared_ptr<std::vector<af::mxnxzmatrix<T>>> spW, spB;
		if( m_spActivation )
		{
			spW = std::shared_ptr<std::vector<af::mxnxzmatrix<T>>>(new std::vector<af::mxnxzmatrix<T>>());
			spB = std::shared_ptr<std::vector<af::mxnxzmatrix<T>>>(new std::vector<af::mxnxzmatrix<T>>());
			spW->resize(static_cast<int>(m_spNetwork->getlayers()->size()));
			spB->resize(static_cast<int>(m_spNetwork->getlayers()->size()));
			auto iSrc = m_spNetwork->getlayers()->cbegin(), end = m_spNetwork->getlayers()->cend();
			auto iDstW = spW->begin();
			auto iDstB = spB->begin();
			for(;iSrc!=end;++iSrc,++iDstB,++iDstW)
			{
				(*iDstW).setrowscols((*iSrc)->getbkwdweight().getrows(),(*iSrc)->getbkwdweight().getcols(),nZ);
				(*iDstB).setrowscols((*iSrc)->getbias().getrows(),(*iSrc)->getbias().getcols(),nZ);
			}
		}
		m_spWeightDelta = spW;
		m_spBiasDelta = spB;
	}
	void getdelta(const afthread::taskscheduler *pScheduler,const af::vectorrange<true,true>& r);
	void processdelta(const afthread::taskscheduler *pScheduler,const af::vectorrange<true,true>& r);
	bool isactivationfpvalid(const af::vectorrange<true,true>& r) const;
};

class trainsetitem
{
public:
	enum inputtype{it_null=0x0,it_user=0x1,
				   it_image_i_id_o_b8g8r8=0x2,				// fixed dim input so output perceptrons known
				   it_image_i_id_xy_o_b8g8r8=0x4,			// max dim known to reduce training set
				   it_image=(it_image_i_id_o_b8g8r8|it_image_i_id_xy_o_b8g8r8),
				   it_all=(it_user|it_image)};
	static std::vector<inputtype> gettypes(void) {return {it_user,it_image_i_id_o_b8g8r8,it_image_i_id_xy_o_b8g8r8};}
	bool isempty(void)const{return m_Input.isempty()||m_Output.isempty();}
	inputtype gettype(void)const{return m_Type;}
	const af::mxnxzmatrix<>& getinput(void)const{return m_Input;}
	const af::mxnxzmatrix<>& getoutput(void)const{return m_Output;}

	virtual bool read(const serialise *pS);
	virtual bool write(const serialise *pS) const;

	void set(const af::mxnxzmatrix<>& i,const af::mxnxzmatrix<>& o){m_Input=i;m_Output=o;}
	void setinputrowcol(const int nR,const int nC,double d,const int nZ){m_Input.setrowcol(nR,nC,d,nZ);}
	void setoutputrowcol(const int nR,const int nC,double d,const int nZ){m_Output.setrowcol(nR,nC,d,nZ);}
	void push_back(const af::mxnxzmatrix<>& i,const af::mxnxzmatrix<>& o){m_Input.push_back(i);m_Output.push_back(o);}
	void eraseinput(const int nZ) {m_Input.erase(nZ);}
	void eraseoutput(const int nZ) {m_Output.erase(nZ);}

	trainsetitem& operator =(const trainsetitem& o){m_Type=o.m_Type;m_Input=o.m_Input;m_Output=o.m_Output;return *this;}
protected:
	trainsetitem():m_Type(it_user){}
	virtual ~trainsetitem(){}
	inputtype m_Type;
	af::mxnxzmatrix<> m_Input;
	af::mxnxzmatrix<> m_Output;
};

class traintsetimageitem : public trainsetitem
{
// image i/o pairs
public:
	enum reloadtype { rt_ok,rt_mismatch_err,rt_file_err};
	traintsetimageitem(){m_nID=0;}
	virtual ~traintsetimageitem(){}
	CString getpath(void)const{return m_csPath;}
	CString getfname(void) const;
	int getid(void)const{return m_nID;}
	int getdim(void) const{return m_nDim;}
	const SIZE& getdibdim(void)const{return m_szDibDim;}
	
	virtual bool read(const serialise *pS) override;
	virtual bool write(const serialise *pS) const override;
	reloadtype reload(LPCTSTR lpsz,const int nDim,const inputtype it);
	
	bool set(LPCTSTR lpsz,const int nDim,const inputtype it);
	bool setid(const int nID);
	bool setdim(const int nDim);
	
	traintsetimageitem& operator =(const traintsetimageitem& o){trainsetitem::operator=(o);	m_csPath=o.m_csPath;m_szDibDim=o.m_szDibDim;m_pt=o.m_pt;m_nID=o.m_nID;m_nDim=o.m_nDim;return *this;}

	static void getrectscale(const double dSrcTLX,const double dSrcTLY,const double dSrcBRX,const double dSrcBRY,
							 const double dDstTLX,const double dDstTLY,const double dDstBRX,const double dDstBRY,
							 const bool bLetterBox,double& dS);
	static std::shared_ptr<afdib::dib> getdib(const af::mxnxzmatrix<>& m,const int nZOffset,const inputtype it,const SIZE& szDibDim,const RECT& rcClip,std::shared_ptr<afdib::dib> spFrom=nullptr);
	static std::shared_ptr<afdib::dib> loadtrnsdib(LPCTSTR lpsz,const int nDim,const inputtype it);
protected:
	CString m_csPath;
	afdib::dib::pixeltype m_pt;
	SIZE m_szDibDim;
	int m_nID;
	int m_nDim;
	bool traintsetimageitem::set(LPCTSTR lpsz,const int nDim,const inputtype it,std::shared_ptr<afdib::dib> spTrnsDib);
};

class traintsetuseritem : public trainsetitem
{
// user i/o pairs
public:
	traintsetuseritem(){}
	virtual ~traintsetuseritem(){}

	virtual bool read(const serialise *pS) override;
	virtual bool write(const serialise *pS) const override;

	void setinputoutput(const int nI,const int nO)
	{
		af::mxnxzmatrix<> i;
		i.setrowscols(nI,1,getinput().getz());
		for(int nZ=0;nZ<i.getz();++nZ)
		{
			i.set(0,nZ);
			int nRows = std::min<int>(i.getrows(),getinput().getrows());
			int nCols = std::min<int>(i.getcols(),getinput().getcols());
			for(int nR=0;nR<nRows;++nR)
				for(int nC=0;nC<nCols;++nC)
					i.setrowcol(nR,nC,getinput().get(nR,nC,nZ),nZ);
		}
		af::mxnxzmatrix<> o;
		o.setrowscols(nO,1,getoutput().getz());
		for(int nZ=0;nZ<o.getz();++nZ)
		{
			o.set(0,nZ);
			int nRows = std::min<int>(o.getrows(),getoutput().getrows());
			int nCols = std::min<int>(o.getcols(),getoutput().getcols());
			for(int nR=0;nR<nRows;++nR)
				for(int nC=0;nC<nCols;++nC)
					o.setrowcol(nR,nC,getoutput().get(nR,nC,nZ),nZ);
		}
		set(i,o);
	}

	traintsetuseritem& operator =(const traintsetuseritem& o){trainsetitem::operator=(o);return *this;}
};

class inputtypeitem
{
public:
	inputtypeitem()
	{
		bTrainingInitialised=false;
		initType=activationfn::it_auto_normal;
		userRandomRange=std::make_pair(-0.5,0.5);
		nEpochs=-5000;
		it=trainsetitem::it_user;
		dLearningRate = 1;
		dTermCost=1e-6;
		bTermCostCheck=false;
		bValidateFP=false;
		gdt=networktraining<>::gdt_batch;
		nMiniBatch=100;
		nEpoch=0;
		minepoch=std::make_pair(0,0);
		maxepoch=std::make_pair(0,0);
		nImageDim=60;
	}
	inputtypeitem(const traintsetimageitem::inputtype t):inputtypeitem(){it=t;dLearningRate = (t==traintsetimageitem::it_user) ? 0.9 : 0.85;}

	bool read(const serialise *pS);
	bool write(const serialise *pS) const;

	trainsetitem::inputtype getinputtype(void)const{return it;}

	int getepoch(void)const{return nEpoch;}
	void setepoch(const int n){nEpoch=n;}
	void clearepoch(void)
	{
		setepoch(0);
		settraininginitialised(false);
		setminepoch(std::make_pair(0,0));
		setmaxepoch(std::make_pair(0,0));
	}

	bool gettraininginitialised(void)const{return bTrainingInitialised;}
	void settraininginitialised(const bool b){bTrainingInitialised=b;}

	const std::vector<std::shared_ptr<trainsetitem>>& gettraining(void)const{return vTraining;}
	void settraining(const std::vector<std::shared_ptr<trainsetitem>>& v){vTraining=v;}

	const std::vector<std::shared_ptr<layer<>>>& getlayers(void)const{return vLayers;}
	void setlayers(const std::vector<std::shared_ptr<layer<>>>& v){vLayers=v;}

	activationfn::initialisetype getinittype(void)const{return initType;}
	void setinittype(const activationfn::initialisetype t){initType=t;}

	const std::pair<double,double>& getuserrandrange(void)const{return userRandomRange;}
	void setuserrandrange(const std::pair<double,double>& r){userRandomRange=r;}

	int getepochs(void)const{return nEpochs;}
	void setepochs(const int n){nEpochs=n;}

	double getlearningrate(void)const{return dLearningRate;}
	void setlearningrate(const double d){dLearningRate=d;}

	const std::pair<double,int>& getminepoch(void)const{return minepoch;}
	void setminepoch(const std::pair<double,int>& e){minepoch=e;}

	const std::pair<double,int>& getmaxepoch(void)const{return maxepoch;}
	void setmaxepoch(const std::pair<double,int>& e){maxepoch=e;}

	int getimagedim(void)const{return nImageDim;}
	void setimagedim(const int n){nImageDim=n;}

	void pushback_epoch(const std::pair<double,int>& c)
	{
		if(minepoch.second==0 || c.first<minepoch.first)
			minepoch=c;
		if(maxepoch.second==0 || c.first>maxepoch.first)
			maxepoch=c;
	}
	
	bool gettermcostcheck(double& d)const { d=dTermCost;return bTermCostCheck;}

	bool getvalidatefp(void)const{return bValidateFP;}

	networktraining<>::gradientdescenttype getgraddescenttype(void)const{return gdt;}

	int getminibatch(void)const{return nMiniBatch;}

	std::shared_ptr<afml::networktraining<>> createtraining(std::shared_ptr<const afml::network<>> spN)const
	{
		af::mxnxzmatrix<> mI,mO;
		auto i=gettraining().cbegin(),end=gettraining().cend();
		for(;i!=end;++i)
		{
			mI.push_back((*i)->getinput());
			mO.push_back((*i)->getoutput());
		}
		std::shared_ptr<afml::networktraining<>> spTrain(new afml::networktraining<>(spN,mI,mO));
		spTrain->setlearningrate(getlearningrate());
		double dTermCostCheck;
		bool bTermCostCheck = gettermcostcheck(dTermCostCheck);
		spTrain->settermcostcheck(gettermcostcheck(dTermCostCheck));
		spTrain->settermcost(dTermCostCheck);
		spTrain->setvalidatefp(getvalidatefp());
		spTrain->setgradientdescenttype(getgraddescenttype());
		spTrain->setminibatchsize(getminibatch());
		
		spTrain->setepochs(1);
		spTrain->setinfiniteepochs(false);

		return spTrain;
	}

	std::shared_ptr<afml::network<>> createnetwork(void)
	{
		af::srand(0);

		std::shared_ptr<afml::network<>> spN = std::shared_ptr<afml::network<>>(new afml::network<>);
		auto i=getlayers().cbegin(),end=getlayers().cend();
		for(;i!=end;++i)
		{
			if(!gettraininginitialised())
			{
				const auto weightbiasinit = (*i)->getweightbiasrand(getinittype(),getuserrandrange());
				(*i)->setbkwdweight(weightbiasinit);
				(*i)->setbias(weightbiasinit);
			}
			spN->push_back(*i);
		}
		settraininginitialised(true);
		return spN;
	}

	inputtypeitem& operator =(const inputtypeitem& o)
	{
		bTrainingInitialised=o.bTrainingInitialised;
		vTraining=o.vTraining;
		vLayers=o.vLayers;
		dLearningRate=o.dLearningRate;
		dTermCost=o.dTermCost;
		bTermCostCheck=o.bTermCostCheck;
		bValidateFP=o.bValidateFP;
		gdt=o.gdt;
		nMiniBatch=o.nMiniBatch;
		it=o.it;
		initType=o.initType;
		userRandomRange=o.userRandomRange;
		nEpochs=o.nEpochs;
		nEpoch=o.nEpoch;
		minepoch=o.minepoch;
		maxepoch=o.maxepoch;
		nImageDim=o.nImageDim;
		return *this;
	}
protected:
	bool bTrainingInitialised;
	std::vector<std::shared_ptr<trainsetitem>> vTraining;
	std::vector<std::shared_ptr<layer<>>> vLayers;
	double dLearningRate;
	double dTermCost;
	bool bTermCostCheck;
	bool bValidateFP;
	networktraining<>::gradientdescenttype gdt;
	int nMiniBatch;
	int nEpoch;
	int nEpochs;
	trainsetitem::inputtype it;
	activationfn::initialisetype initType;
	std::pair<double,double> userRandomRange;
	std::pair<double,int> minepoch;
	std::pair<double,int> maxepoch;
	int nImageDim;
};

}

#include "ml_network.inl"
