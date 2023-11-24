
#pragma once

#include <AfxMt.h>
#include <AfxOle.h>
#include <vector>

class winthread : public CWinThread
{

DECLARE_DYNAMIC(winthread)

public:
	winthread();
	virtual ~winthread();

	bool isrunning(void) const;
	bool stoppending(void) const;

	virtual void stop(const DWORD dwTimeOut);

	static bool signalled( const HANDLE h, const DWORD dwTimeOut );
	static bool signalled( const std::vector<HANDLE>& h, const BOOL bWaitAll, const DWORD dwTimeOut, int& nSignalled);
protected:	
	CEvent *m_pStop;
};
