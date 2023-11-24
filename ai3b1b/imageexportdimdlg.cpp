// imageexportdimdlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "imageexportdimdlg.h"


// imageexportdimdlg dialog

IMPLEMENT_DYNAMIC(imageexportdimdlg, CDialogEx)

imageexportdimdlg::imageexportdimdlg(CWnd* pParent,const CSize& szInput,const CSize& szOutput)
	: CDialogEx(IDD, pParent)
{
	const dimtype dt = dt_output;
	m_nType=getdimtypeindex(dt);
	m_nAspect=BST_CHECKED;
	m_bInitialised=false;
	m_szOutput=szOutput;
	m_szInput=szInput;
	m_szCustom=m_szOutput;
	m_dAspect=m_szInput.cx&&m_szInput.cy?m_szInput.cy/double(m_szInput.cx):1; 
	m_csWidth=getwidth(dt);
	m_csHeight=getheight(dt);
}

imageexportdimdlg::~imageexportdimdlg()
{
}

void imageexportdimdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_IMAGE_EXPORT_TYPE_COMBO,m_Combo);
	DDX_Control(pDX,IDC_IMAGE_EXPORT_WIDTH_SPIN,m_WidthSpin);
	DDX_Control(pDX,IDC_IMAGE_EXPORT_HEIGHT_SPIN,m_HeightSpin);

	DDX_Text(pDX,IDC_IMAGE_EXPORT_WIDTH_EDIT,m_csWidth);
	DDX_Text(pDX,IDC_IMAGE_EXPORT_HEIGHT_EDIT,m_csHeight);

	DDX_Check(pDX,IDC_IMAGE_EXPORT_ASPECT_CHECK,m_nAspect);

	DDX_CBIndex(pDX,IDC_IMAGE_EXPORT_TYPE_COMBO,m_nType);
}


BEGIN_MESSAGE_MAP(imageexportdimdlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_IMAGE_EXPORT_TYPE_COMBO,imageexportdimdlg::OnTypeChanged)
	ON_EN_CHANGE(IDC_IMAGE_EXPORT_WIDTH_EDIT,imageexportdimdlg::OnWidthEditChange)
	ON_EN_CHANGE(IDC_IMAGE_EXPORT_HEIGHT_EDIT,imageexportdimdlg::OnHeightEditChange)
	ON_EN_KILLFOCUS(IDC_IMAGE_EXPORT_WIDTH_EDIT,imageexportdimdlg::OnWidthEditKillFocus)
	ON_EN_KILLFOCUS(IDC_IMAGE_EXPORT_HEIGHT_EDIT,imageexportdimdlg::OnHeightEditKillFocus)
END_MESSAGE_MAP()


// imageexportdimdlg message handlers

BOOL imageexportdimdlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// setup spin ctrls, this gives 'correct' direction of change
	m_WidthSpin.SetRange32(1,0x7fffffff);
	m_HeightSpin.SetRange32(1,0x7fffffff);

	enabledisable();

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void imageexportdimdlg::OnTypeChanged(void)
{
	UpdateData();

	const auto dt = getdimtype(m_nType);

	m_csWidth=getwidth(dt);
	m_csHeight=getheight(dt);
	
	UpdateData(false);

	enabledisable();
}

void imageexportdimdlg::OnWidthEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);
	
	const bool bCustom=getdimtype(m_nType)==dt_custom;
	const int nW = af::getint<true>(m_csWidth,1);
	if(m_nAspect==BST_CHECKED && bCustom)
	{
		const int nH = std::max<int>(1,af::posround<double,int>(nW*m_dAspect));
		CString csH;
		csH.Format(_T("%li"),nH);
		if(csH!=m_csHeight)
		{
			m_csHeight=csH;
			UpdateData(false);
		}
	}
}

