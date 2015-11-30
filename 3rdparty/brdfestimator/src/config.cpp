//
//  config.cpp
//  
//

#include "config.h"

int Config::windowsizex = 512;
int Config::windowsizey = 512;
int Config::nth = 32;
int Config::nph = 32;
std::string Config::scene_object_path;
std::string Config::scene_object_filename;
float Config::eye[ 3 ];
float Config::ref[ 3 ];
float Config::fovy;

float Config::EPS_RAY    = 1e-3f;
float Config::EPS_COSINE = 0.f;//1e-9f;
float Config::EPS_PHONG  = 0.f;//1e-9f;

int Config::nsample = 1;

std::string Config::outputfilename;
std::string Config::envmap_filename;
float Config::envmap_scale;
int Config::max_path_length;


void Config::load( const char* filename )
{
    std::ifstream input( filename, std::ios::in );
    std::string param;
    
    if( !input.is_open() ) {
        std::cout << "Cannot open file " << filename << "\n";
        exit( - 1 );
    }
    
    while( input >> param ) {
        
        if( !checkComment( param ) ) {
            
            if( param == std::string( "windowsizex" ) ) {
                input >> windowsizex;
                std::cout << "windowsizex : " << windowsizex << "\n";
            } else if( param == std::string( "windowsizey" ) ) {
                input >> windowsizey;
                std::cout << "windowsizey : " << windowsizey << "\n";
            } else if( param == std::string( "scene_object_filename" ) ) {
				input >> scene_object_filename;
				std::cout << "scene_object_filename : " << scene_object_filename << "\n";
			} else if( param == std::string( "scene_object_path" ) ) {
				input >> scene_object_path;
				std::cout << "scene_object_path : " << scene_object_path << "\n";
			} else if( param == std::string( "camera" ) ) {
				std::cout << param << "\n";
				input >> eye[ 0 ] >> eye[ 1 ] >> eye[ 2 ];
				input >> ref[ 0 ] >> ref[ 1 ] >> ref[ 2 ];
				input >> fovy;
				std::cout << "eye  : " << eye[ 0 ] << ", " << eye[ 1 ] << ", " << eye[ 2 ] << "\n";
				std::cout << "ref  : " << ref[ 0 ] << ", " << ref[ 1 ] << ", " << ref[ 2 ] << "\n";
				std::cout << "fovy : " << fovy << "\n";
			} else if( param == std::string( "nth") ) {
				input >> nth;
				std::cout << param << " : " << nth << "\n";
			} else if( param == std::string( "nph") ) {
				input >> nph;
				std::cout << param << " : " << nph << "\n";
			} else if( param == std::string( "nsample" ) ) {
				input >> nsample;
				std::cout << param << " : " << nsample << "\n";
			} else if( param == std::string( "outputfilename") ) {
                input >> outputfilename;
                std::cout << param << " : " << outputfilename << "\n";
            } else if( param == std::string( "envmap_filename" ) ) {
                input >> envmap_filename;
                std::cout << param << " : " << envmap_filename << "\n";
            } else if( param == std::string( "envmap_scale" ) ) {
                input >> envmap_scale;
                std::cout << param << " : " << envmap_scale << "\n";
            } else if( param == std::string( "max_path_length" ) ) {
                input >> max_path_length;
                std::cout << param << " : " << max_path_length << "\n";
            }
        }
    }
}
