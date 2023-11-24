
#include "ml_network.h"

namespace afml
{

template <typename T>
networktraining<T>::networktraining(std::shared_ptr<const network<T>> spN,const af::mxnxzmatrix<T>& input,const af::mxnxzmatrix<T>& output):networktraining<T>(spN)
{
	// init
	m_Input=input;
	m_Output=output;
	ASSERT(input.getz()==output.getz());
	m_spNetwork = spN;
	m_nMiniBatch = std::max<int>(1,af::posround<double,int>(input.getz()*(1/6.0)));
	allocactivation(m_spNetwork,m_Input,m_Output,m_spActivation,m_spZ,m_Input.getz());
}

template <typename T>
layer<T>::template successtype networktraining<T>::train(const afthread::taskscheduler *pScheduler)
{
	// initialised?
	layer<T>::template successtype sc = layer<T>::st_error;
	if(!m_spNetwork || m_spNetwork->isempty() || !m_spActivation || m_spActivation->size()==0)
		return sc;
	switch(m_gdt)
	{
		case gdt_stochastic:sc = train_stochastic(pScheduler);break;
		case gdt_batch:sc = train_batch(pScheduler);break;
		case gdt_minibatch:sc = train_minibatch(pScheduler);break;
		default:ASSERT(false);break;
	}
	return sc;
}

template <typename T>
layer<T>::template successtype networktraining<T>::train_stochastic(const afthread::taskscheduler *pScheduler)
{
	// train - process a test set item, update network, repeat
	layer<T>::template successtype sc=layer<T>::st_success;
	const int nTrainingItems = m_Input.getz();
	allocweightbias(nTrainingItems); // wasteful only really need 1
	T dAvgTrainingItemCost = 0;
	for(int nEpoch = 0;nEpoch<m_nEpochs;++nEpoch)
	{
		for(int n=0;n<nTrainingItems;++n)
		{
			const af::vectorrange<true,true> r(n,n);
			getactivation(pScheduler,r);
			if(m_bValidateFP && !isactivationfpvalid(r))
			{
				sc=layer<T>::st_fail;
				break;
			}
			getdelta(pScheduler,r);
			processdelta(pScheduler,r);
		}
		
		if( m_bTermCostCheck )
		{
			const af::vectorrange<true,true> r(0,nTrainingItems-1);
			getactivation(pScheduler,r);
			dAvgTrainingItemCost = getcost();
		}
		if(m_bTermCostCheck && dAvgTrainingItemCost<=m_dTermCost)
			break;
	}
	return sc;
}

template <typename T>
layer<T>::template successtype networktraining<T>::train_batch(const afthread::taskscheduler *pScheduler)
{
	// train - process all test set items, update network with averages
	layer<T>::template successtype sc=layer<T>::st_success;
	const int nTrainingItems = m_Input.getz();
	allocweightbias(nTrainingItems);
	T dAvgTrainingItemCost = 0;
	for(int nEpoch = 0;m_bInfiniteEpochs ? true : ( nEpoch<m_nEpochs);++nEpoch)
	{
		// activations
		const af::vectorrange<true,true> r(0,nTrainingItems-1);
		getactivation(pScheduler,r);
		if(m_bValidateFP && !isactivationfpvalid(r))
		{
			sc=layer<T>::st_fail;
			break;
		}
		dAvgTrainingItemCost = ( m_bTermCostCheck ) ? getcost() : dAvgTrainingItemCost;
		if(m_bTermCostCheck && dAvgTrainingItemCost<=m_dTermCost)
			break;

		// train
		getdelta(pScheduler,r);
		processdelta(pScheduler,r);
	}
	return sc;
}

template <typename T>
layer<T>::template successtype networktraining<T>::train_minibatch(const afthread::taskscheduler *pScheduler)
{
	// train - process test set items in a batch, update network with batch averages, repeat
	layer<T>::template successtype sc=layer<T>::st_success;
	const int nTrainingItems = m_Input.getz();
	const int nMiniBatchSize = std::min<int>(m_nMiniBatch,nTrainingItems);
	const int nMiniBatchWholeIntervals = nTrainingItems / nMiniBatchSize;
	const int nTrailingMiniBatchSize = nTrainingItems - ( nMiniBatchWholeIntervals * nMiniBatchSize );
	allocweightbias(nTrainingItems); // wasteful only really need mini batch size
	std::shared_ptr<std::vector<int>> spShuffled(new std::vector<int>());
	auto rng = std::default_random_engine {};
	T dAvgTrainingItemCost = 0;
	for(int nEpoch = 0;nEpoch<m_nEpochs;++nEpoch)
	{
		if(nMiniBatchWholeIntervals)
			for(int n=0;n<nMiniBatchWholeIntervals;++n)
			{
				const af::vectorrange<true,true> r(n*nMiniBatchSize,(n*nMiniBatchSize)+nMiniBatchSize-1);
				shuffle(rng,r,spShuffled);
				getactivation(pScheduler,r);
				if(m_bValidateFP && !isactivationfpvalid(r))
				{
					sc=layer<T>::st_fail;
					break;
				}
				getdelta(pScheduler,r);
				processdelta(pScheduler,r);
			}
		if( nTrailingMiniBatchSize > 0 && !(sc&layer<T>::st_fail) )
		{
			const af::vectorrange<true,true> r(nTrainingItems-nTrailingMiniBatchSize,nTrainingItems-1);
			shuffle(rng,r,spShuffled);
			getactivation(pScheduler,r);
			if(m_bValidateFP && !isactivationfpvalid(r))
			{
				sc=layer<T>::st_fail;
				break;
			}
			getdelta(pScheduler,r);
			processdelta(pScheduler,r);
		}

		if( m_bTermCostCheck )
		{
			const af::vectorrange<true,true> r(0,nTrainingItems-1);
			getactivation(pScheduler,r);
			dAvgTrainingItemCost = getcost();
		}
		if(m_bTermCostCheck && dAvgTrainingItemCost<=m_dTermCost)
			break;
	}
	return sc;
}

template <typename T>
class getactivationtask
{
public:
	getactivationtask(const network<T> *pN,const af::mxnxzmatrix<T>& input,std::vector<af::mxnxzmatrix<T>> *pActivations,std::vector<af::mxnxzmatrix<T>> *pZ):
	m_pN(pN),m_Input(input),m_pActivations(pActivations),m_pZ(pZ){}
	void operator()( const int nIndex, const afthread::taskinfo *pInfo = nullptr ) const
	{
		operator()( nIndex, nIndex, pInfo );
	}
	void operator()( const int nFrom, const int nTo, const afthread::taskinfo *pInfo = nullptr ) const
	{
		for( int n = nFrom ; n <= nTo ; ++n )
			m_pN->getactivation(m_pActivations,m_pZ,m_Input,n,n);
	}
protected:
	const network<T> *m_pN;
	const af::mxnxzmatrix<T>& m_Input;
	std::vector<af::mxnxzmatrix<T>> *m_pActivations;
	std::vector<af::mxnxzmatrix<T>> *m_pZ;
};

template <typename T>
void networktraining<T>::getactivation(const afthread::taskscheduler *pScheduler,const network<T> *pN,const af::mxnxzmatrix<T>& input,const af::vectorrange<true,true>& r,std::vector<af::mxnxzmatrix<T>> *pA,std::vector<af::mxnxzmatrix<T>> *pZ)
{
	// calculate the activations for the training set items in the range 'r'
	if( pScheduler )
		pScheduler->parallel_for( r.getfrom(), r.getsize(), pScheduler->getcores(), getactivationtask<T>(pN,input,pA,pZ), nullptr);
	else
		getactivationtask<T>(pN,input,pA,pZ)( r.getfrom(), r.getinclusiveto());
}

template <typename T>
void networktraining<T>::getactivation(const afthread::taskscheduler *pScheduler,const af::vectorrange<true,true>& r)
{
	// calculate the activations for the training set items in the range 'r'
	getactivation(pScheduler,m_spNetwork.get(),m_Input,r,m_spActivation.get(),m_spZ.get());
}

template <typename T>
bool networktraining<T>::isactivationfpvalid(const af::vectorrange<true,true>& r) const
{
	const bool bDenormalNormal = true;
	auto i = m_spActivation->cbegin(), end = m_spActivation->cend();
	for(;i!=end;++i)
		for(int nZ=r.getfrom();nZ<=r.getinclusiveto();++nZ)
		if(!(*i).fpvalid(bDenormalNormal,nZ))
			return false;
	return true;
}

template <typename T>
class getdeltatask
{
public:
	getdeltatask(const network<T> *pN,const af::mxnxzmatrix<T>& input,const af::mxnxzmatrix<T>& output,std::vector<af::mxnxzmatrix<T>> *pWeight,std::vector<af::mxnxzmatrix<T>> *pBias,const std::vector<af::mxnxzmatrix<T>> *pActivations,const std::vector<af::mxnxzmatrix<T>> *pZ):
	m_pN(pN),m_Input(input),m_Output(output),m_pActivations(pActivations),m_pZ(pZ),m_pWeight(pWeight),m_pBias(pBias){}
	void operator()( const int nIndex, const afthread::taskinfo *pInfo = nullptr ) const
	{
		operator()( nIndex, nIndex, pInfo );
	}
	void operator()( const int nFrom, const int nTo, const afthread::taskinfo *pInfo = nullptr ) const
	{
		layerdeltascratch<T> scratch;
		for( int n = nFrom ; n <= nTo ; ++n )
			m_pN->getdelta(scratch,m_pWeight,m_pBias,m_pActivations,m_pZ,n,m_Input,m_Output,n);
	}
protected:
	const network<T> *m_pN;
	const af::mxnxzmatrix<T>& m_Input;
	const af::mxnxzmatrix<T>& m_Output;
	std::vector<af::mxnxzmatrix<T>> *m_pWeight;
	std::vector<af::mxnxzmatrix<T>> *m_pBias;
	const std::vector<af::mxnxzmatrix<T>> *m_pActivations;
	const std::vector<af::mxnxzmatrix<T>> *m_pZ;
};

template <typename T>
void networktraining<T>::getdelta(const afthread::taskscheduler *pScheduler,const af::vectorrange<true,true>& r)
{
	// calculate the deltas for the training set items in the range 'r'
	if( pScheduler )
		pScheduler->parallel_for( r.getfrom(), r.getsize(), pScheduler->getcores(), getdeltatask<T>(m_spNetwork.get(),m_Input,m_Output,m_spWeightDelta.get(),m_spBiasDelta.get(),m_spActivation.get(),m_spZ.get()), nullptr );
	else
		getdeltatask<T>(m_spNetwork.get(),m_Input,m_Output,m_spWeightDelta.get(),m_spBiasDelta.get(),m_spActivation.get(),m_spZ.get())(r.getfrom(), r.getinclusiveto());
}

template <typename T>
class processdeltasumztask
{
public:
	processdeltasumztask(af::mxnxzmatrix<T>& dstW,const af::mxnxzmatrix<T>& srcW,af::mxnxzmatrix<T>& dstB,const af::mxnxzmatrix<T>& srcB,const int nParallelSum):m_dstW(dstW),m_srcW(srcW),m_dstB(dstB),m_srcB(srcB),m_nParallelSum(nParallelSum)
	{
		ASSERT(m_srcW.getz()==m_srcB.getz());
	}
	void operator()( const int nIndex, const afthread::taskinfo *pInfo = nullptr ) const
	{
		operator()( nIndex, nIndex, pInfo );
	}
	void operator()( const int nFrom, const int nTo, const afthread::taskinfo *pInfo = nullptr ) const
	{
		for(int n = nFrom;n<=nTo;++n)
		{
			const af::vectorrange<true,true> r(m_nParallelSum*nFrom,
											   ((m_nParallelSum*nFrom)+m_nParallelSum-1) >= m_srcW.getz() ? (m_srcW.getz()-1) : ((m_nParallelSum*nFrom)+m_nParallelSum-1));
			m_srcW.sumz(m_dstW,n,r);
			m_srcB.sumz(m_dstB,n,r);
		}
	}
protected:
	af::mxnxzmatrix<T>& m_dstW;
	const af::mxnxzmatrix<T>& m_srcW;
	af::mxnxzmatrix<T>& m_dstB;
	const af::mxnxzmatrix<T>& m_srcB;
	const int m_nParallelSum;
};

template <typename T>
class processdeltatask
{
public:
	processdeltatask(const afthread::taskscheduler *pScheduler,const network<T> *pN,const std::vector<af::mxnxzmatrix<T>> *pWeight,const std::vector<af::mxnxzmatrix<T>> *pBias,const af::vectorrange<true,true>& zs,const double dScale):
	m_pN(pN),m_pWeight(pWeight),m_pBias(pBias),m_dScale(dScale),m_Zs(zs),m_pScheduler(pScheduler){}
	void operator()( const int nIndex, const afthread::taskinfo *pInfo = nullptr ) const
	{
		operator()( nIndex, nIndex, pInfo );
	}
	void operator()( const int nFrom, const int nTo, const afthread::taskinfo *pInfo = nullptr ) const
	{		
		auto iL = m_pN->getlayers()->cbegin()+nFrom;
		auto iW = m_pWeight->cbegin()+nFrom;
		auto iB = m_pBias->cbegin()+nFrom;
		const int nZs = (*iW).getz();
		const int nCores = m_pScheduler->getcores();
		const int nParallelSum = 1800;
		const bool bParallelSum = nZs > nParallelSum;// && false;
		int nParallelSums = 0;
		{
			const int nWhole = nZs / nParallelSum;
			nParallelSums += nWhole;
			const int nRemaining = nZs - ( nWhole * nParallelSum );
			if(nRemaining)
				++nParallelSums;
		}
		af::mxnxzmatrix<T> dstW;
		af::mxnxzmatrix<T> dstB;
		for( int n = nFrom ; n <= nTo ; ++n, ++iW, ++iB, ++iL )
		{
			// alloc
			if(bParallelSum)
			{
				dstW.setrowscols((*iW).getrows(),(*iW).getcols(),nParallelSums);
				dstB.setrowscols((*iB).getrows(),(*iB).getcols(),nParallelSums);
			}
			else
			{
				dstW.setrowscols((*iW).getrows(),(*iW).getcols());
				dstB.setrowscols((*iB).getrows(),(*iB).getcols());
			}

			// sum
			ASSERT((*iB).getz()==nZs);
			ASSERT((*iW).getz()==nZs);
			if(bParallelSum)
			{
				if( m_pScheduler )
					m_pScheduler->parallel_for( 0, nParallelSums, m_pScheduler->getcores(), processdeltasumztask<T>(dstW,*iW,dstB,*iB,nParallelSum), nullptr);
				else
					processdeltasumztask<T>(dstW,*iW,dstB,*iB,nParallelSum)(0, nParallelSums - 1);
				dstW.sumz();
				dstB.sumz();
			}
			else
			{
				(*iW).sumz(dstW,0,m_Zs);
				(*iB).sumz(dstB,0,m_Zs);
			}

			// scale
			dstW.mul(m_dScale);
			dstB.mul(m_dScale);

			// clip
			(*iL)->getgradientclip().fn(dstW,dstB);

			// update layer
			m_pN->hadamard_sub(n,dstW,dstB);
		}
	}
protected:
	const afthread::taskscheduler *m_pScheduler;
	const network<T> *m_pN;
	const std::vector<af::mxnxzmatrix<T>> *m_pWeight;
	const std::vector<af::mxnxzmatrix<T>> *m_pBias;
	const af::vectorrange<true,true>& m_Zs;
	const double m_dScale;
};

template <typename T>
void networktraining<T>::processdelta(const afthread::taskscheduler *pScheduler,const af::vectorrange<true,true>& r)
{
	// calculate the delta sums for the training set items in the range 'r', sums go into z=0
	const double dScale = m_dLearningRate/r.getsize();
	const af::vectorrange<true,true> normr(0,r.getsize()-1);
	if( pScheduler )
		pScheduler->parallel_for( 0, int(m_spWeightDelta->size()), pScheduler->getcores(), processdeltatask<T>(pScheduler,m_spNetwork.get(),m_spWeightDelta.get(),m_spBiasDelta.get(),normr,dScale), nullptr);
	else
		processdeltatask<T>(pScheduler,m_spNetwork.get(),m_spWeightDelta.get(),m_spBiasDelta.get(),normr,dScale)(0, int(m_spWeightDelta->size()) - 1);
}

template <typename T>
double networktraining<T>::getcost(void)const
{
	// Cx = 1/2 * sum( ( a - c ) * ( a - c ) )
	//af::mxnxzmatrix<T> tmp(m_Output.getrows(),m_Output.getcols());
	af::mxnxzmatrix<T> s(m_Output.getrows(),m_Output.getcols(),0.0);
	const af::mxnxzmatrix<T>& a = (*m_spActivation)[m_spActivation->size()-1];
	for( int nZ = 0 ; nZ < m_Output.getz() ; ++nZ)
	{
		s.hadamard_add_subsq(a,m_Output,nZ,nZ);
		//a.copy(tmp,0,nZ);
		//tmp.hadamard_sub(m_Output,nZ);
		//tmp.hadamard_mul(tmp);
		//s.hadamard_add(tmp);
	}
	const double  dC = s.getsum();
	return dC * (1.0/(2.0*m_Output.getz())); // incorporate the 1/2 term here
}

}
