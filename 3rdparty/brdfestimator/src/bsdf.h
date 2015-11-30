#ifndef _BSDF_H_
#define _BSDF_H_

#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include "scene.h"


class BSDF {

public:

	enum Events
	{
		kNONE = 0,
		kDIFFUSE = 1,
		kPHONG = 2,
		kREFLECT = 4,
		kREFRACT = 8,
		kSCATTER = 16,
		kSPECULAR = ( kREFLECT | kREFRACT ),
		kNONSPECULAR = ( kDIFFUSE | kPHONG | kSCATTER ),
		kALL = ( kSPECULAR | kNONSPECULAR )
	};


	BSDF() {
	}

	//BSDF( const Ray& ray, const Isect& isect, const Scene& scene, const vec3& dir, )




};


#endif