// trainimagedlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "trainimagedlg.h"
#include "traindlg.h"
#include "layerdlg.h"
#include "jpeg.h"
#include "imageexportdimdlg.h"


// trainimagedlg dialog

IMPLEMENT_DYNAMIC(trainimagedlg, CDialogEx)

trainimagedlg::trainimagedlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_bInitialised=false;
	m_nLerpSlider=50;
	m_csLerp.Format(_T("%li"),m_nLerpSlider);
	m_nUpdate = BST_CHECKED;
	m_nSmooth = BST_UNCHECKED;
	m_nScaleToFit = BST_UNCHECKED;
	m_csEpochs.Format(_T("%li"),100);
	m_nEpochTimer=0;
	m_nDibSrc=getdibtypeindex(dt_output);
	m_nLerpDst=0;
}

trainimagedlg::~trainimagedlg()
{
}

void trainimagedlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_IMAGE_UPDATE_SPIN,m_EpochsSpin);
	DDX_Control(pDX,IDC_IMAGE_INPUT_COMBO,m_InputCombo);
	DDX_Control(pDX,IDC_IMAGE_LERP_SPIN,m_LerpSpin);
	DDX_Control(pDX,IDC_IMAGE_LERP_SLIDER,m_LerpSlider);
	DDX_Control(pDX,IDC_IMAGE_LERP_COMBO,m_LerpCombo);

	DDX_Text(pDX,IDC_IMAGE_UPDATE_EDIT,m_csEpochs);
	DDX_Text(pDX,IDC_IMAGE_LERP_EDIT,m_csLerp);

	DDX_Check(pDX,IDC_IMAGE_UPDATE_CHECK,m_nUpdate);
	DDX_Check(pDX,IDC_IMAGE_THUMB_SMOOTH_CHECK,m_nSmooth);
	DDX_Check(pDX,IDC_IMAGE_THUMB_SCALETOFIT_CHECK,m_nScaleToFit);

	DDX_Slider(pDX,IDC_IMAGE_LERP_SLIDER,m_nLerpSlider);

	DDX_CBIndex(pDX,IDC_IMAGE_THUMB_INPUT_COMBO,m_nDibSrc);
	DDX_CBIndex(pDX,IDC_IMAGE_LERP_COMBO,m_nLerpDst);
}


BEGIN_MESSAGE_MAP(trainimagedlg, CDialogEx)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_IMAGE_INPUT_COMBO,trainimagedlg::OnInputChanged)
	ON_BN_CLICKED(IDC_IMAGE_UPDATE_BTN,trainimagedlg::OnEpochsUpdateNow)
	ON_BN_CLICKED(IDC_IMAGE_UPDATE_CHECK,trainimagedlg::OnEpochsUpdate)
	ON_EN_CHANGE(IDC_IMAGE_LERP_EDIT,trainimagedlg::OnLerpEditChange)
	ON_EN_CHANGE(IDC_IMAGE_UPDATE_EDIT,trainimagedlg::OnEpochsEditChange)
	ON_EN_KILLFOCUS(IDC_IMAGE_LERP_EDIT,trainimagedlg::OnLerpEditKillFocus)
	ON_EN_KILLFOCUS(IDC_IMAGE_UPDATE_EDIT,trainimagedlg::OnEpochsEditKillFocus)
	ON_CBN_SELCHANGE(IDC_IMAGE_THUMB_INPUT_COMBO,trainimagedlg::OnThumbInputChanged)
	ON_CBN_SELCHANGE(IDC_IMAGE_LERP_COMBO,trainimagedlg::OnLerpDstChanged)
	ON_BN_CLICKED(IDC_IMAGE_THUMB_SMOOTH_CHECK,trainimagedlg::OnSmooth)
	ON_BN_CLICKED(IDC_IMAGE_THUMB_SCALETOFIT_CHECK,trainimagedlg::OnScaleToFit)
	ON_BN_CLICKED(IDC_IMAGE_EXPORT,trainimagedlg::OnExport)
	ON_BN_CLICKED(IDC_IMAGE_LERP_ANIMATE,trainimagedlg::OnLerpAnimate)
