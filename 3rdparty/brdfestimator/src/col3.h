//
//  col3.h
//
//  Created by 岩崎慶 on 2015/08/08.
//  Copyright (c) 2015年 Kei Iwasaki. All rights reserved.
//

#ifndef _COL3_H_
#define _COL3_H_

#include <xmmintrin.h>
#include <smmintrin.h>
#include <algorithm>
#include <cmath>
#include <sstream>
#include "new_delete_form.h"

__declspec( align( 16 ) ) class col3 :  public aligned_new_delete< 16 >
{
public:
    
    __declspec( noalias ) col3() : c( _mm_setzero_ps() ) {
    }

	__declspec( noalias ) col3( const float rgb ) : c( _mm_set_ps( 0.f, rgb, rgb, rgb ) ) {
	}
    
    __declspec( noalias ) col3( const float _r, const float _g, const float _b ) : c( _mm_set_ps( 0.f, _b, _g, _r ) ) {
    }
    __declspec( noalias ) col3( const col3& _c ) : c( _c.c ) {
    }
    
    __declspec( noalias ) col3( const __m128& _c ) : c( _c ) {
    }
    
    __declspec( noalias ) col3 operator+( const col3& _c ) const
    {
        return col3( _mm_add_ps( c, _c.c ) );
    }
    
    __declspec( noalias ) col3 operator-( const col3& _c ) const
    {
        return col3( _mm_sub_ps( c, _c.c ) );
    }
    
    __declspec( noalias ) col3 operator*( const float scale ) const
    {
        return col3( _mm_mul_ps( _mm_set1_ps( scale ), c ) );
    }

	__declspec( noalias ) col3 operator*( const col3& _c ) const
	{
		return col3( _mm_mul_ps( _c.c, c ) );
	}
    
    __declspec( noalias ) col3 operator/( const float scale ) const
    {
        return col3( _mm_mul_ps( _mm_rcp_ps( _mm_set1_ps( scale ) ), c ) );
    }
    
    __declspec( noalias ) col3& operator=( const col3& _c )
    {
        c = _c.c;
        return *this;
    }
    
    __declspec( noalias ) col3& operator+=( const col3& _c )
    {
        c = _mm_add_ps( c, _c.c );
        return *this;
    }
    
    __declspec( noalias ) col3& operator-=( const col3& _c )
    {
        c = _mm_sub_ps( c, _c.c );
        return *this;
    }
    
    __declspec( noalias ) col3& operator*=( const float scale )
    {
        c = _mm_mul_ps( c, _mm_set1_ps( scale ) );
        return *this;
    }

	__declspec( noalias ) col3& operator*=( const col3& _c )
	{
		c = _mm_mul_ps( c, _c.c );
		return *this;
	}
    
    __declspec( noalias ) col3& operator/=( const float scale )
    {
        c = _mm_mul_ps( c, _mm_rcp_ps( _mm_set1_ps( scale ) ) );
        return *this;
    }
    
    __declspec( noalias ) float norm( void ) const
    {
        float r; _mm_store_ss( &r, _mm_sqrt_ss( _mm_dp_ps( c, c, 0x7f ) ) );
        return r;
    }
    
    __declspec( noalias ) float norm2( void ) const
    {
        float r; _mm_store_ss( &r, _mm_dp_ps( c, c, 0x7f ) );
        return r;
    }
    
    __declspec( noalias ) float max( void ) const
    {
        return std::max( r, std::max( g, b ) );
    }
    
    __declspec( noalias ) float min( void ) const
    {
        return std::min( r, std::min( g, b ) );
    }
    
    inline friend float norm( const col3& c )
    {
        float r; _mm_store_ss( &r, _mm_sqrt_ss( _mm_dp_ps( c.c, c.c, 0x7f ) ) );
        return r;
    }
    
    inline friend col3 normalize( const col3& c )
    {
        return col3( _mm_mul_ps( c.c, _mm_rsqrt_ps( _mm_dp_ps( c.c, c.c, 0x7f ) ) ) );
        //const float l = v.norm();
        //return vec3( _mm_mul_ps( v.v, _mm_rcp_ps( _mm_set1_ps( l ) ) ) );
    }
    
    inline friend float dot( const col3& c0, const col3& c1 )
    {
        float r; _mm_store_ss( &r, _mm_dp_ps( c0.c, c1.c, 0x7f ) );
        return r;
    }
    
    inline friend col3 cross( const col3& c0, const col3& c1 )
    {
        return col3(
                    _mm_sub_ps( _mm_mul_ps(
                                           _mm_shuffle_ps( c0.c, c0.c, _MM_SHUFFLE( 1, 0, 2, 1 )), //y1 z1 x1 y1
                                           _mm_shuffle_ps( c1.c, c1.c, _MM_SHUFFLE( 2, 1, 0, 2 ))  //z2 x2 y2 z2
                                           ),
                               _mm_mul_ps(
                                          _mm_shuffle_ps( c0.c, c0.c, _MM_SHUFFLE( 1, 1, 0, 2 )), //z1 x1 y1 y1
                                          _mm_shuffle_ps( c1.c, c1.c, _MM_SHUFFLE( 2, 0, 2, 1 ))  //y2 z2 x2 z2
                                          )
                               )
                    );
    }
    
    inline friend col3 operator*( const float scale, const col3& c0 )
    {
        return col3( _mm_mul_ps( _mm_set1_ps( scale ), c0.c ) );
    }
    
    friend std::ostream& operator<<( std::ostream& os, const col3& c )
    {
        os << "(" << c.r << ", " << c.g << ", " << c.b << ")";
        return os;
    }
    
    friend col3 max( const col3& c0, const col3& c1 )
    {
        return col3( _mm_max_ps( c0.c, c1.c ) );
    }
    
    friend col3 min( const col3& c0, const col3& c1 )
    {
        return col3( _mm_min_ps( c0.c, c1.c ) );
    }
    
    const std::string toString( void ) const
    {
        std::stringstream ss;
        ss << "col3 [ " << r << ", " << g << ", " << b << ", " << a << " ]\n";
        return ss.str();
    }
    
    union {
        struct{ float r, g, b, a; };
        __m128 c;
    };
};


#endif
