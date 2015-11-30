#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <iostream>
#include "vec3.h"
#include "col3.h"
#include "frame.h"
#include "envmap.h"

/**
 * 
 *
 */
struct SceneSphere {

    vec3 center_;
    float radius_;
    float invradius2_; // 1.f / ( radius_ * radius_ );

};

/**
 * @class AbstractLight
 *
 */
class AbstractLight {

public:
    AbstractLight() {
    }

    ~AbstractLight() {
    }

    virtual col3 illuminate( const vec3& x, //point to be shaded
                             const float xi0, //random number 
                             const float xi1, //random number
                             vec3& w,  //incident direction from light to x
                             float& d, //length between x and point on light 
                             float& pdfw, //pdf of having chosen this direction
                             float *emissionPDFW = nullptr, //pdf of emitting particle in this direction
                             float *cosAtLight = nullptr    //cosine from light normal
                             ) const = 0;

    virtual col3 emit( const float xi0,
                       const float xi1,
                       const float xi2,
                       const float xi3,
                       vec3& x,
                       vec3& w,
                       float& emissionPDFW,
                       float *pdfA,
                       float *costhLight ) const = 0;
                       
    virtual col3 radiance( const vec3& w, 
                           const vec3& hitpoint,
                           float *pdfA = nullptr,
                           float *emissionPDFW = nullptr
                           ) const = 0;

    virtual bool isFinite( void ) const = 0;

    virtual bool isDelta( void ) const = 0;

public:

    static SceneSphere sphere_;
};


/**
 * @class AreaLight
 *
 */
class AreaLight : public AbstractLight
{
public:

    AreaLight( const vec3& p0, const vec3& p1, const vec3& p2 ) {
        p0_ = p0;
        e0_ = p1 - p0;
        e1_ = p2 - p0;
        vec3 normal = cross( e0_, e1_ );
        area_ = normal.norm() / 2.f;
        pdfA_ = 1.f / area_;
        frame_.set( normal );
    }

    float area( void ) const {
        return area_;
    }

    col3& intensity( void ) {
        return intensity_;
    }

    const col3 intensity( void ) const {
        return intensity_;
    }

    virtual col3 illuminate( const vec3& x, const float xi0, const float xi1, vec3& w, float& d, float& pdfw, float *emissionPDFW = nullptr, float *cosAtLight = nullptr ) const
    {
        const vec3 p = sampleUniformTriangle( xi0, xi1, p0_, p0_ + e0_, p0_ + e1_ );
        w = p - x;
        const float d2 = dot( w, w );
        d = std::sqrtf( d2 );
        w /= d;
        const float cosnormal = - dot( frame_.normal(), w );
        if( cosnormal < Config::EPS_COSINE ) {
            return col3( 0.f );
        }
        pdfw = pdfA_ * d2 / cosnormal;
        if( emissionPDFW != nullptr ) {
            *emissionPDFW = pdfA_ * cosnormal * invpi;
        }
        if( cosAtLight != nullptr ) {
            *cosAtLight = cosnormal;
        }
        return intensity_;
    }

    virtual col3 emit( const float xi0, const float xi1, const float xi2, const float xi3, vec3& x, vec3& w, float& emissionPDFW, float *pdfA, float *costhLight ) const 
    {
        const vec3 p = sampleUniformTriangle( xi0, xi1, p0_, p0_ + e0_, p0_ + e1_ );
        vec3 lw = sampleCosHemisphere( xi2, xi3, &emissionPDFW );
        emissionPDFW *= pdfA_;
        lw.z = std::max( lw.z, Config::EPS_COSINE );
        w = frame_.toWorld( lw );
        if( pdfA != nullptr ) {
            *pdfA = pdfA_;
        }
        if( costhLight != nullptr ) {
            *costhLight = lw.z;
        }
        return intensity_ * lw.z;
    }

    virtual col3 radiance( const vec3& w, const vec3& hitpoint, float *pdfA, float *emissionPDFW ) const 
    {
        const float costh = std::max( 0.f, - dot( frame_.normal(), w ) );
        if( costh == 0 ) return col3( 0.f );
        if( pdfA != nullptr ) *pdfA = pdfA_;
        if( emissionPDFW != nullptr ) {
            *emissionPDFW = cosHemispherePDF( frame_.normal(), - w ) * pdfA_;
        }
        return intensity_;
    }

    virtual bool isFinite( void ) const { return true; }
    virtual bool isDelta ( void ) const { return false; } 

private:
    vec3 p0_, e0_, e1_;
    Frame frame_;
    float area_;
    float pdfA_;
    col3 intensity_;

};

/**
 * @class DirectionalLight
 *
 */
class DirectionalLight : public AbstractLight 
{
public:

    DirectionalLight( const vec3& w ) 
    {
        frame_.set( w );
    }

    col3& intensity( void ) 
    {
        return intensity_;
    }

    const col3 intensity( void ) const 
    {
        return intensity_;
    }

    virtual col3 illuminate( const vec3& x, const float xi0, const float xi1, vec3& w, float& d, float& pdfw, float *emissionPDFW = nullptr, float *cosAtLight = nullptr ) const
    {
        w = - frame_.normal();
        d = std::numeric_limits< float >::max();
        pdfw = 1.f;
        if( cosAtLight != nullptr ) {
            *cosAtLight = 1.f;
        }
        if( emissionPDFW != nullptr ) {
            *emissionPDFW = sphere_.invradius2_ * invpi;
        }
        return intensity_;
    }