END_MESSAGE_MAP()


// trainimagedlg message handlers

BOOL trainimagedlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// setup spin ctrls, this gives 'correct' direction of change
	m_LerpSpin.SetRange32(0,100);
	m_EpochsSpin.SetRange32(1,0x7fffffff);

	// create dib wnds
	CRect rcDib;
	GetDlgItem(IDC_IMAGE_DIB_RECT)->GetWindowRect(rcDib);
	::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcDib,2);
	m_spDibWnd=std::shared_ptr<dibwnd>(new dibwnd);
	m_spDibWnd->Create(AfxRegisterWndClass(0,theApp.LoadStandardCursor(IDC_ARROW)),_T("dib"),WS_VISIBLE|WS_CHILD|WS_BORDER,rcDib,this,1024);
	m_spDibWnd->setscaletofit(m_nScaleToFit==BST_CHECKED);
	m_spReqCache=std::shared_ptr<mlthreaddibrequestcache>(new mlthreaddibrequestcache);
	
	populatetrainingitemcombos();

	updateepochtimer();
	enabledisable();

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void trainimagedlg::OnLerpAnimate(void)
{
	const bool bIdle = !theApp.getthread();
	const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

	if(bIdle || !bLerp)
		return;

	const bool bPaused = !bIdle && theApp.getthread()->getpaused();
	bool bRestart = false;
	if(!bIdle && !bPaused)
	{
		bRestart = true;
		theApp.pause(true);
	}

	const int nFrom = af::getint<true,true>(m_csLerp,0,100);
	std::vector<int> vIndices;
	for(int n=nFrom;n<=100;++n)
		vIndices.push_back(n);
	for(int n=100;n>=0;--n)
		vIndices.push_back(n);
	for(int n=0;n<=nFrom;++n)
		vIndices.push_back(n);
	auto i = vIndices.cbegin(),end=vIndices.cend();
	for(;i!=end;++i)
	{
		m_nLerpSlider=*i;
		m_csLerp.Format(_T("%li"),m_nLerpSlider);
		UpdateData(false);
		GetDlgItem(IDC_IMAGE_LERP_EDIT)->UpdateWindow();

		SIZE sz;
		std::shared_ptr<mlthreadqueryrequest> sp = createrequest(nullptr,dt_lerp,getcustomdim(dt_lerp,m_nScaleToFit==BST_UNCHECKED||m_nSmooth==BST_UNCHECKED,sz));
		if(sp)
			theApp.getthread()->push_back(sp,true);

		m_spDibWnd->set(static_cast<mlthreaddibrequestbase*>(sp.get())->getdib());
		m_spDibWnd->UpdateWindow();
	}
	postdibrequest(dt_lerp);

	if(bRestart)
		theApp.play();
}

void trainimagedlg::OnLerpDstChanged(void)
{
	UpdateData();

	const bool bIdle = !theApp.getthread();
	const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

	if(bIdle || !bLerp)
		return;
	postdibrequest(getdibtype(m_nDibSrc));
}

