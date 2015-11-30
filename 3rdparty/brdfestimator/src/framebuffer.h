//
//  framebuffer.h
//
//

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include <iostream>
#include <memory>
#include <fstream>
#include "col3.h"

class Framebuffer {
    
public:
    
    Framebuffer() : resX( 512 ), resY( 512 ) {
        init();
    }
    
    Framebuffer( const int _resX, const int _resY ) : resX( _resX ), resY( _resY ) {
        init();
    }
    
    inline const int x( void ) const {
        return resX;
    }
    
    inline const int y( void ) const {
        return resY;
    }
    
    inline void clear( void )
    {
        memset( pixel.get(), sizeof( col3 ) * resX * resY, 0 );
    }
    
    Framebuffer& operator*=( const float scale )
    {
        for( int i = 0, n = resX * resY; i < n; i++ ) {
            pixel[ i ] *= scale;
        }
        return *this;
    }
    
    const col3& operator[]( const int i ) const
    {
        assert( 0 <= i && i < resX * resY );
        return pixel[ i ];
    }
    
    col3& operator[]( const int i )
    {
        assert( 0 <= i && i < resX * resY );
        return pixel[ i ];
    }
    
    const col3& operator()( const int x, const int y ) const
    {
        assert( 0 <= x && x < resX && 0 <= y && y < resY );
        return pixel[ y * resX + x ];
    }
    
    col3& operator()( const int x, const int y )
    {
        assert( 0 <= x && x < resX && 0 <= y && y < resY );
        return pixel[ y * resX + x ];
    }
    
    void set( const int x, const int y, const col3& col )
    {
        assert( 0 <= x && x < resX && 0 <= y && y < resY );
        pixel[ y * resX + x ] = col;
	}

	void add( const int x, const int y, const col3& col )
	{
		assert( 0 <= x && x < resX && 0 <= y && y < resY );
        pixel[ y * resX + x ] += col;
	}
    
    
    void savePPM( const char* aFilename, const float aGamma = 1.f )
    {
        const float invGamma = 1.f / aGamma;
        std::ofstream ppm( aFilename );
        ppm << "P3\n";
        ppm << resX << " " << resY << "\n";
        ppm << "255\n";
        
        for( int y = 0; y < resY; y++ ) {
            for( int x = 0; x < resX; x++ ) {
                const int r = int( std::powf( pixel[ y * resX + x ].r, invGamma ) * 255 );
                const int g = int( std::powf( pixel[ y * resX + x ].g, invGamma ) * 255 );
                const int b = int( std::powf( pixel[ y * resX + x ].b, invGamma ) * 255 );
                
                ppm << std::min( 255, std::max( 0, r ) ) << " "
                    << std::min( 255, std::max( 0, g ) ) << " "
                    << std::min( 255, std::max( 0, b ) ) << " ";
            }
            ppm << "\n";
        }
        ppm.close();
    }
  
    struct BmpHeader
    {
		unsigned int mFileSize;        // Size of file in bytes
		unsigned int mReserved01;      // 2x 2 reserved bytes
        unsigned int mDataOffset;      // Offset in bytes where data can be found (54)
        
        unsigned int mHeaderSize;      // 40B
        int    mWidth;           // Width in pixels
        int    mHeight;          // Height in pixels
        
        short  mColorPlates;     // Must be 1
        short  mBitsPerPixel;    // We use 24bpp
        unsigned int mCompression;     // We use BI_RGB ~ 0, uncompressed
        unsigned int mImageSize;       // mWidth x mHeight x 3B
        unsigned int mHorizRes;        // Pixels per meter (75dpi ~ 2953ppm)
        unsigned int mVertRes;         // Pixels per meter (75dpi ~ 2953ppm)
        unsigned int mPaletteColors;   // Not using palette - 0
        unsigned int mImportantColors; // 0 - all are important
    };
    
