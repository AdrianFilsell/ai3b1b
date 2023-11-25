// layerdlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "layerdlg.h"
#include "newlayerdlg.h"
#include "ai3b1bDlg.h"
#include "serialise.h"

// layerdlg dialog

IMPLEMENT_DYNAMIC(layerdlg, CDialogEx)

layerdlg::layerdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_nInputType=getinputtypeindex(afml::trainsetitem::it_user);
	m_csInputUserPerceptrons.Format(_T("%li"),1);
	m_csInputPerceptrons=getinputperceptrons();
	m_csInputDim=getinputimagedim();

	m_nActivationFnType=-1;
	m_nActivationNormType=-1;
	m_nGradientClipType=-1;
	
	m_bInitialised=false;
	createdefaultlayers(afml::trainsetitem::it_all);
}

layerdlg::~layerdlg()
{
}

void layerdlg::DoDataExchange(CDataExchange* pDX)
{
	// pDX->m_bSaveAndValidate == save into member variables

	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_LAYER_INPUT_PERCEPTRONS_SPIN,m_InputPerceptronsSpin);
	DDX_Control(pDX,IDC_LAYER_PERCEPTRONS_SPIN,m_PerceptronsSpin);
	DDX_Control(pDX,IDC_LAYER_INPUT_DIM_SPIN,m_InputDimSpin);
	DDX_Control(pDX,IDC_LAYER_LIST,m_List);
	
	DDX_CBIndex(pDX,IDC_LAYER_INPUT_TYPE_COMBO,m_nInputType);
	DDX_CBIndex(pDX,IDC_LAYER_ACTIVATION_FN_COMBO,m_nActivationFnType);
	DDX_CBIndex(pDX,IDC_LAYER_NORMALISE_ACTIVATIONS_COMBO,m_nActivationNormType);
	DDX_CBIndex(pDX,IDC_LAYER_GRADIENT_CLIP_COMBO,m_nGradientClipType);
	
	DDX_Text(pDX,IDC_LAYER_INPUT_PERCEPTRONS_EDIT,m_csInputPerceptrons);
	DDX_Text(pDX,IDC_LAYER_PERCEPTRONS_EDIT,m_csPerceptrons);		
	DDX_Text(pDX,IDC_LAYER_GRADIENT_CLIP_EDIT,m_csGradientClipThreshold);
	DDX_Text(pDX,IDC_LAYER_INPUT_DIM_EDIT,m_csInputDim);
}


BEGIN_MESSAGE_MAP(layerdlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_LAYER_INPUT_TYPE_COMBO,layerdlg::OnInputTypeSelChange)
	ON_EN_CHANGE(IDC_LAYER_INPUT_PERCEPTRONS_EDIT,layerdlg::OnInputPerceptronsEditChange)
	ON_EN_CHANGE(IDC_LAYER_PERCEPTRONS_EDIT,layerdlg::OnPerceptronsEditChange)
	ON_EN_CHANGE(IDC_LAYER_GRADIENT_CLIP_EDIT,layerdlg::OnGradientClipEditChange)
	ON_EN_CHANGE(IDC_LAYER_INPUT_DIM_EDIT,layerdlg::OnInputDimEditChange)
	ON_EN_KILLFOCUS(IDC_LAYER_INPUT_PERCEPTRONS_EDIT,layerdlg::OnInputPerceptronsEditKillFocus)
	ON_EN_KILLFOCUS(IDC_LAYER_PERCEPTRONS_EDIT,layerdlg::OnPerceptronsEditKillFocus)
	ON_EN_KILLFOCUS(IDC_LAYER_GRADIENT_CLIP_EDIT,layerdlg::OnGradientClipEditKillFocus)
	ON_EN_KILLFOCUS(IDC_LAYER_INPUT_DIM_EDIT,layerdlg::OnInputDimEditKillFocus)
	ON_CBN_SELCHANGE(IDC_LAYER_ACTIVATION_FN_COMBO,layerdlg::OnActivationFnChange)
	ON_CBN_SELCHANGE(IDC_LAYER_NORMALISE_ACTIVATIONS_COMBO,layerdlg::OnActivationNormChange)
	ON_CBN_SELCHANGE(IDC_LAYER_GRADIENT_CLIP_COMBO,layerdlg::OnGradientClipChange)
	ON_NOTIFY(LVN_ITEMCHANGED,IDC_LAYER_LIST,layerdlg::OnSelChange)
	ON_BN_CLICKED(IDC_LAYER_UP,layerdlg::OnLayerUp)
	ON_BN_CLICKED(IDC_LAYER_DOWN,layerdlg::OnLayerDown)
	ON_BN_CLICKED(IDC_LAYER_ADD,layerdlg::OnLayerAdd)
	ON_BN_CLICKED(IDC_LAYER_ERASE,layerdlg::OnLayerErase)
END_MESSAGE_MAP()


// layerdlg message handlers

