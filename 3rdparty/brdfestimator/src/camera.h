/**
 * @file camera.h
 * @brief camera class
 * @author Kei Iwasaki
 */
#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <iostream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>
#include "vec3.h"
#include "ray.h"

class Camera
{
    
public:
    
    Camera() :  eye( 0.f, 0.f, 10.f ), ref( 0.f, 0.f, 0.f ), vr_( 10.f ), the_( 0.f ), phi_( 0.f ),
                fovy_( 45.f ), tfov_( tanf( fovy_ * M_PI / 360.f ) ), resx_( 512 ), resy_( 512 )
    {
        makeMatrix();
    }
    
    Camera( const vec3& _eye, const vec3& _ref, float _fovy, int _resx = 512, int _resy = 512 )
    :   eye( _eye ), ref( _ref ), fovy_( _fovy ), tfov_( tanf( fovy_ * M_PI / 360.f ) ), resx_( _resx ), resy_( _resy )
    {
        const vec3 v = eye - ref;
        const vec3 d = normalize( eye - ref ); //( costh * sinph, sinth, costh * cosph )
        const float cth = sqrtf( d.x * d.x + d.z * d.z );
        //const float sth = lydir;
        const float cph = d.z / cth;
        const float sph = d.x / cth;
        //the = acosf( std::max( std::min( cth, 1.f ), -1.f ) );
        vr_  = norm( v );
        the_ = acosf( cth );
        phi_ = atan2f( sph, cph );
        
        makeMatrix();
    }
    
    void walk( const float delta )
    {
        eye -= delta * w_;
        ref -= delta * w_;
    }
    
    void turn( const float dth, const float dph )
    {
        the_ += dth;
        phi_ += dph;
        const float cth = cosf( the_ );
        const float sth = sinf( the_ );
        const float cph = cosf( phi_ );
        const float sph = sinf( phi_ );
        ref.x = - vr_ * cth * sph + eye.x;
        ref.y = - vr_ * sth       + eye.y;
        ref.z = - vr_ * cth * cph + eye.z;
        w_ = vec3( cth * sph, sth, cth * cph );
        u_ = vec3( cph, 0.f, - sph );
        v_ = vec3( - sth * sph, cth, -sth * cph );
    }
    
    void pan( const float dx, const float dy )
    {
        if( fabs( dx ) > fabs( dy ) ) {
            eye += dx * u_;
            ref += dx * u_;
        } else {
            eye += dy * v_;
            ref += dy * v_;
        }
    }
    
    void rotate( const float dth, const float dph )
    {
        the_ += dth;
        phi_ += dph;
        const float cth = cosf( the_ );
        const float sth = sinf( the_ );
        const float cph = cosf( phi_ );
        const float sph = sinf( phi_ );
        eye.x = vr_ * cth * sph + ref.x;
        eye.y = vr_ * sth       + ref.y;
        eye.z = vr_ * cth * cph + ref.z;
        w_ = vec3( cth * sph, sth, cth * cph );
        u_ = vec3( cph, 0.f, - sph );
        v_ = vec3( - sth * sph, cth, -sth * cph );
    }
    
    void makeMatrix( void )
    {
        const float lxdir = ( eye.x - ref.x ) / vr_; // costh * sinph
        const float lydir = ( eye.y - ref.y ) / vr_; // sinth
        const float lzdir = ( eye.z - ref.z ) / vr_; // costh * cosph
        const float cth = sqrtf( lxdir * lxdir + lzdir * lzdir );
        const float sth = lydir;
        const float cph = lzdir / cth;
        const float sph = lxdir / cth;
        
        if( fabs( cth ) < 1e-10f ) {
            u_ = vec3( 1.f, 0.f, 0.f );
            v_ = vec3( 0.f, 1.f, 0.f );
            w_ = vec3( 0.f, 0.f, 1.f );
        } else {
            u_ = vec3( cph,   0.f, - sph );
            v_ = vec3( - sth * sph, cth, - sth * cph );
            w_ = vec3( lxdir, lydir, lzdir );
        }
    }
    
    const vec3 viewpoint( void ) const
    {
        return eye;
    }
    
    const vec3 reference( void ) const
    {
        return ref;
    }
    
    const vec3 u( void ) const
    {
        return u_;
    }
    
    const vec3 v( void ) const
    {
        return v_;
    }
    
    const vec3 w( void ) const
    {
        return w_;
    }
    
    vec3& u( void )
    {
        return u_;
    }
    
    vec3& v( void )
    {
        return v_;
    }
    
    vec3& w( void )
    {
        return w_;
    }
    
    std::string toString( void ) const
    {
        std::stringstream oss;
        oss << "Camera view :" << eye.toString()
            << "       ref : " << ref.toString()
            << "u   : " << u_.toString() << "v   : " << v_.toString() << "w   : " << w_.toString()
            << "resolution : "  << resx_ << " : " << resy_ << "\n";
        return oss.str();
    }
    
    Ray generateRay( float x, float y ) const
    {
        Ray r;
        const float delta = 2.f * tfov_ / ( float ) resy_;
        const float xx = ( x - resx_ / 2.f + 0.5f ) * delta;
        const float yy = ( y - resy_ / 2.f + 0.5f ) * delta;
        r.o = eye;
        r.d = normalize( xx * u_ + yy * v_ - w_ );
        return r;
    }
    
    float fovy() {
        return fovy_;
    }

    const float vr( void ) const
    {
        return vr_;
    }

    const float theta( void ) const 
    {
        return the_;
    }

    const float phi( void ) const 
    {
        return phi_;
    }

private:
    //float vx, vy, vz;
    //float rx, ry, rz;
    vec3 eye; //viewpoint
    vec3 ref; //reference
    float vr_, the_, phi_;
    float fovy_, tfov_;
    vec3 u_, v_, w_;
    int resx_, resy_;
    
};



#endif
