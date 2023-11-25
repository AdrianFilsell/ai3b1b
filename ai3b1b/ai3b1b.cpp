
// ai3b1b.cpp : Defines the class behaviors for the application.
//

#include "pch.h"
#include "framework.h"
#include "ai3b1b.h"
#include "ai3b1bDlg.h"
#include "trainpanelwnd.h"
#include "traincostwnd.h"
#include "trainimagewnd.h"
#include "serialise.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Cai3b1bApp

BEGIN_MESSAGE_MAP(Cai3b1bApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// Cai3b1bApp construction

Cai3b1bApp::Cai3b1bApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_spScheduler=std::shared_ptr<afthread::taskscheduler>(new afthread::taskscheduler());
	std::vector<afml::trainsetitem::inputtype> v = afml::trainsetitem::gettypes();
	auto i=v.cbegin(),end=v.cend();
	for(;i!=end;++i)
		m_mInputs[*i]=std::shared_ptr<afml::inputtypeitem>(new afml::inputtypeitem(*i));
}


// The one and only Cai3b1bApp object

Cai3b1bApp theApp;


// Cai3b1bApp initialization

BOOL Cai3b1bApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	// dlg
	Cai3b1bDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
		TRACE(traceAppMsg, 0, "Warning: if you are using MFC controls on the dialog, you cannot #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS.\n");
	}

	// thread
	m_spThread=nullptr;
	
	// Delete the shell manager created above.
	if (pShellManager != nullptr)
	{
		delete pShellManager;
	}

#if !defined(_AFXDLL) && !defined(_AFX_NO_MFC_CONTROLS_IN_DIALOGS)
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

Cai3b1bDlg *Cai3b1bApp::getdlg(void)const
{
	return static_cast<Cai3b1bDlg*>(m_pMainWnd);
}

layerdlg *Cai3b1bApp::getlayerdlg(void) const
{
	return getdlg()&&getdlg()->gettabctrl()?getdlg()->gettabctrl()->getlayerdlg():nullptr;
}

traindlg *Cai3b1bApp::gettraindlg(void) const
{
	return getdlg()&&getdlg()->gettabctrl()?getdlg()->gettabctrl()->gettraindlg():nullptr;
}

trainpanelwnd *Cai3b1bApp::getpanelwnd(void) const
{
	return m_spPanelWnd.get();
}

traincostwnd *Cai3b1bApp::getcostwnd(void) const
{
	return m_spCostWnd.get();
}

trainimagewnd *Cai3b1bApp::getimagewnd(void) const
{
	return m_spImageWnd.get();
}

trainpaneldlg *Cai3b1bApp::getpaneldlg(void) const
{
	return getpanelwnd()?getpanelwnd()->getdlg():nullptr;
}

traincostdlg *Cai3b1bApp::getcostdlg(void) const
{
	return getcostwnd()?getcostwnd()->getdlg():nullptr;
}

trainimagedlg *Cai3b1bApp::getimagedlg(void) const
{
	return getimagewnd()?getimagewnd()->getdlg():nullptr;
}

void Cai3b1bApp::enableui(const bool b)
{
	if(getdlg())
		getdlg()->EnableWindow(b);
	if(getlayerdlg())
		getlayerdlg()->EnableWindow(b);
	if(gettraindlg())
		gettraindlg()->EnableWindow(b);
	if(getpaneldlg())
		getpaneldlg()->EnableWindow(b);
	if(getcostdlg())
		getcostdlg()->EnableWindow(b);
	if(getimagedlg())
		getimagedlg()->EnableWindow(b);
}

void Cai3b1bApp::broadcasthint(const hint *p)
{
	onhint(p);
	if(getdlg())
		getdlg()->onhint(p);
	if(getlayerdlg())
		getlayerdlg()->onhint(p);
	if(gettraindlg())
		gettraindlg()->onhint(p);
	if(getpaneldlg())
		getpaneldlg()->onhint(p);
	if(getcostdlg())
		getcostdlg()->onhint(p);
	if(getimagedlg())
		getimagedlg()->onhint(p);
}