BOOL layerdlg::OnInitDialog()
{
	// call the base class
	CDialogEx::OnInitDialog();

	// setup spin ctrls, this gives 'correct' direction of change
	m_InputPerceptronsSpin.SetRange32(1,0x7fffffff);
	m_PerceptronsSpin.SetRange32(1,0x7fffffff);
	m_InputDimSpin.SetRange32(1,0x7fffffff);
	
	// insert columns	
	m_List.InsertColumn( 0, _T("layer"), LVCFMT_LEFT, 80 );
	m_List.InsertColumn( 1, _T("perceptrons"), LVCFMT_LEFT, 90 );
	m_List.InsertColumn( 2, _T("fn"), LVCFMT_LEFT, 90 );
	m_List.InsertColumn( 3, _T("normalise"), LVCFMT_LEFT, 90 );
	m_List.InsertColumn( 4, _T("gradient clip"), LVCFMT_LEFT, 90 );
	m_List.InsertColumn( 5, _T("params"), LVCFMT_LEFT, 90 );

	// populate list ctrl
	populatelistctrl();

	// update selection
	if(theApp.getinput(getinputtype())->getlayers().size()>0)
		m_List.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

	// enable/disable
	enabledisable();

	m_bInitialised = true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void layerdlg::setinputtype(const afml::trainsetitem::inputtype it)
{
	m_nInputType=getinputtypeindex(it);
	UpdateData(false);
	OnInputTypeSelChange();
}

void layerdlg::OnInputTypeSelChange(void)
{
	// update members from ui
	UpdateData(true);
	m_csInputPerceptrons=getinputperceptrons();
	m_csInputDim=getinputimagedim();
	
	// populate list ctrl
	populatelistctrl();
	
	// update selection
	clearlistctrlsel();
	if(theApp.getinput(getinputtype())->getlayers().size()>0)
		m_List.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

	// update members from list ctrl sel
	initfromlistctrl();

	// create thread
	theApp.createthread(getinputtype(),true,true);

	// update ui from members
	UpdateData(false);
	theApp.broadcasthint(&hint(hint::t_input_type));
}

void layerdlg::OnInputPerceptronsEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);	
	switch(getinputtype())
	{
		case afml::trainsetitem::it_user:m_csInputUserPerceptrons=m_csInputPerceptrons;break;
	}
	
	// update layers
	const auto it = getinputtype();
	layerparams p(theApp.getinput(it)->getlayers());
	std::vector<std::shared_ptr<afml::layer<>>> vHiddenOutput(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	theApp.getinput(it)->setlayers(vHiddenOutput);

	// populate list ctrl ( inplace i.e. no items deleted just reinitialised )
	populatelistctrl();
}

void layerdlg::OnInputPerceptronsEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csInputPerceptrons,1));
	const bool bMutated = cs!=m_csInputPerceptrons;
	m_csInputPerceptrons=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_LAYER_INPUT_PERCEPTRONS_EDIT)->SetWindowText(cs);
	switch(getinputtype())
	{
		case afml::trainsetitem::it_user:m_csInputUserPerceptrons=m_csInputPerceptrons;break;
	}
}

void layerdlg::OnSelChange(NMHDR* pNMHDR,LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	*pResult = 0;

	if(pNMListView->uChanged != LVIF_STATE)
		return;

	// update members from list ctrl sel
	initfromlistctrl();
	
	// update ui from members
	UpdateData(false);
	
	// enable/disable
	enabledisable();
}

afx_msg void layerdlg::OnLayerUp(void)
{
	// what can be moved up move 1 space
	movelayers(true);
}

afx_msg void layerdlg::OnLayerDown(void)
{
	// what can be moved down move 1 space
	movelayers(false);
}

