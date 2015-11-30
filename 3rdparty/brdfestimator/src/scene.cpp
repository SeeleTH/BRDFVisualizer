//
//  scene.cpp
//

#include <iostream>
#include "scene.h"

SceneSphere AbstractLight::sphere_;

/** 
 * @fn bool Scene::intersect( const Ray& ray, Isect& isect ) const 
 * @brief ray-scene geometry(triangle) intersection test
 */
bool Scene::intersect( const Ray& ray, Isect& isect ) const
{
    RTCRay _ray;
	_ray.org[ 0 ] = ray.o.x;// + Config::EPS_RAY * ray.d.x;
	_ray.org[ 1 ] = ray.o.y;// + Config::EPS_RAY * ray.d.y;
	_ray.org[ 2 ] = ray.o.z;// + Config::EPS_RAY * ray.d.z;
	_ray.dir[ 0 ] = ray.d.x;
	_ray.dir[ 1 ] = ray.d.y;
	_ray.dir[ 2 ] = ray.d.z;
	_ray.tnear = Config::EPS_RAY;
	_ray.tfar  = std::numeric_limits< float >::max();
    _ray.geomID = RTC_INVALID_GEOMETRY_ID;
	_ray.primID = RTC_INVALID_GEOMETRY_ID;
	_ray.instID = RTC_INVALID_GEOMETRY_ID;


	rtcIntersect( scene_, _ray );
    if( _ray.geomID != RTC_INVALID_GEOMETRY_ID ) {
		//setIsect( _ray, isect );
		set_isect( _ray, isect );
        return true;
	} else {
		return false;
	}
}

/**
 * @fn bool Scene::occlusion( const Ray& ray ) const
 * @brief occlusion test : return true if ray intersects something in the scene and return false otherwise
 */
bool Scene::occlusion( const Ray& ray ) const
{
	RTCRay _ray;
	_ray.org[ 0 ] = ray.o.x;
	_ray.org[ 1 ] = ray.o.y;
	_ray.org[ 2 ] = ray.o.z;
	_ray.dir[ 0 ] = ray.d.x;
	_ray.dir[ 1 ] = ray.d.y;
	_ray.dir[ 2 ] = ray.d.z;
	_ray.tnear = Config::EPS_RAY;
	_ray.tfar  = std::numeric_limits< float >::max();
    _ray.geomID = RTC_INVALID_GEOMETRY_ID;
	_ray.primID = RTC_INVALID_GEOMETRY_ID;
	_ray.instID = RTC_INVALID_GEOMETRY_ID;

	rtcOccluded( scene_, _ray );
	if( _ray.geomID == 0 ) {	//ray intersects something
		return true;
	} else {
		return false;
	}
}



/**
 * @fn void Scene::setRTCGeometry( const std::vector< ObjLoader::Mesh >& _mesh )
 * @brief convert obj to Embree RTC geometry
 */
void Scene::setRTCGeometry( const std::vector< ObjLoader::Mesh >& _mesh )
{
    const size_t geometry_size = _mesh.size();
    std::vector< unsigned int > geometryID;
    
    geometryID.reserve( geometry_size );
    
    for( size_t i = 0; i < geometry_size; i++ ) {
        const unsigned int tsize = static_cast< unsigned int >( _mesh[ i ].triangles.size() );
        const unsigned int vsize = static_cast< unsigned int >( _mesh[ i ].positions.size() );
        
        unsigned int geomID = rtcNewTriangleMesh( scene_, RTC_GEOMETRY_STATIC, tsize, vsize );
        geometryID.push_back( geomID );
        
        Vertex *vertex = ( Vertex* ) rtcMapBuffer( scene_, geomID, RTC_VERTEX_BUFFER );
        
        for( unsigned int j = 0; j < vsize; j++ ) {
            vertex[ j ].x = _mesh[ i ].positions[ j ].x;
            vertex[ j ].y = _mesh[ i ].positions[ j ].y;
            vertex[ j ].z = _mesh[ i ].positions[ j ].z;
        }
        
        rtcUnmapBuffer( scene_, geomID, RTC_VERTEX_BUFFER );
        
        Triangle *triangle = ( Triangle* ) rtcMapBuffer( scene_, geomID, RTC_INDEX_BUFFER );
        for( unsigned int j = 0; j < tsize; j++ ) {
            triangle[ j ].v0 = _mesh[ i ].triangles[ j ].i;
            triangle[ j ].v1 = _mesh[ i ].triangles[ j ].j;
            triangle[ j ].v2 = _mesh[ i ].triangles[ j ].k;
        }
        
        rtcUnmapBuffer( scene_, geomID, RTC_INDEX_BUFFER );
        
    }
    rtcCommit( scene_ );
}