    void saveBMP( const char *aFilename, const float aGamma = 1.f )
    {
        std::ofstream bmp(aFilename, std::ios::binary);
        if( bmp.fail() ) {
            std::cerr << "Cannot open file : " << aFilename << "\n";
            exit( -1 );
        }
        
        BmpHeader header;
        bmp.write("BM", 2);
        header.mFileSize   = ( unsigned int ) (sizeof(BmpHeader) + 2) + resX * resY * 3;
        header.mReserved01 = 0;
        header.mDataOffset = ( unsigned int ) (sizeof(BmpHeader) + 2);
        header.mHeaderSize = 40;
        header.mWidth      = resX;
        header.mHeight     = resY;
        header.mColorPlates     = 1;
        header.mBitsPerPixel    = 24;
        header.mCompression     = 0;
        header.mImageSize       = resX * resY * 3;
        header.mHorizRes        = 2953;
        header.mVertRes         = 2953;
        header.mPaletteColors   = 0;
        header.mImportantColors = 0;
        
        bmp.write((char*)&header, sizeof(header));
        
        const float invGamma = 1.f / aGamma;
        for( int y = 0; y < resY; y++ ) {
            for( int x = 0; x < resX; x++ ){
                // bmp is stored from bottom up
                //const col3 &rgbF = pixel[ ( resY - y - 1 ) * resX + x ];
                const col3& rgbF = pixel[ y * resX + x ];
				typedef unsigned char byte;
                float gammaBgr[ 3 ];
                gammaBgr[ 0 ] = std::powf( rgbF.b, invGamma ) * 255.f;
                gammaBgr[ 1 ] = std::powf( rgbF.g, invGamma ) * 255.f;
                gammaBgr[ 2 ] = std::powf( rgbF.r, invGamma ) * 255.f;
                
                byte bgrB[ 3 ];
                bgrB[ 0 ] = byte( std::min( 255.f, std::max( 0.f, gammaBgr[ 0 ] ) ) );
                bgrB[ 1 ] = byte( std::min( 255.f, std::max( 0.f, gammaBgr[ 1 ] ) ) );
                bgrB[ 2 ] = byte( std::min( 255.f, std::max( 0.f, gammaBgr[ 2 ] ) ) );
                
                bmp.write( ( char* ) &bgrB, sizeof( bgrB ) );
            }
        }
		bmp.close();
    }
    
    //////////////////////////////////////////////////////////////////////////
    // Saving HDR
    void saveHDR( const char* aFilename )
    {
        std::ofstream hdr( aFilename, std::ios::binary );
        
        hdr << "#?RADIANCE" << '\n';
        hdr << "# " << '\n';
        hdr << "FORMAT=32-bit_rle_rgbe" << '\n' << '\n';
        hdr << "-Y " << resY << " +X " << resX << '\n';
        
        for( int y = 0; y < resY; y++ ) {
            for( int x = 0; x < resX; x++ ) {
                typedef unsigned char byte;
                byte rgbe[ 4 ] = { 0, 0, 0, 0 };
                const col3 &rgbF = pixel[ y * resX + x ];
                float v = std::max( rgbF.r, std::max( rgbF.g, rgbF.b ) );
                if(v >= 1e-32f) {
                    int e;
                    v = float( frexp( v, &e ) * 256.f / v );
                    rgbe[ 0 ] = byte( rgbF.r * v);
                    rgbe[ 1 ] = byte( rgbF.g * v);
                    rgbe[ 2 ] = byte( rgbF.b * v);
                    rgbe[ 3 ] = byte(e + 128);
                }
                
                hdr.write( ( char* ) &rgbe[ 0 ], 4 );
            }
        }
		hdr.close();
    }

    
private:
    
    int resX, resY;
    std::unique_ptr< col3[] > pixel;
    
    inline void init( void )
    {
        pixel.reset( new col3 [ resX * resY ] );
        memset( pixel.get(), sizeof( col3 ) * resX * resY, 0 );
    }
    
};


#endif
