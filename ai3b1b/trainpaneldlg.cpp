// trainpaneldlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "trainpaneldlg.h"
#include "traindlg.h"
#include "layerdlg.h"
#include "traincostdlg.h"
#include "trainimagedlg.h"


// trainpaneldlg dialog

IMPLEMENT_DYNAMIC(trainpaneldlg, CDialogEx)

trainpaneldlg::trainpaneldlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_nEpoch=theApp.getinput(theApp.getlayerdlg()->getinputtype())->getepoch();
	m_nEpochTimer=0;
	m_nEpochTimerInterval=100;
	m_nCost=BST_UNCHECKED;
	m_nImage=BST_UNCHECKED;
}

trainpaneldlg::~trainpaneldlg()
{
}

void trainpaneldlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	DDX_Text(pDX,IDC_EPOCH_TEXT,m_nEpoch);

	DDX_Control(pDX,IDC_TRAIN_PLAY_BTN,m_Play);
	DDX_Control(pDX,IDC_TRAIN_PAUSE_BTN,m_Pause);
	DDX_Control(pDX,IDC_TRAIN_CLEAR_BTN,m_Clear);
	DDX_Control(pDX,IDC_TRAIN_STEP_BTN,m_Step);

	DDX_Check(pDX,IDC_TRAIN_COST_CHECK,m_nCost);
	DDX_Check(pDX,IDC_TRAIN_IMAGE_CHECK,m_nImage);
}


BEGIN_MESSAGE_MAP(trainpaneldlg, CDialogEx)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_TRAIN_PLAY_BTN,trainpaneldlg::OnPlay)
	ON_BN_CLICKED(IDC_TRAIN_PAUSE_BTN,trainpaneldlg::OnPause)
	ON_BN_CLICKED(IDC_TRAIN_CLEAR_BTN,trainpaneldlg::OnClear)
	ON_BN_CLICKED(IDC_TRAIN_STEP_BTN,trainpaneldlg::OnStep)
	ON_BN_CLICKED(IDC_TRAIN_COST_CHECK,trainpaneldlg::OnCost)
	ON_BN_CLICKED(IDC_TRAIN_IMAGE_CHECK,trainpaneldlg::OnImage)
END_MESSAGE_MAP()


// trainpaneldlg message handlers

BOOL trainpaneldlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	HBITMAP hIcn= (HBITMAP)LoadImage(
  AfxGetApp()->m_hInstance,
  MAKEINTRESOURCE(IDB_PLAY),
  IMAGE_BITMAP,
  0,0, // use actual size
  LR_DEFAULTCOLOR
  );

HBITMAP h = m_Play.SetBitmap( hIcn );

	hIcn= (HBITMAP)LoadImage(
  AfxGetApp()->m_hInstance,
  MAKEINTRESOURCE(IDB_PAUSE),
  IMAGE_BITMAP,
  0,0, // use actual size
  LR_DEFAULTCOLOR
  );

	h = m_Pause.SetBitmap( hIcn );

	hIcn= (HBITMAP)LoadImage(
  AfxGetApp()->m_hInstance,
  MAKEINTRESOURCE(IDB_CLEAR),
  IMAGE_BITMAP,
  0,0, // use actual size
  LR_DEFAULTCOLOR
  );

	h = m_Clear.SetBitmap( hIcn );

	hIcn= (HBITMAP)LoadImage(
  AfxGetApp()->m_hInstance,
  MAKEINTRESOURCE(IDB_STEP),
  IMAGE_BITMAP,
  0,0, // use actual size
  LR_DEFAULTCOLOR
  );

	h = m_Step.SetBitmap( hIcn );

	enabledisable();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void trainpaneldlg::OnPlay(void)
{
	theApp.play();
}

void trainpaneldlg::OnPause(void)
{
	theApp.pause();
}

void trainpaneldlg::OnClear(void)
{
	theApp.clearepochs();
}

void trainpaneldlg::OnStep(void)
{
	theApp.step();
}

void trainpaneldlg::OnCost(void)
{
	UpdateData();
	if(m_nCost==BST_CHECKED)
		theApp.opencostui();
	else
		theApp.closecostui();
}

void trainpaneldlg::OnImage(void)
{
	UpdateData();
	if(m_nImage==BST_CHECKED)
		theApp.openimageui();
	else
		theApp.closeimageui();
}