void layerdlg::OnLayerAdd(void)
{
	// selection
	const auto it  = getinputtype();
	const sel s(this,false,it==afml::trainsetitem::it_user?false:true);
	layerparams p(theApp.getinput(it)->getlayers());
	
	// layer dlg
	newlayerdlg dlg(this);
	if(!s.isempty())
	{
		dlg.m_nActivationFnType = getactivationfntype(p.getactivationfn()[0].gettype());
		dlg.m_csPerceptrons.Format(_T("%li"),p.getperceptrons()[0]);
		dlg.m_nActivationNormType = getactivationnormtype(p.getactivationnorm()[0].gettype());
		dlg.m_nGradientClipType = getgradientcliptype(p.getgradientclip()[0].gettype());
		dlg.m_csGradientClipThreshold = m_csGradientClipThreshold;
	}
	diasableui disui;
	if(dlg.DoModal()!=IDOK)
		return;

	// update layers
	const int nInsert = s.getindices().size() ? s.getindices()[0] : s.getoutput();
	p.getperceptrons().insert(p.getperceptrons().cbegin()+nInsert,af::getint<true>(dlg.m_csPerceptrons));
	p.getactivationfn().insert(p.getactivationfn().cbegin()+nInsert,getactivationfntype(dlg.m_nActivationFnType));
	p.getactivationnorm().insert(p.getactivationnorm().cbegin()+nInsert,getactivationnormtype(dlg.m_nActivationNormType));
	p.getgradientclip().insert(p.getgradientclip().cbegin()+nInsert,afml::gradientclip<>::t_null);
	std::vector<std::shared_ptr<afml::layer<>>> vHiddenOutput(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	theApp.getinput(it)->setlayers(vHiddenOutput);

	// populate list ctrl
	populatelistctrl();
	
	// update selection
	clearlistctrlsel();
	if(theApp.getinput(it)->getlayers().size()>0)
		m_List.SetItemState(nInsert, LVIS_SELECTED, LVIS_SELECTED);

	// update members from list ctrl sel
	initfromlistctrl();

	// update ui from members
	UpdateData(false);

	// enable/disable
	enabledisable();
}

void layerdlg::OnLayerErase(void)
{
	// selection
	const sel s(this);

	// anything to do?
	if(!s.getindices().size())
		return;

	// update layers
	const auto it  = getinputtype();
	std::vector<std::shared_ptr<afml::layer<>>> vLayers=theApp.getinput(it)->getlayers();
	const int nInputPerceptrons = vLayers[0]->getprevperceptrons();
	auto i = s.getindices().crbegin(),end=s.getindices().crend();
	for(;i!=end;++i)
		vLayers.erase(vLayers.cbegin()+(*i));
	for(int n=*s.getindices().cbegin();n<static_cast<int>(vLayers.size());++n)
	{
		const afml::layer<> *pL = vLayers[n].get();
		const int nPrevPerceptrons=n==0?nInputPerceptrons:vLayers[n-1]->getperceptrons();
		std::shared_ptr<afml::layer<>> spL(new afml::layer<>(pL->getperceptrons(),nPrevPerceptrons,pL->getactivationfn(),
															 pL->getactivationnorm(),pL->getgradientclip()));
		vLayers[n]=spL;
	}
	{
		afml::layer<>* pPrev=nullptr;
		auto i = vLayers.cbegin(),end=vLayers.cend();
		for(;i!=end;++i)
		{
			(*i)->setprev(pPrev);
			if(pPrev)
				pPrev->setnext((*i).get());
		}
	}
	theApp.getinput(it)->setlayers(vLayers);

	// populate list ctrl
	populatelistctrl();
	
	// update selection
	clearlistctrlsel();
	const int nSel = (*s.getindices().crbegin())-int(s.getindices().size())+1;
	m_List.SetItemState(nSel, LVIS_SELECTED, LVIS_SELECTED);

	// update members from list ctrl sel
	initfromlistctrl();

	// update ui from members
	UpdateData(false);

	// enable/disable
	enabledisable();
}

void layerdlg::OnPerceptronsEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// selection
	const auto it = getinputtype();
	const sel s(this,false,it==afml::trainsetitem::it_user?false:true);
	if(s.isempty())
		return;
	
	// update layers
	const int nPerceptrons = af::getint<true>(m_csPerceptrons,1);
	layerparams p(theApp.getinput(it)->getlayers());
	auto i=s.getindices().cbegin(),end=s.getindices().cend();
	for(;i!=end;++i)
		p.getperceptrons()[*i]=nPerceptrons;
	theApp.getinput(it)->setlayers(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	
	// populate list ctrl ( inplace i.e. no items deleted just reinitialised )
	populatelistctrl();
}

void layerdlg::OnPerceptronsEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csPerceptrons,1));
	const bool bMutated = cs!=m_csPerceptrons;
	m_csPerceptrons=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_LAYER_PERCEPTRONS_EDIT)->SetWindowText(cs);
}

void layerdlg::OnInputDimEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);
	
	// update layers
	const int nDim = af::getint<true>(m_csInputDim,1);
	const int nOutputPixels = nDim * nDim;
	const auto it = getinputtype();
	switch(it)
	{
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			const int nInputPerceptrons = theApp.getinput(it)->getlayers()[0]->getprevperceptrons();

			layerparams p(theApp.getinput(it)->getlayers());
			if(p.getperceptrons().size()>0)
			{
				p.getperceptrons()[p.getperceptrons().size()-1]=nOutputPixels*3;
				theApp.getinput(it)->setlayers(p.createlayers(nInputPerceptrons));
				theApp.getinput(it)->setimagedim(nDim);
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			const int nInputPerceptrons = theApp.getinput(it)->getlayers()[0]->getprevperceptrons();

			layerparams p(theApp.getinput(it)->getlayers());
			if(p.getperceptrons().size()>0)
			{
				p.getperceptrons()[p.getperceptrons().size()-1]=3;
				theApp.getinput(it)->setlayers(p.createlayers(nInputPerceptrons));
				theApp.getinput(it)->setimagedim(nDim);
			}
		}
		break;
	}
	
	// populate list ctrl ( inplace i.e. no items deleted just reinitialised )
	populatelistctrl();
}

void layerdlg::OnInputDimEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csInputDim,1));
	const bool bMutated = cs!=m_csInputDim;
	m_csInputDim=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_LAYER_INPUT_DIM_EDIT)->SetWindowText(cs);
}

