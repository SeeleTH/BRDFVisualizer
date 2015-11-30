#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Config {
    
public:
    
    Config() : comment( false ) {
    }
    
    ~Config() {
    }
    
    void load( const char* filename );
    
    static int windowsizex;
    static int windowsizey;
	static float eye[ 3 ];
	static float ref[ 3 ];
	static float fovy;
	
	static std::string scene_object_path;
	static std::string scene_object_filename;
    static std::string envmap_filename;
    static float envmap_scale;

    static std::string outputfilename;
    
	static int nth;
	static int nph;

	static int nsample;
	static int max_path_length;
	static float EPS_COSINE;
	static float EPS_PHONG;
	static float EPS_RAY;

private:
    
    bool comment;
    
    bool checkComment( const std::string& param ) {
        if( !comment ) {
            if( param == std::string( "//" ) )
                return true;
            else if( param == std::string( "/*" ) )
                comment = true;
        } else if( param == std::string( "*/" ) ) {
            comment = false;
        }
        return comment;
    }
    
};


#endif