//
//  scene.h
//  

#ifndef _SCENE_H_
#define _SCENE_H_

#include <iostream>
#include <vector>
#include <memory>
#include "config.h"
#include "camera.h"
#include "ray.h"
#include "objLoader.h"
#include "material.h"
#include "light.h"
#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>
#include <embree2/rtcore_scene.h>

class Scene {
    struct Vertex {
        float x, y, z, a;
    };
    
    struct Triangle {
        int v0, v1, v2;
    };
    
public:
    
    Scene()
    {
        rtcInit();
        scene_ = rtcNewScene( RTC_SCENE_STATIC, RTC_INTERSECT1 );
        RTCError err = rtcGetError();
        if( err != RTC_NO_ERROR ) {
            std::cerr << err << "\n";
            exit( -1 );
        }
    }
    
    ~Scene()
    {
        rtcDeleteScene( scene_ );
        rtcExit();
    }
    
	/*
    void setLight( void )
    {
        lightsize_ = 1;
        const float L = 1.f;
        const float Y = 1.f;
        const vec3 p0( - L / 2.f, Y, - L / 2.f );
        const vec3 d0( L, 0.f, 0.f );
        const vec3 d1( 0.f, 0.f, L );
        std::unique_ptr< SquareLight > ptr( new SquareLight( p0, d0, d1 ) );
        lightptr_.push_back( ( AbstractLight* ) ptr.get() );
    }
	*/

    void setBackground( void )
    {
        background_.reset( new BackgroundLight( Config::envmap_filename, 0.53f, 0.81f, 0.98f, Config::envmap_scale ) );
        background_->sphere_.center_ = vec3( 0.f, 0.f, 0.f );
        background_->sphere_.radius_ = 1e3f;
        background_->sphere_.invradius2_ = 1.f / ( background_->sphere_.radius_ * background_->sphere_.radius_ );
    }
    
    void setCamera( void )
    {
        const vec3 eye( Config::eye[ 0 ], Config::eye[ 1 ], Config::eye[ 2 ] );
        const vec3 ref( Config::ref[ 0 ], Config::ref[ 1 ], Config::ref[ 2 ] );
        const float fovy = Config::fovy;
        cameraptr_.reset( new Camera( eye, ref, fovy, Config::windowsizex, Config::windowsizey ) );
    }
    
    void loadObj( const char* path, const char* filename )
    {
		mesh_ = ObjLoader::loadOBJ( path, filename );
        boundingbox( mesh_, bbmin, bbmax );
        std::cout << "bounding box : " << bbmin << " : " << bbmax << "\n";
        setRTCGeometry( mesh_ );
    }
    
    bool intersect( const Ray& ray, Isect &isect ) const;

	bool occlusion( const Ray& ray ) const;
    
    
	const std::vector< ObjLoader::Mesh >& mesh( void ) const
	{
		return mesh_;
	}

	inline void setMaterial( const Isect& isect, Material& mat ) const 
	{
		assert( isect.geomID_ >= 0 && isect.primID_ >= 0 );
		ObjLoader::Material _mat = mesh_[ isect.geomID_ ].material;
		mat.diffuse = col3( _mat.Kd.r, _mat.Kd.g, _mat.Kd.b );
		mat.glossy  = col3( _mat.Ks.r, _mat.Ks.g, _mat.Ks.b );
		mat.glossy.a = _mat.Ns;
	}

    int lightsize_;
    std::unique_ptr< Camera > cameraptr_;
    std::unique_ptr< BackgroundLight > background_;
    vec3 bbmin, bbmax;
    
private:

    RTCScene scene_;
	std::vector< ObjLoader::Mesh > mesh_;
    void setRTCGeometry( const std::vector< ObjLoader::Mesh >& _mesh );

	
    inline void set_isect( const RTCRay& ray, Isect& isect ) const
    {
        isect.geomID_         = ray.geomID;
        isect.primID_         = ray.primID;
        isect.dist_           = ray.tfar;
        isect.normal_         = normalize( vec3( ray.Ng[ 0 ], ray.Ng[ 1 ], ray.Ng[ 2 ] ) );
        isect.shadingnormal_  = calculate_shading_normal( ray );
        isect.uv_.x           = ray.u;
        isect.uv_.y           = ray.v;
    }

    inline void set_isect( const RTCRay& ray, Isect& isect, Material& mat ) const
    {
        isect.geomID_         = ray.geomID;
        isect.primID_         = ray.primID;
        isect.dist_           = ray.tfar;
        isect.normal_         = normalize( vec3( ray.Ng[ 0 ], ray.Ng[ 1 ], ray.Ng[ 2 ] ) );
        isect.shadingnormal_  = calculate_shading_normal( ray );
        isect.uv_.x           = ray.u;
        isect.uv_.y           = ray.v;
        ObjLoader::Material _mat = mesh_[ isect.geomID_ ].material;
        mat.diffuse = col3( _mat.Kd.r, _mat.Kd.g, _mat.Kd.b );
        mat.glossy  = col3( _mat.Ks.r, _mat.Ks.g, _mat.Ks.b );
        mat.glossy.a = _mat.Ns;
    }

