// traindlg.cpp : implementation file
//

#include "pch.h"
#include "ai3b1b.h"
#include "afxdialogex.h"
#include "traindlg.h"
#include "ai3b1bDlg.h"
#include "serialise.h"


// traindlg dialog

IMPLEMENT_DYNAMIC(traindlg, CDialogEx)

traindlg::traindlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRAIN, pParent)
{
	m_bInitialised=false;
	m_InputType.first=false;
	const auto it = theApp.getlayerdlg()->getinputtype();
	m_nInitialiseType=getinitialisetypeindex(theApp.getinput(it)->getinittype());
	m_csUserRandomFrom.Format(_T("%f"),theApp.getinput(it)->getuserrandrange().first);
	m_csUserRandomTo.Format(_T("%f"),theApp.getinput(it)->getuserrandrange().second);
	m_nInfiniteEpochs=theApp.getinput(it)->getepochs()<0?BST_CHECKED:BST_UNCHECKED;
	m_csEpochs.Format(_T("%li"),abs(theApp.getinput(it)->getepochs()));
	m_csLearningRate.Format(_T("%f"),theApp.getinput(it)->getlearningrate());
}

traindlg::~traindlg()
{
}

void traindlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	
	DDX_Control(pDX,IDC_TRAIN_SET_TREE,m_SetTree);
	DDX_Control(pDX,IDC_TRAIN_INPUT_TREE,m_InputTree);
	DDX_Control(pDX,IDC_TRAIN_OUTPUT_TREE,m_OutputTree);
	DDX_Control(pDX,IDC_TRAIN_EPOCHS_SPIN,m_EpochsSpin);
	
	DDX_Text(pDX,IDC_TRAIN_EPOCHS_EDIT,m_csEpochs);
	DDX_Text(pDX,IDC_TRAIN_INPUT_EDIT,m_csInput);
	DDX_Text(pDX,IDC_TRAIN_OUTPUT_EDIT,m_csOutput);
	DDX_Text(pDX,IDC_TRAIN_INITIALISE_FROM_EDIT,m_csUserRandomFrom);
	DDX_Text(pDX,IDC_TRAIN_INITIALISE_TO_EDIT,m_csUserRandomTo);
	DDX_Text(pDX,IDC_TRAIN_RATE_EDIT,m_csLearningRate);

	DDX_CBIndex(pDX,IDC_TRAIN_INITIALISE_COMBO,m_nInitialiseType);

	DDX_Check(pDX,IDC_TRAIN_EPOCHS_CHECK,m_nInfiniteEpochs);
}


BEGIN_MESSAGE_MAP(traindlg, CDialogEx)
	ON_BN_CLICKED(IDC_TRAIN_ADD,traindlg::OnAdd)
	ON_BN_CLICKED(IDC_TRAIN_ERASE,traindlg::OnErase)
	ON_BN_CLICKED(IDC_TRAIN_EPOCHS_CHECK,traindlg::OnInfiniteEpochs)
	ON_EN_CHANGE(IDC_TRAIN_EPOCHS_EDIT,traindlg::OnEpochsEditChange)
	ON_EN_CHANGE(IDC_TRAIN_INPUT_EDIT,traindlg::OnInputEditChange)
	ON_EN_CHANGE(IDC_TRAIN_OUTPUT_EDIT,traindlg::OnOutputEditChange)
	ON_EN_CHANGE(IDC_TRAIN_INITIALISE_FROM_EDIT,traindlg::OnInitialiseFromEditChange)
	ON_EN_CHANGE(IDC_TRAIN_INITIALISE_TO_EDIT,traindlg::OnInitialiseToEditChange)
	ON_EN_CHANGE(IDC_TRAIN_RATE_EDIT,traindlg::OnLearningRateEditChange)
	ON_EN_KILLFOCUS(IDC_TRAIN_EPOCHS_EDIT,traindlg::OnEpochsEditKillFocus)
	ON_EN_KILLFOCUS(IDC_TRAIN_INPUT_EDIT,traindlg::OnInputEditKillFocus)
	ON_EN_KILLFOCUS(IDC_TRAIN_OUTPUT_EDIT,traindlg::OnOutputEditKillFocus)
	ON_EN_KILLFOCUS(IDC_TRAIN_INITIALISE_FROM_EDIT,traindlg::OnInitialiseFromEditKillFocus)
	ON_EN_KILLFOCUS(IDC_TRAIN_INITIALISE_TO_EDIT,traindlg::OnInitialiseToEditKillFocus)
	ON_EN_KILLFOCUS(IDC_TRAIN_RATE_EDIT,traindlg::OnLearningRateEditKillFocus)
	ON_NOTIFY(TVN_SELCHANGED,IDC_TRAIN_SET_TREE,traindlg::OnSetTreeChange)
	ON_NOTIFY(TVN_SELCHANGED,IDC_TRAIN_INPUT_TREE,traindlg::OnInputTreeChange)
	ON_NOTIFY(TVN_SELCHANGED,IDC_TRAIN_OUTPUT_TREE,traindlg::OnOutputTreeChange)
	ON_CBN_SELCHANGE(IDC_TRAIN_INITIALISE_COMBO,traindlg::OnInitialiseChange)
END_MESSAGE_MAP()


// traindlg message handlers

BOOL traindlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// setup spin ctrls, this gives 'correct' direction of change
	m_EpochsSpin.SetRange32(1,0x7fffffff);
	
	// enable/disable
	enabledisable();

	m_bInitialised=true;
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void traindlg::OnSetTreeChange(NMHDR* pNMHDR,LRESULT* pResult)
{
	// notify header
	*pResult = 0;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// populate
	populateinputoutput();
}

void traindlg::OnInputTreeChange(NMHDR* pNMHDR,LRESULT* pResult)
{
	// notify header
	*pResult = 0;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// update members from list ctrl sel
	initfrominputtreectrl();
	
	// update ui from members
	UpdateData(false);
}

void traindlg::OnOutputTreeChange(NMHDR* pNMHDR,LRESULT* pResult)
{
	// notify header
	*pResult = 0;
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
	// update members from list ctrl sel
	initfromoutputtreectrl();
	
	// update ui from members
	UpdateData(false);
}

