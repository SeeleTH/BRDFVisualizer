// ======================================================================== //
// Copyright 2009-2013 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <string.h>
//#include <strings.h>
#include "objLoader.h"
#include <stdio.h>
#include <fstream>

namespace ObjLoader {

static inline bool operator < (const Vec3i &a, const Vec3i &b) {

    if (a.i != b.i) return(a.i < b.i);
    if (a.j != b.j) return(a.j < b.j);
    if (a.k != b.k) return(a.k < b.k);
    return(false);

}

OBJLoader::OBJLoader( const std::string& _path, FILE *objFile ) : path( _path ) {

    /*! bookkeeping buffer */
    std::vector<std::vector<Vec3i> > faceGroup;

    /*! current mesh material name */
    char materialName[1024];  sprintf(materialName, "default");

    /*! iterate over lines of the file, flush the face group on EOF */
    for (char line[1024] ; fgets(line, 1024, objFile) ? true : (flushFaceGroup(faceGroup, materialName), false); ) {

        /*! acquire the first token on this line */
        char token[1024];  if (!sscanf(line, "%s", token)) continue;

        /*! face definition */
        if (!strcmp(token, "f")) { std::vector<Vec3i> face;  loadFace(line, face);  faceGroup.push_back(face); }

        /*! load material library */
        if (!strcmp(token, "mtllib")) {
            char libraryName[1024];  sscanf(line, "%*s %s", libraryName);  loadMTL( path + libraryName );
        }

        /*! use material */
        if (!strcmp(token, "usemtl")) { flushFaceGroup(faceGroup, materialName);  sscanf(line, "%*s %s", materialName); }

        /*! vertex coordinates */
        if (!strcmp(token, "v"))  { Vec3f value;  sscanf(line, "%*s %f %f %f", &value.x, &value.y, &value.z);  v.push_back(value); }

        /*! vertex normal */
        if (!strcmp(token, "vn")) { Vec3f value;  sscanf(line, "%*s %f %f %f", &value.x, &value.y, &value.z);  vn.push_back(value); }

        /*! texture coordinates */
        if (!strcmp(token, "vt")) { Vec2f value;  sscanf(line, "%*s %f %f", &value.x, &value.y);  vt.push_back(value); }

    }

}

uint32_t OBJLoader::appendVertex(const Vec3i &vertex, Mesh &mesh, std::map<Vec3i, uint32_t> &vertexMap) {

    /*! determine if we've seen this vertex before */
    const std::map<Vec3i, uint32_t>::iterator &entry = vertexMap.find(vertex);

    /*! two vertices match only if positions, normals, and texture coordinates match */
    if (entry != vertexMap.end()) return(entry->second);

    /*! this is a new vertex, store the indices */
    if (vertex.i >= 0) mesh.positions.push_back(v[vertex.i]);
    if (vertex.j >= 0) mesh.normals.push_back(vn[vertex.j]);
    if (vertex.k >= 0) mesh.texcoords.push_back(vt[vertex.k]);

    /*! map this vertex to a unique id */
    return(vertexMap[vertex] = int(mesh.positions.size()) - 1);

}

void OBJLoader::flushFaceGroup(std::vector<std::vector<Vec3i> > &faceGroup, const std::string materialName) {

    /*! temporary storage */
    std::map<Vec3i, uint32_t> vertexMap;

    /*! mesh that will be constructed from this face group */
    Mesh mesh;  mesh.material = materials[materialName];

    /*! construct a mesh for this face group */
    for (size_t face=0 ; face < faceGroup.size() ; face++) {

        /*! triangulate the face with a triangle fan */
        for (size_t i=0, j=1, k=2 ; k < faceGroup[face].size() ; j++, k++) {

            Vec3i triangle;
            triangle.i = appendVertex(faceGroup[face][i], mesh, vertexMap);
            triangle.j = appendVertex(faceGroup[face][j], mesh, vertexMap);
            triangle.k = appendVertex(faceGroup[face][k], mesh, vertexMap);
            mesh.triangles.push_back(triangle);

        }
    } 

    /* fix some materials */
    if (//mesh.material.name == "BeltStrap1" ||
        //mesh.material.name == "BeltBuckle1" || 
        //mesh.material.name == "Belt1" || 
        mesh.material.name == "JeansStraps1" ||
        //mesh.material.name == "JeansButton1" || 
        mesh.material.name == "Jeans1")// ||
        //mesh.material.name == "2_SkinHip") 
    {
      mesh.material.Ks.r = 0; mesh.material.Ks.g = 0; mesh.material.Ks.b = 0;
    }
    
    /* fix some texture coordinate issues */
    for (size_t i=0; i<mesh.texcoords.size(); i++)
      mesh.texcoords[i].y = 1.0f-mesh.texcoords[i].y;
    
    /*! append the mesh to the model */
    if (faceGroup.size()) model.push_back(mesh);  
    faceGroup.clear();
}

void OBJLoader::flushMaterial(Material &material, const std::string materialName) {

  if (strstr(material.name.c_str(),"Skin")) {
    material.Ks.r = 1.0f; material.Ks.g = 1.0f; material.Ks.b = 1.0f; 
  }

    /*! store the material */
    materials[materialName] = material;

    /*! clear the material */
    material = Material();

}

void OBJLoader::loadFace(char *line, std::vector<Vec3i> &face) {

    for (char *token = strtok(line, " f\t\r\n") ; token ; token = strtok(NULL, " \t\r\n")) {

        /*! vertex is defined as indices into position, normal, texture coordinate buffers */
        Vec3i vertex;  vertex.i = -1, vertex.j = -1, vertex.k = -1;

        /*! vertex has texture coordinates and a normal */
        if (sscanf(token, "%d/%d/%d", &vertex.i, &vertex.k, &vertex.j) == 3) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 :  v.size() + vertex.i);
            vertex.j = (vertex.j > 0) ? vertex.j - 1 : (vertex.j == 0 ? 0 : vn.size() + vertex.j);
            vertex.k = (vertex.k > 0) ? vertex.k - 1 : (vertex.k == 0 ? 0 : vt.size() + vertex.k);
            face.push_back(vertex);

        /*! vertex has a normal */
        } else if (sscanf(token, "%d//%d", &vertex.i, &vertex.j) == 2) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 :  v.size() + vertex.i);
            vertex.j = (vertex.j > 0) ? vertex.j - 1 : (vertex.j == 0 ? 0 : vn.size() + vertex.j);
            face.push_back(vertex);

        /*! vertex has texture coordinates */
        } else if (sscanf(token, "%d/%d", &vertex.i, &vertex.k) == 2) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 :  v.size() + vertex.i);
            vertex.k = (vertex.k > 0) ? vertex.k - 1 : (vertex.k == 0 ? 0 : vt.size() + vertex.k);
            face.push_back(vertex);

        /*! vertex has no texture coordinates or normal */
        } else if (sscanf(token, "%d", &vertex.i) == 1) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 : v.size() + vertex.i);
            face.push_back(vertex);

        }

    }

}