void trainimagedlg::OnExport(void)
{
	const bool bIdle = !theApp.getthread();
	
	const auto it = theApp.getlayerdlg()->getinputtype();
	const auto dt = getdibtype(m_nDibSrc);
	
	if(!(it & afml::trainsetitem::it_image) )
		return;

	if(bIdle)
		return;

	diasableui disui;

	CFileDialog cf(false);
	if(cf.DoModal()!=IDOK)
		return;
	CString cs = cf.GetPathName();

	std::shared_ptr<const afdib::dib> spDib;
	const afml::traintsetimageitem *pInputItem = gettrainingitem(&m_InputCombo,m_InputCombo.GetCurSel());
	switch(dt)
	{
		case trainimagedlg::dibtype::dt_input:spDib = pInputItem->loadtrnsdib(pInputItem->getpath(),pInputItem->getdim(),it);break;
		case trainimagedlg::dibtype::dt_output:
		case trainimagedlg::dibtype::dt_lerp:
		{
			switch(it)
			{
				case afml::trainsetitem::it_user:ASSERT(false);return;
				case afml::trainsetitem::it_image_i_id_o_b8g8r8:
				{
					std::shared_ptr<mlthreadqueryrequest> sp;
					sp=createrequest(nullptr,dt);
					if(!sp)
						return;
					theApp.getthread()->push_back(sp,true);
					spDib = static_cast<mlthreaddibrequestbase*>(sp.get())->getdib();
				}
				break;
				case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
				{
					const bool bLerp = dt == trainimagedlg::dibtype::dt_lerp;

					SIZE szLerpDim;double dLerp;
					if(bLerp)
					{
						dLerp = (af::getint<true,true>(m_csLerp,0,100)/100.0);
						const afml::traintsetimageitem *pFromItem = pInputItem;
						const afml::traintsetimageitem *pToItem = gettrainingitem(&m_LerpCombo,m_LerpCombo.GetCurSel());
						szLerpDim = getlerpdim(pFromItem->getdibdim(),pToItem->getdibdim(),dLerp);
					}

					imageexportdimdlg dlg(this,bLerp?szLerpDim:pInputItem->getdibdim(),getoutputdim(bLerp?szLerpDim:pInputItem->getdibdim()));
					if(dlg.DoModal()!=IDOK)
						return;
					const auto dimtype = dlg.getdimtype(dlg.m_nType);

					std::shared_ptr<mlthreadqueryrequest> sp;					
					switch(dimtype)
					{
						case imageexportdimdlg::dt_input:
						case imageexportdimdlg::dt_output:
						{
							SIZE sz;
							const bool bInput = dimtype == imageexportdimdlg::dt_input;
							sp = createrequest(nullptr,dt,mlthreaddibrequestbasecustomdim(getcustomdim(dt,bInput,bLerp?szLerpDim:sz)));
						}
						break;
						case imageexportdimdlg::dt_custom:
						{
							const unsigned int nMemLimit=100*1024*1024;
							sp = createrequest(nullptr,dt,mlthreaddibrequestbasecustomdim(&CSize(af::getint<true>(dlg.m_csWidth,1),af::getint<true>(dlg.m_csHeight,1)),true,nMemLimit));
						}
						break;
						default:ASSERT(false);
					}
					if(!sp)
						return;
					theApp.getthread()->push_back(sp,true);
					spDib = static_cast<mlthreaddibrequestbase*>(sp.get())->getdib();
				}
				break;
				default:ASSERT(false);
			}
		}
		break;
		default:ASSERT(false);
	}
	if(spDib)
	{
		// attempt jpeg save
		const bool b = afdib::jpeg::save8bpp(spDib,cs);
	}
}

void trainimagedlg::OnScaleToFit(void)
{
	UpdateData();

	m_spDibWnd->setscaletofit(m_nScaleToFit==BST_CHECKED);

	enabledisable();

	const bool bIdle = !theApp.getthread();

	if(getdibtype(m_nDibSrc) == dt_input)
		return;

	if(bIdle)
		return;
	postdibrequest(getdibtype(m_nDibSrc));
}

void trainimagedlg::OnSmooth(void)
{
	UpdateData();
	
	const bool bIdle = !theApp.getthread();

	if(getdibtype(m_nDibSrc) == dt_input)
		return;

	if(bIdle)
		return;
	postdibrequest(getdibtype(m_nDibSrc));
}

void trainimagedlg::OnThumbInputChanged(void)
{
	UpdateData();

	enabledisable();

	const bool bIdle = !theApp.getthread();
	const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

	if(getdibtype(m_nDibSrc) == dt_input)
	{
		setdib(getinputdib(),dt_input);
		return;
	}

	cleardib(dt_output);
	cleardib(dt_lerp);

	if(bIdle)
		return;
	postdibrequest(getdibtype(m_nDibSrc));
}

void trainimagedlg::OnInputChanged(void)
{
	UpdateData();

	const bool bIdle = !theApp.getthread();
	const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

	if(getdibtype(m_nDibSrc) == dt_input)
	{
		setdib(getinputdib(),dt_input);
		return;
	}

	if(bIdle)
	{
		cleardib(dt_output);
		cleardib(dt_lerp);
		return;
	}

	if(bIdle)
		return;
	postdibrequest(getdibtype(m_nDibSrc));
}