void traindlg::OnAdd(void)
{
	// validate
	int nLayerI,nLayerO,nTrainI,nTrainO;
	const bool bValid = getlayerio(&nLayerI,&nLayerO) ? (!gettrainio(&nTrainI,&nTrainO) || ( nTrainI == nLayerI && nTrainO == nLayerO )) : false;
	if(!bValid)
		return;
	CWaitCursor w;

	// update training data
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const afml::trainsetitem::inputtype it = pLayerDlg->getinputtype();
	switch(it)
	{
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			diasableui disui;
			
			CFileDialog dlg(TRUE);
			if(dlg.DoModal()!=IDOK)
				return;
			const CString csPath=dlg.GetPathName();
			const int nDim=theApp.getinput(it)->getimagedim();
			std::shared_ptr<afml::traintsetimageitem> sp(new afml::traintsetimageitem());
			const int nID = static_cast<int>(theApp.getinput(it)->gettraining().size());
			if(!sp->set(csPath,nDim,it) || !sp->setid(nID))
				return;
			auto v = theApp.getinput(it)->gettraining();
			v.push_back(sp);
			theApp.getinput(it)->settraining(v);
		}
		break;
		case afml::trainsetitem::it_user:
		{
			af::mxnxzmatrix<> mI(nLayerI,1);
			af::mxnxzmatrix<> mO(nLayerO,1);
			mI.set(0); // init to zero
			mO.set(0); // init to zero
			if(theApp.getinput(it)->gettraining().size()==1)
				theApp.getinput(it)->gettraining()[0]->push_back(mI,mO);
			else
			{
				std::shared_ptr<afml::traintsetuseritem> sp(new afml::traintsetuseritem);
				sp->set(mI,mO);
				std::vector<std::shared_ptr<afml::trainsetitem>> v{sp};
				theApp.getinput(it)->settraining(v);
			}
		}
		break;
		default:ASSERT(false);return;
	}

	// populate
	const HTREEITEM ht = pushbacksettreectrl();

	// update selection
	if(ht)
	{
		if(m_SetTree.GetParentItem(ht))
		{
			if(!(m_SetTree.GetItemState(m_SetTree.GetParentItem(ht), TVIS_EXPANDED) & TVIS_EXPANDED))
				m_SetTree.Expand(m_SetTree.GetParentItem(ht),TVE_EXPAND);
			m_SetTree.EnsureVisible(m_SetTree.GetParentItem(ht));
		}
		else
			m_SetTree.EnsureVisible(ht);
		m_SetTree.Select(ht,TVGN_CARET); // sel change handler will do the work
	}
	theApp.broadcasthint(&hint(hint::t_training_data));
}

void traindlg::OnErase(void)
{
	// validate
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it=pLayerDlg->getinputtype();
	if(theApp.getinput(it)->gettraining().size()==0)
		return;
	const HTREEITEM h = m_SetTree.GetSelectedItem();
	if(!h)
		return;
	afml::trainsetitem *pTrain = gettrainsetitem(&m_SetTree,h);
	
	// erase
	switch(it)
	{
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			// erase
			if(!pTrain)
				return;
			int nImage;
			const HTREEITEM hParent = m_SetTree.GetParentItem(h);
			gettreectrlchildindex(&m_SetTree,hParent,h,nImage);
			auto vTraining = theApp.getinput(it)->gettraining();
			vTraining.erase(vTraining.cbegin()+nImage);
			for(int n=nImage;n<static_cast<int>(vTraining.size());++n)
				static_cast<afml::traintsetimageitem*>(vTraining[n].get())->setid(n);
			theApp.getinput(it)->settraining(vTraining);
			m_SetTree.DeleteItem(h);

			// select
			const int nChildren = gettreectrlchildcount(&m_SetTree,hParent);
			m_SetTree.Select(nImage<nChildren?gettreectrlchild(&m_SetTree,hParent,nImage):
									(nChildren?gettreectrlchild(&m_SetTree,hParent,nChildren-1):hParent),TVGN_CARET);
			if(nChildren)
			{
				theApp.broadcasthint(&hint(hint::t_training_data));
				return; // sel change handler will do the work
			}
		}
		break;
		case afml::trainsetitem::it_user:
		{
			// erase
			if(pTrain)
				return;
			int nZ;
			const HTREEITEM hParent = m_SetTree.GetParentItem(h);
			afml::trainsetitem *pTrain = gettrainsetitem(&m_SetTree,hParent);
			gettreectrlchildindex(&m_SetTree,hParent,h,nZ);
			pTrain->eraseinput(nZ);
			pTrain->eraseoutput(nZ);
			m_SetTree.DeleteItem(h);
			
			// setup the text of the remaining
			const int nChildren = gettreectrlchildcount(&m_SetTree,hParent);
			if(nZ<nChildren)
			{
				HTREEITEM hChild = gettreectrlchild(&m_SetTree,hParent,nZ);
				for(int nChild = nZ;nChild<nChildren;++nChild)
				{
					CString cs;
					cs.Format(_T("i/o [%li]"),nChild);
					m_SetTree.SetItemText(hChild,cs);
					hChild = m_SetTree.GetNextSiblingItem(hChild);
				}
			}

			// select
			m_SetTree.Select(nZ<nChildren?gettreectrlchild(&m_SetTree,hParent,nZ):
									(nChildren?gettreectrlchild(&m_SetTree,hParent,nChildren-1):hParent),TVGN_CARET);
			if(nChildren)
			{
				theApp.broadcasthint(&hint(hint::t_training_data));
				return; // sel change handler will do the work
			}
			else
			{
				theApp.getinput(it)->settraining({});
				m_SetTree.Select(nullptr,TVGN_CARET);
				m_SetTree.DeleteAllItems();
			}
		}
		break;
		default:ASSERT(false);
	}

	// populate
	populateinputoutput();

	theApp.broadcasthint(&hint(hint::t_training_data));
}

