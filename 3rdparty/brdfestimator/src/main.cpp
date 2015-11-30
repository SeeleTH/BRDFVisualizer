#include "main.h"


void init( void )
{
	config.load( "config.txt" );
	initScene();
	//initRender();
    initBRDFEstimator();
}

void init_distribution( void )
{
    Rng rng;
    float pdf;
    EnvMap map( Config::envmap_filename, 0.f, 1.f );
    const int width  = map.img->w();
    const int height = map.img->h();
    /*
    for( int i = 0; i < 10; i++ ) {
        map.distribution->sampleContinuous( rng.getFloat(), &pdf, &offset );
        const int h = offset / width;
        const int w = offset - h * width;
        const float L = luminance( map.img->operator()( w, h ) ) * sinf( ( h + 0.5f ) / ( float ) height * pi );
        std::cout << L / pdf << "\n";
        std::cout << L << " : " << pdf << "\n";
    }
    */
    for( int i = 0; i < 10; i++ ) {
        float uv[ 2 ];
        map.distribution->sampleContinuous( rng.getFloat(), rng.getFloat(), uv, &pdf );
        const int w = ( int ) floor( uv[ 0 ] * width  );
        const int h = ( int ) floor( uv[ 1 ] * height );
        const float L = luminance( map.img->operator()( w, h ) ) * sinf( ( h + 0.5f ) / ( float ) height * pi ) * 0.5f * invpi * invpi;
        std::cout << L / pdf << " : " << L << " : " << pdf << "\n";
    }
}



void initBRDFEstimator( void )
{
	estimator.reset( new BRDFEstimator( Config::nth, Config::nph, *scene ) );
    estimator->estimate( 10000 );
	estimator->write_result("output_result.bmp");
    estimator->visualize( *( scene->background_->envmap() ), "sphere.bmp" );
}

void initScene( void )
{
	scene.reset( new Scene() );
	scene->setCamera();
    scene->setBackground();
	scene->loadObj( Config::scene_object_path.c_str(), Config::scene_object_filename.c_str() );
}

void initRender( void )
{
	/*
	render.reset( new SimpleRender( *scene.get() ) );
	render->runIteration( 0 );
	render->writeBMP( "test.bmp" );
	*/

	pathtracer.reset( new PathTraceRender( *scene.get() ));

    
    auto start = std::chrono::system_clock::now();
    for( int i = 0; i < Config::nsample; i++ ) {
        //pathtracer->directIlluminationEnvMap( 100 );
        //pathtracer->direct_sample_illumination( 100 );
        pathtracer->runIterationEnvmap( 0 );
    }
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count() << "ms.\n";
    
/*    
    auto start = std::chrono::system_clock::now();
    pathtracer->direct_illumination_ground_truth( Config::nsample );
    auto end   = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast< std::chrono::milliseconds >( end - start ).count() << "ms\n";
*/  
    pathtracer->scale( 1.f / Config::nsample );
	pathtracer->writeBMP( Config::outputfilename.c_str() );

}


void initGL( int argc, char** argv )
{
    int main_window_id, render_menu, main_menu, camera_menu;
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_ALPHA );
	glutInitWindowSize( Config::windowsizex, Config::windowsizey );
	glutInitWindowPosition( 0, 0 );
	main_window_id = glutCreateWindow( "OpenGL Based Renderer" );

	render_menu = glutCreateMenu( menu_operation );
	glutAddMenuEntry( "Line", LINE );
	glutAddMenuEntry( "Display", DISPLAY );

	camera_menu = glutCreateMenu( menu_operation );
	glutAddMenuEntry( "Walk", WALK );
	glutAddMenuEntry( "Turn", TRN );
	glutAddMenuEntry( "Pan", PAN );
	glutAddMenuEntry( "Rotate", ROTATE );

	/*---CAMERA CONTROL SUB-MENU ----*/
	main_menu = glutCreateMenu( menu_operation );
	glutAddSubMenu( "CameraMode", camera_menu );
	glutAddSubMenu( "RenderMode", render_menu );
	glutAddMenuEntry( "Save", SAVE_BMP );
    glutAddMenuEntry( "Quit", QUIT );                         /* quit */

	glutAttachMenu( GLUT_RIGHT_BUTTON );
	glutMouseFunc( mouse_operation );
	glutMotionFunc( motion_operation );
	glutKeyboardFunc( key_operation );

	glutDisplayFunc( displayImage );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc ( GL_LEQUAL );

	//wglSwapIntervalEXT( 0 );
	glutMainLoop();
}


void quit( void )
{
    exit( 0 );
}