void trainimagedlg::OnLerpEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);	
	
	const int n = af::getint<true,true>(m_csLerp,0,100);
	if(m_nLerpSlider!=n)
	{
		m_nLerpSlider=n;
		m_csLerp.Format(_T("%li"),m_nLerpSlider);
		UpdateData(false);
		
		const bool bIdle = !theApp.getthread();
		const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

		if(bIdle || !bLerp)
			return;
		postdibrequest(getdibtype(m_nDibSrc));
	}
}

void trainimagedlg::OnLerpEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true,true>(m_csLerp,0,100));
	const bool bMutated = cs!=m_csLerp;
	m_csLerp=cs;
	
	// update ui from members
	if(bMutated)
	{
		GetDlgItem(IDC_IMAGE_LERP_EDIT)->SetWindowText(cs);

		m_nLerpSlider = af::getint<true,true>(m_csLerp,0,100);
		m_csLerp.Format(_T("%li"),m_nLerpSlider);
		UpdateData(false);
		
		const bool bIdle = !theApp.getthread();
		const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

		if(bIdle || !bLerp)
			return;
		postdibrequest(getdibtype(m_nDibSrc));
	}
}

void trainimagedlg::OnEpochsEditChange(void)
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

void trainimagedlg::OnEpochsEditKillFocus(void)
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
		GetDlgItem(IDC_IMAGE_UPDATE_EDIT)->SetWindowText(cs);
	}

	// update ms
	if(bMutated)
	{
		killepochtimer();
		updateepochtimer();
	}
}

void trainimagedlg::OnEpochsUpdate(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	UpdateData();

	updateepochtimer();
	enabledisable();
}

void trainimagedlg::OnEpochsUpdateNow(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	const bool bIdle = !theApp.getthread();

	if(bIdle || getdibtype(m_nDibSrc)==dt_input)
		return;
	postdibrequest(getdibtype(m_nDibSrc));
}

void trainimagedlg::OnTimer(UINT_PTR nEvent)
{
	CWnd::OnTimer(nEvent);
	switch(nEvent)
	{
		case IDC_EPOCH_TIMER:
		{
			const auto t = getdibtype(m_nDibSrc);
			if(t&(dt_output|dt_lerp))
				postdibrequest(t);
		}
		break;
	}
}

void trainimagedlg::OnHScroll(UINT nSBCode,UINT nPos,CScrollBar* pScrollBar)
{
	CDialogEx::OnHScroll(nSBCode,nPos,pScrollBar);

	const int n=m_LerpSlider.GetPos();
	if(m_nLerpSlider!=n)
	{
		m_nLerpSlider=n;
		m_csLerp.Format(_T("%li"),m_nLerpSlider);
		UpdateData(false);
		
		const bool bIdle = !theApp.getthread();//!bRunning && !bPaused;
		const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;

		if(bIdle || !bLerp)
			return;
		postdibrequest(getdibtype(m_nDibSrc));
	}
}

void trainimagedlg::onhint(const hint *p)
{
	switch(p->gettype())
	{
		case hint::t_clear_epochs:
		{
			setdib(nullptr,dt_output);
			setdib(nullptr,dt_lerp);
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
				postdibrequest(getdibtype(m_nDibSrc));
		}
		break;
		case hint::t_thread_query_results:
		{
			const threadqueryresulthint *pH=static_cast<const threadqueryresulthint*>(p);
			const auto dt = getdibtype(m_nDibSrc);
			const bool bDib = dt == dt_output;
			const bool bLerp = dt == dt_lerp;
			const int nTypes = bDib ? mlthreadqueryrequest::t_dib :
								(bLerp ? mlthreadqueryrequest::t_lerp_dib : 0);
			if(pH && pH->getresults() && (pH->getresults()->gettypes() & nTypes))
			{
				bool bFound=false;
				auto i = pH->getresults()->getresults().crbegin(),end=pH->getresults()->getresults().crend();
				for(;!bFound && i!=end;++i)
					switch((*i)->gettype())
					{
						case mlthreadqueryrequest::t_dib:
						{
							const mlthreaddibrequest *pDibReq=static_cast<const mlthreaddibrequest*>((*i).get());
							if(bDib && pDibReq && pDibReq->getdib() ? bFound=(pDibReq->getowner()==this) : false)
								setdib(pDibReq->getdib(),dt_output);
						}
						break;
						case mlthreadqueryrequest::t_lerp_dib:
						{
							const mlthreadlerpdibrequest *pDibReq=static_cast<const mlthreadlerpdibrequest*>((*i).get());
							if(bLerp && pDibReq && pDibReq->getdib() ? bFound=(pDibReq->getowner()==this) : false)
								setdib(pDibReq->getdib(),dt_lerp);
						}
						break;
					}
			}
		}
		break;
		case hint::t_input_type:
		case hint::t_training_data:
		case hint::t_load:
		{
			populatetrainingitemcombos();
			enabledisable();
		}
		break;
	}
}