void imageexportdimdlg::OnWidthEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csWidth,1));
	const bool bMutated = cs!=m_csWidth;
	m_csWidth=cs;
	
	// update ui from members
	bool bUpdate=false;
	if(bMutated)
	{
		GetDlgItem(IDC_IMAGE_EXPORT_WIDTH_EDIT)->SetWindowText(cs);
		bUpdate=true;
	}

	const bool bCustom=getdimtype(m_nType)==dt_custom;
	const int nW = af::getint<true>(m_csWidth,1);
	if(m_nAspect==BST_CHECKED && bCustom)
	{
		const int nH = std::max<int>(1,af::posround<double,int>(nW*m_dAspect));
		CString csH;
		csH.Format(_T("%li"),nH);
		if(csH!=m_csHeight)
		{
			m_csHeight=csH;
			bUpdate=true;
		}
	}

	if(bUpdate)
		UpdateData(false);
}

void imageexportdimdlg::OnHeightEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);
	
	const bool bCustom=getdimtype(m_nType)==dt_custom;
	const int nH = af::getint<true>(m_csHeight,1);
	if(m_nAspect==BST_CHECKED && bCustom)
	{
		const int nW = std::max<int>(1,af::posround<double,int>(nH/m_dAspect));
		CString csW;
		csW.Format(_T("%li"),nW);
		if(csW!=m_csWidth)
		{
			m_csWidth=csW;
			UpdateData(false);
		}
	}
}

void imageexportdimdlg::OnHeightEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csHeight,1));
	const bool bMutated = cs!=m_csHeight;
	m_csHeight=cs;
	
	// update ui from members
	bool bUpdate=false;
	if(bMutated)
	{
		GetDlgItem(IDC_IMAGE_EXPORT_HEIGHT_EDIT)->SetWindowText(cs);
		bUpdate=true;
	}

	const bool bCustom=getdimtype(m_nType)==dt_custom;
	const int nH = af::getint<true>(m_csHeight,1);
	if(m_nAspect==BST_CHECKED && bCustom)
	{
		const int nW = std::max<int>(1,af::posround<double,int>(nH/m_dAspect));
		CString csW;
		csW.Format(_T("%li"),nW);
		if(csW!=m_csWidth)
		{
			m_csWidth=csW;
			bUpdate=true;
		}
	}

	if(bUpdate)
		UpdateData(false);
}

void imageexportdimdlg::enabledisable(void)
{
	const auto dt = getdimtype(m_nType);

	GetDlgItem(IDC_IMAGE_EXPORT_ASPECT_CHECK)->EnableWindow(dt==dt_custom);
	GetDlgItem(IDC_IMAGE_EXPORT_WIDTH_EDIT)->EnableWindow(dt==dt_custom);
	GetDlgItem(IDC_IMAGE_EXPORT_HEIGHT_EDIT)->EnableWindow(dt==dt_custom);
}

imageexportdimdlg::dimtype imageexportdimdlg::getdimtype(const int n)
{
	switch(n)
	{
		case 0:return dt_input;
		case 1:return dt_output;
		case 2:return dt_custom;
		default:ASSERT(false);return dt_input;
	}
}

int imageexportdimdlg::getdimtypeindex(const dimtype t)
{
	switch(t)
	{
		case dt_input:return 0;
		case dt_output:return 1;
		case dt_custom:return 2;
		default:ASSERT(false);return 0;
	}
}

CString imageexportdimdlg::getwidth(const dimtype dt)const
{
	CString cs;
	switch(dt)
	{
		case dt_input:cs.Format(_T("%li"),m_szInput.cx);break;
		case dt_output:cs.Format(_T("%li"),m_szOutput.cx);break;
		case dt_custom:cs.Format(_T("%li"),m_szCustom.cx);break;
		default:ASSERT(false);break;
	}
	return cs;
}

CString imageexportdimdlg::getheight(const dimtype dt)const
{
	CString cs;
	switch(dt)
	{
		case dt_input:cs.Format(_T("%li"),m_szInput.cy);break;
		case dt_output:cs.Format(_T("%li"),m_szOutput.cy);break;
		case dt_custom:cs.Format(_T("%li"),m_szCustom.cy);break;
		default:ASSERT(false);break;
	}
	return cs;
}
