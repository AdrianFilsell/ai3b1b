// traincostdlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "traincostdlg.h"
#include "traindlg.h"
#include "layerdlg.h"


// traincostdlg dialog

IMPLEMENT_DYNAMIC(traincostdlg, CDialogEx)

traincostdlg::traincostdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_nUpdate = BST_CHECKED;
	m_csEpochs.Format(_T("%li"),100);
	m_nEpochTimer=0;
	m_bInitialised = false;

	layerdlg *pLayerDlg=theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();

	const std::pair<double,int> minepoch=theApp.getinput(it)->getminepoch();
	const std::pair<double,int> maxepoch=theApp.getinput(it)->getmaxepoch();

	const std::pair<double,double> r(0,0);
	const std::pair<double,int> c(0,0);
	updatecosts(r,minepoch,maxepoch,c,false);
}

traincostdlg::~traincostdlg()
{
}

void traincostdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_COST_UPDATE_SPIN,m_EpochsSpin);

	DDX_Text(pDX,IDC_COST_UPDATE_EDIT,m_csEpochs);
	DDX_Text(pDX,IDC_COST_MIN_VALUE_TEXT,m_csMinValue);
	DDX_Text(pDX,IDC_COST_MAX_VALUE_TEXT,m_csMaxValue);
	DDX_Text(pDX,IDC_COST_LAST_VALUE_TEXT,m_csLastValue);
	DDX_Text(pDX,IDC_COST_MIN_EPOCH_TEXT,m_csMinEpoch);
	DDX_Text(pDX,IDC_COST_MAX_EPOCH_TEXT,m_csMaxEpoch);
	DDX_Text(pDX,IDC_COST_LAST_EPOCH_TEXT,m_csLastEpoch);
	DDX_Text(pDX,IDC_COST_HISTORY_RANGE_MIN_TEXT,m_csLocalMin);
	DDX_Text(pDX,IDC_COST_HISTORY_RANGE_MAX_TEXT,m_csLocalMax);

	DDX_Check(pDX,IDC_COST_UPDATE_CHECK,m_nUpdate);
}


BEGIN_MESSAGE_MAP(traincostdlg, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_TIMER()

	ON_EN_CHANGE(IDC_COST_UPDATE_EDIT,traincostdlg::OnEpochsEditChange)
	ON_EN_KILLFOCUS(IDC_COST_UPDATE_EDIT,traincostdlg::OnEpochsEditKillFocus)

	ON_BN_CLICKED(IDC_COST_UPDATE_BTN,traincostdlg::OnEpochsUpdateNow)
	ON_BN_CLICKED(IDC_COST_UPDATE_CHECK,traincostdlg::OnEpochsUpdate)
END_MESSAGE_MAP()


// traincostdlg message handlers

BOOL traincostdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	{
		CRect rc;
		GetDlgItem(IDC_COST_HISTORY_RECT)->GetWindowRect(rc);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
		m_spGraph = std::shared_ptr<traincostgraphwnd>(new traincostgraphwnd);
		const BOOL b = m_spGraph->Create(AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW)),_T("history"),WS_VISIBLE|WS_CHILD|WS_BORDER,rc,this,1024);
	}

	// setup spin ctrls, this gives 'correct' direction of change
	m_EpochsSpin.SetRange32(1,0x7fffffff);

	updateepochtimer();
	enabledisable();
	
	m_bInitialised = true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void traincostdlg::OnDestroy(void)
{
	if(m_spGraph)
	{
		if(m_spGraph->GetSafeHwnd())
			m_spGraph->DestroyWindow();
		m_spGraph=nullptr;
	}

	CDialogEx::OnDestroy();
}

void traincostdlg::OnEpochsUpdate(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	UpdateData();

	updateepochtimer();
	enabledisable();
}

void traincostdlg::OnEpochsUpdateNow(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	const bool bIdle = !theApp.getthread();
	if(!bIdle)
	{
		auto sp = createrequest();
		if(sp)
			theApp.getthread()->push_back(sp);
	}
}

void traincostdlg::OnEpochsEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// update ms
	if(true)
	{
		killepochtimer();
		updateepochtimer();
	}
}

void traincostdlg::OnEpochsEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csEpochs,1));
	const bool bMutated = cs!=m_csEpochs;
	m_csEpochs=cs;
	
	// update ui from members
	if(bMutated)
	{
		GetDlgItem(IDC_COST_UPDATE_EDIT)->SetWindowText(cs);
	}

	// update ms
	if(bMutated)
	{
		killepochtimer();
		updateepochtimer();
	}
}

void traincostdlg::OnTimer(UINT_PTR nEvent)
{
	CWnd::OnTimer(nEvent);
	switch(nEvent)
	{
		case IDC_EPOCH_TIMER:postepochrequest();break;
	}
}