void trainimagedlg::populatetrainingitemcombos(void)
{
	// output
	cleardib(dt_output);
	cleardib(dt_input);
	cleardib(dt_lerp);

	// input
	if(populatetrainingitemcombo(&m_InputCombo) && getdibtype(m_nDibSrc)==dt_input)
		setdib(getinputdib(),dt_input);

	// lerp
	if(populatetrainingitemcombo(&m_LerpCombo))
	{
		m_nLerpDst = m_LerpCombo.GetCount()-1;
		UpdateData(false);
	}

	if(getdibtype(m_nDibSrc)==dt_input)
		return;

	const bool bIdle = !theApp.getthread();

	if(bIdle)
		return;

	postdibrequest(getdibtype(m_nDibSrc));
}

bool trainimagedlg::populatetrainingitemcombo(CComboBox *p)
{
	switch(theApp.getlayerdlg()->getinputtype())
	{
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			int nPreSel=CB_ERR;
			p->GetCurSel();
			const afml::traintsetimageitem *pPreInputItem = gettrainingitem(p,nPreSel); // this may be non null BUT invalid

			p->ResetContent();

			const auto& v = theApp.getinput(theApp.getlayerdlg()->getinputtype())->gettraining();
			auto i = v.cbegin(),end=v.cend();
			for(;i!=end;++i)
				if((*i)->gettype() & afml::trainsetitem::it_image)
				{
					const afml::traintsetimageitem *pImageItem = static_cast<const afml::traintsetimageitem*>((*i).get());
					const CString csFName = pImageItem->getfname();
					const int n = p->AddString(csFName);
					p->SetItemData(n,reinterpret_cast<DWORD_PTR>(pImageItem));
				}

			bool bPreInputItemExists=false;
			for(int n = 0;!bPreInputItemExists && n<p->GetCount();++n)
			{
				bPreInputItemExists = gettrainingitem(p,n) == pPreInputItem;
				if(bPreInputItemExists)
					p->SetCurSel(n);
			}
			if(!bPreInputItemExists)
			{
				const bool bSelIdxMutated = ( nPreSel==CB_ERR ) ? ( p->GetCount() > 0 ) : ( nPreSel>=p->GetCount() );
				if(p->GetCount())
					p->SetCurSel(nPreSel==CB_ERR ? 0 :
										   ( !bSelIdxMutated ? nPreSel : p->GetCount()-1 ));
				return true;
			}
		}
		break;
		case afml::trainsetitem::it_user:p->ResetContent();break;
		default:ASSERT(false);
	}
	return false;
}

void trainimagedlg::killepochtimer(void)
{
	if(m_nEpochTimer)
	{
		KillTimer(m_nEpochTimer);
		m_nEpochTimer=0;
		TRACE(_T("trainimagedlg::KillTimer\r\n"));
	}
}