void layerdlg::OnActivationFnChange(void)
{
	// update members from ui
	UpdateData(true);

	// selection
	const sel s(this);

	// anything to do?
	if(!s.getindices().size())
		return;

	// update layers
	const auto it  = getinputtype();
	const afml::activationfn actFn = getactivationfntype(m_nActivationFnType);
	layerparams p(theApp.getinput(it)->getlayers());
	auto i = s.getindices().crbegin(),end=s.getindices().crend();
	for(;i!=end;++i)
		p.getactivationfn()[*i]=actFn;
	theApp.getinput(it)->setlayers(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	
	// populate list ctrl
	populatelistctrl();
}

void layerdlg::OnActivationNormChange(void)
{
	// update members from ui
	UpdateData(true);

	// selection
	const sel s(this);

	// anything to do?
	if(!s.getindices().size())
		return;

	// update layers	
	const auto it  = getinputtype();
	const afml::activationnorm<> actNormFn = getactivationnormtype(m_nActivationNormType);
	layerparams p(theApp.getinput(it)->getlayers());
	auto i = s.getindices().crbegin(),end=s.getindices().crend();
	for(;i!=end;++i)
		p.getactivationnorm()[*i]=actNormFn;
	theApp.getinput(it)->setlayers(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	
	// populate list ctrl
	populatelistctrl();
}

void layerdlg::OnGradientClipChange(void)
{
	// update members from ui
	UpdateData(true);

	// selection
	const sel s(this);

	// anything to do?
	if(!s.getindices().size())
		return;

	// update layers
	const auto it = getinputtype();
	const afml::gradientclip<> gradClip = getgradientcliptype(m_nGradientClipType);
	layerparams p(theApp.getinput(it)->getlayers());
	auto i = s.getindices().crbegin(),end=s.getindices().crend();
	for(;i!=end;++i)
		p.getgradientclip()[*i]=gradClip;
	theApp.getinput(it)->setlayers(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	
	// populate list ctrl
	populatelistctrl();

	// update members from list ctrl sel
	initfromlistctrl();

	// update ui from members
	UpdateData(false);

	// enable/disable
	enabledisable();
}

void layerdlg::OnGradientClipEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// selection
	const auto it = getinputtype();
	const sel s(this,false,it==afml::trainsetitem::it_user?false:true);
	if(s.isempty())
		return;
	
	// update layers
	const double dGradClip = af::getfloat<>(m_csGradientClipThreshold);
	layerparams p(theApp.getinput(it)->getlayers());
	auto i=s.getindices().cbegin(),end=s.getindices().cend();
	for(;i!=end;++i)
		p.getgradientclip()[*i].setthreshold(dGradClip);
	theApp.getinput(it)->setlayers(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));
	
	// populate list ctrl ( inplace i.e. no items deleted just reinitialised )
	populatelistctrl();
}

void layerdlg::OnGradientClipEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%f"),af::getfloat<>(m_csGradientClipThreshold));
	const bool bMutated = cs!=m_csGradientClipThreshold;
	m_csGradientClipThreshold=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_LAYER_GRADIENT_CLIP_EDIT)->SetWindowText(cs);
}

void layerdlg::onhint(const hint *p)
{
	switch(p->gettype())
	{
		case hint::t_thread_playconfig_results:
		{
			const threadplayconfigresulthint *pH=static_cast<const threadplayconfigresulthint*>(p);
			
			const int nTypes = (mlthreadplayconfigrequest::t_play|mlthreadplayconfigrequest::t_pause);
			if(pH && pH->getresults() && (pH->getresults()->gettypes() & nTypes))
				enabledisable();
		}
		break;
		case hint::t_clear_epochs:
		case hint::t_input_type:enabledisable();break;
		case hint::t_load:
		{
			// update members from ui
			m_csInputPerceptrons=getinputperceptrons();
		
			// populate list ctrl
			clearlistctrlsel();
			m_List.DeleteAllItems();
			populatelistctrl();
	
			// update selection
			clearlistctrlsel();
			if(theApp.getinput(getinputtype())->getlayers().size()>0)
				m_List.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);

			// update members from list ctrl sel
			initfromlistctrl();

			// update ui from members
			UpdateData(false);
		
			// enable/disable
			enabledisable();
		}
		break;
	}
}

void layerdlg::movelayers(const bool bUp)
{
	// selection
	const sel s(this);
	const movablesel ms(&s,bUp);

	// anything to do?
	if( ms.getlayers().size()==0)
		return;

	// update layers
	const auto it  = getinputtype();
	layerparams p(theApp.getinput(it)->getlayers());
	auto i = ms.getindices().cbegin(),end = ms.getindices().cend();
	for(;i!=end;++i)
		p.swap(bUp?(*i)-1:(*i)+1,*i);
	theApp.getinput(it)->setlayers(p.createlayers(af::getint<true>(m_csInputPerceptrons,1)));

	// populate list ctrl
	populatelistctrl();

	// update selection
	clearlistctrlsel();
	i = ms.getindices().cbegin();
	end = ms.getindices().cend();
	for(;i!=end;++i)
	{
		const int nSel = bUp?(*i)-1:(*i)+1;
		m_List.SetItemState(nSel, LVIS_SELECTED, LVIS_SELECTED);
	}

	// update members from list ctrl sel
	initfromlistctrl();

	// update ui from members
	UpdateData(false);

	// enable/disable
	enabledisable();
}

void layerdlg::clearlistctrlsel(void)
{
	const sel s(this);

	auto i = s.getindices().cbegin(),end=s.getindices().cend();
	for(;i!=end;++i)
	{
		UINT nState = m_List.GetItemState(*i, LVIS_SELECTED);
		nState &= ~LVIS_SELECTED;
		const BOOL b = m_List.SetItemState(*i, nState, LVIS_SELECTED);
	}
}