void Cai3b1bApp::openpanelui(void)
{
	// already visible
	if(m_spPanelWnd)
		return;

	m_spPanelWnd=std::shared_ptr<trainpanelwnd>(new trainpanelwnd);
	const CSize sz(500,100);
	
	const BOOL b = m_spPanelWnd->CreateEx(WS_EX_CONTROLPARENT|WS_EX_TOOLWINDOW,AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW)),_T("Control Panel"),WS_POPUPWINDOW|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|MFS_MOVEFRAME|MFS_SYNCACTIVE,CRect(CPoint(0,0),sz),nullptr,0);
	CRect rcWindow, rcClient;
	m_spPanelWnd->GetWindowRect(rcWindow);
	m_spPanelWnd->GetClientRect(rcClient);
	const CSize szDelta(rcWindow.Width()-rcClient.Width(),rcWindow.Height()-rcClient.Height()); 

	trainpaneldlg *pDlg = m_spPanelWnd->getdlg();
	CRect rcDlgWindow;
	pDlg->GetWindowRect(rcDlgWindow);
	rcWindow = CRect(rcWindow.TopLeft(),CSize(rcDlgWindow.Width(),rcDlgWindow.Height())+szDelta);

	anchorui(rcWindow,theApp.GetMainWnd(),{},{at_right,at_left,at_bottom,at_top});

	m_spPanelWnd->MoveWindow(rcWindow);
	m_spPanelWnd->ShowWindow(SW_NORMAL);
	broadcasthint(&hint(hint::t_widget));
}

void Cai3b1bApp::opencostui(void)
{
	// already visible
	if(!m_spPanelWnd || m_spCostWnd)
		return;
	
	m_spCostWnd=std::shared_ptr<traincostwnd>(new traincostwnd);
	const CSize sz(500,100);
	
	const BOOL b = m_spCostWnd->CreateEx(WS_EX_CONTROLPARENT|WS_EX_TOOLWINDOW,AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW)),_T("Cost"),WS_POPUPWINDOW|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|MFS_MOVEFRAME|MFS_SYNCACTIVE,CRect(CPoint(0,0),sz),nullptr,0);
	CRect rcWindow, rcClient;
	m_spCostWnd->GetWindowRect(rcWindow);
	m_spCostWnd->GetClientRect(rcClient);
	const CSize szDelta(rcWindow.Width()-rcClient.Width(),rcWindow.Height()-rcClient.Height()); 

	traincostdlg *pDlg = m_spCostWnd->getdlg();
	CRect rcDlgWindow;
	pDlg->GetWindowRect(rcDlgWindow);
	rcWindow = CRect(rcWindow.TopLeft(),CSize(rcDlgWindow.Width(),rcDlgWindow.Height())+szDelta);

	CWnd *pAnchor = m_spPanelWnd ? static_cast<CWnd*>(m_spPanelWnd.get()) : static_cast<CWnd*>(m_spImageWnd.get());
	anchorui(rcWindow,pAnchor,{m_spPanelWnd.get(),m_spImageWnd.get()},{at_bottom,at_top});

	m_spCostWnd->MoveWindow(rcWindow);
	m_spCostWnd->ShowWindow(SW_NORMAL);
	broadcasthint(&hint(hint::t_widget));
}

