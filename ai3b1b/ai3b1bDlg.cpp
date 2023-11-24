
// ai3b1bDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "ai3b1b.h"
#include "ai3b1bDlg.h"
#include "afxdialogex.h"
#include <algorithm>
#include "serialise.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cai3b1bDlg dialog



Cai3b1bDlg::Cai3b1bDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nPanelUI=BST_UNCHECKED;
}

void Cai3b1bDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX,IDC_AI3B1B_TAB,m_TabCtrl);

	DDX_Check(pDX,IDC_PANEL_UI_CHECK,m_nPanelUI);
}

INT_PTR Cai3b1bDlg::DoModal()
{
	const INT_PTR n = CDialogEx::DoModal();
	return n;
}

BEGIN_MESSAGE_MAP(Cai3b1bDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCLOSE,Cai3b1bDlg::OnClose)
	ON_BN_CLICKED(IDC_LOAD,Cai3b1bDlg::OnLoad)
	ON_BN_CLICKED(IDC_SAVE,Cai3b1bDlg::OnSave)
	ON_BN_CLICKED(IDC_PANEL_UI_CHECK,Cai3b1bDlg::OnPanelUI)
END_MESSAGE_MAP()


// Cai3b1bDlg message handlers

BOOL Cai3b1bDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// enable/disable
	enabledisable();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void Cai3b1bDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		diasableui disui;
		
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();

		return;
	}
	switch(nID)
	{
		case SC_CLOSE:EndDialog(IDCLOSE);break;
		default:CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Cai3b1bDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Cai3b1bDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cai3b1bDlg::OnClose(void)
{
	EndDialog(IDCLOSE);
}

void Cai3b1bDlg::OnLoad(void)
{
	theApp.load();
}

void Cai3b1bDlg::OnSave(void)
{
	theApp.save();
}

void Cai3b1bDlg::OnPanelUI(void)
{
	UpdateData();
	if(m_nPanelUI == BST_UNCHECKED)
		theApp.closepanelui();
	else
		theApp.openpanelui();
}

void Cai3b1bDlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;

	GetDlgItem(IDC_LOAD)->EnableWindow(true);
	GetDlgItem(IDC_SAVE)->EnableWindow(true);
}

void Cai3b1bDlg::onhint(const hint *p)
{
	switch(p->gettype())
	{
		case hint::t_widget:
		{
			const bool bVisible = !!theApp.getpaneldlg();
			const int nWidget = bVisible?BST_CHECKED:BST_UNCHECKED;
			if(nWidget != m_nPanelUI)
			{
				m_nPanelUI=nWidget;
				UpdateData(false);
			}
		}
		break;
	}
}