char* OBJLoader::parseString(const char* in, char* out)
{
  in+=strspn(in, " \t");
  if (in[0] == '\"') in++;
  strcpy(out,in);
  while (true) {
    size_t len = strlen(out);
    if (len == 0) return NULL;
    if (out[len-1] != '\"' && out[len-1] != ' ' && out[len-1] != '\r' && out[len-1] != '\n') break;
    out[len-1] = 0;
  }

  if (out[0] == '/') out++;
  const char* apath = "C:/Users/swoop/Documents/DAZ 3D/Studio/My Library/Runtime/";
  if (strstr(out,apath) == out) {
    out+=strlen(apath);
    if (out[0] == 'T') out[0] = 't';
  }
  return out;
}

void OBJLoader::loadMTL(const std::string libraryName) {

    /*! open the MTL file */
    FILE *mtlFile = fopen(libraryName.c_str(), "r" );
    if (!mtlFile) { printf("  ERROR:  unable to open %s\n", libraryName.c_str());  return; }

    /*! current material and name */
    Material material;  char materialName[1024];  sprintf(materialName, "default");

    /*! iterate over lines of the file, store the current material on EOF */
    for (char line[1024] ; fgets(line, 1024, mtlFile) ? true : (flushMaterial(material, materialName), false); ) {

        /*! acquire the first token on this line */
        char token[1024];  if (!sscanf(line, "%s", token)) continue;

        /*! ignore comments */
        if (!strcmp(token, "#")) continue;

        /*! opacity value */
        if (!strcmp(token, "d")) { sscanf(line, "%*s %f", &material.d); }

        /*! ambient color */
        if (!strcmp(token, "Ka")) { sscanf(line, "%*s %f %f %f", &material.Ka.r, &material.Ka.g, &material.Ka.b); }

        /*! diffuse color */
        if (!strcmp(token, "Kd")) { sscanf(line, "%*s %f %f %f", &material.Kd.r, &material.Kd.g, &material.Kd.b); }

        /*! specular color */
        if (!strcmp(token, "Ks")) { sscanf(line, "%*s %f %f %f", &material.Ks.r, &material.Ks.g, &material.Ks.b); }

        /*! opacity texture */
        if (!strcmp(token, "map_d")) { char textureName[1024];  material.map_d = parseString(line+5,textureName); }

        /*! ambient color texture */
        if (!strcmp(token, "map_Ka")) { char textureName[1024];  material.map_Ka = parseString(line+6,textureName); }

        /*! diffuse color texture */
        if (!strcmp(token, "map_Kd")) { char textureName[1024];  material.map_Kd = parseString(line+6,textureName); }

        /*! specular color texture */
        if (!strcmp(token, "map_Ks")) { char textureName[1024];  material.map_Ks = parseString(line+6,textureName); }

        /*! specular coefficient texture */
        if (!strcmp(token, "map_Ns")) { char textureName[1024];  material.map_Ns = parseString(line+6,textureName); }

        /*! bump map */
        if (!strcmp(token, "map_Bump")) { char textureName[1024];  material.map_Bump = parseString(line+8,textureName); }

        /*! new material delimiter */
        if (!strcmp(token, "newmtl")) { flushMaterial(material, materialName);  sscanf(line, "%*s %s", materialName); material.name = materialName; }

        /*! specular coefficient */
        if (!strcmp(token, "Ns")) { sscanf(line, "%*s %f", &material.Ns); }

    } fclose(mtlFile);

}