    /**
     * @fn void boundingbox( const std::vector< ObjLoader::Mesh >& mesh, vec3& min, vec3& max )
     * @brief calculate bounding box
     */
    void boundingbox( const std::vector< ObjLoader::Mesh >& mesh, vec3& min, vec3& max )
    {
        const auto size = mesh.size();
        const auto _max = std::numeric_limits< float >::max();
        const auto _min = - _max;
        min.x = _max;
        min.y = _max;
        min.z = _max;
        max.x = _min;
        max.y = _min;
        max.z = _min;
    
        for( int i = 0; i < size; i++ ) {
            const auto fsize = mesh[ i ].positions.size();
            for( auto j = 0; j < fsize; j++ ) {
                auto v = mesh[ i ].positions[ j ];
                if( v.x < min.x ) min.x = v.x;
                if( v.y < min.y ) min.y = v.y;
                if( v.z < min.z ) min.z = v.z;
                if( v.x > max.x ) max.x = v.x;
                if( v.y > max.y ) max.y = v.y;
                if( v.z > max.z ) max.z = v.z;
            }
        }
    }


	//inline void setIsect( const RTCRay& ray, Isect& isect ) const 
	//{
	//	isect.geomID_ = ray.geomID;
	//	isect.primID_ = ray.primID;
	//	isect.dist_ = ray.tfar;
	//	isect.normal_ = calculateNormal( ray );
	//	isect.uv_.x = ray.u;
	//	isect.uv_.y = ray.v;
	//}
	//
	//inline void setIsect( const RTCRay& ray, Isect& isect, Material& mat ) const
	//{
	//	isect.geomID_ = ray.geomID;
	//	isect.primID_ = ray.primID;
	//	isect.dist_ = ray.tfar;
	//	isect.normal_ = calculateNormal( ray );
	//	isect.uv_.x = ray.u;
	//	isect.uv_.y = ray.v;
	//	ObjLoader::Material _mat = mesh_[ isect.geomID_ ].material;
	//	mat.diffuse = col3( _mat.Kd.r, _mat.Kd.g, _mat.Kd.b );
	//	mat.glossy  = col3( _mat.Ks.r, _mat.Ks.g, _mat.Ks.g );
	//	mat.glossy.a = _mat.Ns;
	//}
	
	//inline vec3 calculateNormal( const RTCRay& ray ) const 
	//{
	//	const int geomID = ray.geomID;
	//	const int primID = ray.primID;
	//	const int i      = mesh_[ geomID ].triangles[ primID ].i;
	//	const int j      = mesh_[ geomID ].triangles[ primID ].j;
	//	const int k      = mesh_[ geomID ].triangles[ primID ].k;
	//	const float u    = ray.u;
	//	const float v    = ray.v;
	//	const ObjLoader::Vec3f n0 = mesh_[ geomID ].normals[ i ];
	//	const ObjLoader::Vec3f n1 = mesh_[ geomID ].normals[ j ];
	//	const ObjLoader::Vec3f n2 = mesh_[ geomID ].normals[ k ];
	//	const vec3 nn0( n0.x, n0.y, n0.z );
	//	const vec3 nn1( n1.x, n1.y, n1.z );
	//	const vec3 nn2( n2.x, n2.y, n2.z );
	//	return normalize( ( 1.f - u - v ) * nn0 + u * nn1 + v * nn2 );
	//}
	
    inline vec3 calculate_shading_normal( const RTCRay& ray ) const
    {
        const int geomID = ray.geomID;
		const int primID = ray.primID;
		const int i      = mesh_[ geomID ].triangles[ primID ].i;
		const int j      = mesh_[ geomID ].triangles[ primID ].j;
		const int k      = mesh_[ geomID ].triangles[ primID ].k;
		const float u    = ray.u;
		const float v    = ray.v;
		const ObjLoader::Vec3f n0 = mesh_[ geomID ].normals[ i ];
		const ObjLoader::Vec3f n1 = mesh_[ geomID ].normals[ j ];
		const ObjLoader::Vec3f n2 = mesh_[ geomID ].normals[ k ];
		const vec3 nn0( n0.x, n0.y, n0.z );
		const vec3 nn1( n1.x, n1.y, n1.z );
		const vec3 nn2( n2.x, n2.y, n2.z );
		return normalize( ( 1.f - u - v ) * nn0 + u * nn1 + v * nn2 );
    
    }
    
};



#endif