void Cai3b1bApp::openimageui(void)
{
	// already visible
	if(!m_spPanelWnd || m_spImageWnd)
		return;
	
	m_spImageWnd=std::shared_ptr<trainimagewnd>(new trainimagewnd);
	const CSize sz(500,100);
	
	const BOOL b = m_spImageWnd->CreateEx(WS_EX_CONTROLPARENT|WS_EX_TOOLWINDOW,AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW)),_T("Image"),WS_POPUPWINDOW|WS_CAPTION|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|MFS_MOVEFRAME|MFS_SYNCACTIVE,CRect(CPoint(0,0),sz),nullptr,0);
	CRect rcWindow, rcClient;
	m_spImageWnd->GetWindowRect(rcWindow);
	m_spImageWnd->GetClientRect(rcClient);
	const CSize szDelta(rcWindow.Width()-rcClient.Width(),rcWindow.Height()-rcClient.Height()); 

	trainimagedlg *pDlg = m_spImageWnd->getdlg();
	CRect rcDlgWindow;
	pDlg->GetWindowRect(rcDlgWindow);
	rcWindow = CRect(rcWindow.TopLeft(),CSize(rcDlgWindow.Width(),rcDlgWindow.Height())+szDelta);

	anchorui(rcWindow,theApp.GetMainWnd(),{m_spPanelWnd.get(),m_spCostWnd.get()},{at_left,at_right});
	
	m_spImageWnd->MoveWindow(rcWindow);
	m_spImageWnd->ShowWindow(SW_NORMAL);
	broadcasthint(&hint(hint::t_widget));
}

void Cai3b1bApp::closepanelui()
{
	pause();
	if(!m_spPanelWnd)
		return;
	closecostui();
	closeimageui();
	if(m_spPanelWnd->GetSafeHwnd())
		m_spPanelWnd->DestroyWindow();
	m_spPanelWnd=nullptr;
	broadcasthint(&hint(hint::t_widget));
}

void Cai3b1bApp::closecostui()
{
	if(!m_spCostWnd)
		return;
	if(m_spCostWnd->GetSafeHwnd())
		m_spCostWnd->DestroyWindow();
	m_spCostWnd=nullptr;
	broadcasthint(&hint(hint::t_widget));
}

void Cai3b1bApp::closeimageui()
{
	if(!m_spImageWnd)
		return;
	if(m_spImageWnd->GetSafeHwnd())
		m_spImageWnd->DestroyWindow();
	m_spImageWnd=nullptr;
	broadcasthint(&hint(hint::t_widget));
}

void Cai3b1bApp::anchorui(CRect& rcWnd,CWnd *pAnchor,const std::vector<CWnd*> vAvoid, const std::vector<anchortype> vAnchors) const
{
	ASSERT(pAnchor);
	CRect rcAnchor;
	pAnchor->GetWindowRect(rcAnchor);

	auto iA=vAnchors.cbegin(),end=vAnchors.cend();
	for(;iA!=end;++iA)
	{
		CRect rcCandidate=rcWnd;
		switch(*iA)
		{
			case at_left:
			{
				const CPoint rcCandidateTR(rcCandidate.right,rcCandidate.top);
				const CPoint cpAnchorTL(rcAnchor.left-5,rcAnchor.top);
				rcCandidate.OffsetRect(cpAnchorTL-rcCandidateTR);
			}
			break;
			case at_top:
			{
				const CPoint rcCandidateBL(rcCandidate.left,rcCandidate.bottom);
				const CPoint cpAnchorTL(rcAnchor.left,rcAnchor.top-5);
				rcCandidate.OffsetRect(cpAnchorTL-rcCandidateBL);
			}
			break;
			case at_right:
			{
				const CPoint cpAnchorTR(rcAnchor.right+5,rcAnchor.top);
				rcCandidate.OffsetRect(cpAnchorTR-rcCandidate.TopLeft());
			}
			break;
			case at_bottom:
			{
				const CPoint cpAnchorBL(rcAnchor.left,rcAnchor.bottom+5);
				rcCandidate.OffsetRect(cpAnchorBL-rcCandidate.TopLeft());
			}
			break;
		}

		{
			const HMONITOR h = ::MonitorFromRect(rcCandidate,MONITOR_DEFAULTTONEAREST);
			MONITORINFO info;
			info.cbSize=sizeof(info);
			::GetMonitorInfo(h,&info);
			CRect rcUnion;
			rcUnion.UnionRect(CRect(info.rcMonitor),rcCandidate);
			if(rcUnion!=info.rcMonitor)
				continue;
		}

		bool bIntersect=false;
		auto iW=vAvoid.cbegin(),end=vAvoid.cend();
		for(;!bIntersect && iW!=end;++iW)
		{
			CRect rcAvoid, rcIntersect;
			if(!(*iW)) continue;
			(*iW)->GetWindowRect(rcAvoid);
			bIntersect=rcIntersect.IntersectRect(rcCandidate,rcAvoid);
		}
		if(bIntersect)
			continue;
		rcWnd=rcCandidate;
		return;
	}

	rcWnd.OffsetRect(rcAnchor.CenterPoint()-rcWnd.TopLeft());
}

