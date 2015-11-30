/**
 * @file main.h
 *
 */


#include <iostream>
#include <gl/glew.h>
#include <gl/glut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>
#include "col3.h"
#include "vec3.h"
#include "config.h"
#include "render.h"
#include "light.h"
#include "scene.h"
#include "brdfestimator.h"
#include "image.h"
#include "envmap.h"
#include "distribution.h"
#include "display.h"


#pragma comment( lib, "glew64.lib" )

Config config;

std::unique_ptr< Scene > scene;
std::unique_ptr< SimpleRender > render;
std::unique_ptr< PathTraceRender > pathtracer;
std::unique_ptr< BRDFEstimator > estimator;


enum {
	LINE = 1,
	DISPLAY,
	TEST,
};

enum {
	OBJECT0 = 100,
};

enum {
	WALK = 1000,
	TRN,
	PAN,
	ROTATE,
};

enum {
	SAVE_BMP = 1100,
	PRINT_VIEW,
	QUIT,
};

const float FOVY = 50;
const float D_MOVE_RATIO = 0.1f;
const float D_THETA = M_PI/144.0f;
const float D_PHI = M_PI/144.0f;
const float D_MOVE_RATIO2 = 0.1f;
const float NEAR_Z = 0.1f;
const float FAR_Z =  1000.f;

int idrag;
int prev_x;
int prev_y;
int render_mode = LINE;
int main_window_id;
int imove_mode = WALK;

void key_operation( unsigned char key, int x, int y );
void mouse_operation( int button, int state, int x, int y );
void menu_operation( int val );
void motion_operation( int x, int y );
void displayImage( void );
void quit( void );


void init( void );
void initBRDFEstimator( void );
void initObjLoader( void );
void initScene( void );
void initRender( void );
void initGL( void );
void display( void );
void setCamera( void );

void init_distribution( void );