void trainpaneldlg::OnTimer(UINT_PTR nEvent)
{
	CWnd::OnTimer(nEvent);
	switch(nEvent)
	{
		case IDC_EPOCH_TIMER:updateepoch();break;
	}
}

void trainpaneldlg::onhint(const hint *p)
{
	switch(p->gettype())
	{
		case hint::t_clear_epochs:
		{
			m_nEpoch=0;
			UpdateData(false);
			enabledisable();
		}
		break;
		case hint::t_widget:
		{
			bool bVisible = !!theApp.getcostdlg();
			const int nCostWidget=bVisible ? BST_CHECKED : BST_UNCHECKED;
			
			bool bMutate=nCostWidget!=m_nCost;
			m_nCost=nCostWidget;

			bVisible = !!theApp.getimagedlg();
			const int nImageWidget=bVisible ? BST_CHECKED : BST_UNCHECKED;
			
			bMutate=bMutate || (nImageWidget!=m_nImage);
			m_nImage=nImageWidget;
			
			if(bMutate)
				UpdateData(false);
		}
		break;
		case hint::t_thread_playconfig_results:
		{
			const threadplayconfigresulthint *pH=static_cast<const threadplayconfigresulthint*>(p);
			const int nTypes = (mlthreadplayconfigrequest::t_step|mlthreadplayconfigrequest::t_pause|mlthreadplayconfigrequest::t_play);
			if(pH && pH->getresults() && (pH->getresults()->gettypes() & nTypes))
			{
				updateepochtimer();
				updateepoch();
				enabledisable();
			}
		}
		break;
		case hint::t_epoch_range:
		{
			enabledisable();
		}
		break;
		case hint::t_input_type:
		case hint::t_load:
		{
			updateepoch();
			enabledisable();
		}
		break;
	}
}

void trainpaneldlg::updateepochtimer(void)
{
	if(!theApp.getlayerdlg())
	{
		if(m_nEpochTimer)
		{
			KillTimer(m_nEpochTimer);
			m_nEpochTimer=0;
		}
		return;
	}

	const auto it = theApp.getlayerdlg()->getinputtype();
	const auto nEpochs = theApp.getinput(it)->getepochs();

	const bool bIdle = !theApp.getthread();
	const bool bPaused = !bIdle && theApp.getthread()->getpaused();
	const bool bEpochPending = ( nEpochs<0 || ( m_nEpoch < nEpochs ) );

	if(bPaused || bIdle)
	{
		if(m_nEpochTimer)
		{
			KillTimer(m_nEpochTimer);
			m_nEpochTimer=0;
			TRACE(_T("trainpaneldlg::KillTimer\r\n"));
		}
	}
	else
	if(bEpochPending)
	{
		if(m_nEpochTimer==0)
		{
			m_nEpochTimer=SetTimer(IDC_EPOCH_TIMER,m_nEpochTimerInterval,nullptr);
			TRACE(_T("trainpaneldlg::SetTimer\r\n"));
		}
	}
}

void trainpaneldlg::updateepoch(void)
{
	if(!theApp.getlayerdlg())
		return;
	const int nEpoch = theApp.getinput(theApp.getlayerdlg()->getinputtype())->getepoch();
	const auto it = theApp.getlayerdlg()->getinputtype();
	if(nEpoch==m_nEpoch)
		return;
	m_nEpoch=nEpoch;
	UpdateData(false);
}

void trainpaneldlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;

	const auto it = theApp.getlayerdlg()->getinputtype();
	const auto nEpochs = theApp.getinput(it)->getepochs();
	
	const bool bIdle = !theApp.getthread();
	const bool bEpochPaused = !bIdle && (theApp.getthread()->getpaused());
	const bool bEpochPending = ( nEpochs<0 || ( m_nEpoch < nEpochs ) );

	GetDlgItem(IDC_TRAIN_PLAY_BTN)->EnableWindow(bEpochPending && ( bIdle || bEpochPaused ));
	GetDlgItem(IDC_TRAIN_PAUSE_BTN)->EnableWindow(bEpochPending && !bIdle && !bEpochPaused);
	GetDlgItem(IDC_TRAIN_CLEAR_BTN)->EnableWindow(!bIdle);
	GetDlgItem(IDC_TRAIN_STEP_BTN)->EnableWindow(bEpochPending && bEpochPaused);
}
