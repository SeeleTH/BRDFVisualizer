#ifndef _VEC3_H_
#define _VEC3_H_

#include <iostream>
#include <sstream>
#include <xmmintrin.h>
#include <smmintrin.h>
#include "new_delete_form.h"

/**
 * @class vec3 
 * @brief three-dimensional vector class
 */
__declspec( align( 16 ) ) class vec3 : public aligned_new_delete< 16 > {
	
public:

	__declspec(noalias) vec3() : v( _mm_setzero_ps() ) {
	}

	__declspec(noalias) vec3( const float _x, const float _y, const float _z ) : v( _mm_set_ps( 0.f, _z, _y, _x ) ) {
	}

	__declspec(noalias) vec3( const vec3& _v ) : v( _v.v ) {
	}

	__declspec(noalias) vec3( const __m128& _v ) : v( _v ) {
	}

	__declspec(noalias) vec3 operator+( const vec3& _v ) const 
	{
		return vec3( _mm_add_ps( v, _v.v ) );
	}

	__declspec(noalias) vec3 operator-( const vec3& _v ) const 
	{
		return vec3( _mm_sub_ps( v, _v.v ) );
	}

	__declspec(noalias) vec3 operator*( const float scale ) const 
	{
		return vec3( _mm_mul_ps( _mm_set1_ps( scale ), v ) );
	}

	__declspec(noalias) vec3 operator/( const float scale ) const 
	{
		return vec3( _mm_mul_ps( _mm_rcp_ps( _mm_set1_ps( scale ) ), v ) );
	}

	__declspec(noalias) vec3& operator=( const vec3& _v )
	{
		v = _v.v;
		return *this;
	}

	__declspec(noalias) vec3& operator+=( const vec3& _v ) 
	{
		v = _mm_add_ps( v, _v.v );
		return *this;
	}

	__declspec(noalias) vec3& operator-=( const vec3& _v ) 
	{
		v = _mm_sub_ps( v, _v.v );
		return *this;
	}

	__declspec(noalias) vec3& operator*=( const float scale ) 
	{
		v = _mm_mul_ps( v, _mm_set1_ps( scale ) );
		return *this;
	}

	__declspec(noalias) vec3& operator/=( const float scale ) 
	{
		v = _mm_mul_ps( v, _mm_rcp_ps( _mm_set1_ps( scale ) ) );
		return *this;
	}

	__declspec( noalias ) vec3 operator-( void ) const
	{
		return vec3( _mm_mul_ps( _mm_set1_ps( -1.f ), v ) );
	}

	__declspec(noalias) float norm( void ) const 
	{
		float r; _mm_store_ss( &r, _mm_sqrt_ss( _mm_dp_ps( v, v, 0x7f ) ) );
		return r;
	}

	__declspec(noalias) float norm2( void ) const 
	{
		float r; _mm_store_ss( &r, _mm_dp_ps( v, v, 0x7f ) );
		return r;
	}
    
    __declspec(noalias) std::string toString( void ) const
    {
        std::stringstream oss;
        oss << "vec3 : [ " << x << ", " << y << ", " << z << " ]\n";
        return oss.str();
    }

	inline friend float norm( const vec3& v )
	{
		float r; _mm_store_ss( &r, _mm_sqrt_ss( _mm_dp_ps( v.v, v.v, 0x7f ) ) );
		return r;
	}

	inline friend vec3 normalize( const vec3& v ) 
	{
		//return vec3( _mm_mul_ps( v.v, _mm_rsqrt_ps( _mm_dp_ps( v.v, v.v, 0x7f ) ) ) ); //_mm_rsqrt_ps is approximation, not so accurate
		//const float l = sqrtf( v.x * v.x + v.y * v.y + v.z * v.z );
		//return vec3( v.x / l, v.y / l, v.z / l );
		return vec3( _mm_div_ps( v.v, _mm_sqrt_ps( _mm_dp_ps( v.v, v.v, 0x7f ) ) ) );
	}

	inline friend float dot( const vec3& v0, const vec3& v1 ) 
	{
		float r; _mm_store_ss( &r, _mm_dp_ps( v0.v, v1.v, 0x7f ) );
		return r;
	}

	inline friend vec3 cross( const vec3& v0, const vec3& v1 )
	{
		return vec3(
		_mm_sub_ps( _mm_mul_ps( 
			_mm_shuffle_ps( v0.v, v0.v, _MM_SHUFFLE( 1, 0, 2, 1 )), //y1 z1 x1 y1
			_mm_shuffle_ps( v1.v, v1.v, _MM_SHUFFLE( 2, 1, 0, 2 ))  //z2 x2 y2 z2
			),
		_mm_mul_ps( 
			_mm_shuffle_ps( v0.v, v0.v, _MM_SHUFFLE( 1, 1, 0, 2 )), //z1 x1 y1 y1
			_mm_shuffle_ps( v1.v, v1.v, _MM_SHUFFLE( 2, 0, 2, 1 ))  //y2 z2 x2 z2
			)
			)
		);
	}

	inline friend vec3 operator*( const float scale, const vec3& v0 )
	{
		return vec3( _mm_mul_ps( _mm_set1_ps( scale ), v0.v ) );
	}

	friend std::ostream& operator<<( std::ostream& os, const vec3& v )
	{
		os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
		return os;
	}

	friend vec3 max( const vec3& v0, const vec3& v1 )
	{
		return vec3( _mm_max_ps( v0.v, v1.v ) );
	}

	friend vec3 min( const vec3& v0, const vec3& v1 )
	{
		return vec3( _mm_min_ps( v0.v, v1.v ) );
	}

	union {
		struct{ float x, y, z, w; };
		__m128 v;
	};
	
	
};



#endif