#ifndef _UTILITY_H_
#define _UTILITY_H_

#include "col3.h"
#include "vec3.h"
#include <algorithm>

static const float pi     = ( float ) M_PI;
static const float invpi  = ( float ) ( 1.f / M_PI );
static const float inv4pi = ( float ) ( 1.f / ( 4.f * M_PI ) );


inline static float luminance( const col3& col )
{
	return 0.212671f * col.r + 0.715160f * col.g + 0.072169f * col.b;
}


/**
 * @fn inline static float fresnel( float cosine, float ior )
 * @brief calculate fresnel reflection coefficient
 * @param cosine : cosine of incident angle 
 * @param ior : relative index of refraction eta_i / eta_t 
 */
inline static float fresnel( float cosine, float ior )
{
	if( ior < 0.f ) return 1.f;
	if( cosine < 0.f ) cosine *= - 1.f;
	const float sinT2 = ior * ior * ( 1.f - cosine * cosine );
	const float cosT  = sqrtf( std::max( 0.f, 1.f - sinT2 ) );
	const float term1 = ior * cosT;
	const float R0 = ( cosine - term1 ) / ( cosine + term1 );
	const float term2 = ior * cosine;
	const float R1 = ( term2 - cosT ) / ( term2 + cosT );
	return 0.5f * ( R0 * R0 + R1 * R1 );
}


inline vec3 reflectLocal( const vec3& wi )
{
	return vec3( - wi.x, - wi.y, wi.z );
}

/**
 * @fn inline vec3 samplePowerCosHemisphere( const float xi0, const float xi1, const float power, float *pdfw )
 * @brief sample direction with pdf proportional to cos^power
 */
inline vec3 samplePowerCosHemisphere( const float xi0, const float xi1, const float power, float *pdfw )
{
	const float term1 = 2.f * pi * xi0;
	const float term2 = std::powf( xi1, 1.f / ( power + 1.f ) );
	const float term3 = std::sqrtf( 1.f - term2 * term2 );
	if( pdfw != nullptr ) {
		*pdfw = ( power + 1.f ) * std::powf( term2, power ) * ( 0.5f * invpi );
	}
	return vec3( cosf( term1 ) * term3, sinf( term1 ) * term3, term2 );
}

/**
 * @fn inline float powerCosHemispherePDF( const vec3& normal, const vec3& w, const float power )
 * @brief calculate PDF proportional to cos^power
 */
inline float powerCosHemispherePDF( const vec3& normal, const vec3& w, const float power )
{
	const float cosine = std::max( 0.f, dot( normal, w ) );
	return ( power + 1.f ) * std::powf( cosine, power ) * ( 0.5f * invpi );
}

/**
 * @fn inline float powerCosHemispherePDF( const float cosine, const float power )
 * @brief calculate PDF proportional to cos^power
 */
inline float powerCosHemispherePDF( const float cosine, const float power )
{
    assert( cosine >= 0.f );
    return ( power + 1.f ) * std::powf( cosine, power ) * 0.5f * invpi;
}

/**
 * @fn inline vec3 sampleCosHemisphere( const float xi0, const float xi1, float *pdfw )
 * @brief sample direction with pdf proportional to cosine
 */
inline vec3 sampleCosHemisphere( const float xi0, const float xi1, float *pdfw )
{
	const float term1 = 2.f * pi * xi0;
	const float term2 = std::sqrtf( 1.f - xi1 );
	const vec3 w( cosf( term1 ) * term2, sinf( term1 ) * term2, std::sqrtf( xi1 ) );
	if( pdfw != nullptr ) {
		*pdfw = w.z * invpi;
	}
	return w;
}

/**
 * @fn inline float cosHemispherePDF( const vec3& normal, const vec3& w )
 * @brief calculate PDF proportional to cosine
 */
inline float cosHemispherePDF( const vec3& normal, const vec3& w )
{
	return std::max( 0.f, dot( normal, w ) ) * invpi;
}

/**
 * @fn inline vec3 sampleUniformTriangle( const float xi0, const float xi1 )
 * @brief sample uv(barycentric coordinate) 
 */
inline vec3 sampleUniformTriangle( const float xi0, const float xi1 )
{
    vec3 result;
    const float term = std::sqrtf( xi0 );
    result.x = 1.f - term;
    result.y = term * xi1;
    return result;
}

/**
 * @fn inline vec3 sampleUniformTriangle( const float xi0, const float xi1, const vec3& v0, const vec3& v1, const vec3& v2 )
 * @brief uniform sample a point on a triangle ( v0, v1, v2 )
 */
inline vec3 sampleUniformTriangle( const float xi0, const float xi1, const vec3& v0, const vec3& v1, const vec3& v2 )
{
    vec3 p;
    const float term = std::sqrtf( xi0 );
    const float u = 1.f - term;
    const float v = term * xi1;
    p = ( 1.f - u - v ) * v0 + u * v1 + v * v2;
    return p;
}

/**
 * @fn inline vec3 sampleConcentricDisc( const float xi0, const float xi1 )
 * @brief uniformly sample a point on concentric disc
 */
inline vec3 sampleConcentricDisc( const float xi0, const float xi1 )
{
    vec3 p;
    float phi, r;
    const float a = 2.f * xi0 - 1.f;
    const float b = 2.f * xi1 - 1.f;

    if( a > - b ) {
        if( a > b ) {
            r = a;
            phi = ( pi / 4.f ) * ( b / a );
        } else {
            r = b;
            phi = ( pi / 4.f ) * ( 2.f - a / b );
        }
    } else {
        if( a < b ) {
            r = - a;
            phi = ( pi / 4.f ) * ( 4.f + b / a );
        } else {
            r = - b;
            if( b != 0 ) {
                phi = ( pi / 4.f ) * ( 6.f - a / b );
            } else {
                phi = 0.f;
            }
        }
    }
    p.x = r * std::cosf( phi );
    p.y = r * std::sinf( phi );
    p.z = 0.f;
    return p;
}

/**
 * @fn inline vec3 sampleUniformSphere( const float xi0, const float xi1, float *pdfw )
 *
 */
inline vec3 sampleUniformSphere( const float xi0, const float xi1, float *pdfw )
{
    const float term1 = 2.f * pi * xi0;
    const float term2 = 2.f * std::sqrtf( xi1 - xi1 * xi1 );
    const vec3 w( std::cosf( term1 ) * term2, std::sinf( term1 ) * term2, 1.f - 2.f * xi1 );
    if( pdfw != nullptr ) {
        *pdfw = inv4pi;
    }
}

inline col3 tonemap( const col3& col, const float gamma = 2.2f )
{
    const float invgamma = 1.f / gamma;
    col3 val;
    val.r = std::powf( col.r, invgamma );
    val.g = std::powf( col.g, invgamma );
    val.b = std::powf( col.b, invgamma );
    return val;
}

/**
 * @fn inline bool is_black_or_negative( const col3& col )
 * @brief if col.r <= 0 && col.g <= 0 && col.b <= 0 return true, otherwise false
 */
inline bool is_black_or_negative( const col3& col )
{
    return ( ( _mm_movemask_ps( _mm_cmple_ps( col.c, _mm_set1_ps( 0 ) ) ) & 0x7 ) == 0x7 );
}

/***
 * @fn inline float clamp( const float x, const float _min, const float _max )
 * @brief return clamped value of x between _min and _max
 */
inline float clamp( const float x, const float _min, const float _max ) 
{
    return std::max( std::min( x, _max ), _min );
}

template< typename T > T clamp( const T& x, const T& _min, const T& _max )
{
    return std::max( std::min( x, _max ), _min );
}



#endif