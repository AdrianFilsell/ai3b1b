
#pragma once

#include <memory>
#include <vector>

class serialise
{
public:
	enum type {t_null,t_read,t_write};
	serialise(){m_Type=t_null;}

	bool getpath(CString &cs)const{if(!m_spFile)return false;cs=m_spFile->GetFilePath();return true;}
	
	bool read(const CString& csPath);
	bool write(const CString& csPath);

	template <typename T> bool read(std::vector<T>& v) const 
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;
			
		int n;
		if(!read<>(n))
			return false;
		v.resize(n);
		
		try { return m_spArchive->Read(&v[0],n*sizeof(T)) == n*sizeof(T); }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
	template <typename T> bool write(const std::vector<T>& v) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
			
		const int n = static_cast<int>(v.size());
		if(!write<>(n))
			return false;
			
		try { m_spArchive->Write(&v[0],n*sizeof(T));return true; }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
	template <typename T,typename S=T> bool read(T& t) const
	{
		if(m_Type!=t_read || !m_spArchive)
			return false;

		S s;
		try { (*m_spArchive)>>s;t=static_cast<T>(s);return true; }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
	template <typename T,typename S=T> bool write(const T t) const
	{
		if(m_Type!=t_write || !m_spArchive)
			return false;
			
		try { (*m_spArchive)<<static_cast<const S>(t);return true; }
		catch (CArchiveException *e) { e->Delete(); return false; }
	}
protected:
	type m_Type;
	std::shared_ptr<CFile> m_spFile;
	std::shared_ptr<CArchive> m_spArchive;
	static CString s_GUID;

	bool create(const type t, const CString& csPath);
	void close(void)
	{
		m_spArchive=nullptr;
		m_spFile=nullptr;
	}
};
