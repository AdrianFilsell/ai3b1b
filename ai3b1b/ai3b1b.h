
// ai3b1b.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "ml_network.h"
#include "winthread_ml.h"
#include "thread_taskscheduler.h"
#include "hint.h"
#include <map>


// Cai3b1bApp:
// See ai3b1b.cpp for the implementation of this class
//

class Cai3b1bDlg;
class layerdlg;
class traindlg;
class trainpanelwnd;
class trainpaneldlg;
class traincostdlg;
class trainimagedlg;
class traincostwnd;
class trainimagewnd;
class serialise;

class Cai3b1bApp : public CWinApp
{
public:
	Cai3b1bApp();

// Overrides
public:
	virtual BOOL InitInstance();

	std::shared_ptr<const afthread::taskscheduler> getscheduler(void)const{return m_spScheduler;}

	Cai3b1bDlg *getdlg(void)const;
	layerdlg *getlayerdlg(void) const;
	traindlg *gettraindlg(void) const;
	trainpanelwnd *getpanelwnd(void) const;
	trainpaneldlg *getpaneldlg(void) const;
	trainimagewnd *getimagewnd(void) const;
	trainimagedlg *getimagedlg(void) const;
	traincostwnd *getcostwnd(void) const;
	traincostdlg *getcostdlg(void) const;

	mlthread *getthread(void) const{return m_spThread.get();}
	void createthread(afml::trainsetitem::inputtype it,const bool bCheckEpoch,const bool bPause);
	void flushthreadqueryresults(void);
	void flushthreadplayconfigresults(void);
	
	std::shared_ptr<afml::inputtypeitem> getinput(const afml::trainsetitem::inputtype it) { const auto i = m_mInputs.find(it); return (i == m_mInputs.cend()) ? nullptr : (*i).second; }
	std::map<afml::trainsetitem::inputtype,std::shared_ptr<afml::inputtypeitem>>& getinputs(void) { return m_mInputs; }

	void openpanelui();
	void openimageui();
	void opencostui();
	void closepanelui();
	void closeimageui();
	void closecostui();
	void enableui(const bool b);

	void play(void);
	void pause(const bool bWait=false);
	void step(void);
	bool clearepochs(void);

	bool load(void);
	bool save(void);
	bool read(const serialise *pS);
	bool write(const serialise *pS) const;

	void broadcasthint(const hint *p);

	virtual int ExitInstance(void)override{closecostui();closeimageui();closepanelui();return CWinApp::ExitInstance();}

// Implementation

	DECLARE_MESSAGE_MAP()

protected:
	std::shared_ptr<afthread::taskscheduler> m_spScheduler;

	std::shared_ptr<mlthread> m_spThread;
	std::shared_ptr<trainpanelwnd> m_spPanelWnd;
	std::shared_ptr<traincostwnd> m_spCostWnd;
	std::shared_ptr<trainimagewnd> m_spImageWnd;

	std::map<afml::trainsetitem::inputtype,std::shared_ptr<afml::inputtypeitem>> m_mInputs;
		
	void stop(void);

	void onhint(const hint *p);

	enum anchortype {at_left,at_right,at_top,at_bottom};
	void anchorui(CRect& rcWnd,CWnd *pAnchor,const std::vector<CWnd*> vAvoid, const std::vector<anchortype> vAnchors) const;
};

extern Cai3b1bApp theApp;

class diasableui
{
public:
	diasableui(){theApp.enableui(false);}
	~diasableui(){theApp.enableui(true);}
};
