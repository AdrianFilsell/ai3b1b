
#include "pch.h"
#include "serialise.h"
#include "ai3b1b.h"
#include "layerdlg.h"
#include "traindlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString serialise::s_GUID(_T("{3F22C34C-8E2F-4b2b-B0E1-6C1E8C2169E7}"));

bool serialise::read(const CString& csPath)
{
	// create
	if(!create(t_read,csPath))
		return false;

	// no doc so lets save what we need

	// guid
	CString cs;
	if(!read<>(cs))
		return false;
	if(!(cs==s_GUID))
		return false;
		
	// members
	int nVersion=0;
	if(!read<>(nVersion))
		return false;
	if(nVersion > 0)
	{
		// app
		if(!theApp.read(this))
			return false;

		// layer dialog
		if(!theApp.getlayerdlg()->read(this))
			return false;

		// training dialog
		if(!theApp.gettraindlg()->read(this))
			return false;
	}
	
	// flush
	close();

	// success
	return true;
}

bool serialise::write(const CString& csPath)
{
	// create
	if(!create(t_write,csPath))
		return false;

	// no doc so lets save what we need

	// guid
	if(!write<>(s_GUID))
		return false;
	
	// version
	const int nVersion = 1;
	if(!write<>(nVersion))
		return false;

	// app
	if(!theApp.write(this))
		return false;

	// layer dialog
	if(!theApp.getlayerdlg()->write(this))
		return false;

	// training dialog
	if(!theApp.gettraindlg()->write(this))
		return false;
	
	// flush
	close();

	// success
	return true;
}

bool serialise::create(const type t, const CString& csPath)
{
	switch(t)
	{
		case t_read:
		{
			close();
			std::shared_ptr<CFile> spFile(new CFile);
			if(!spFile->Open(csPath,CFile::modeRead|CFile::typeBinary))
				return false;
			std::shared_ptr<CArchive> spArchive(new CArchive(spFile.get(),CArchive::load));
			m_spFile=spFile;
			m_spArchive=spArchive;
			m_Type=t;
			return true;
		}
		break;
		case t_write:
		{
			close();
			std::shared_ptr<CFile> spFile(new CFile);
			if(!spFile->Open(csPath,CFile::modeWrite|CFile::modeCreate|CFile::typeBinary))
				return false;
			std::shared_ptr<CArchive> spArchive(new CArchive(spFile.get(),CArchive::store));
			m_spFile=spFile;
			m_spArchive=spArchive;
			m_Type=t;
			return true;
		}
		break;
		default:ASSERT(false);return false;
	}
	return FALSE;
}
