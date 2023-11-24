// newlayerdlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "newlayerdlg.h"
#include "layerdlg.h"

// newlayerdlg dialog

IMPLEMENT_DYNAMIC(newlayerdlg, CDialogEx)

newlayerdlg::newlayerdlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_csPerceptrons.Format(_T("%li"),1);
	m_nActivationFnType=layerdlg::getactivationfntypeindex(afml::activationfn::t_normtanh);
	m_nActivationNormType=layerdlg::getactivationnormtypeindex(afml::activationnorm<>::t_null);
	m_nGradientClipType=layerdlg::getgradientcliptypeindex(afml::gradientclip<>::t_null);
	m_csGradientClipThreshold.Format(_T("%f"),3.0);
	
	m_bInitialised=false;
}

newlayerdlg::~newlayerdlg()
{
}

void newlayerdlg::DoDataExchange(CDataExchange* pDX)
{
	// pDX->m_bSaveAndValidate == save into member variables

	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_LAYER_PERCEPTRONS_SPIN,m_PerceptronsSpin);
	
	DDX_CBIndex(pDX,IDC_LAYER_ACTIVATION_FN_COMBO,m_nActivationFnType);
	DDX_CBIndex(pDX,IDC_LAYER_NORMALISE_ACTIVATIONS_COMBO,m_nActivationNormType);
	DDX_CBIndex(pDX,IDC_LAYER_GRADIENT_CLIP_COMBO,m_nGradientClipType);
	
	DDX_Text(pDX,IDC_LAYER_PERCEPTRONS_EDIT,m_csPerceptrons);
	DDX_Text(pDX,IDC_LAYER_GRADIENT_CLIP_EDIT,m_csGradientClipThreshold);
}


BEGIN_MESSAGE_MAP(newlayerdlg, CDialogEx)
	ON_EN_CHANGE(IDC_LAYER_PERCEPTRONS_EDIT,newlayerdlg::OnPerceptronsEditChange)
	ON_EN_KILLFOCUS(IDC_LAYER_PERCEPTRONS_EDIT,newlayerdlg::OnPerceptronsEditKillFocus)
	ON_CBN_SELCHANGE(IDC_LAYER_ACTIVATION_FN_COMBO,newlayerdlg::OnActivationFnChange)
	ON_CBN_SELCHANGE(IDC_LAYER_NORMALISE_ACTIVATIONS_COMBO,newlayerdlg::OnActivationNormChange)
	ON_EN_CHANGE(IDC_LAYER_GRADIENT_CLIP_EDIT,newlayerdlg::OnGradientClipEditChange)
	ON_EN_KILLFOCUS(IDC_LAYER_GRADIENT_CLIP_EDIT,newlayerdlg::OnGradientClipEditKillFocus)
	ON_CBN_SELCHANGE(IDC_LAYER_GRADIENT_CLIP_COMBO,newlayerdlg::OnGradientClipChange)
END_MESSAGE_MAP()


// newlayerdlg message handlers

BOOL newlayerdlg::OnInitDialog()
{
	// call the base class
	CDialogEx::OnInitDialog();

	// setup spin ctrls, this gives 'correct' direction of change
	m_PerceptronsSpin.SetRange32(1,0x7fffffff);

	// enable/disable
	enabledisable();

	m_bInitialised = true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void newlayerdlg::OnPerceptronsEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);
}

void newlayerdlg::OnPerceptronsEditKillFocus(void)
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

void newlayerdlg::OnActivationFnChange(void)
{
	// update members from ui
	UpdateData(true);
}

void newlayerdlg::OnActivationNormChange(void)
{
	// update members from ui
	UpdateData(true);
}

void newlayerdlg::OnGradientClipChange(void)
{
	// update members from ui
	UpdateData(true);

	// enable/disable
	enabledisable();
}

void newlayerdlg::OnGradientClipEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);
}

void newlayerdlg::OnGradientClipEditKillFocus(void)
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

void newlayerdlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;

	switch(layerdlg::getgradientcliptype(m_nGradientClipType))
	{
		case afml::gradientclip<>::t_threshold:GetDlgItem(IDC_LAYER_GRADIENT_CLIP_EDIT)->EnableWindow(true);break;
		default:GetDlgItem(IDC_LAYER_GRADIENT_CLIP_EDIT)->EnableWindow(false);
	}
}