    virtual col3 emit( const float xi0, const float xi1, const float xi2, const float xi3, vec3& x, vec3& w, float& emissionPDFW, float *pdfA, float *costhLight ) const 
    {
        const vec3 xy = sampleConcentricDisc( xi0, xi1 );
        x = sphere_.center_ + sphere_.radius_ * ( - frame_.normal() + frame_.binormal() * xy.x + frame_.tangent() * xy.y );
        w = frame_.normal();
        emissionPDFW = sphere_.invradius2_ * invpi;
        if( pdfA != nullptr ) *pdfA = 1.f;
        if( costhLight != nullptr ) *costhLight = 1.f;
        return intensity_;
    }

    virtual col3 radiance( const vec3& w, const vec3& hitpoint, float *pdfA, float *emissionPDFW ) const 
    {
        return col3( 0.f );
    }

    virtual bool isFinite( void ) const { return false; }
    virtual bool isDelta( void ) const { return true; }

private:
    
    Frame frame_;
    col3 intensity_;

};


/**
 * @class PointLight 
 *
 *
 */
class PointLight : public AbstractLight  
{
public:

    PointLight( const vec3& p ) : p_( p ) 
    {
    }

    vec3& position( void ) 
    {
        return p_;
    }

    const vec3 position( void ) const
    {
        return p_;
    }

    col3& intensity( void )
    {
        return intensity_;
    }

    const col3 intensity( void ) const
    {
        return intensity_;
    }

    virtual col3 illuminate( const vec3& x, const float xi0, const float xi1, vec3& w, float& d, float& pdfw, float *emissionPDFW = nullptr, float *cosAtLight = nullptr ) const
    {
        w = p_ - x;
        const float d2 = dot( p_ - x, p_ - x );
        d = std::sqrtf( d2 );
        pdfw = d2;
        w = w / d;
        if( cosAtLight != nullptr ) {
            *cosAtLight = 1.f;
        }
        if( emissionPDFW != nullptr ) {
            *emissionPDFW = inv4pi; // 1 / 4pi
        }
        return intensity_;
    }

    virtual col3 emit( const float xi0, const float xi1, const float xi2, const float xi3, vec3& x, vec3& w, float& emissionPDFW, float *pdfA, float *costhLight ) const 
    {
       x = p_;
       w = sampleUniformSphere( xi0, xi1, &emissionPDFW );
       if( pdfA != nullptr ) *pdfA = 1.f;
       if( costhLight != nullptr ) *costhLight = 1.f;
    }

    virtual col3 radiance( const vec3& w, const vec3& hitpoint, float *pdfA, float *emissionPDFW ) const 
    {
        return col3( 0.f );
    }

    virtual bool isFinite( void ) const { return true; }
    virtual bool isDelta( void ) const { return true; }

private:
    vec3 p_;
    col3 intensity_;

};



/**
 * @class BackgroundLight
 * @brief 
 *
 */
class BackgroundLight : public AbstractLight 
{
public:

    BackgroundLight( const std::string& filename, const float r = 135.f / 255.f, const float g = 206.f / 255.f, const float b = 250.f / 255.f, const float scale = 1.f ) 
    {
        background_.r = r;
        background_.g = g;
        background_.b = b;
        scale_ = scale;
        map_.reset( new EnvMap( filename, 0, 1.f ) );
    }

    virtual ~BackgroundLight()
    {}

    virtual col3 illuminate( const vec3& x, const float xi0, const float xi1, vec3& w, float& d, float& pdfw, float *emissionPDFW = nullptr, float *cosAtLight = nullptr ) const
    {
        col3 L;
        w = map_->sample( xi0, xi1, pdfw, &L );
        if( emissionPDFW != nullptr ) *emissionPDFW = pdfw * invpi * sphere_.invradius2_;
        if( cosAtLight != nullptr ) *cosAtLight = 1.f;
        L *= scale_;
        return L;
    }

    virtual col3 emit( const float xi0, const float xi1, const float xi2, const float xi3, vec3& x, vec3& w, float& emissionPDFW, float *pdfA, float *costhLight ) const 
    {
        float directPDF;
        col3 L;
        w = - map_->sample( xi0, xi1, directPDF, &L );
        L *= scale_;

        const vec3 xy = sampleConcentricDisc( xi2, xi3 );
        Frame frame;
        frame.set( w );
        
        x = sphere_.center_ + sphere_.radius_ * ( - w + frame.binormal() * xy.x + frame.tangent() * xy.y );
        emissionPDFW = directPDF * invpi * sphere_.invradius2_;
        if( pdfA != nullptr ) *pdfA = directPDF;
        if( costhLight != nullptr ) *costhLight = 1.f;
        return L;
    }

    virtual col3 radiance( const vec3& w, const vec3& hitpoint, float *pdfA, float *emissionPDFW ) const 
    {
        float directPDF;
        col3 L;
        L = scale_ * map_->lookup( w, &directPDF );
        const float positionPDF = invpi * sphere_.invradius2_;

        if( pdfA != nullptr ) *pdfA = directPDF;
        if( emissionPDFW != nullptr ) *emissionPDFW = directPDF * positionPDF;
        return L;
    }

    virtual bool isFinite( void ) const { return false; }
    virtual bool isDelta( void ) const { return false; }

    const std::unique_ptr< EnvMap >& envmap( void ) const 
    {
        return map_;
    }

private:
    float scale_;
    col3 background_;
    std::unique_ptr< EnvMap > map_;

};






#endif