void Cai3b1bApp::play(void)
{
	const bool bIdle = !getthread();
	if(bIdle)
	{
		// validate
		layerdlg *pLayerDlg = getlayerdlg();
		const auto it = pLayerDlg->getinputtype();
		if(!getinput(it)->gettraininginitialised())
		{
			// validate the layers/training data
			traindlg *pTrainDlg = gettraindlg();
			bool bMismatch,bFixed;
			pTrainDlg->validatenetworktrainingmismatch(bMismatch,bFixed);
			if(bMismatch && !bFixed)
				return;
			if(getinput(it)->gettraining().size()==0)
			{
				CString cs(_T("no network training data"));
				::MessageBox(getpaneldlg()->GetSafeHwnd(),cs,_T("Play"),MB_OK);
				return;
			}
			if(getinput(it)->getlayers().size()==0)
			{
				CString cs(_T("no network layers data"));
				::MessageBox(getpaneldlg()->GetSafeHwnd(),cs,_T("Play"),MB_OK);
				return;
			}
		}

		createthread(getlayerdlg()->getinputtype(),false,false);
	}
	getthread()->push_back(std::shared_ptr<mlthreadplayconfigrequest>(new mlthreadpauserequest(this,false)));
	getthread()->push_back(std::shared_ptr<mlthreadplayconfigrequest>(new mlthreadplayrequest(this)));
}

void Cai3b1bApp::pause(const bool bWait)
{
	const bool bIdle = !getthread();
	if(!bIdle)
		getthread()->push_back(std::shared_ptr<mlthreadplayconfigrequest>(new mlthreadpauserequest(this,true)),bWait);
}

void Cai3b1bApp::step(void)
{
	const bool bIdle = !getthread();
	if(!bIdle)
		getthread()->push_back(std::shared_ptr<mlthreadplayconfigrequest>(new mlthreadsteprequest(this)));
}

bool Cai3b1bApp::clearepochs(void)
{
	const bool bIdle = !getthread();
	if(!bIdle)
	{
		CString cs(_T("clear training?"));
		if(AfxMessageBox(cs,MB_YESNO|MB_ICONQUESTION)!=IDYES)
			return false;

		stop();
	}

	getinput(getlayerdlg()->getinputtype())->clearepoch();
	broadcasthint(&hint(hint::t_clear_epochs));

	return true;
}

void Cai3b1bApp::stop(void)
{
	if(!m_spThread)
		return;
	m_spThread->stop();
	m_spThread=nullptr;
}

