#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "col3.h"

class Material
{
public:

	Material() {
	}

	col3 diffuse;
	col3 glossy;

	//to be implemented
	col3 specular;
	float ior;

private:
};


#endif