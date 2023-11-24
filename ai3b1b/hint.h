
#pragma once

#include "winthread_ml.h"

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

class hint
{
public:
	enum type {t_null=0x0,							// null
			   t_widget=0x1,						// opened/closed widget
			   t_input_type=0x2,					// input type ( ie user/image ) changed
			   t_thread_query_results=0x4,			// thread processed requests
			   t_thread_playconfig_results=0x8,		// thread processed requests
			   t_epoch_range=0x10,					// epoch range changed
			   t_clear_epochs=0x20,					// epochs cleared
			   t_training_data=0x40,				// training data mutated
			   t_load=0x80,							// serialise in
			  };
	hint():hint(t_null){}
	hint(const type t):m_Type(t){}
	type gettype(void)const{return m_Type;}
	
	hint& operator=(const hint& o) {m_Type=o.m_Type;return *this;}
protected:
	type m_Type;
};

class threadqueryresulthint : public hint
{
public:
	threadqueryresulthint(std::shared_ptr<const mlthreadqueryresults> sp):hint(t_thread_query_results){m_sp=sp;}
	type gettype(void)const{return m_Type;}
	std::shared_ptr<const mlthreadqueryresults> getresults(void)const{return m_sp;}
	
	threadqueryresulthint& operator=(const threadqueryresulthint& o) {hint::operator=(o);m_sp=o.m_sp;return *this;}
protected:
	std::shared_ptr<const mlthreadqueryresults> m_sp;
};

class threadplayconfigresulthint : public hint
{
public:
	threadplayconfigresulthint(std::shared_ptr<const mlthreadplayconfigresults> sp):hint(t_thread_playconfig_results){m_sp=sp;}
	type gettype(void)const{return m_Type;}
	std::shared_ptr<const mlthreadplayconfigresults> getresults(void)const{return m_sp;}
	
	threadplayconfigresulthint& operator=(const threadplayconfigresulthint& o) {hint::operator=(o);m_sp=o.m_sp;return *this;}
protected:
	std::shared_ptr<const mlthreadplayconfigresults> m_sp;
};