bool Cai3b1bApp::load(void)
{
	const bool bIdle = !theApp.getthread();
	const bool bPaused = !bIdle && theApp.getthread()->getpaused();

	diasableui disui;

	CFileDialog dlg(true);
	if(dlg.DoModal()!=IDOK)
		return false;

	if(!bIdle && !clearepochs())
		return false;

	serialise s;
	if(!s.read(dlg.GetPathName()))
		return false;

	createthread(getlayerdlg()->getinputtype(),true,true);
	
	theApp.broadcasthint(&hint(hint::t_load));

	const bool bSwapInputType = true;
	const auto it = getlayerdlg()->getinputtype();
	const bool bTrainingData = (getinput(it)->gettraining().size()>0);
	if(bSwapInputType && !bTrainingData)
	{
		const std::vector<afml::trainsetitem::inputtype> v{afml::trainsetitem::it_image_i_id_xy_o_b8g8r8,afml::trainsetitem::it_image_i_id_o_b8g8r8,afml::trainsetitem::it_user};
		auto i = v.cbegin(),end=v.cend();
		for(;i!=end;++i)
		{
			const bool bTrainingData = (getinput(*i)->gettraining().size()>0);
			if(!bTrainingData)continue;

			CString cs;
			cs.Format(_T("loaded \"%s\" data, switch to this type?"),layerdlg::getinputtypename(*i));
			if(AfxMessageBox(cs,MB_YESNO|MB_ICONQUESTION)==IDYES)
				getlayerdlg()->setinputtype(*i);
			break;
		}
	}

	return true;
}

bool Cai3b1bApp::save(void)
{
	const bool bIdle = !theApp.getthread();
	const bool bPaused = !bIdle && theApp.getthread()->getpaused();

	diasableui disui;

	CFileDialog dlg(false);
	if(dlg.DoModal()!=IDOK)
		return false;

	bool bRestart = false;
	if(!bIdle && !bPaused)
	{
		bRestart = true;
		pause(true);
	}

	serialise s;
	const bool bSuccess = s.write(dlg.GetPathName());

	if(bRestart)
		play();
		
	return bSuccess;
}

void Cai3b1bApp::createthread(afml::trainsetitem::inputtype it,const bool bCheckEpoch,const bool bPause)
{
	stop();
	if(!bCheckEpoch || getinput(it)->getepoch())
	{
		m_spThread = std::shared_ptr<mlthread>(new mlthread);
		m_spThread->start(getinput(it));
		if(bPause)
			pause();
	}
}

void Cai3b1bApp::flushthreadqueryresults(void)
{
	ASSERT(getthread());
	if(!getthread())
		return;
	std::shared_ptr<const mlthreadqueryresults> sp = getthread()->flushqueryresults();
	if(!sp || sp->isempty())
		return;
	broadcasthint(&threadqueryresulthint(sp));
}

void Cai3b1bApp::flushthreadplayconfigresults(void)
{
	ASSERT(getthread());
	if(!getthread())
		return;
	std::shared_ptr<const mlthreadplayconfigresults> sp = getthread()->flushplayconfigresults();
	if(!sp || sp->isempty())
		return;
	broadcasthint(&threadplayconfigresulthint(sp));
}

void Cai3b1bApp::onhint(const hint *p)
{
}

bool Cai3b1bApp::read(const serialise *pS)
{
	// version
	int nVersion=0;
	if(!pS->read<>(nVersion))
		return false;

	// members
	if(nVersion > 0)
	{
		int nInputs;
		if(!pS->read(nInputs))
			return false;
		std::map<afml::trainsetitem::inputtype,std::shared_ptr<afml::inputtypeitem>> mInputs;
		for(int n=0;n<nInputs;++n)
		{
			afml::trainsetitem::inputtype it;
			if(!pS->read<afml::trainsetitem::inputtype,int>(it))
				return false;
			std::shared_ptr<afml::inputtypeitem> sp(new afml::inputtypeitem);
			if(!sp->read(pS))
				return false;
			mInputs[it]=sp;
		}

		// load the image training data
		traindlg *pTrainDlg = gettraindlg();
		if(!pTrainDlg->loadimagetrainingdata(pS,mInputs))
			return false;
		m_mInputs=mInputs;
	}

	return true;
}

bool Cai3b1bApp::write(const serialise *pS) const
{
	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;

	// members
	if(!pS->write<size_t,int>(m_mInputs.size()))
		return false;
	auto i = m_mInputs.cbegin(),end = m_mInputs.cend();
	for(;i!=end;++i)
	{
		if(!pS->write<afml::trainsetitem::inputtype,int>((*i).first))
			return false;
		if(!(*i).second->write(pS))
			return false;
	}

	return true;
}