/**
 * @fn void loadAUX( const char* filename, Media& media, Materials& material )
 * @brief load aux file that contains material and media information
 */
void loadAUX( const char* filename, Media& media, Materials& material )
{
	std::ifstream input( filename, std::ios::in );
    std::string param, name;
	float val[ 4 ];
	int ival;
    AMaterial *mat = nullptr;
	Medium   *med = nullptr;


    if( !input.is_open() ) {
        std::cout << "Cannot open file " << filename << "\n";
        exit( - 1 );
    }
    
    while( input >> param ) {
	
		if( param == std::string( "medium" ) ) {
			input >> name;
			std::cout << "medium : " << name << "\n";
			med = findMedium( name, media );
		} else if( param == std::string( "absorption" ) ) {
			input >> val[ 0 ] >> val[ 1 ] >> val[ 2 ];
			std::cout << param << " : " << val[ 0 ] << ", " << val[ 1 ] << ", " << val[ 2 ] << "\n";
			if( med == nullptr ) {
				std::cerr << "Error : using absorption option without medium selection.\n";
				exit( -1 );
			}
			med->sigmaa[ 0 ] = val[ 0 ];
			med->sigmaa[ 1 ] = val[ 1 ];
			med->sigmaa[ 2 ] = val[ 2 ];
		} else if( param == std::string( "emission" ) ) {
			input >> val[ 0 ] >> val[ 1 ] >> val[ 2 ];
			std::cout << param << " : " << val[ 0 ] << ", " << val[ 1 ] << ", " << val[ 2 ] << "\n";
			if( med == nullptr ) {
				std::cerr << "Error : using emission option without medium selection.\n";
				exit( -1 );
			}
			med->sigmae[ 0 ] = val[ 0 ];
			med->sigmae[ 1 ] = val[ 1 ];
			med->sigmae[ 2 ] = val[ 2 ];
		} else if( param == std::string( "scattering" ) ) {
			input >> val[ 0 ] >> val[ 1 ] >> val[ 2 ];
			std::cout << param << " : " << val[ 0 ] << ", " << val[ 1 ] << ", " << val[ 2 ] << "\n";
			if( med == nullptr ) {
				std::cerr << "Error : using scattering option without medium selection.\n";
				exit( -1 );
			}
			med->sigmas[ 0 ] = val[ 0 ];
			med->sigmas[ 1 ] = val[ 1 ];
			med->sigmas[ 2 ] = val[ 2 ];
		} else if( param == std::string( "g" ) ) {
			input >> val[ 0 ];
			std::cout << param << " : " << val[ 0 ] << "\n";
			if( med == nullptr ) {
				std::cerr << "Error : using g option without medium selection.\n";
				exit( -1 );
			}
			med->g = val[ 0 ];
		} else if( param == std::string( "material" ) ) {
			input >> name;
			std::cout << "material : "  << name << "\n";
			mat = findMaterial( name, material );
		} else if( param == std::string( "mirror" ) ) {
			input >> val[ 0 ] >> val[ 1 ] >> val[ 2 ];
			std::cout << param << " : " << val[ 0 ] << ", " << val[ 1 ] << ", " << val[ 2 ] << "\n";
			if( mat == nullptr ) {
				std::cerr << "Error : using mirror option without material selection.\n";
				exit( -1 );
			}
			mat->mirror[ 0 ] = val[ 0 ];
			mat->mirror[ 1 ] = val[ 1 ];
			mat->mirror[ 2 ] = val[ 2 ];
		} else if( param == std::string( "ior" ) ) {
			input >> val[ 0 ];
			std::cout << param << " : " << val[ 0 ] << "\n";
			if( mat == nullptr ) {
				std::cerr << "Error : using ior option without material selection.\n";
				exit( -1 );
			}
			mat->ior = val[ 0 ];
		} else if ( param == std::string( "mediumId" ) ) {
			input >> name;
			std::cout << param << " : " << name << " ";
			ival = findMediumID( name, media );
			if( ival == -1 ) {
				std::cerr << "\n Error : medium " << name << "is not found.\n";
				exit( -1 );
			} else if( mat == nullptr ) {
				std::cerr << "\n Error : using mediumId option without material selection.\n";
			} else {
				mat->mediumID = ival;
				std::cout << mat->mediumID << "\n";
			}
		} else if( param == std::string( "priority" ) ) {
			input >> ival;
			std::cout << param << " : " << ival << "\n";
			if( mat == nullptr ) {
				std::cerr << "Error : using priority option without material selection.\n";
				exit( -1 );
			}
			mat->priority = ival;
		}
    }
}