void traincostdlg::onhint(const hint *p)
{
	switch(p->gettype())
	{
		case hint::t_input_type:
		case hint::t_clear_epochs:
		case hint::t_load:
		{
			layerdlg *pLayerDlg=theApp.getlayerdlg();
			const auto it = pLayerDlg->getinputtype();

			const std::pair<double,int> minepoch=theApp.getinput(it)->getminepoch();
			const std::pair<double,int> maxepoch=theApp.getinput(it)->getmaxepoch();

			const std::pair<double,double> r(0,0);
			const std::pair<double,int> c(0,0);
			updatecosts(r,minepoch,maxepoch,c,false);
			if(m_spGraph)
				m_spGraph->clear();
			UpdateData(false);
			enabledisable();
		}
		break;
		case hint::t_thread_playconfig_results:
		{
			const threadplayconfigresulthint *pH=static_cast<const threadplayconfigresulthint*>(p);

			const int nTypes = (mlthreadplayconfigrequest::t_pause|mlthreadplayconfigrequest::t_play);
			if(pH && pH->getresults() && (pH->getresults()->gettypes() & nTypes))
			{
				updateepochtimer();
				enabledisable();
			}

			if(pH && pH->getresults() && (pH->getresults()->gettypes() & mlthreadplayconfigrequest::t_step))
				postepochrequest();
		}
		break;
		case hint::t_thread_query_results:
		{
			const threadqueryresulthint *pH=static_cast<const threadqueryresulthint*>(p);
			const int nTypes = (mlthreadqueryrequest::t_cost);
			if(pH && pH->getresults() && (pH->getresults()->gettypes() & nTypes))
			{
				bool bFound=false;
				auto i = pH->getresults()->getresults().crbegin(),end=pH->getresults()->getresults().crend();
				for(;!bFound && i!=end;++i)
					switch((*i)->gettype())
					{
						case mlthreadqueryrequest::t_cost:
						{
							const mlthreadcostrequest *pCostReq=static_cast<const mlthreadcostrequest*>((*i).get());
							if(bFound=(pCostReq->getowner()==this))
							{
								layerdlg *pLayerDlg=theApp.getlayerdlg();
								const auto it = pLayerDlg->getinputtype();

								theApp.getinput(it)->pushback_epoch(pCostReq->getcost());
						
								if(!m_spGraph)
									return;
								if(m_spGraph->getlength()==0)
								{
									const std::pair<double,int> minepoch=theApp.getinput(it)->getminepoch();
									const std::pair<double,int> maxepoch=theApp.getinput(it)->getmaxepoch();
									m_spGraph->set(minepoch,maxepoch);
								}
								m_spGraph->push_back(pCostReq->getcost());
								std::pair<double,int> l;
								const std::pair<double,int> c(0,0);
								const bool bLast=m_spGraph->getgloballast(l);
								updatecosts(m_spGraph->getinflatedlocalrange(),m_spGraph->getglobalmin(),m_spGraph->getglobalmax(),bLast?l:c);
							}
						}
						break;
					}
			}
		}
		break;
	}
}

void traincostdlg::updatecosts(const std::pair<double,double> inflatedlocalrange,const std::pair<double,int> globalmin,const std::pair<double,int> globalmax,const std::pair<double,int> last,const bool bUpdateData)
{
	const bool bMinMaxSciNotation = true;
	const CString csMinMaxSciNotation(_T("%.10e"));
	m_csMinValue.Format(bMinMaxSciNotation?csMinMaxSciNotation:_T("%f"),globalmin.first);
	m_csMaxValue.Format(bMinMaxSciNotation?csMinMaxSciNotation:_T("%f"),globalmax.first);
	m_csLastValue.Format(bMinMaxSciNotation?csMinMaxSciNotation:_T("%f"),last.first);

	const bool bLocalSciNotation = true;
	const CString csLocalSciNotation(_T("%.4e"));
	m_csLocalMin.Format(bLocalSciNotation?csLocalSciNotation:_T("%f"),inflatedlocalrange.first);
	m_csLocalMax.Format(bLocalSciNotation?csLocalSciNotation:_T("%f"),inflatedlocalrange.second);

	m_csMinEpoch.Format(_T("%li"),globalmin.second);
	m_csMaxEpoch.Format(_T("%li"),globalmax.second);
	m_csLastEpoch.Format(_T("%li"),last.second);

	if(bUpdateData)
		UpdateData(false);
}

void traincostdlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;
	
	const bool bIdle = !theApp.getthread();

	GetDlgItem(IDC_COST_UPDATE_BTN)->EnableWindow(!bIdle);
	GetDlgItem(IDC_COST_UPDATE_EDIT)->EnableWindow(m_nUpdate==BST_CHECKED);
}

void traincostdlg::killepochtimer(void)
{
	if(m_nEpochTimer)
	{
		KillTimer(m_nEpochTimer);
		m_nEpochTimer=0;
		TRACE(_T("traincostdlg::KillTimer\r\n"));
	}
}

void traincostdlg::updateepochtimer(void)
{
	const auto it = theApp.getlayerdlg()->getinputtype();
	const auto nEpochs = theApp.getinput(it)->getepochs();

	const bool bIdle = !theApp.getthread();
	const bool bPaused = !bIdle && theApp.getthread()->getpaused();
	const bool bEpochPending = ( nEpochs<0 || ( theApp.getinput(it)->getepoch() < nEpochs ) );

	if(!bEpochPending || bPaused || bIdle || m_nUpdate==BST_UNCHECKED)
		killepochtimer();
	else
	if(bEpochPending)
	{
		if(m_nEpochTimer==0)
		{
			if(m_nUpdate == BST_CHECKED)
			{
				const int nMilli = af::getint<true>(m_csEpochs,1);
				m_nEpochTimer=SetTimer(IDC_EPOCH_TIMER,nMilli,nullptr);
				TRACE(_T("traincostdlg::SetTimer\r\n"));
			}
		}
	}
}

void traincostdlg::postepochrequest(void)
{
	if(theApp.getthread())
	{
		std::shared_ptr<mlthreadqueryrequest> sp = createrequest();
		if(sp)
			theApp.getthread()->push_back(sp);
	}
}

std::shared_ptr<mlthreadqueryrequest> traincostdlg::createrequest(void)const
{
	return std::shared_ptr<mlthreadqueryrequest>(new mlthreadcostrequest(this));
}