void traindlg::OnInputEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// selection
	const HTREEITEM hSel=m_SetTree.GetSelectedItem();
	if(!hSel)
		return;
	const HTREEITEM hI=m_InputTree.GetSelectedItem();
	if(!hI)
		return;
	const HTREEITEM hIParent=m_InputTree.GetParentItem(hI);
	const HTREEITEM hSelParent=m_SetTree.GetParentItem(hSel);
	afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,hSel);
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	const double d = af::getfloat<>(m_csInput);
	switch(it)
	{
		case afml::trainsetitem::it_user:
		{
			if(pTrain)
				return;
			int nZ,nR;
			gettreectrlchildindex(&m_SetTree,hSelParent,hSel,nZ);
			gettreectrlchildindex(&m_InputTree,hIParent,hI,nR);
			afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,hSelParent);
			pTrain->setinputrowcol(nR,0,d,nZ);

			CString cs;
			cs.Format(_T("i [%li] : %f"),nR,d);
			m_InputTree.SetItemText(hI,cs);
		}
		break;
		default:ASSERT(false);
	}
}

void traindlg::OnInputEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%f"),af::getfloat<>(m_csInput));
	const bool bMutated = cs!=m_csInput;
	m_csInput=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_TRAIN_INPUT_EDIT)->SetWindowText(cs);
}

void traindlg::OnOutputEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// selection
	const HTREEITEM hSel=m_SetTree.GetSelectedItem();
	if(!hSel)
		return;
	const HTREEITEM hO=m_OutputTree.GetSelectedItem();
	if(!hO)
		return;
	const HTREEITEM hOParent=m_OutputTree.GetParentItem(hO);
	const HTREEITEM hSelParent=m_SetTree.GetParentItem(hSel);
	afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,hSel);
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	const double d = af::getfloat<>(m_csOutput);
	switch(it)
	{
		case afml::trainsetitem::it_user:
		{
			if(pTrain)
				return;
			int nZ,nR;
			gettreectrlchildindex(&m_SetTree,hSelParent,hSel,nZ);
			gettreectrlchildindex(&m_OutputTree,hOParent,hO,nR);
			afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,hSelParent);
			pTrain->setoutputrowcol(nR,0,d,nZ);

			CString cs;
			cs.Format(_T("o [%li] : %f"),nR,d);
			m_OutputTree.SetItemText(hO,cs);
		}
		break;
		default:ASSERT(false);
	}
}

void traindlg::OnOutputEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%f"),af::getfloat<>(m_csOutput));
	const bool bMutated = cs!=m_csOutput;
	m_csOutput=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_TRAIN_OUTPUT_EDIT)->SetWindowText(cs);
}

void traindlg::OnEpochsEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// update input type item
	if(true)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto nEpochs = theApp.getinput(it)->getepochs();
		nEpochs = af::getint<true>(m_csEpochs,1);		
		if(m_nInfiniteEpochs==BST_CHECKED)
			nEpochs=-nEpochs;
		theApp.getinput(it)->setepochs(nEpochs);
		
		theApp.broadcasthint(&hint(hint::t_epoch_range));
	}
}

void traindlg::OnEpochsEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%li"),af::getint<true>(m_csEpochs,1));
	const bool bMutated = cs!=m_csEpochs;
	m_csEpochs=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_TRAIN_EPOCHS_EDIT)->SetWindowText(cs);
	
	// update input type item
	if(bMutated)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto nEpochs = theApp.getinput(it)->getepochs();
		nEpochs = af::getint<true>(m_csEpochs,1);
		if(m_nInfiniteEpochs==BST_CHECKED)
			nEpochs=-nEpochs;
		theApp.getinput(it)->setepochs(nEpochs);

		theApp.broadcasthint(&hint(hint::t_epoch_range));
	}
}

void traindlg::OnLearningRateEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// update input type item
	if(true)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		const double d = af::getfloat<>(m_csLearningRate);
		if(d!=theApp.getinput(it)->getlearningrate())
			theApp.getinput(it)->setlearningrate(d);
	}
}

void traindlg::OnLearningRateEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%f"),af::getfloat<>(m_csLearningRate));
	const bool bMutated = cs!=m_csLearningRate;
	m_csLearningRate=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_TRAIN_RATE_EDIT)->SetWindowText(cs);

	// update input type item
	if(bMutated)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		const double d = af::getfloat<>(m_csLearningRate);
		if(d!=theApp.getinput(it)->getlearningrate())
			theApp.getinput(it)->setlearningrate(d);
	}
}

void traindlg::OnInitialiseFromEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// update input type item
	if(true)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto r = theApp.getinput(it)->getuserrandrange();
		r.first = af::getfloat<>(m_csUserRandomFrom);
		theApp.getinput(it)->setuserrandrange(r);
	}
}

void traindlg::OnInitialiseFromEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%f"),af::getfloat<>(m_csUserRandomFrom));
	const bool bMutated = cs!=m_csUserRandomFrom;
	m_csUserRandomFrom=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_TRAIN_INITIALISE_FROM_EDIT)->SetWindowText(cs);

	// update input type item
	if(bMutated)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto r = theApp.getinput(it)->getuserrandrange();
		r.first = af::getfloat<>(m_csUserRandomFrom);
		theApp.getinput(it)->setuserrandrange(r);
	}
}

void traindlg::OnInitialiseToEditChange(void)
{
	// is dialog initialised
	if(!m_bInitialised)
		return;

	// update members from ui
	UpdateData(true);

	// update input type item
	if(true)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto r = theApp.getinput(it)->getuserrandrange();
		r.second = af::getfloat<>(m_csUserRandomTo);
		theApp.getinput(it)->setuserrandrange(r);
	}
}

void traindlg::OnInitialiseToEditKillFocus(void)
{
	// update members from ui
	UpdateData(true);	
	CString cs;
	cs.Format(_T("%f"),af::getfloat<>(m_csUserRandomTo));
	const bool bMutated = cs!=m_csUserRandomTo;
	m_csUserRandomTo=cs;
	
	// update ui from members
	if(bMutated)
		GetDlgItem(IDC_TRAIN_INITIALISE_TO_EDIT)->SetWindowText(cs);

	// update input type item
	if(bMutated)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto r = theApp.getinput(it)->getuserrandrange();
		r.second = af::getfloat<>(m_csUserRandomTo);
		theApp.getinput(it)->setuserrandrange(r);
	}
}

