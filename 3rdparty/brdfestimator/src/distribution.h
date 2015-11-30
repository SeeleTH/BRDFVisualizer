#ifndef _DISTRIBUTION_H_
#define _DISTRIBUTION_H_

#include <algorithm>
#include <vector>
#include <memory>
#include <cassert>

/**
 * @struct Distribution1D
 * @brief piecewise constant function
 */
struct Distribution1D
{
public:

    Distribution1D( float  *f, int n ) : count( n )
    {
        func.reset( new float [ count ] );
        cdf.reset( new float [ count + 1 ] );

        for( int i = 0; i < count; i++ ) {
            func[ i ] = f[ i ];
        }
        cdf[ 0 ] = 0.f;
        for( int i = 1; i < count + 1; i++ ) {
            cdf[ i ] = cdf[ i - 1 ] + func[ i - 1 ] / ( float ) n;
        }
        funcInt = cdf[ count ];
        if( funcInt == 0.f ) {
            for( int i = 0; i < count + 1; i++ ) {
                cdf[ i ] = ( float ) i / ( float ) count;
            }
        } else {
            for( int i = 0; i < count + 1; i++ ) {
                cdf[ i ] /= funcInt;
            }
        }
    
    }

    ~Distribution1D()
    {}

    /**
     * @fn float sampleContinuous( float u, float *pdf, int *off = nullptr ) const
     * @brief 
     * 
     */
    float sampleContinuous( float u, float *pdf, int *off = nullptr ) const
    {
        float *ptr = std::upper_bound( cdf.get(), cdf.get() + count + 1, u );
        int offset = std::min( std::max( 0, int( ptr - cdf.get() - 1 ) ), count - 1 );
        if( off != nullptr ) *off = offset;
        assert( offset < count );
        assert( cdf[ offset ] <= u && ( u < cdf[ offset + 1 ] || u == 1 ) );
        if( cdf[ offset ] == cdf[ offset + 1 ] ) {
            assert( u == 1.f );
            do{ offset--;}
            while( cdf[ offset ] == cdf[ offset + 1 ] && offset > 0 );
            assert( cdf[ offset ] != cdf[ offset + 1 ] );
        }
        const float du = ( u - cdf[ offset ] ) / ( cdf[ offset + 1 ] - cdf[ offset ] );
        assert( !_isnan( du ) );
        if( pdf != nullptr ) *pdf = func[ offset ] / funcInt;
        assert( func[ offset ] > 0 );
        return ( offset + du ) / ( float ) count;
    }

    int sampleDiscrete( float u, float *pdf ) const
    {
        float *ptr = std::upper_bound( cdf.get(), cdf.get() + count + 1, u );
        int offset = std::max( 0, int( ptr - cdf.get() - 1 ) );
        assert( offset < count );
        assert( cdf[ offset ] <= u && u < cdf[ offset + 1 ] );
        if( pdf != nullptr ) *pdf = func[ offset ] / ( funcInt * count );
        return offset;
    }


    std::unique_ptr< float [] > func;
    std::unique_ptr< float [] > cdf;
    float funcInt;
    int count;

};

/***
 * @class Distribution2D
 *
 */
struct Distribution2D {

public:

    Distribution2D( const std::unique_ptr< float [] >& func, int nu, int nv )
    {
        conditional.reserve( nv );
        for( int i = 0; i < nv; i++ ) {
           //conditional.emplace_back( new Distribution1D( &func.get()[ i * nv ], nu ) );
            conditional.emplace_back( new Distribution1D( &func[ i * nu ], nu ) );
        }
        std::vector< float > marginalfunc;
        marginalfunc.reserve( nv );
        for( int i = 0; i < nv; i++ ) {
            marginalfunc.push_back( conditional[ i ]->funcInt );
        }
        marginal.reset( new Distribution1D( &marginalfunc[ 0 ], nv ) );
    }

    ~Distribution2D() {
    }

    void sampleContinuous( float u0, float u1, float uv[ 2 ], float *pdf ) const {
        float pdfs[ 2 ];
        int v;
        uv[ 1 ] = marginal->sampleContinuous( u1, &pdfs[ 1 ], &v );
        uv[ 0 ] = conditional[ v ]->sampleContinuous( u0, &pdfs[ 0 ] );
        *pdf = pdfs[ 0 ] * pdfs[ 1 ];
    }

    float pdf( float u, float v ) const {
        int iu = std::min( std::max( 0, ( int ) ( u * conditional[ 0 ]->count ) ), conditional[ 0 ]->count - 1 );
        int iv = std::min( std::max( 0, ( int ) ( v * marginal->count ) ), marginal->count - 1 );
        if( conditional[ iv ]->funcInt * marginal->funcInt == 0.f ) return 0.f;
        return ( conditional[ iv ]->func[ iu ] * marginal->func[ iv ] ) / ( conditional[ iv ]->funcInt * marginal->funcInt );
    }


    std::vector< std::unique_ptr< Distribution1D > > conditional;
    std::unique_ptr< Distribution1D > marginal;


};




#endif