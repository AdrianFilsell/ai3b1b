#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <random>
#include <numeric>
#include <algorithm>

namespace af
{
	// T - in
	template <typename T> inline T sqrt( const T d ) { return T( ::sqrt( double(d) ) ); }
	template <> inline float sqrt( const float d ) { return ::sqrtf( d ); }
	template <> inline double sqrt( const double d ) { return ::sqrt( d ); }

	// Interpolation
	// T1 - in from and to interpolated data type
	// T2 - float interpolation value type and return type
	template<typename T1,typename T2> __forceinline T2 getlerp( const T1 from, const T1 to, const T2 dI ) { return ( from * ( 1 - dI ) + ( to * dI ) ); }
	template<typename T1,typename T2> __forceinline T2 getlerpI( const T1 from, const T1 to, const T1 l ) { const T2 d = ( to - from ); return d ? ( ( l - from ) / d ) : 0; }

	// T - in data type
	// R - floored out data type
	template <typename T, typename R> constexpr __forceinline R posfloor( const T d )
	{
		// Floored integer
		return R( d );
	}
	// T - in data type
	// R - rounded out data type
	template <typename T, typename R> constexpr __forceinline R posround( const T d )
	{
		// Nearest integer
		return R( 0.5 + d );
	}
	// T - in data type
	// R - ceiled out data type
	template <typename T, typename R> constexpr __forceinline R posceil( const T d )
	{
		// Ceiled integer
		const R r = R( d );
		return T( r ) == d ? r : 1 + r;
	}
	__forceinline void srand( const unsigned int n )
	{
		// seed random number
		::srand(n);
	}
	// T - type
	template <typename T> constexpr __forceinline T rand( const T from, const T to )
	{
		// generate random number
		const double dI = ::rand()/static_cast<double>(RAND_MAX);
		return getlerp<T,double>( from, to, dI );
	}
	// T - e data type
	template <typename T> constexpr __forceinline T gete(void)
	{
		return T( 2.7182818284590452353602874713527 );
	}
	// T - pi data type
	template <typename T> constexpr __forceinline T getpi( void )
	{
		return T( 3.1415926535897932384626433832795 );
	}
	// T - 2pi data type
	template <typename T> constexpr __forceinline T getpi_2( void )
	{
		return 2 * getpi<T>();
		//T( 6.283185307179586476925286766559 );
	}

	template <typename T> __forceinline bool fpclassify( const T d, const unsigned int nTest )
	{
		return ::fpclassify(d) == nTest;
	}
	template <typename T> __forceinline bool fpvalid( const T d, const bool bDenormalNormal = false/*non-zero number with magnitude smaller than the smallest 'normal' number*/ )
	{
		const int n = ::fpclassify(d);
		if( FP_ZERO == n )
			return true;
		if( FP_NORMAL == n )
			return true;
		if( FP_SUBNORMAL == n )
			return bDenormalNormal;
		// could be NAN or INFINITE
		return false;
	}

	template <bool FROM=false,bool INCLUSIVETO=false> struct vectorrange
	{
		vectorrange()
		{
			if( FROM )
				m_nFrom = 0;
			if( INCLUSIVETO )
				m_nInclusiveTo = -1;
		}
		vectorrange(const int nA){if(FROM)m_nFrom=nA;else if(INCLUSIVETO)m_nInclusiveTo=nA;}
		vectorrange(const int nFrom,const int nInclusiveTo){ASSERT(FROM && INCLUSIVETO);if(FROM)m_nFrom=nFrom;if(INCLUSIVETO)m_nInclusiveTo=nInclusiveTo;}
		vectorrange(const vectorrange& o){*this=o;}
	
		bool isempty( void ) const { return getsize() < 1; }
		bool isvalid( const int nSize ) const { if( FROM && ( getfrom() >= nSize || getfrom() < 0 ) ) return false; if( INCLUSIVETO && ( getinclusiveto() >= nSize || getinclusiveto() < 0 ) ) return false; return true; }
		bool issubset( const vectorrange<FROM,INCLUSIVETO>& other ) const
		{
			if( FROM )
			{
				if( other.getfrom() < getfrom() ) return false;
				if( INCLUSIVETO )
				{
					if( other.getfrom() > getinclusiveto() ) return false;
				}
			}
			if( INCLUSIVETO )
			{
				if( other.getinclusiveto() > getinclusiveto() ) return false;
			}
			return true;
		}
		int getfrom( void ) const { return m_nFrom; }
		int getinclusiveto( void ) const { return m_nInclusiveTo; }
		int getsize( void ) const { return getinclusiveto() - getfrom() + 1; }

		template <typename V_t> typename std::vector<V_t>::iterator getstdbegin( std::vector<V_t> *p ) const { return FROM ? p->begin() + getfrom() : p->begin(); }
		template <typename V_t> typename std::vector<V_t>::const_iterator getstdcbegin( const std::vector<V_t> *p ) const { return FROM ? p->cbegin() + getfrom() : p->cbegin(); }

		template <typename V_t> typename std::vector<V_t>::iterator getstdend( std::vector<V_t> *p ) const { return INCLUSIVETO ? ( ( getinclusiveto() == int( p->size() - 1 ) ) ? p->end() : p->begin() + getinclusiveto() + 1 ) : p->end(); }
		template <typename V_t> typename std::vector<V_t>::const_iterator getstdcend( const std::vector<V_t> *p ) const { return INCLUSIVETO ? ( ( getinclusiveto() == int( p->size() - 1 ) ) ? p->cend() : p->cbegin() + getinclusiveto() + 1 ) : p->cend(); }

		void setfrom( const int n ) { m_nFrom=n; }
		void setinclusiveto( const int n ) { m_nInclusiveTo=n; }

		vectorrange& operator =( const vectorrange& o ) { if( FROM ) m_nFrom=o.m_nFrom; if( INCLUSIVETO ) m_nInclusiveTo=o.m_nInclusiveTo; return *this; }
	protected:
		int m_nFrom;
		int m_nInclusiveTo;
	};
	template <bool MINCLIP=false,bool MAXCLIP=false> int getint(const wchar_t *cs,const int nMinClip=0,const int nMaxClip=0)
	{
		int n = _ttoi(cs);
		if(MINCLIP)
			n = std::max<int>(n,nMinClip);
		if(MAXCLIP)
			n = std::min<int>(n,nMaxClip);
		return n;
	}
	template <bool MINCLIP=false,bool MAXCLIP=false> double getfloat(const wchar_t *cs,const double dMinClip=0,const double dMaxClip=0)
	{
		double d = _ttof(cs);
		if(MINCLIP)
			d = std::max<double>(d,dMinClip);
		if(MAXCLIP)
			d = std::min<double>(d,dMaxClip);
		return d;
	}
}