void traindlg::OnInitialiseChange(void)
{
	// update members from ui
	UpdateData(true);

	// enable/disable
	enabledisable();

	// update input type item
	if(true)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		theApp.getinput(it)->setinittype(getinitialisetype(m_nInitialiseType));
	}
}

void traindlg::OnInfiniteEpochs(void)
{
	// update members from ui
	UpdateData(true);

	// enable/disable
	enabledisable();

	// update input type item
	if(true)
	{
		const auto it = theApp.getlayerdlg()->getinputtype();
		auto nEpochs = theApp.getinput(it)->getepochs();
		if((m_nInfiniteEpochs==BST_CHECKED && nEpochs>0) || (m_nInfiniteEpochs==BST_UNCHECKED && nEpochs<0))
			nEpochs=-nEpochs;
		theApp.getinput(it)->setepochs(nEpochs);
	
		theApp.broadcasthint(&hint(hint::t_epoch_range));
	}
}

void traindlg::onhint(const hint *p)
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
		case hint::t_clear_epochs:enabledisable();break;
		case hint::t_input_type:
		{
			populatecontrol();
			UpdateData(false);

			enabledisable();
		}
		break;
		case hint::t_load:
		{
			// populate
			populatecontrol();
			UpdateData(false);
			const HTREEITEM ht = populatesettreectrl(&m_SetTree);

			// update selection
			if(ht)
			{
				if(m_SetTree.GetParentItem(ht))
				{
					if(!(m_SetTree.GetItemState(m_SetTree.GetParentItem(ht), TVIS_EXPANDED) & TVIS_EXPANDED))
						m_SetTree.Expand(m_SetTree.GetParentItem(ht),TVE_EXPAND);
				}
				m_SetTree.Select(ht,TVGN_CARET); // sel change handler will do the work
				return;
			}
	
			// populate
			populateinputoutput();
		}
		break;
	}
}

afml::activationfn::initialisetype traindlg::getinitialisetype(const int n)
{
	switch(n)
	{
		case 0:return afml::activationfn::it_auto_normal;
		case 1:return afml::activationfn::it_auto_random;
		case 2:return afml::activationfn::it_user_random;
		default:ASSERT(false);return afml::activationfn::it_auto_normal;
	}
}

int traindlg::getinitialisetypeindex(const afml::activationfn::initialisetype t)
{
	switch(t)
	{
		case afml::activationfn::it_auto_normal:return 0;
		case afml::activationfn::it_auto_random:return 1;
		case afml::activationfn::it_user_random:return 2;
		default:ASSERT(false);return afml::activationfn::it_auto_normal;
	}
}

void traindlg::setactive(const bool b)
{
	// active?
	if(!b)return;
		
	// populate
	const HTREEITEM ht = populatesettreectrl(&m_SetTree);

	// update selection
	if(ht)
	{
		if(m_SetTree.GetParentItem(ht))
		{
			if(!(m_SetTree.GetItemState(m_SetTree.GetParentItem(ht), TVIS_EXPANDED) & TVIS_EXPANDED))
				m_SetTree.Expand(m_SetTree.GetParentItem(ht),TVE_EXPAND);
		}
		m_SetTree.Select(ht,TVGN_CARET); // sel change handler will do the work
		return;
	}
	
	// populate
	populateinputoutput();
}

void traindlg::populatecontrol(void)
{
	const auto it = theApp.getlayerdlg()->getinputtype();
	m_nInitialiseType=getinitialisetypeindex(theApp.getinput(it)->getinittype());
	m_csUserRandomFrom.Format(_T("%f"),theApp.getinput(it)->getuserrandrange().first);
	m_csUserRandomTo.Format(_T("%f"),theApp.getinput(it)->getuserrandrange().second);
	m_nInfiniteEpochs=theApp.getinput(it)->getepochs()<0?BST_CHECKED:BST_UNCHECKED;
	m_csEpochs.Format(_T("%li"),abs(theApp.getinput(it)->getepochs()));
	m_csLearningRate.Format(_T("%f"),theApp.getinput(it)->getlearningrate());
}

void traindlg::populateinputoutput(void)
{
	// populate
	TRACE(_T("populateinputoutput %li\r\n"),GetTickCount());
	HTREEITEM ht = populateinputtreectrl(&m_SetTree,&m_InputTree);

	// update selection
	if(ht)
		m_InputTree.Select(ht,TVGN_CARET);

	// populate
	ht = populateoutputtreectrl();

	// update selection
	if(ht)
		m_OutputTree.Select(ht,TVGN_CARET);

	// update members from list ctrl sel
	initfrominputtreectrl();
	initfromoutputtreectrl();

	// update ui from members
	UpdateData(false);

	// enable/disable
	enabledisable();
}

bool traindlg::repopulate(void)
{
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	const bool b = ( m_InputType.first && it != m_InputType.second );
	m_InputType=std::make_pair(true,it);
	return b;
}