void trainimagedlg::updateepochtimer(void)
{
	const auto it = theApp.getlayerdlg()->getinputtype();
	const auto nEpochs = theApp.getinput(it)->getepochs();

	const bool bIdle = !theApp.getthread();
	const bool bPaused = !bIdle && theApp.getthread()->getpaused();
	const bool bEpochPending = ( nEpochs<0 || ( theApp.getinput(it)->getepoch() < nEpochs ) );

	if(bPaused || bIdle || m_nUpdate==BST_UNCHECKED)
		killepochtimer();
	else
	if(bEpochPending)
	{
		if(m_nEpochTimer==0)
		{
			if(m_nUpdate==BST_CHECKED)
			{
				const int nMilli = af::getint<true>(m_csEpochs,1);
				m_nEpochTimer=SetTimer(IDC_EPOCH_TIMER,nMilli,nullptr);
				TRACE(_T("trainimagedlg::SetTimer\r\n"));
			}
		}
	}
}

const afml::traintsetimageitem *trainimagedlg::gettrainingitem(const CComboBox *p,const int n)
{
	return n == CB_ERR ? nullptr : reinterpret_cast<const afml::traintsetimageitem*>(p->GetItemData(n));
}

void trainimagedlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;

	const bool bIdle = !theApp.getthread();
	const bool bLerp = getdibtype(m_nDibSrc)==dt_lerp;
	const bool bPaused = !bIdle && theApp.getthread()->getpaused();

	const bool bImage = theApp.getlayerdlg()->getinputtype() & afml::trainsetitem::it_image;
	const bool bInputs = theApp.getinput(theApp.getlayerdlg()->getinputtype())->gettraining().size();

	GetDlgItem(IDC_IMAGE_INPUT_COMBO)->EnableWindow(bImage && bInputs);

	GetDlgItem(IDC_IMAGE_LERP_COMBO)->EnableWindow(!bIdle && bImage && bLerp);
	GetDlgItem(IDC_IMAGE_LERP_SLIDER)->EnableWindow(!bIdle && bImage && bLerp);
	GetDlgItem(IDC_IMAGE_LERP_EDIT)->EnableWindow(!bIdle && bImage && bLerp);
	GetDlgItem(IDC_IMAGE_LERP_ANIMATE)->EnableWindow(!bIdle && bImage && bLerp);
	
	GetDlgItem(IDC_IMAGE_UPDATE_BTN)->EnableWindow(!bIdle);
	GetDlgItem(IDC_IMAGE_UPDATE_EDIT)->EnableWindow(m_nUpdate==BST_CHECKED);

	GetDlgItem(IDC_IMAGE_THUMB_SMOOTH_CHECK)->EnableWindow(!bIdle && theApp.getlayerdlg()->getinputtype()==afml::trainsetitem::it_image_i_id_xy_o_b8g8r8 &&
															 getdibtype(m_nDibSrc)!=dt_input &&
															 m_nScaleToFit==BST_CHECKED);

	GetDlgItem(IDC_IMAGE_THUMB_SCALETOFIT_CHECK)->EnableWindow(bImage);

	const bool bInput = getdibtype(m_nDibSrc)==dt_input;
	GetDlgItem(IDC_IMAGE_EXPORT)->EnableWindow(bImage && ( bInput ? bInputs : bPaused ));
}

std::shared_ptr<const afdib::dib> trainimagedlg::getinputdib(void)const
{
	const int n = m_InputCombo.GetCurSel();
	const afml::traintsetimageitem *pInputItem = gettrainingitem(&m_InputCombo,n);
	return pInputItem?pInputItem->getdib(pInputItem->getoutput(),0,pInputItem->gettype(),pInputItem->getdibdim(),CRect(CPoint(0,0),pInputItem->getdibdim())):nullptr;
}

void trainimagedlg::postdibrequest(const dibtype t)
{
	if(theApp.getthread())
	{
		SIZE sz;
		std::shared_ptr<mlthreadqueryrequest> sp = createrequest(this,t,getcustomdim(t,m_nScaleToFit==BST_UNCHECKED||m_nSmooth==BST_UNCHECKED,sz));
		if(sp)
			theApp.getthread()->push_back(sp);
	}
}

