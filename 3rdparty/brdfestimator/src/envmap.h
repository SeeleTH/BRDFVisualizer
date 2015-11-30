#ifndef _ENVMAP_H_
#define _ENVMAP_H_

#include <iostream>
#include "vec3.h"
#include "utility.h"
#include "image.h"
#include "distribution.h"

/**
 * @class EnvMap
 * @brief environment map represented by latitude longitude map
 */
class EnvMap {
public:

    EnvMap( const std::string filename, const float rotate, const float scale )
    {
        img.reset( new image );
        img->load_hdr( filename );
        convert_image_to_pdf();
    }

    /**
     * @fn vec3 sample( const float xi0, const float xi1, float& pdfw, col3* rad = nullptr ) const
     * @brief 
     */
    vec3 sample( const float xi0, const float xi1, float& pdfw, col3 *rad = nullptr ) const
    {
        float uv[ 2 ];
        float pdf;
        distribution->sampleContinuous( xi0, xi1, uv, &pdf );
        pdfw = 0.5f * invpi * invpi * pdf / sintheta( uv[ 1 ], img->h() );
        const vec3 w = latitude_longitude_to_vec( uv[ 0 ], uv[ 1 ] );
        if( rad != nullptr ) {
            *rad = radiance( uv[ 0 ], uv[ 1 ] );
        }
        return w;
    }

    /**
     * @fn col3 lookup( const vec3& w, float *pdfw = nullptr ) const 
     * @brief 
     */
    col3 lookup( const vec3& w, float *pdfw = nullptr ) const
    {
        vec3 uv = vec_to_latitude_longitude( w );
        col3 rad = radiance( uv.x, uv.y );
        if( pdfw != nullptr ) {
            *pdfw = 0.5f * invpi * invpi * distribution->pdf( uv.x, uv.y ) / sintheta( uv.y, img->h() );        
        }
        return rad;
    }

    /**
     * @fn vec3 latitude_longitude_to_vec( const float u, const float v ) const
     * @brief mapping ( u, v ) to direction vector
     */
    vec3 latitude_longitude_to_vec( const float u, const float v ) const
    {
        const float theta = v * pi;
        //const float phi   = 2.f * u * pi;
        const float phi = 2.f * u * pi - pi;
        return vec3( sinf( theta ) * sin( phi ), - cosf( theta ), sinf( theta ) * cos( phi ) );
    }
    
    /**
     * @fn vec3 vec_to_latitude_longitude( const vec3& w ) const 
     * @brief mapping direction vector to ( u, v )
     */
    vec3 vec_to_latitude_longitude( const vec3& w ) const 
    {
        const float theta = acosf( - w.y );
        const float phi   = atan2f( w.x, w.z );
        const float u = std::min( std::max( 0.f, 0.5f + 0.5f * phi * invpi ), 1.f );
        const float v = std::min( std::max( 0.f, theta * invpi ), 1.f );
        return vec3( u, v, 0.f );
    }


    col3 radiance( const float u, const float v ) const 
    {
        const int width  = img->w();
        const int height = img->h();
        const float x = u * width;
        const float y = v * height;
        int w[ 2 ], h[ 2 ];
        w[ 0 ] = std::min( std::max( 0, ( int ) floorf( x ) ), width  - 1 );
        h[ 0 ] = std::min( std::max( 0, ( int ) floorf( y ) ), height - 1 );
        w[ 1 ] = ( w[ 0 ] == width - 1  ) ? w[ 0 ] : w[ 0 ] + 1;
        h[ 1 ] = ( h[ 0 ] == height - 1 ) ? h[ 0 ] : h[ 0 ] + 1;
        const float tx = x - w[ 0 ];
        const float ty = y - h[ 0 ];
        return ( 1.f - tx ) * ( 1.f - ty ) * img->operator()( w[ 0 ], h[ 0 ] ) + tx * ( 1.f - ty ) * img->operator()( w[ 1 ], h[ 0 ] ) + ( 1.f - tx ) * ty * img->operator()( w[ 0 ], h[ 1 ] ) + tx * ty * img->operator()( w[ 1 ], h[ 1 ] );
    }

    /**
     * @fn void convert_image_to_pdf( void )
     *
     */
    /*
    void convert_image_to_pdf( void ) {
        const int width  = img->w();
        const int height = img->h();
        std::unique_ptr< float [] > p( new float [ width * height ] );
        int id = 0;
        for( int h = 0; h < height; h++ ) {
            const float v = ( h + 0.5f ) / ( float ) height;
            const float sinth = sinf( pi * v );
            for( int w = 0; w < width; w++ ) {
                p[ id ] = sinth * luminance( img->operator()( w, h ) );
                id++;
            }
        }    
        distribution.reset( new Distribution1D( p.get(), width * height ) );
    }
    */
    void convert_image_to_pdf( void )
    {
        const int width  = img->w();
        const int height = img->h();
        std::unique_ptr< float [] > p( new float [ width * height ] );
        for( int h = 0; h < height; h++ ) {
            const float v = ( h + 0.5f ) / ( float ) height;
            const float sinth = sinf( pi * v );
            const int offset = h * width;
            for( int w = 0; w < width; w++ ) {
                p[ offset + w ] = sinth * luminance( img->operator()( w, h ) );
            }
        }
        distribution.reset( new Distribution2D( p, width, height ) );
    }


    float sintheta( const float v, const float height ) const
    {
        float result;
        if( v < 1 ) {
            result = sinf( pi * ( float )( ( int )( v * height ) + 0.5f ) / ( float ) height );
        } else {
            result = sinf( pi * ( float )( ( height - 1 ) + 0.5f ) / ( float ) height );
        }
        assert( 0.f < result && result <= 1.f );
        return result;
    }


    std::unique_ptr< image > img;
    //std::unique_ptr< Distribution1D > distribution;
    std::unique_ptr< Distribution2D > distribution; 




};



#endif