bool traindlg::loadimagetrainingdata(const serialise *pS,std::map<afml::trainsetitem::inputtype,std::shared_ptr<afml::inputtypeitem>>& m)
{
	CString csArchive;
	if(!pS || !pS->getpath(csArchive))
		return false;

	wchar_t _drive[_MAX_DRIVE], _dir[_MAX_DIR], _fname[_MAX_FNAME], _ext[_MAX_EXT];
	_wsplitpath_s( csArchive, _drive, _MAX_DRIVE, _dir, _MAX_DIR, _fname, _MAX_FNAME, _ext, _MAX_EXT );
	csArchive = CString(_drive)+CString(_dir);
	csArchive.TrimRight(_T("\\"));
	
	std::shared_ptr<diasableui> spDisui;
	auto i = m.cbegin(),end=m.cend();
	for(;i!=end;++i)
		if((*i).second->getinputtype() & afml::trainsetitem::it_image)
		{
			auto j = (*i).second->gettraining().cbegin(),end=(*i).second->gettraining().cend();
			for(;j!=end;++j)
			{
				bool bAuto = true;
				afml::traintsetimageitem *pItem = static_cast<afml::traintsetimageitem*>((*j).get());
				CString csPath = pItem->getpath();
				_wsplitpath_s( csPath, _drive, _MAX_DRIVE, _dir, _MAX_DIR, _fname, _MAX_FNAME, _ext, _MAX_EXT );
				afml::traintsetimageitem::reloadtype rl;
				while((rl = pItem->reload(csPath,pItem->getdim(),pItem->gettype()))!=afml::traintsetimageitem::rt_ok)
				{
					if(bAuto)
					{
						bAuto = false;
						csPath = csArchive + _T("\\") + CString(_fname)+CString(_ext);
						continue;
					}
					
					if(!spDisui)
						spDisui=std::shared_ptr<diasableui>(new diasableui);
					
					CString cs;
					switch(rl)
					{
						case afml::traintsetimageitem::rt_mismatch_err:
						{
							wchar_t __fname[_MAX_FNAME], __ext[_MAX_EXT];
							_wsplitpath_s( csPath, nullptr, 0, nullptr, 0, __fname, _MAX_FNAME, __ext, _MAX_EXT );
							cs.Format(_T("properties mismatched between:\r\n\"%s <-> %s\"\r\nwould you like to load an alternative file?"),CString(__fname)+CString(__ext),CString(_fname)+CString(_ext));
						}
						break;
						case afml::traintsetimageitem::rt_file_err:
							cs.Format(_T("could not load: \"%s\"\r\nwould you like to load an alternative file?"),CString(_fname)+CString(_ext));
						break;
						default:ASSERT(false);
					}

					if(AfxMessageBox(cs,MB_YESNO|MB_ICONQUESTION)==IDNO)
						return false;
					CFileDialog dlg(true);
					if(dlg.DoModal()!=IDOK)
						return false;
					csPath=dlg.GetPathName();
				}
			}
		}
	return true;
}