SIZE trainimagedlg::getoutputdim(const SIZE& sz)const
{
	double dS;
	const bool bLetterBox = true;
	const double dSrcTLX=0,dSrcTLY=0,dSrcBRX=sz.cx,dSrcBRY=sz.cy;
	const double dDstTLX=0,dDstTLY=0,dDstBRX=m_spDibWnd->getwindowdim().cx,dDstBRY=m_spDibWnd->getwindowdim().cy;
	afml::traintsetimageitem::getrectscale(dSrcTLX,dSrcTLY,dSrcBRX,dSrcBRY,dDstTLX,dDstTLY,dDstBRX,dDstBRY,bLetterBox,dS);

	SIZE szOut;
	szOut.cx=af::posfloor<double,int>(dS*dSrcBRX);
	szOut.cy=af::posfloor<double,int>(dS*dSrcBRY);
	return szOut;
}

SIZE trainimagedlg::getlerpdim(const SIZE& szFrom,const SIZE& szTo,const double dLerp)const
{
	const double dLerpWidth = af::getlerp<int,double>(szFrom.cx,szTo.cx,dLerp);
	const double dLerpHeight = af::getlerp<int,double>(szFrom.cy,szTo.cy,dLerp);

	SIZE szOut;
	szOut.cx=af::posround<double,int>(dLerpWidth);
	szOut.cy=af::posround<double,int>(dLerpHeight);
	return szOut;
}

SIZE *trainimagedlg::getcustomdim(const dibtype t,const bool bInputDim,SIZE& sz)const
{
	const afml::traintsetimageitem *pInputItem = gettrainingitem(&m_InputCombo,m_InputCombo.GetCurSel());

	switch(t)
	{
		case dt_input:return nullptr;
		case dt_output:
		{
			if(bInputDim)
				return nullptr;
			sz=getoutputdim(pInputItem->getdibdim());
		}
		break;
		case dt_lerp:
		{
			const afml::traintsetimageitem *pFromItem = pInputItem;
			const afml::traintsetimageitem *pToItem = gettrainingitem(&m_LerpCombo,m_LerpCombo.GetCurSel());
			if(!pToItem)
				return nullptr;
			
			const double dLerp = (af::getint<true,true>(m_csLerp,0,100)/100.0);

			sz = getlerpdim(pFromItem->getdibdim(),pToItem->getdibdim(),dLerp);
			if(!bInputDim)
				sz=getoutputdim(sz);
		}
		break;
		default:ASSERT(false);return nullptr;
	}
	return &sz;
}

std::shared_ptr<mlthreadqueryrequest> trainimagedlg::createrequest(const void *pOwner,const dibtype t,const mlthreaddibrequestbasecustomdim customdim)const
{
	if(!m_spDibWnd)
		return nullptr;
	const afml::traintsetimageitem *pInputItem = gettrainingitem(&m_InputCombo,m_InputCombo.GetCurSel());
	if(!pInputItem)
		return nullptr;

	switch(t)
	{
		case dt_output:
		{
			std::shared_ptr<mlthreaddibrequest> sp(new mlthreaddibrequest(pOwner,pInputItem->getid(),m_spReqCache,customdim));
			return sp;
		}
		break;
		case dt_lerp:
		{
			const afml::traintsetimageitem *pFromItem = pInputItem;
			const afml::traintsetimageitem *pToItem = gettrainingitem(&m_LerpCombo,m_LerpCombo.GetCurSel());
			if(!pToItem)
				return nullptr;

			std::shared_ptr<mlthreadlerpdibrequest> sp(new mlthreadlerpdibrequest(pOwner,pFromItem->getid(),pToItem->getid(),af::getint<true,true>(m_csLerp,0,100)/100.0,m_spReqCache,customdim));
			return sp;
		}
		break;
		default:ASSERT(false);
	}
	return nullptr;
}

trainimagedlg::dibtype trainimagedlg::getdibtype(const int n)
{
	switch(n)
	{
		case 0:return dt_input;
		case 1:return dt_output;
		case 2:return dt_lerp;
		default:ASSERT(false);return dt_input;
	}
}

int trainimagedlg::getdibtypeindex(const dibtype t)
{
	switch(t)
	{
		case dt_input:return 0;
		case dt_output:return 1;
		case dt_lerp:return 2;
		default:ASSERT(false);return 0;
	}
}
