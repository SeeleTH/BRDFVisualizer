//
//  frame.h
//  VRL
//
//  Created by 岩崎慶 on 2015/08/06.
//  Copyright (c) 2015年 Kei Iwasaki. All rights reserved.
//

#ifndef _FRAME_H_
#define _FRAME_H_

#include <iostream>
#include <cmath>


class Frame
{
public:
    
    Frame() : X( 1.f, 0.f, 0.f ), Y( 0.f, 1.f, 0.f ), Z( 0.f, 0.f, 1.f )
    {}
    
    Frame( const vec3& aX, const vec3& aY, const vec3& aZ ) : X( aX ), Y( aY ), Z( aZ )
    {}
    
    void set( const vec3& z )
    {
        vec3 tmpZ = Z = normalize( z );
        vec3 tmpX = ( std::fabsf( tmpZ.x ) > 0.99f ) ? vec3( 0.f, 1.f, 0.f ) : vec3( 1.f, 0.f, 0.f );
        Y = normalize( cross( tmpZ, tmpX ) );
        X = cross( Y, Z );
    }
    
    inline vec3 toWorld( const vec3& a ) const
    {
        return ( a.x * X + a.y * Y + a.z * Z );
    }
    
    inline vec3 toLocal( const vec3& a ) const
    {
        return vec3( dot( a, X ), dot( a, Y ), dot( a, Z ) );
    }
    
    const vec3 binormal()   const { return X; }
    const vec3 tangent()    const { return Y; }
    const vec3 normal()     const { return Z; }


    
    
public:
    vec3 X, Y, Z;
    
};


#endif
