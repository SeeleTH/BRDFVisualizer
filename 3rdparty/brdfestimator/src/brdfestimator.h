#ifndef _BRDF_ESTIMATOR_H_
#define _BRDF_ESTIMATOR_H_


#include <iostream>
#include <memory>
#include <vector>
#include "vec3.h"
#include "col3.h"
#include "ray.h"
#include "camera.h"
#include "rng.h"
#include "frame.h"
#include "utility.h"
#include "objLoader.h"
#include "brdf.h"
#include "scene.h"
#include "framebuffer.h"
#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>
#include <embree2/rtcore_scene.h>

/***
 * @class BRDFEstimator
 * @brief estimate BRDF from small-scale geometry using path tracing
 *
 */
class BRDFEstimator {

    /**
     * @struct 
     *
     */
    struct DirectionalLight {

        DirectionalLight( const float _theta, const float _dth, const float _phi, const float _dph ) : theta( _theta ), dth( _dth ), phi( _phi ), dph( _dph ) {
            omega = dph * ( cosf( theta ) - cosf( theta + dth ) );
            pdf = 1.f / omega;
            L.r = 1.f;
            L.g = 1.f;
            L.b = 1.f;
        }

        ~DirectionalLight() {
        }

        //sample light and return direction wi (from recieving point to light source) and its pdf
        col3 illuminate( const float xi0, const float xi1, vec3& wi, float& PDF ) const
        {
            const float th = theta + dth * xi0;
            const float ph = phi   + dph * xi1;
            wi.x = sinf( th ) * cosf( ph );
            wi.y = cosf( th );
            wi.z = sinf( th ) * sinf( ph );
            PDF = pdf;
            return L;
        }

        //return radiance for ray randomly hitting the light
        col3 radiance( const vec3& wi ) const
        {
            float th = acosf( std::min( std::max( wi.y, 0.f ), 1.f )  );
            float ph = atan2f( wi.z, wi.x ); if( ph < 0.f ) ph += 2.f * pi;
            if( theta <= th && th < theta + dth && phi <= ph && ph < phi + dph ) {
                return L;
            } else {
                return col3( 0.f );
            }
        }

        float solid_angle( void ) const
        {
            return omega;
        }

        float theta;
        float phi;
        float dth;
        float dph;
        float pdf;
        float omega; //solid angle
        col3 L;
    };


public:

	BRDFEstimator( const int _nth, const int _nph, const Scene& scene ) : nth_( _nth ), nph_( _nph ), scene_( scene ), mesh_( scene.mesh() )
	{
		init();
	}

	~BRDFEstimator() 
	{
	}

    //randomly sample a point on a triangle based on its area
	void sample_point( const float xi0, const float xi1, const float xi2, vec3& x, vec3& normal, int& geomID, int& triangleID ) const;

    //estimate projected area of micro-geometry towards wo using monte carlo integration
    float calculate_projected_area( const int nsample, const vec3& wo );

    void estimate( const int N = 1024 );

    void visualize( const EnvMap& map, const char* filename ) const;

	void write_result(const char* filename) const;

private:

	int nth_;
	int nph_;
	int ntriangle_;   //number of triangles
	float totalArea_; //area of small scale geometry
	std::unique_ptr< col3 [] > fr_;
	std::unique_ptr< float [] > area_; //area of each triangle
	std::unique_ptr< float [] > cdf_;  //cumulative distribution function (CDF) to sample triangle
    std::unique_ptr< float [] > omega_;
	std::unique_ptr< int [] > geomID_;
	std::unique_ptr< int [] > triID_;
	
    const Scene& scene_;
    const std::vector< ObjLoader::Mesh >& mesh_;
    vec3 center_;
    float radius_;

	Rng rng_;

    void init( void );
    
    void init_boundary_sphere( void );
    
    //calculate throughput (energy) using path tracing
    col3 calculate_throughput( const Ray& ray, const DirectionalLight& light, bool& hit );

    void calculate_omega( void );

    inline bool inside( const vec3 w, const int i, const int j )
    {
        if( w.y < 0.f ) return false; //w is under hemisphere
        const float theta = acosf( w.y );
        const float phi   = atan2f( w.z, w.x );
        const int thi = ( int ) floor( theta * 2.f * nth_ / pi - 0.5f );
        const int phj = ( int ) floor( phi * nph_ / ( 2.f * pi ) - 0.5f );
        return ( ( thi == i ) && ( phj == j ) );
    }

    /**
     * @fn col3 illuminate( const float xi0, const float xi1, const int i, const int j, vec3& wi, float& pdf )
     * @brief sample light that spans [ theta_i, theta_i+1 ] x [ phi_j, phi_j+1  ]
     */
    col3 illuminate( const float xi0, const float xi1, const int i, const int j, vec3& wi, float& pdf )
    {
        const col3 L( 1.f );
        const float theta = ( i + xi0 + 0.5f ) / ( float ) nth_ * pi / 2.f;
        const float phi   = ( j + xi1 + 0.5f ) / ( float ) nph_ * 2.f * pi;
        wi.x = std::sinf( theta ) * std::cosf( phi );
        wi.y = std::cosf( theta );
        wi.z = std::sinf( theta ) * std::sinf( phi );
        pdf = 1.f / omega_[ i * nph_ + j ];
        return L;
    }

    float mis( const float pdf ) const
    {
        return pdf;
    }

    float mis2( const float pdf0, const float pdf1 ) const
    {
        return mis( pdf0 ) / ( mis( pdf0 ) + mis( pdf1 ) );
    }

};




#endif