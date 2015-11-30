//
//  ray.h
//
//  Created by 岩崎慶 on 2015/08/09.
//  Copyright (c) 2015年 Kei Iwasaki. All rights reserved.
//

#ifndef _RAY_H_
#define _RAY_H_

#include <iostream>
#include "vec3.h"
#include "col3.h"


namespace VCL {



/**
 * @class Ray
 *
 */
class Ray {
    
public:
    
    Ray() : o( 0.f, 0.f, 0.f ), d( 0.f, 0.f, 1.f )
    {}
    
    Ray( const vec3& _o, const vec3& _d ) : o( _o ), d( _d )
    {}
    
    inline vec3 target( float t = 1.f )
    {
        return o + t * d;
    }
    
    std::string toString( void ) const
    {
        std::stringstream oss;
        oss << "Ray [ " << o.toString()  << d.toString() << " ]\n";
        return oss.str();
    }
    
    vec3 o, d;
    
};
    
}

typedef VCL::Ray Ray;


/**
 * @struct Isect
 * @brief information of intersection point
 */
struct Isect {
    
public:
    
    Isect( float _maxdist = std::numeric_limits< float >::max() )
    {
        dist_ = _maxdist;
        matID_ = -1;
        medID_ = -1;
        lightID_ = -1;
        enter_ = false;
        geomID_ = - 1;
		primID_ = -1;
    }
    
    float dist_;         // Distance to the closest intersection on a ray (serves as ray.tmax).
	int geomID_;         // ID of geometry
	int primID_;         // ID of primitive in the geometry with ID geomID_
	int matID_;          // ID of intersected material, -1 indicates scattering event inside medium.
    int medID_;          // ID of interacting medium or medium behind the hit surface, -1 means that crossing the hit surface does not affect medium.
    int lightID_;        // ID of intersected light, -1 means none.
    vec3 normal_;        // Normal at the intersection.
    vec3 shadingnormal_; //shading normal at the intersection.
    bool enter_;         // Whether the ray enters geometry at this intersection (cosine of its direction and the normal is negative).
    vec3 uv_;            // Barycentric coordinates of a hitpoint.
    
    bool isInMedium( void ) const
    {
        return ( matID_ < 0 );
    }
    
    bool isOnSurface( void ) const
    {
        return ( matID_ >= 0 );
    }
    
    bool isValid( void ) const
    {
        const bool validDist = ( dist_ > 0.f );
        const bool validMedia = ( matID_ < 0 && medID_ >= 0 );
        const bool validSurface = ( matID_ >= 0 && ( dot( normal_, normal_ ) > 1e-20 ) );
        const bool validLight = ( matID_ >= 0 || lightID_ < 0 );
        
        return ( validDist && ( validMedia || validSurface ) && validLight );
    }
    
    bool operator<( const Isect& right )
    {
        return ( dist_ < right.dist_ );
    }
    
    
    
};



#endif
