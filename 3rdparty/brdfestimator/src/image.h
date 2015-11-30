#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <memory>
#include "col3.h"

class image {

public:

    image() : h_( 0 ), w_( 0 ) {
    }

    image( const int _h, const int _w ) : h_( _h ), w_( _w ) {
        pixel_.reset( new col3 [ h_ * w_ ] );
    }

    ~image() {
    }

    const col3& operator() ( const int x, const int y ) const {
        return pixel_[ y * w_ + x ];
    }

    col3& operator() ( const int x, const int y ) {
        return pixel_[ y * w_ + x ];
    }

    const int w( void ) const {
        return w_;
    }

    const int h( void ) const {
        return h_;
    }

    void load_hdr( const std::string& filename )
    {
        std::ifstream ifs( filename, std::ios::binary );
        if( ifs.fail() ) {
            std::cerr << "Cannot open file " << filename << "\n";
            exit( -1 );
        }
    
        std::string line;

        while( std::getline( ifs, line ), !line.empty() ) {
            std::stringstream ss( line );
            std::string buf; ss >> buf;
            if( buf.compare( 0, 2, "#?") == 0 ) {
                if( buf.compare( 2, 8, "RADIANCE" ) != 0 ) {
                    std::cerr << "load_hdr: " << filename << " is not a hdr image.\n";
                    exit( -1 );
                }
            }
            if( buf.compare( 0, 5, "GAMMA" ) == 0 ) {
            }
        }

        std::getline( ifs, line );
        std::stringstream ss( line );
        std::string buf1, buf2;
        int val1, val2;
        ss >> buf1 >> val1;
        ss >> buf2 >> val2;

        const int width  = ( buf1[ 1 ] == 'X' ) ? val1 : val2;
        const int height = ( buf1[ 1 ] == 'Y' ) ? val1 : val2;
        const int n = val1;
        const int m = val2;
        const int step1 = ( buf1[ 0 ] == '+' ) ? 1 : - 1;
        const int step2 = ( buf2[ 0 ] == '+' ) ? 1 : - 1;
        const int init_idx1 = ( buf1[ 0 ] == '+' ) ? 0 : n - 1;
        const int init_idx2 = ( buf2[ 0 ] == '+' ) ? 0 : m - 1;

        w_ = width;
        h_ = height;
        pixel_.reset( new col3 [ w_ * h_ ] );

        std::vector< unsigned char > pixels( 4 * width * height );
        std::vector< unsigned char > rgbe[] = {
            std::vector< unsigned char >( m ),
            std::vector< unsigned char >( m ),
            std::vector< unsigned char >( m ),
            std::vector< unsigned char >( m ),
        };

        for( int i = 0, idx1 = init_idx1; i < n; i++, idx1 += step1 ) {
            char magic[ 4  ];
            ifs.read( magic, 4 );

            if( ( magic[ 0 ] != 0x02 ) || ( magic[ 1 ] != 0x02 ) ) {
                std::cerr << "load_hdr: " << filename << "is invalid.\n";
                exit( -1 );
            }

            for( int j = 0; j < 4; j++ ) {
                for( int k = 0; k < m; ) {
                    const int flag = ifs.get();
                    if( flag > 128 ) {
                        const int col = ifs.get();
                        const int count = flag - 128;
                        memset( ( char* ) &rgbe[ j ][ k ], col, count );
                        k += count;
                    } else {
                        const int count = flag;
                        ifs.read( ( char* ) &rgbe[ j ][ k ], count );
                        k += count;
                    }
                }
            }

            for( int j = 0, idx2 = init_idx2; j < m; j++, idx2 += step2 ) {
                const unsigned char r = rgbe[ 0 ][ j ];
                const unsigned char g = rgbe[ 1 ][ j ];
                const unsigned char b = rgbe[ 2 ][ j ];
                const unsigned char e = rgbe[ 3 ][ j ];

                if( e ) {
                    int tmp_val = ( 127 + ( e - ( 128 + 8 ) ) ) << 23;
                    const float val = ( float& ) tmp_val;
                    pixel_[ idx2 + m * idx1 ].r = r * val;
                    pixel_[ idx2 + m * idx1 ].g = g * val;
                    pixel_[ idx2 + m * idx1 ].b = b * val;
                    pixel_[ idx2 + m * idx1 ].a = 0.f;
                } else {
                    pixel_[ idx2 + m * idx1 ].r = 0.f;
                    pixel_[ idx2 + m * idx1 ].g = 0.f;
                    pixel_[ idx2 + m * idx1 ].b = 0.f;
                    pixel_[ idx2 + m * idx1 ].a = 0.f;
                }
            }
        }

    }

private:

    int h_, w_; 
    std::unique_ptr< col3 [] > pixel_;


};


#endif