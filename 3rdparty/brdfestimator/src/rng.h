//
//  rng.h
//  VRL
//
//  Created by 岩崎慶 on 2015/08/01.
//  Copyright (c) 2015年 Kei Iwasaki. All rights reserved.
//

#ifndef _RNG_H_
#define _RNG_H_

#include <random>

class Rng {

public:
    
    Rng( const int seed = 1234 ) : mRng( seed ) {
    }
    
    int getInt( void )
    {
        return mDistInt( mRng );
    }
    
    unsigned int getUint( void )
    {
        return mDistUint( mRng );
    }
    
    float getFloat( void )
    {
        return mDistFloat( mRng );
    }
    
private:
    
    std::mt19937_64 mRng;
    std::uniform_int_distribution< int > mDistInt;
    std::uniform_int_distribution< unsigned int > mDistUint;
    std::uniform_real_distribution< float > mDistFloat;
    
};



#endif