void traindlg::validatenetworktrainingmismatch(bool& bMismatch,bool& bFixed)
{
	bMismatch=false;
	bFixed=false;
	
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	switch(it)
	{
		case afml::trainsetitem::it_user:
		{
			std::vector<std::shared_ptr<afml::trainsetitem>> v = getnetworktrainingmismatch(it);
			if(!v.size())return;

			bMismatch=true;

			int nI,nO,nTrainI,nTrainO;
			getlayerio(&nI,&nO);
			nTrainI=v[0]->getinput().getrows();
			nTrainO=v[0]->getoutput().getrows();
			
			CString cs;
			cs.Format(_T("User training data exists that was created using [%li/%li] input/output perceptrons and input/output perceptrons are now [%li/%li]. Fix the invalid training data?"),
					  nTrainI,nTrainO,nI,nO);
			if(AfxMessageBox(cs,MB_YESNO|MB_ICONQUESTION)!=IDYES)return;

			bFixed=true;

			auto i = v.begin(),end=v.end();
			for(;i!=end;++i)
			{
				afml::traintsetuseritem *pT = static_cast<afml::traintsetuseritem*>((*i).get());
				pT->setinputoutput(nI,nO);
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			std::vector<std::shared_ptr<afml::trainsetitem>> v = getnetworktrainingmismatch(it);
			if(!v.size())return;

			bMismatch=true;

			afml::traintsetimageitem *pT = static_cast<afml::traintsetimageitem*>(v[0].get());

			CString cs;
			cs.Format(_T("Fixed dim image training data exists that was created using %li and fixed dim is now %li. Fix the invalidated training data?"),
					  pT->getdim(),theApp.getinput(it)->getimagedim());
			if(AfxMessageBox(cs,MB_YESNO|MB_ICONQUESTION)!=IDYES)return;

			bFixed=true;

			const int nDim=theApp.getinput(it)->getimagedim();
			auto i = v.begin(),end=v.end();
			for(;i!=end;++i)
			{
				afml::traintsetimageitem *pTrain = static_cast<afml::traintsetimageitem*>((*i).get());
				pTrain->setdim(nDim);
			}
		}
		break;
		default:ASSERT(false);
	}

	if(bFixed)
	{
		// populate
		const HTREEITEM ht = populatesettreectrl(&m_SetTree);

		// update selection
		if(ht)
		{
			if(m_SetTree.GetParentItem(ht))
			{
				if(!(m_SetTree.GetItemState(m_SetTree.GetParentItem(ht), TVIS_EXPANDED) & TVIS_EXPANDED))
					m_SetTree.Expand(m_SetTree.GetParentItem(ht),TVE_EXPAND);
			}
			m_SetTree.Select(ht,TVGN_CARET); // sel change handler will do the work
			return;
		}
	
		// populate
		populateinputoutput();
	}
}

std::vector<std::shared_ptr<afml::trainsetitem>> traindlg::getnetworktrainingmismatch(const afml::trainsetitem::inputtype t)
{
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	switch(t)
	{
		case afml::trainsetitem::it_user:
		{
			if(theApp.getinput(t)->gettraining().size()!=1)
				return {};

			int nI,nO;
			getlayerio(&nI,&nO);
			if(theApp.getinput(t)->gettraining()[0]->getinput().getrows()!=nI || theApp.getinput(t)->gettraining()[0]->getoutput().getrows()!=nO)
				return std::vector<std::shared_ptr<afml::trainsetitem>>{theApp.getinput(t)->gettraining()[0]};
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			const int nDim=theApp.getinput(t)->getimagedim();

			std::vector<std::shared_ptr<afml::trainsetitem>> v;
			auto iT = theApp.getinput(t)->gettraining().cbegin(),end=theApp.getinput(t)->gettraining().cend();
			for(;iT!=end;++iT)
			{
				const afml::traintsetimageitem *pTrain = static_cast<const afml::traintsetimageitem*>((*iT).get());
				if(pTrain->getdim()!=nDim)
					v.push_back(*iT);
			}
			return v;
		}
	}
	return {};
}

bool traindlg::getlayerio(int *pI,int *pO)
{
	layerdlg *pDlg = theApp.getlayerdlg();
	if(!pDlg->m_List.GetItemCount())
		return false;

	const int nLayers = pDlg->m_List.GetItemCount();
	afml::layer<> *pL = reinterpret_cast<afml::layer<>*>(pDlg->m_List.GetItemData(nLayers-1));
	
	if(pI)
		*pI = af::getint<true>(pDlg->m_csInputPerceptrons,1);
	if(pO)
		*pO = pL->getperceptrons();

	return true;
}

bool traindlg::gettrainio(int *pI,int *pO)
{
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	if(theApp.getinput(it)->gettraining().size()==0)
		return false;
	if(pI)
		*pI=theApp.getinput(it)->gettraining()[0]->getinput().getrows();
	if(pO)
		*pO=theApp.getinput(it)->gettraining()[0]->getoutput().getrows();
	return true;
}

HTREEITEM traindlg::populatesettreectrl(CTreeCtrl *pSetTree)
{
	HTREEITEM ht=nullptr;
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	pSetTree->Select(nullptr,TVGN_CARET);
	pSetTree->DeleteAllItems();
	switch(it)
	{
		case afml::trainsetitem::it_user:
		{
			const HTREEITEM hParent = TVI_ROOT;
			const HTREEITEM hUser = pSetTree->InsertItem( _T("user"), hParent, TVI_LAST );

			const auto& v = theApp.getinput(it)->gettraining();
			if(v.size()==1)
			{
				pSetTree->SetItemData(hUser,reinterpret_cast<DWORD_PTR>(v[0].get()));
				
				for( int nZ = 0 ; nZ < v[0]->getinput().getz() ; ++nZ )
				{
					CString cs;
					cs.Format(_T("i/o [%li]"),nZ);
					const HTREEITEM h = pSetTree->InsertItem( cs, hUser, TVI_LAST );
					if(!ht)
						ht = h;
				}
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			// n pixels

			// z[0]: id -> pixel[0], ..., pixel[n-2], pixel[n-1]
			const auto& v = theApp.getinput(it)->gettraining();
			auto it = v.cbegin(),end=v.cend();
			for(;it!=end;++it)
			{
				const afml::traintsetimageitem *pTrain = static_cast<const afml::traintsetimageitem*>((*it).get());
				const HTREEITEM h = pSetTree->InsertItem( pTrain->getfname(), TVI_ROOT, TVI_LAST );
				if(!ht)
					ht = h;
				pSetTree->SetItemData(h,reinterpret_cast<DWORD_PTR>(pTrain));
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			// n pixels

			// z[0]: id + xy -> pixel[0]
			// ...
			// z[n-2]: id + xy -> pixel[n-2]
			// z[n-1]: id + xy -> pixel[n-1]
			const auto& v = theApp.getinput(it)->gettraining();
			auto it = v.cbegin(),end=v.cend();
			for(;it!=end;++it)
			{
				const afml::traintsetimageitem *pTrain = static_cast<const afml::traintsetimageitem*>((*it).get());
				const HTREEITEM h = pSetTree->InsertItem( pTrain->getfname(), TVI_ROOT, TVI_LAST );
				pSetTree->SetItemData(h,reinterpret_cast<DWORD_PTR>(pTrain));
				for(int nZ = 0 ; nZ < pTrain->getinput().getz();++nZ)
				{
					CString cs;
					cs.Format(_T("pixel [%li]"),nZ);
					HTREEITEM hP=pSetTree->InsertItem( cs, h, TVI_LAST );
					if(!ht)
						ht = hP;
				}
			}
		}
		break;
		default:ASSERT(false);
	}
	return ht;
}

HTREEITEM traindlg::pushbacksettreectrl(void)
{
	HTREEITEM ht=nullptr;
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it  = pLayerDlg->getinputtype();
	switch(it)
	{
		case afml::trainsetitem::it_user:
		{
			const auto& v = theApp.getinput(it)->gettraining();
			if(v.size()==1)
			{
				HTREEITEM hUser=TVI_ROOT;
				if(!m_SetTree.GetCount())
				{
					hUser = m_SetTree.InsertItem( _T("user"), hUser, TVI_LAST );
				}
				else
					hUser = m_SetTree.GetChildItem(TVI_ROOT);

				if(!gettrainsetitem(&m_SetTree,hUser))
					m_SetTree.SetItemData(hUser,reinterpret_cast<DWORD_PTR>(v[0].get()));

				CString cs;
				cs.Format(_T("i/o [%li]"),v[0]->getinput().getz()-1);
				const HTREEITEM h = m_SetTree.InsertItem( cs, hUser, TVI_LAST );
				ht=h;
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			// n pixels

			// z[0]: id -> pixel[0], ..., pixel[n-2], pixel[n-1]
			const auto& v = theApp.getinput(it)->gettraining();
			if(v.size())
			{
				const afml::traintsetimageitem *pTrain = static_cast<const afml::traintsetimageitem*>(v[v.size()-1].get());
				HTREEITEM hUser=TVI_ROOT;
				hUser = m_SetTree.InsertItem( pTrain->getfname(), hUser, TVI_LAST );
				m_SetTree.SetItemData(hUser,reinterpret_cast<DWORD_PTR>(pTrain));
				ht=hUser;
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			// n pixels

			// z[0]: id + xy -> pixel[0]
			// ...
			// z[n-2]: id + xy -> pixel[n-2]
			// z[n-1]: id + xy -> pixel[n-1]
			const auto& v = theApp.getinput(it)->gettraining();
			if(v.size())
			{
				const afml::traintsetimageitem *pTrain = static_cast<const afml::traintsetimageitem*>(v[v.size()-1].get());
				HTREEITEM hUser=TVI_ROOT;
				hUser = m_SetTree.InsertItem( pTrain->getfname(), hUser, TVI_LAST );
				m_SetTree.SetItemData(hUser,reinterpret_cast<DWORD_PTR>(pTrain));
				for(int nZ = 0 ; nZ < pTrain->getinput().getz();++nZ)
				{
					CString cs;
					cs.Format(_T("pixel [%li]"),nZ);
					const HTREEITEM h=m_SetTree.InsertItem( cs, hUser, TVI_LAST );
					if(!ht)
						ht=h;
				}
			}
		}
		break;
		default:ASSERT(false);
	}
	return ht;
}

HTREEITEM traindlg::populateinputtreectrl(CTreeCtrl *pSetTree,CTreeCtrl *pInputTree)
{
	pInputTree->Select(nullptr,TVGN_CARET);
	pInputTree->DeleteAllItems();
	const HTREEITEM h = pSetTree->GetSelectedItem();	
	if(!h)
		return h;
	afml::trainsetitem *pTrain=gettrainsetitem(pSetTree,h);

	HTREEITEM ht=nullptr;
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	switch(pLayerDlg->getinputtype())
	{
		case afml::trainsetitem::it_user:
		{
			if(pTrain)
				return ht;
			const HTREEITEM hParent = pSetTree->GetParentItem(h);
			afml::trainsetitem *pTrain=gettrainsetitem(pSetTree,hParent);
			if(pTrain)
			{
				int nZ;
				gettreectrlchildindex(pSetTree,hParent,h,nZ);
				for(int nR=0;nR<pTrain->getinput().getrows();++nR)
				{
					const double d = pTrain->getinput().get(nR,0,nZ);
					CString cs;
					cs.Format(_T("i [%li] : %f"),nR,d);
					const HTREEITEM h = pInputTree->InsertItem( cs, TVI_ROOT, TVI_LAST );
					if(!ht)
						ht=h;
				}
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			if(!pTrain)
				return ht;
			ASSERT(pTrain->getinput().getz()==1);
			ASSERT(pTrain->getinput().getrows()==1);
			const double d = pTrain->getinput().get(0,0);
			CString cs;
			cs.Format(_T("id : %f"),d);
			const HTREEITEM h = pInputTree->InsertItem( cs, TVI_ROOT, TVI_LAST );
			if(!ht)
				ht=h;
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			if(pTrain)
				return ht;
			int nZ;
			const HTREEITEM hParent = pSetTree->GetParentItem(h);
			afml::trainsetitem *pTrain=gettrainsetitem(pSetTree,hParent);
			gettreectrlchildindex(pSetTree,hParent,h,nZ);
			ASSERT(pTrain->getinput().getrows()==3);
			
			CString cs;
			cs.Format(_T("id : %f"),pTrain->getinput().get(0,0,nZ));
			HTREEITEM h = pInputTree->InsertItem( cs, TVI_ROOT, TVI_LAST );
			if(!ht)
				ht=h;
			
			cs.Format(_T("x : %f"),pTrain->getinput().get(1,0,nZ));
			h = pInputTree->InsertItem( cs, TVI_ROOT, TVI_LAST );
			
			cs.Format(_T("y : %f"),pTrain->getinput().get(2,0,nZ));
			h = pInputTree->InsertItem( cs, TVI_ROOT, TVI_LAST );
		}
		break;
		default:ASSERT(false);
	}

	return ht;
}

HTREEITEM traindlg::populateoutputtreectrl(void)
{
	m_OutputTree.Select(nullptr,TVGN_CARET);
	m_OutputTree.DeleteAllItems();
	const HTREEITEM h = m_SetTree.GetSelectedItem();	
	if(!h)
		return h;
	afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,h);

	HTREEITEM ht=nullptr;
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	switch(pLayerDlg->getinputtype())
	{
		case afml::trainsetitem::it_user:
		{
			if(pTrain)
				return ht;
			const HTREEITEM hParent = m_SetTree.GetParentItem(h);
			afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,hParent);
			if(pTrain)
			{
				int nZ;
				gettreectrlchildindex(&m_SetTree,hParent,h,nZ);
				for(int nR=0;nR<pTrain->getoutput().getrows();++nR)
				{
					const double d = pTrain->getoutput().get(nR,0,nZ);
					CString cs;
					cs.Format(_T("o [%li] : %f"),nR,d);
					const HTREEITEM h = m_OutputTree.InsertItem( cs, TVI_ROOT, TVI_LAST );
					if(!ht)
						ht=h;
				}
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			if(!pTrain)
				return ht;
			ASSERT(pTrain->getoutput().getz()==1);
			for(int nR=0;nR<pTrain->getoutput().getrows();++nR)
			{
				const double d = pTrain->getoutput().get(nR,0);
				CString cs;
				cs.Format(_T("ch : %f"),d);
				const HTREEITEM h = m_OutputTree.InsertItem( cs, TVI_ROOT, TVI_LAST );
				if(!ht)
					ht=h;
			}
		}
		break;
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			if(pTrain)
				return ht;
			int nZ;
			const HTREEITEM hParent = m_SetTree.GetParentItem(h);
			afml::trainsetitem *pTrain=gettrainsetitem(&m_SetTree,hParent);
			gettreectrlchildindex(&m_SetTree,hParent,h,nZ);
			
			const int nBytesPerPixel=3;
			for(int n=0;n<nBytesPerPixel;++n)
			{
				CString cs;
				cs.Format(_T("ch : %f"),pTrain->getoutput().get(n,0,nZ));
				const HTREEITEM h = m_OutputTree.InsertItem( cs, TVI_ROOT, TVI_LAST );
				if(!ht)
					ht=h;
			}
		}
		break;
		default:ASSERT(false);
	}

	return ht;
}

void traindlg::initfrominputtreectrl(void)
{
	m_csInput = _T("");
	
	const HTREEITEM h = m_InputTree.GetSelectedItem();
	if(!h)
		return;
	const HTREEITEM hSel = m_SetTree.GetSelectedItem();
	if(!hSel)
		return;
	afml::trainsetitem *pTrain = gettrainsetitem(&m_SetTree,hSel);
	
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	switch(pLayerDlg->getinputtype())
	{
		case afml::trainsetitem::it_user:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			if(pTrain)
				return;
			int nZ,nR;
			gettreectrlchildindex(&m_SetTree,m_SetTree.GetParentItem(hSel),hSel,nZ);
			gettreectrlchildindex(&m_InputTree,NULL,h,nR);
			afml::trainsetitem *pTrain = gettrainsetitem(&m_SetTree,m_SetTree.GetParentItem(hSel));
			if(pTrain)
				m_csInput.Format(_T("%f"),pTrain->getinput().get(nR,0,nZ));
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			if(!pTrain)
				return;
			int nZ=0,nR;
			gettreectrlchildindex(&m_InputTree,NULL,h,nR);
			m_csInput.Format(_T("%f"),pTrain->getinput().get(nR,0,nZ));
		}
		break;
		default:ASSERT(false);
	}
}

void traindlg::initfromoutputtreectrl(void)
{
	m_csOutput = _T("");
	
	const HTREEITEM h = m_OutputTree.GetSelectedItem();
	if(!h)
		return;
	const HTREEITEM hSel = m_SetTree.GetSelectedItem();
	if(!hSel)
		return;
	afml::trainsetitem *pTrain = gettrainsetitem(&m_SetTree,hSel);
	
	layerdlg *pLayerDlg = theApp.getlayerdlg();
	switch(pLayerDlg->getinputtype())
	{
		case afml::trainsetitem::it_user:
		case afml::trainsetitem::it_image_i_id_xy_o_b8g8r8:
		{
			if(pTrain)
				return;
			int nZ,nR;
			gettreectrlchildindex(&m_SetTree,m_SetTree.GetParentItem(hSel),hSel,nZ);
			gettreectrlchildindex(&m_OutputTree,NULL,h,nR);
			afml::trainsetitem *pTrain = gettrainsetitem(&m_SetTree,m_SetTree.GetParentItem(hSel));
			if(pTrain)
				m_csOutput.Format(_T("%f"),pTrain->getoutput().get(nR,0,nZ));
		}
		break;
		case afml::trainsetitem::it_image_i_id_o_b8g8r8:
		{
			if(!pTrain)
				return;
			int nZ=0,nR;
			gettreectrlchildindex(&m_OutputTree,NULL,h,nR);
			m_csOutput.Format(_T("%f"),pTrain->getoutput().get(nR,0,nZ));
		}
		break;
		default:ASSERT(false);
	}
}

afml::trainsetitem *traindlg::gettrainsetitem(CTreeCtrl *pSetTree,const HTREEITEM h)
{
	if(!h)
		return nullptr;
	const DWORD_PTR dw = pSetTree->GetItemData(h);
	return reinterpret_cast<afml::trainsetitem*>(dw);
}

bool traindlg::gettreectrlchildindex(CTreeCtrl *pTree,const HTREEITEM hParent,const HTREEITEM hChild,int& n)
{
	HTREEITEM h;
	if(hParent)
		h = pTree->GetChildItem(hParent);
	else
		h = pTree->GetRootItem();
	for(n=0;h;++n)
	{
		if(h==hChild)
			return true;
		h = pTree->GetNextSiblingItem(h);
	}
	ASSERT(false);
	return false;
}

HTREEITEM traindlg::gettreectrlchild(CTreeCtrl *pTree,const HTREEITEM hParent,const int n)
{
	HTREEITEM h;
	if(hParent)
		h = pTree->GetChildItem(hParent);
	else
		h = pTree->GetRootItem();
	for(int nC=0;h && nC<n;++nC)
	{
		h = pTree->GetNextSiblingItem(h);
	}
	ASSERT(h);
	return h;
}

int traindlg::gettreectrlchildcount(CTreeCtrl *pTree,const HTREEITEM hParent)
{
	HTREEITEM h;
	if(hParent)
		h = pTree->GetChildItem(hParent);
	else
		h = pTree->GetRootItem();
	int n=0;
	for(;h;++n)
		h = pTree->GetNextSiblingItem(h);
	return n;
}

void traindlg::validatefocus(const HWND hPreFocus)
{
	const HWND hFocus = ::GetFocus();
	if(hFocus)
		return;
	if(!hPreFocus)
		return;
	NextDlgCtrl();
}

void traindlg::enabledisable(void)
{
	if(!theApp.getdlg() || !::IsWindow(GetSafeHwnd()))
		return;

	const HWND hPreFocus = ::GetFocus();

	layerdlg *pLayerDlg = theApp.getlayerdlg();
	const auto it = pLayerDlg->getinputtype();
	
	const bool bIdle = !theApp.getthread();
	const bool bSet = theApp.getinput(it)->gettraining().size()>0;
	
	{
		int nLayerI,nLayerO,nTrainI,nTrainO;
		const bool bEnable = getlayerio(&nLayerI,&nLayerO) ? (!gettrainio(&nTrainI,&nTrainO) || ( nTrainI == nLayerI && nTrainO == nLayerO )) : false;
		GetDlgItem(IDC_TRAIN_ADD)->EnableWindow(bIdle && bEnable);
	}
	
	if(afml::trainsetitem::it_image & it)
		GetDlgItem(IDC_TRAIN_ERASE)->EnableWindow(bIdle && bSet && !!gettrainsetitem(&m_SetTree,m_SetTree.GetSelectedItem()));
	else
		GetDlgItem(IDC_TRAIN_ERASE)->EnableWindow(bIdle && bSet && !gettrainsetitem(&m_SetTree,m_SetTree.GetSelectedItem()));

	GetDlgItem(IDC_TRAIN_INPUT_EDIT)->EnableWindow(bIdle && !!m_InputTree.GetSelectedItem() && it==afml::trainsetitem::it_user);

	GetDlgItem(IDC_TRAIN_OUTPUT_EDIT)->EnableWindow(bIdle && !!m_OutputTree.GetSelectedItem() && it==afml::trainsetitem::it_user);

	GetDlgItem(IDC_TRAIN_INITIALISE_COMBO)->EnableWindow(bIdle);
	switch(getinitialisetype(m_nInitialiseType))
	{
		case afml::activationfn::it_user_random:
		{
			GetDlgItem(IDC_TRAIN_INITIALISE_FROM_EDIT)->EnableWindow(bIdle);
			GetDlgItem(IDC_TRAIN_INITIALISE_TO_EDIT)->EnableWindow(bIdle);
		}
		break;
		default:
		{
			GetDlgItem(IDC_TRAIN_INITIALISE_FROM_EDIT)->EnableWindow(false);
			GetDlgItem(IDC_TRAIN_INITIALISE_TO_EDIT)->EnableWindow(false);
		}
		break;
	}

	GetDlgItem(IDC_TRAIN_EPOCHS_EDIT)->EnableWindow(m_nInfiniteEpochs==BST_CHECKED?false:true);
	GetDlgItem(IDC_TRAIN_EPOCHS_CHECK)->EnableWindow(true);

	validatefocus(hPreFocus);
}
