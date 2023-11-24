
#include "pch.h"
#include "winthread_ml.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(winthread, CWinThread)

winthread::winthread()
{
	m_bAutoDelete = FALSE;
	m_pStop = new CEvent(FALSE,TRUE);
}

winthread::~winthread()
{
	stop(INFINITE);
	delete m_pStop;
}

void winthread::stop(const DWORD dwTimeOut)
{
	bool bRetVal = true;
	if(m_hThread)
	{
		m_pStop->SetEvent();
		bRetVal = signalled(m_hThread,dwTimeOut);
		m_hThread = nullptr;
	}
	m_pStop->ResetEvent();
	m_pMainWnd = NULL;
}

bool winthread::signalled(const HANDLE h,const DWORD dwTimeOut)
{
	if( !h )
		return false;
	const DWORD dwWait = ::WaitForSingleObject(h,dwTimeOut);
	if(dwWait == WAIT_OBJECT_0)
		return true;
	return false;
}

bool winthread::signalled( const std::vector<HANDLE>& h, const BOOL bWaitAll, const DWORD dwTimeOut, int& nSignalled)
{
	if(!h.size())
		return false;
	const DWORD dwWait = ::WaitForMultipleObjects(DWORD(h.size()), &h[0], bWaitAll, dwTimeOut);
	if((dwWait >= WAIT_OBJECT_0) && (dwWait <= (WAIT_OBJECT_0+h.size()-1)))
	{
		nSignalled = (dwWait-WAIT_OBJECT_0);
		return true;
	}
	return false;
}

bool winthread::isrunning(void) const
{
	return m_hThread && !signalled(m_hThread,0);
}

bool winthread::stoppending(void) const
{
	return m_pStop && signalled(m_pStop->m_hObject,0);
}
