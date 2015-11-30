#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <gl/glew.h>
#include <gl/glut.h>

#include "scene.h"


void display_scene_wireframe( const Scene& scene )
{
    const std::vector< ObjLoader::Mesh >& mesh = scene.mesh();
    const int mesh_size = mesh.size();

    for( int i = 0; i < mesh_size; i++ ) {
        const int face_size = mesh[ i ].triangles.size();
        const float r = mesh[ i ].material.Kd.r;
        const float g = mesh[ i ].material.Kd.g;
        const float b = mesh[ i ].material.Kd.b;
        glColor3f( r, g, b );
        for( int j = 0; j < face_size; j++ ) {
            const ObjLoader::Vec3f v0 = mesh[ i ].positions[ mesh[ i ].triangles[ j ].i ];
            const ObjLoader::Vec3f v1 = mesh[ i ].positions[ mesh[ i ].triangles[ j ].j ];
            const ObjLoader::Vec3f v2 = mesh[ i ].positions[ mesh[ i ].triangles[ j ].k ];
            glBegin( GL_LINE_LOOP );
            glVertex3f( v0.x, v0.y, v0.z );
            glVertex3f( v1.x, v1.y, v1.z );
            glVertex3f( v2.x, v2.y, v2.z );
            glEnd();

        }
    }
}

/**
 * @fn void display_envmap( const std::unique_ptr< EnvMap >& map )
 * @brief
 */
void display_envmap( const std::unique_ptr< EnvMap >& map )
{
    const int nth = 128;
    const int nph = 256;
    const float r = 100.f;

    glPointSize( 10.f );
    glBegin( GL_POINTS );
    for( int i = 0; i < nth; i++ ) {
        const float v = ( i + 0.5f ) / ( float ) nth;
        const float sth = sinf( pi * v );
        const float cth = cosf( pi * v );

        for( int j = 0; j < nph; j++ ) {
            const float u = ( j + 0.5f ) / ( float ) nph;
            const float sph = sinf( 2.f * pi * u );
            const float cph = cosf( 2.f * pi * u );
         
            //const col3 rad = tonemap( map->radiance( u, v ) );
            //glColor3f( rad.r, rad.g, rad.b );
            const vec3 p( sth * sph, - cth, sth * cph );
            const col3 rad = tonemap( map->lookup( p ) );
            glColor3f( rad.r, rad.g, rad.b );
            glVertex3f( r * p.x, r * p.y, r * p.z );

        }
    }
    glEnd();
}

void display_envmap_sphere( const std::unique_ptr< EnvMap >& map )
{
    const int nth = 1024;
    const int nph = 2048;

    glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( 40.0, ( GLdouble ) Config::windowsizex / ( GLdouble ) Config::windowsizey, 1e-5, 10.0 );
    
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glViewport( 0, 0, Config::windowsizex, Config::windowsizey );    
	gluLookAt( 0.0, 0.0, 0.9, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0 );

    glBegin( GL_POINTS );
    for( int i = 0; i < nth; i++ ) {
        const float v = ( i + 0.5f ) / ( float ) nth;
        const float sth = sinf( pi * v );
        const float cth = cosf( pi * v );

        for( int j = 0; j < nph; j++ ) {
            const float u = ( j + 0.5f ) / ( float ) nph;
            const float sph = sinf( 2.f * pi * u );
            const float cph = cosf( 2.f * pi * u );
         
            //const col3 rad = tonemap( map->radiance( u, v ) );
            //glColor3f( rad.r, rad.g, rad.b );
            const vec3 p( sth * sph, - cth, sth * cph );
            const col3 rad = tonemap( map->lookup( p ) );
            glColor3f( rad.r, rad.g, rad.b );
            glVertex3f( p.x, p.y, p.z );

        }
    }
    glEnd();
}




#endif