afml::trainsetitem::inputtype layerdlg::getinputtype(const int n)
{
	switch(n)
	{
		case 0:return afml::trainsetitem::it_user;
		case 1:return afml::trainsetitem::it_image_i_id_o_b8g8r8;
		case 2:return afml::trainsetitem::it_image_i_id_xy_o_b8g8r8;
		default:return afml::trainsetitem::it_user;
	}
}

int layerdlg::getinputtypeindex(const afml::trainsetitem::inputtype t)
{
	switch(t)
	{
		case afml::trainsetitem::it_user:return 0;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:return 1;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:return 2;
		default:return 0;
	}
}

CString layerdlg::getinputtypename(const afml::trainsetitem::inputtype t)
{
	switch(t)
	{
		case afml::trainsetitem::it_user:return _T("User");
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:return _T("Image id -> b8g8r8");
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:return _T("Image id,x,y -> b8g8r8");
		default:return _T("");
	}
}

afml::activationfn::type layerdlg::getactivationfntype(const int n)
{
	switch(n)
	{
		case 0:return afml::activationfn::t_normtanh;
		case 1:return afml::activationfn::t_tanh;
		case 2:return afml::activationfn::t_sigmoid;
		case 3:return afml::activationfn::t_relu;
		case 4:return afml::activationfn::t_leaky_relu;
		case 5:return afml::activationfn::t_silu;
		case 6:return afml::activationfn::t_gelu;
		case 7:return afml::activationfn::t_softplus;
		default:return afml::activationfn::t_normtanh;
	}
}

int layerdlg::getactivationfntypeindex(const afml::activationfn::type t)
{
	switch(t)
	{
		case afml::activationfn::t_normtanh:return 0;
		case afml::activationfn::t_tanh:return 1;
		case afml::activationfn::t_sigmoid:return 2;
		case afml::activationfn::t_relu:return 3;
		case afml::activationfn::t_leaky_relu:return 4;
		case afml::activationfn::t_silu:return 5;
		case afml::activationfn::t_gelu:return 6;
		case afml::activationfn::t_softplus:return 7;
		default:return 0;
	}
}

afml::activationnorm<>::type layerdlg::getactivationnormtype(const int n)
{
	switch(n)
	{
		case 0:return afml::activationnorm<>::t_null;
		case 1:return afml::activationnorm<>::t_minmax;
		case 2:return afml::activationnorm<>::t_standardise;
		default:return afml::activationnorm<>::t_minmax;
	}
}

int layerdlg::getactivationnormtypeindex(const afml::activationnorm<>::type t)
{
	switch(t)
	{
		case afml::activationnorm<>::t_null:return 0;
		case afml::activationnorm<>::t_minmax:return 1;
		case afml::activationnorm<>::t_standardise:return 2;
		default:return 0;
	}
}

afml::gradientclip<>::type layerdlg::getgradientcliptype(const int n)
{
	switch(n)
	{
		case 0:return afml::gradientclip<>::t_null;
		case 1:return afml::gradientclip<>::t_threshold;
		default:return afml::gradientclip<>::t_threshold;
	}
}

int layerdlg::getgradientcliptypeindex(const afml::gradientclip<>::type t)
{
	switch(t)
	{
		case afml::gradientclip<>::t_null:return 0;
		case afml::gradientclip<>::t_threshold:return 1;
		default:return 0;
	}
}

CString layerdlg::getinputperceptrons(void) const
{
	switch(getinputtype())
	{
		case afml::trainsetitem::it_user:return m_csInputUserPerceptrons;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:return CString(_T("1"));
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:return CString(_T("3"));
		default:return m_csInputUserPerceptrons;
	}
}

CString layerdlg::getinputimagedim(void) const
{
	CString cs;
	switch(getinputtype())
	{
		case afml::trainsetitem::it_user:return _T("");
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:cs.Format(_T("%li"),theApp.getinput(getinputtype())->getimagedim());break;
		default:ASSERT(false);
	}
	return cs;
}

void layerdlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;

	const HWND hPreFocus = ::GetFocus();

	const bool bIdle = !theApp.getthread();
	const bool bImageInputType = getinputtype()&afml::trainsetitem::it_image;
		
	const sel s(this);

	GetDlgItem(IDC_LAYER_INPUT_TYPE_COMBO)->EnableWindow(true);
	
	GetDlgItem(IDC_LAYER_INPUT_DIM_EDIT)->EnableWindow(bIdle && bImageInputType);

	switch(getinputtype())
	{
		case afml::trainsetitem::it_user:
		{
			GetDlgItem(IDC_LAYER_PERCEPTRONS_EDIT)->EnableWindow(bIdle &&  s.getindices().size()>0);
			GetDlgItem(IDC_LAYER_INPUT_PERCEPTRONS_EDIT)->EnableWindow(bIdle);
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			GetDlgItem(IDC_LAYER_PERCEPTRONS_EDIT)->EnableWindow(bIdle && !s.isoutputselected() && s.getindices().size()>0);
			GetDlgItem(IDC_LAYER_INPUT_PERCEPTRONS_EDIT)->EnableWindow(false);
		}
		break;
		default:ASSERT(false);
	}

	GetDlgItem(IDC_LAYER_UP)->EnableWindow(bIdle && !s.isoutputselected() && movablesel(&s,true).getindices().size()>0);
	GetDlgItem(IDC_LAYER_DOWN)->EnableWindow(bIdle && !s.isoutputselected() && movablesel(&s,false).getindices().size()>0);

	GetDlgItem(IDC_LAYER_ADD)->EnableWindow(bIdle);
	GetDlgItem(IDC_LAYER_ERASE)->EnableWindow(bIdle && !s.isoutputselected() && s.getindices().size()>0);

	GetDlgItem(IDC_LAYER_ACTIVATION_FN_COMBO)->EnableWindow(bIdle && s.getindices().size()>0);

	GetDlgItem(IDC_LAYER_NORMALISE_ACTIVATIONS_COMBO)->EnableWindow(bIdle && s.getindices().size()>0);

	GetDlgItem(IDC_LAYER_GRADIENT_CLIP_COMBO)->EnableWindow(bIdle && s.getindices().size()>0);
	switch(getgradientcliptype(m_nGradientClipType))
	{
		case afml::gradientclip<>::t_threshold:GetDlgItem(IDC_LAYER_GRADIENT_CLIP_EDIT)->EnableWindow(bIdle && s.getindices().size()>0);break;
		default:GetDlgItem(IDC_LAYER_GRADIENT_CLIP_EDIT)->EnableWindow(false);
	}

	validatefocus(hPreFocus);
}

void layerdlg::populatelistctrl(void)
{
	const std::vector<std::shared_ptr<afml::layer<>>>& v=theApp.getinput(getinputtype())->getlayers();

	const bool bInsert = (m_List.GetItemCount()!=int(v.size()));
	if(bInsert)
	{
		clearlistctrlsel();
		m_List.DeleteAllItems();
	}

	int nParams=0;
	auto i=v.cbegin(),end=v.cend(); 
	for(int n=0;i!=end;++i,++n)
	{
		nParams += (*i)->getparams();

		const bool bOutput=n==int(v.size()-1);
		CString csLayer;
		if(bOutput)
			csLayer=_T("output");
		else
			csLayer.Format(_T("hidden [%li]"),n);

		if(bInsert)
		{
			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.iItem = n;
			lvi.iSubItem = 0;
			lvi.pszText = (LPTSTR)(LPCTSTR)(csLayer);
			const int nItem = m_List.InsertItem( &lvi ); // nItem == n because unsorted list ctrl
		}
		else
			setlistctrlcolumntext(&m_List,n,0,csLayer);
		setlistctrllayer(n,(*i).get());
	}
	CString csFmt(_T("Params: %li / %.2f KB")),cs;
	cs.Format(csFmt,nParams,(nParams*sizeof(double))/1024.0);
	GetDlgItem(IDC_PARAMS_TEXT)->SetWindowText(cs);
}

void layerdlg::setlistctrllayer(const int n,const afml::layer<> *pL)
{
	CString cs;
	const BOOL b = m_List.SetItemData( n, DWORD_PTR(pL));

	cs.Format(_T("%li"),pL->getperceptrons());
	setlistctrlcolumntext(&m_List,n,1,cs);
		
	setlistctrlcolumntext(&m_List,n,2,getactivationfn(pL->getactivationfn().gettype()));

	setlistctrlcolumntext(&m_List,n,3,getactivationnorm(pL->getactivationnorm().gettype()));
		
	setlistctrlcolumntext(&m_List,n,4,getgradientclip(pL->getgradientclip().gettype()));

	cs.Format(_T("%li"),pL->getparams());
	setlistctrlcolumntext(&m_List,n,5,cs);
}

void layerdlg::setlistctrlcolumntext(CListCtrl *pList,const int n,const int nC,const CString& cs)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = n;
	lvi.iSubItem = nC;
	lvi.pszText = (LPTSTR)(LPCTSTR)(cs);
	const BOOL b = pList->SetItem( &lvi );
}

void layerdlg::initfromlistctrl(void)
{
	const sel s(this);

	if(s.getlayers().size())
	{
		m_csPerceptrons.Format(_T("%li"),s.getlayers()[0]->getperceptrons());
		m_nActivationFnType=getactivationfntypeindex(s.getlayers()[0]->getactivationfn().gettype());
		m_nActivationNormType=getactivationnormtypeindex(s.getlayers()[0]->getactivationnorm().gettype());
		m_nGradientClipType=getgradientcliptypeindex(s.getlayers()[0]->getgradientclip().gettype());
		m_csGradientClipThreshold.Format(_T("%f"),s.getlayers()[0]->getgradientclip().getthreshold());
	}
	else
	{
		m_csPerceptrons=_T("");
		m_nActivationFnType=-1;
		m_nActivationNormType=-1;
		m_nGradientClipType=-1;
		m_csGradientClipThreshold=_T("");
	}
}

CString layerdlg::getactivationfn(const afml::activationfn::type t) const
{
	switch(t)
	{
		case afml::activationfn::t_normtanh:return _T("Tanh [0,1]");
		case afml::activationfn::t_tanh:return _T("Tanh [1,-1]");
		case afml::activationfn::t_sigmoid:return _T("Sigmoid");
		case afml::activationfn::t_relu:return _T("ReLU");
		case afml::activationfn::t_leaky_relu:return _T("Leaky ReLU");
		case afml::activationfn::t_silu:return _T("SiLU");
		case afml::activationfn::t_softplus:return _T("SoftPlus");
		case afml::activationfn::t_gelu:return _T("GeLu");
		default:return _T("");
	}
}