/**
 * @fn void key_operation( unsigned char key, int x, int y )
 * @brief 
 */
void key_operation( unsigned char key, int x, int y )
{
    switch( key ) {
        case 'w':
            imove_mode = WALK;
            break;
        
	    case 't':
            imove_mode = TRN;
            break;

	    case 'p':
            imove_mode = PAN;
            break;

	    case 'r':
		    imove_mode = ROTATE;
		    break;

	    case 's':
		    //rt->traceSVO();
		    //rt->traceOctree();
		    break;
      
	    case 'c':
		
		    break;

	    case 'P':
		    std::cout << scene->cameraptr_->toString() << "\n";
		    break;

	    case 'q':
		    quit();
		    break;

	    default:
		    break;
    }
}


void mouse_operation( int button, int state, int x, int y )
{
    switch( button ) {
		case GLUT_LEFT_BUTTON:
			if( state == GLUT_DOWN ) {
				idrag = 1;
				prev_x = x;
				prev_y = y;
			} else if( state == GLUT_UP ) {
				idrag = 0;
			}

			if( imove_mode == WALK ) {
			} else if( imove_mode == TRN ) {
				scene->cameraptr_->turn( 0.f, 0.f );
			} else if( imove_mode == PAN ) {
			} else if( imove_mode == ROTATE ) {
				scene->cameraptr_->rotate( 0.f, 0.f );
			}
			glutPostRedisplay();
		break;
    }
}


void menu_operation( int val )
{
    switch( val ) {
		case WALK:
		case TRN:
		case PAN:
		case ROTATE:
			imove_mode = val;
			break;

		case PRINT_VIEW:
            {
                const vec3 view = scene->cameraptr_->viewpoint();
                const vec3 refr = scene->cameraptr_->reference();
                const float vr  = scene->cameraptr_->vr();
                const float the = scene->cameraptr_->theta();
                const float phi = scene->cameraptr_->phi();
                std::cout << "view : " << view.x << " " << view.y << " " << view.z << "\n";
                std::cout << "ref  : " << refr.x << " " << refr.y << " " << refr.z << "\n";
                std::cout << "pola : " << vr     << " " << the    << " " << phi    << "\n";
        
        
            }
			break;

		case LINE:
		case DISPLAY:
			render_mode = val;
			break;

		case SAVE_BMP:
			
			break;

		case QUIT:
			quit();
			exit( 1 );
			break;
	
		default:

			break;
	}

	glutPostRedisplay();
}

void motion_operation( int x, int y ) {
    float dx, dy;
	
    dx = prev_x - x;
    dy = prev_y - y;
    dx *= ( float ) D_MOVE_RATIO;
    dy *= ( float ) D_MOVE_RATIO;

	if( idrag == 1 ) {
		switch( imove_mode ) {
		case WALK:
			scene->cameraptr_->walk( dy );
			break;

		case TRN:
			scene->cameraptr_->turn( D_THETA * dy, D_PHI * dx );
			break;

		case ROTATE:
			scene->cameraptr_->rotate( D_THETA * dy, D_PHI * dx );
			break;

		case PAN:
			scene->cameraptr_->pan( dx, dy );
			break;
		
		} 
		
		prev_x = x;
		prev_y = y;
	}
	glutPostRedisplay();
}

void set_camera( void )
{
    scene->cameraptr_->makeMatrix();
    const vec3 view = scene->cameraptr_->viewpoint();
    const vec3 refr = scene->cameraptr_->reference();
    const vec3 up   = scene->cameraptr_->v();
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( ( GLdouble ) scene->cameraptr_->fovy(), ( GLdouble ) Config::windowsizex / ( GLdouble ) Config::windowsizey, NEAR_Z, FAR_Z );
    
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glViewport( 0, 0, Config::windowsizex, Config::windowsizey );
    
	gluLookAt(( GLdouble ) view.x, ( GLdouble ) view.y, ( GLdouble ) view.z,
			  ( GLdouble ) refr.x, ( GLdouble ) refr.y, ( GLdouble ) refr.z,
			  ( GLdouble ) up.x  , ( GLdouble ) up.y  , ( GLdouble ) up.z  );

}



void displayImage( void )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
	set_camera();

	switch( render_mode ) {
	case LINE:

       display_scene_wireframe( *scene );

        //display_envmap_sphere( scene->background_->envmap() );

        //display_envmap( scene->background_->envmap() );
		break;

	default:
		break;
	}
	glutSwapBuffers();
}


int main( int argc, char** argv )
{
	init();
    initGL( argc, argv );

    return 0;
    
}