/**
 * @fn Medium* findMedium( const std::string _name, Media& media )
 * @brief find medium whose name is _name from media
 */
Medium* findMedium( const std::string _name, Media& media )
{
	for( unsigned int i = 0, n = media.size(); i < n; i++ ) {
		if( media[ i ].name == _name ) {
			return &media[ i ];
		}
	}

	Medium medium;
	medium.name = _name;
	medium.sigmaa[ 0 ] = 0.0f;
	medium.sigmaa[ 1 ] = 0.0f;
	medium.sigmaa[ 2 ] = 0.0f;
	medium.sigmae[ 0 ] = 0.0f;
	medium.sigmae[ 1 ] = 0.0f;
	medium.sigmae[ 2 ] = 0.0f;
	medium.sigmas[ 0 ] = 0.5f;
	medium.sigmas[ 1 ] = 0.5f;
	medium.sigmas[ 2 ] = 0.5f;
	medium.g = 0.f;
	medium.probability = - 1.f;
	media.push_back( medium );
	return &( * media.rbegin() );
}

AMaterial* findMaterial( const std::string _name, Materials& material )
{
	for( unsigned int i = 0, n = material.size(); i < n; i++ ) {
		if( material[ i ].name == _name ) {
			return &material[ i ];
		}

		AMaterial mat;
		mat.name = _name;
		mat.diffuse[ 0 ] = 0.0f;
		mat.diffuse[ 1 ] = 0.0f;
		mat.diffuse[ 2 ] = 0.0f;
		mat.ambient[ 0 ] = 0.0f;
		mat.ambient[ 1 ] = 0.0f;
		mat.ambient[ 2 ] = 0.0f;
		mat.specular[ 0 ] = 0.0f;
		mat.specular[ 1 ] = 0.0f;
		mat.specular[ 2 ] = 0.0f;
		mat.emissive[ 0 ] = 0.0f;
		mat.emissive[ 1 ] = 0.0f;
		mat.emissive[ 2 ] = 0.0f;
		mat.mirror[ 0 ] = 0.0f;
		mat.mirror[ 1 ] = 0.0f;
		mat.mirror[ 2 ] = 0.0f;
		mat.ior = - 1.0f;
		mat.isEmissive = false;
		mat.mediumID = - 1;
		mat.priority = - 1;
		mat.shininess = 0.0f;
		material.push_back( mat );
		return &( *material.rbegin() );
	}
}

int findMediumID( const std::string _name, Media& media )
{
	for( unsigned int i = 0, n = media.size(); i < n; i++ ) {
		if( media[ i ].name == _name ) return i;
	}
	//
	return - 1;
}

}