CString layerdlg::getactivationnorm(const afml::activationnorm<>::type t) const
{
	switch(t)
	{
		case afml::activationnorm<>::t_null:return _T("None");
		case afml::activationnorm<>::t_minmax:return _T("MinMax");
		case afml::activationnorm<>::t_standardise:return _T("Standardise");
		default:return _T("");
	}
}

CString layerdlg::getgradientclip(const afml::gradientclip<>::type t) const
{
	switch(t)
	{
		case afml::gradientclip<>::t_null:return _T("None");
		case afml::gradientclip<>::t_threshold:return _T("Threshold");
		default:return _T("");
	}
}

void layerdlg::createdefaultlayers(const int nTypes)
{
	std::vector<afml::trainsetitem::inputtype> vIts;
	if(nTypes & afml::trainsetitem::it_user) vIts.push_back(afml::trainsetitem::it_user);
	if(nTypes & afml::trainsetitem::it_image_i_id_o_b8g8r8) vIts.push_back(afml::trainsetitem::it_image_i_id_o_b8g8r8);
	if(nTypes & afml::trainsetitem::it_image_i_id_xy_o_b8g8r8) vIts.push_back(afml::trainsetitem::it_image_i_id_xy_o_b8g8r8);
	int nInputPerceptrons=af::getint<true>(m_csInputPerceptrons,1);
	auto i = vIts.cbegin(),end = vIts.cend();
	for(;i!=end;++i)
	{
		layerparams p;
		switch(*i)
		{
			case afml::trainsetitem::it_user:
			{
				p.getperceptrons() = {3,3};
				p.getactivationfn() = std::vector<afml::activationfn>(p.getperceptrons().size(),afml::activationfn::t_normtanh);
				p.getactivationnorm() = std::vector<afml::activationnorm<>>(p.getperceptrons().size(),afml::activationnorm<>::t_null);
				p.getgradientclip() = std::vector<afml::gradientclip<>>(p.getperceptrons().size(),afml::gradientclip<>::t_null);
				
				p.getperceptrons().push_back(1);
				p.getactivationfn().push_back(afml::activationfn::t_normtanh);
				p.getactivationnorm().push_back(afml::activationnorm<>::t_null);
				p.getgradientclip().push_back(afml::gradientclip<>::t_null);
			}
			break;
			case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
			{
				nInputPerceptrons = 3;
				
				p.getperceptrons() = {30,30,50,50};
				p.getactivationfn() = std::vector<afml::activationfn>(p.getperceptrons().size(),afml::activationfn::t_tanh);
				p.getactivationnorm() = std::vector<afml::activationnorm<>>(p.getperceptrons().size(),afml::activationnorm<>::t_null);
				p.getgradientclip() = std::vector<afml::gradientclip<>>(p.getperceptrons().size(),afml::gradientclip<>::t_null);
				
				p.getperceptrons().push_back(3);
				p.getactivationfn().push_back(afml::activationfn::t_normtanh);
				p.getactivationnorm().push_back(afml::activationnorm<>::t_null);
				p.getgradientclip().push_back(afml::gradientclip<>::t_null);

				theApp.getinput(*i)->setlearningrate(0.9);
			}
			break;
			case afml::trainsetitem::it_image_i_id_o_b8g8r8:
			{
				nInputPerceptrons = 1;

				p.getperceptrons() = {15};
				p.getactivationfn() = std::vector<afml::activationfn>(p.getperceptrons().size(),afml::activationfn::t_leaky_relu);
				p.getactivationnorm() = std::vector<afml::activationnorm<>>(p.getperceptrons().size(),afml::activationnorm<>::t_minmax);
				p.getgradientclip() = std::vector<afml::gradientclip<>>(p.getperceptrons().size(),afml::gradientclip<>::t_null);
				
				const int nDim = theApp.getinput(*i)->getimagedim();
				const int nOutputPixels = nDim * nDim;
				p.getperceptrons().push_back(nOutputPixels*3);
				p.getactivationfn().push_back(afml::activationfn::t_normtanh);
				p.getactivationnorm().push_back(afml::activationnorm<>::t_null);
				p.getgradientclip().push_back(afml::gradientclip<>::t_null);

				theApp.getinput(*i)->setlearningrate(0.9);
			}
			break;
			default:ASSERT(false);
		}
		std::vector<std::shared_ptr<afml::layer<>>> vHiddenOutput(p.createlayers(nInputPerceptrons));
		theApp.getinput(*i)->setlayers(vHiddenOutput);
	}
}

void layerdlg::validatefocus(const HWND hPreFocus)
{
	const HWND hFocus = ::GetFocus();
	if(hFocus)
		return;
	if(!hPreFocus)
		return;
	NextDlgCtrl();
}

afml::trainsetitem::inputtype layerdlg::getinputtype(void) const
{
	return getinputtype(m_nInputType);
}

bool layerdlg::read(const serialise *pS)
{
	// version
	int nVersion;
	if(!pS->read<>(nVersion))
		return false;

	// members
	int nDim;
	if(!pS->read<>(nDim))
		return false;
	m_csInputDim.Format(_T("%li"),nDim);
	if(!pS->read<>(m_csInputUserPerceptrons))
		return false;

	return true;
}

bool layerdlg::write(const serialise *pS) const
{
	// version
	const int nVersion = 1;
	if(!pS->write<>(nVersion))
		return false;

	// members
	const int nDim = af::getint<true>(m_csInputDim,1);
	if(!pS->write<>(nDim) || !pS->write<>(m_csInputUserPerceptrons))
		return false;

	return true;
}

layerdlg::sel::sel(layerdlg *p,const bool bRemoveHidden,const bool bRemoveOutput):layerstg()
{
	const auto it  = p->getinputtype();
	m_nOutput = static_cast<int>(theApp.getinput(it)->getlayers().size()-1);
	
	m_vLayers.reserve(p->m_List.GetSelectedCount());
	m_vIndices.reserve(p->m_List.GetSelectedCount());
	POSITION pos = p->m_List.GetFirstSelectedItemPosition();
	while(pos!=NULL)
	{
		const int nSel = p->m_List.GetNextSelectedItem(pos);
		afml::layer<> *pL = reinterpret_cast<afml::layer<>*>(p->m_List.GetItemData(nSel));
		const bool bOutput = nSel==m_nOutput;
		if( (bRemoveHidden && !bOutput) || (bRemoveOutput && bOutput) )
			continue;
		m_vLayers.push_back(pL);
		m_vIndices.push_back(nSel);
		m_bOutput=m_bOutput||bOutput;
	}
}

layerdlg::movablesel::movablesel(const sel *p,const bool bUp):layerstg()
{
	m_bOutput=false;
	m_nOutput=p->getoutput();
	m_vLayers.reserve(p->getindices().size());
	m_vIndices.reserve(p->getindices().size());
	
	auto iI=p->getindices().cbegin(),end=p->getindices().cend();
	auto iL=p->getlayers().cbegin();
	
	for(;iI!=end;++iI,++iL)
	{
		const int nSel = *iI;
		if(nSel==m_nOutput)
			continue;
		if((bUp && nSel>0) || (!bUp && nSel<(m_nOutput-1)))
		{
			afml::layer<> *pL = *iL;
			m_vIndices.push_back(nSel);
			m_vLayers.push_back(*iL);
		}
	}
}

layerdlg::outputlayer::outputlayer(layerdlg *p):layerstg()
{
	const auto it = p->getinputtype();
	m_nOutput = static_cast<int>(theApp.getinput(it)->getlayers().size()-1);
	
	m_vLayers.reserve(p->m_List.GetSelectedCount());
	m_vIndices.reserve(p->m_List.GetSelectedCount());
	if(p->m_List.GetItemCount()==0)
		return;
	
	const int nItem = p->m_List.GetItemCount()-1;
	afml::layer<> *pL = reinterpret_cast<afml::layer<>*>(p->m_List.GetItemData(nItem));
	m_vLayers.push_back(pL);
	m_vIndices.push_back(nItem);
	m_bOutput=true;
}

template <typename T> layerdlg::layerparams::layerparams(const std::vector<T>& v)
{
	auto i = v.cbegin(),end=v.cend();
	for(;i!=end;++i)
	{
		m_vPerceptrons.push_back((*i)->getperceptrons());
		m_vActFns.push_back((*i)->getactivationfn());
		m_vNormActFns.push_back((*i)->getactivationnorm().gettype());
		m_vGClips.push_back((*i)->getgradientclip().gettype());
	}
}

std::vector<std::shared_ptr<afml::layer<>>> layerdlg::layerparams::createlayers(const int nInputPerceptrons) const
{
	std::shared_ptr<afml::layer<>> spHiddenLayer;
	std::vector<std::shared_ptr<afml::layer<>>> vHiddenOutput;

	auto iP=m_vPerceptrons.cbegin(), end = m_vPerceptrons.cend();
	auto iA=m_vActFns.cbegin();
	auto iNA=m_vNormActFns.cbegin();
	auto iGC=m_vGClips.cbegin();
	for(--end;iP!=end;++iP,++iA,++iNA,++iGC)
	{
		std::shared_ptr<afml::layer<>> spLayer(new afml::layer<>(*iP,spHiddenLayer?spHiddenLayer->getperceptrons():nInputPerceptrons,*iA,*iNA,*iGC));
		vHiddenOutput.push_back(spLayer);
		spHiddenLayer = spLayer;
	}
	std::shared_ptr<afml::layer<>> spOutputLayer(new afml::layer<>(*iP,spHiddenLayer?spHiddenLayer->getperceptrons():nInputPerceptrons,*iA,*iNA,*iGC));
	vHiddenOutput.push_back(spOutputLayer);
	{
		afml::layer<>* pPrev=nullptr;
		auto i = vHiddenOutput.cbegin(),end=vHiddenOutput.cend();
		for(;i!=end;++i)
		{
			(*i)->setprev(pPrev);
			if(pPrev)
				pPrev->setnext((*i).get());
		}
	}
	return vHiddenOutput;
}
