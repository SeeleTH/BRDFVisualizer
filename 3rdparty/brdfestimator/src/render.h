//
//  render.h
//
//

#ifndef __RENDER_H_
#define __RENDER_H_

#include <iostream>
#include <atomic>
#include <thread>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range2d.h>

#include "scene.h"
#include "framebuffer.h"
#include "config.h"
#include "rng.h"
#include "brdf.h"

class AbstractRender {

public:
    
	AbstractRender( const Scene& _scene ) : scene_( _scene )
    {
        buffer_.reset( new Framebuffer( Config::windowsizex, Config::windowsizey ) );
    }
    
    virtual void runIteration( int iteration ) = 0;
    
	void writeBMP( const char* filename ) 
	{
		buffer_->saveBMP( filename );
	}

	void writeHDR( const char* filename )
	{
		buffer_->saveHDR( filename );
	}

	void scale( const float a ) {
		( *buffer_ ) *= a;
	}

protected:
    const Scene& scene_;
    std::unique_ptr< Framebuffer > buffer_;
};



class SimpleRender : public AbstractRender {
    
public:
    
    SimpleRender( const Scene& _scene ) : AbstractRender( _scene )
    {
        //std::cout << buffer_->x() << " : " << buffer_->y() << "\n";
    }
    
    void runIteration( int iteration )
    {
        const int resX = buffer_->x();
        const int resY = buffer_->y();
        const Camera *camera = scene_.cameraptr_.get();
        Isect isect;
		col3  col;
        
        for( int x = 0; x < resX; x++ ) {
            for( int y = 0; y < resY; y++ ) {
                const Ray ray = camera->generateRay( x, y );
                
                if( scene_.intersect( ray, isect ) ) {
					col.r = 0.5f * ( 1.f + isect.normal_.x );
					col.g = 0.5f * ( 1.f + isect.normal_.y );
					col.b = 0.5f * ( 1.f + isect.normal_.z );
                    buffer_->set( x, y, col );
                }
            }
        }
    }
};


/***
 * @fn class PathTraceRender : public AbstractRender 
 * @brief path tracing
 */
class PathTraceRender : public AbstractRender {


public:

	PathTraceRender( const Scene& _scene, const int _seed = 1234 ) : AbstractRender( _scene ), rng_( _seed ), maxPathLength_( Config::max_path_length )
	{
		intensity_.r = 3.f;
		intensity_.g = 3.f;
		intensity_.b = 3.f;

		const float lth = pi / 6.f;
		const float lph = pi / 2.f;
		wi.x = sinf( lth ) * cosf( lph );
		wi.y = cosf( lth );
		wi.z = sinf( lth ) * sinf( lph );
	}

	void runIteration( int iteration )
	{
		const int resX = buffer_->x();
		const int resY = buffer_->y();
		const int nPixel = resX * resY;

        auto f = [&]( const tbb::blocked_range2d< int, int >& range ) {
           for( int y = range.rows().begin(); y < range.rows().end(); y++ ) {
               for( int x = range.cols().begin(); x < range.cols().end(); x++ ) {
                    Ray ray = scene_.cameraptr_->generateRay( x + rng_.getFloat(), y + rng_.getFloat() );
			        Isect isect;
			        col3 col;
			        int pathLength = 1;
			        col3 pathWeight( 1.f );

			        for(;; pathLength++ ) {
				        if( !scene_.intersect( ray, isect ) ) {	 //ray does not hit object
					        // do nothing
					        break;
				        }

				        if( pathLength >= maxPathLength_ ) break;

				        const vec3 hitpoint = ray.o + isect.dist_ * ray.d;
				        BRDF brdf( ray, isect, scene_ );

				        //next event estimation
				        const col3 Li = intensity_;
				        float ctho, brdfpdf;
				        col3 fr = brdf.evaluate( wi, ctho, &brdfpdf );
				        col3 C = Li * fr * ctho;
				        //shadow ray
				        Ray shadowray;
				        shadowray.o = hitpoint;
				        shadowray.d = wi;
                     
				        if( dot( wi, isect.shadingnormal_ ) > 0.f ) {
					        if( !scene_.occlusion( shadowray ) ) {
						        col += pathWeight * C;
					        }
				        }

				        //continue random walk;
				        const vec3 rnd3( rng_.getFloat(), rng_.getFloat(), rng_.getFloat() );
				        float pdf, cth;
				        vec3 wo;
				        fr = brdf.sample( rnd3, wo, pdf, cth);
				        pathWeight *= ( cth / pdf ) * fr;

				        ray.o = hitpoint;
				        ray.d = wo;
                    }
                   buffer_->add( x, y, col );
               }
           }
        };			    
        tbb::parallel_for( tbb::blocked_range2d< int, int >( 0, resY, 0, resX ), f );
    }

    /**
     * @fn void runIterationAtomic( int iteration )
     * @brief multithreading using atomic operation
     */
    void runIterationAtomic( int iteration )
    {
        const int resX = buffer_->x();
		const int resY = buffer_->y();
		const int nPixel = resX * resY;

        std::atomic_size_t atomic_index( 0 );
        auto f = [ & ]()
        {
            for( size_t index = atomic_index.fetch_add( 1 ); index < nPixel; index = atomic_index.fetch_add( 1 ) ) {
                const int y = index / resX;
                const int x = index - y * resX;
                Ray ray = scene_.cameraptr_->generateRay( x + rng_.getFloat(), y + rng_.getFloat() );
			    Isect isect;
			    col3 col;
			    int pathLength = 1;
			    col3 pathWeight( 1.f );

			    for(;; pathLength++ ) {
				    if( !scene_.intersect( ray, isect ) ) {	 //ray does not hit object
					    // do nothing
					    break;
				    }

				    if( pathLength >= maxPathLength_ ) break;

				    const vec3 hitpoint = ray.o + isect.dist_ * ray.d;
				    BRDF brdf( ray, isect, scene_ );

				    //next event estimation
				    const col3 Li = intensity_;
				    float ctho, brdfpdf;
				    col3 fr = brdf.evaluate( wi, ctho, &brdfpdf );
				    col3 C = Li * fr * ctho;
				    //shadow ray
				    Ray shadowray;
				    shadowray.o = hitpoint;
				    shadowray.d = wi;
                     
				    if( dot( wi, isect.normal_ ) > 0.f ) {
					    if( !scene_.occlusion( shadowray ) ) {
						    col += pathWeight * C;
					    }
				    }

				    //continue random walk;
				    const vec3 rnd3( rng_.getFloat(), rng_.getFloat(), rng_.getFloat() );
				    float pdf, cth;
				    vec3 wo;
				    fr = brdf.sample( rnd3, wo, pdf, cth);
				    pathWeight *= ( cth / pdf ) * fr;

				    ray.o = hitpoint;
				    ray.d = wo;
                }
                buffer_->add( x, y, col );
            }
        };

        std::vector< std::thread > ths( std::thread::hardware_concurrency() - 1 );
        for( size_t i = 0; i < ths.size(); i++ ) {
            ths[ i ] = std::thread( std::ref( f ) );
        }
        for( size_t i = 0; i < ths.size(); i++ ) {
            ths[ i ].join();
        }
    }

    /**
     * @fn void directIlluminationEnvMap( const int iteration )
     * @brief
     */
    void directIlluminationEnvMap( const int iteration )
    {
       const int resX = buffer_->x();
       const int resY = buffer_->y();
       const int nPixel = resX * resY;

        auto f = [&]( const tbb::blocked_range2d< int, int >& range ) {
           for( int y = range.rows().begin(); y < range.rows().end(); y++ ) {
               for( int x = range.cols().begin(); x < range.cols().end(); x++ ) {
                    Ray ray = scene_.cameraptr_->generateRay( x + rng_.getFloat(), y + rng_.getFloat() );
			        Isect isect;
			        col3 col;
                    float d, pdf;
                    vec3 w;
                    float ctho;
			        
                    if( !scene_.intersect( ray, isect ) ) {
                        col = scene_.background_->radiance( ray.d, ray.o, nullptr, nullptr );
                    } else {
                        const vec3 hitpoint = ray.o + isect.dist_ * ray.d;
                        BRDF brdf( ray, isect, scene_ );
                        col.r = col.g = col.b = 0.f;
                        for( int j = 0; j < iteration; j++ ) {
                            const vec3 rn = vec3( rng_.getFloat(), rng_.getFloat(), rng_.getFloat() );
                            const col3 fr = brdf.sample( rn, w, pdf, ctho );
                            const col3 Li = scene_.background_->radiance( w, hitpoint, nullptr, nullptr );
                            Ray shadowray;
                            shadowray.o = hitpoint;
                            shadowray.d = w;
                            if( !scene_.occlusion( shadowray ) && ctho > Config::EPS_COSINE ) {
                                col += Li * fr * ctho / pdf;
                                if( std::isnan( col.r ) ) {
                                    std::cout << col.r << "\n";
                                    const col3 fr_ = brdf.sample( rn, w, pdf, ctho );
                                }
                            }
                        }
                        col /= ( float ) iteration;
                    }
                    buffer_->add( x, y, col );
               }
           }
        };			    
        tbb::parallel_for( tbb::blocked_range2d< int, int >( 0, resY, 0, resX ), f );
    }

    /**
     * @fn void direct_illumination_ground_truth( const int iteration ) 
     * @brief 
     */
    void direct_illumination_ground_truth( const int iteration )
    {
        const int resX = buffer_->x();
        const int resY = buffer_->y();
        const int width  = scene_.background_->envmap()->img->w();
        const int height = scene_.background_->envmap()->img->h();
        const std::unique_ptr< EnvMap >& map = scene_.background_->envmap();
        const float du = 1.f / ( float ) width;
        const float dv = 1.f / ( float ) height;
        const float scale = du * dv * 2.f * pi * pi;

        auto f = [&]( const tbb::blocked_range2d< int, int >& range ) {
            for( int y = range.rows().begin(); y < range.rows().end(); y++ ) {
                for( int x = range.cols().begin(); x < range.cols().end(); x++ ) {
                    for( int i = 0; i < iteration; i++ ) {
                        Ray ray = scene_.cameraptr_->generateRay( x + rng_.getFloat(), y + rng_.getFloat() );
                        Isect isect;
                        col3 col;
                        float ctho;

                        if( !scene_.intersect( ray, isect ) ) {
                            col = scene_.background_->radiance( ray.d, ray.o, nullptr, nullptr );
                        } else {
                            const vec3 hitpoint = ray.target( isect.dist_ );
                            BRDF brdf( ray, isect, scene_ );
                            col.r = 0.f; col.g = 0.f; col.b = 0.f;
                            for( int h = 0; h < height; h++ ) {
                                for( int w = 0; w < width; w++ ) {
                                    const float u = ( w + 0.5f ) / ( float ) width;
                                    const float v = ( h + 0.5f ) / ( float ) height;
                                    const float sth = sinf( v * pi );
                                    //const col3 Li = map->img->operator()( w, h ) * sth;
                                    const vec3 wi = map->latitude_longitude_to_vec( u, v );
                                    const col3 Li = map->lookup( wi ) * sth;
                                    const col3 fr = brdf.evaluate( wi, ctho );
                                    const float cth = dot( wi, isect.normal_ ); 
                                    //if( fabs( ctho - cth ) > 1e-5f ) std::cout << ctho << " : " << cth << "\n";
                                    if( cth > 0.f ) {
                                        Ray shadowray;
                                        shadowray.o = hitpoint;
                                        shadowray.d = wi;
                                        if( !scene_.occlusion( shadowray ) ) {
                                            col += Li * fr * cth;
                                        }
                                    }
                                }
                            }
                            col *= scale * Config::envmap_scale;
                        }
                        buffer_->add( x, y, col );
                    }
                }
            }
        };
        tbb::parallel_for( tbb::blocked_range2d< int, int >( 0, resY, 0, resX ), f );
    }

    /**
     * @fn void direct_sample_illumination( const int iteration ) 
     * @brief 
     */
    void direct_sample_illumination( const int iteration )
    {
        const int resX = buffer_->x();
        const int resY = buffer_->y();
        const int width  = scene_.background_->envmap()->img->w();
        const int height = scene_.background_->envmap()->img->h();
        const std::unique_ptr< EnvMap >& map = scene_.background_->envmap();
        const std::unique_ptr< BackgroundLight >& light = scene_.background_;

        auto f = [&]( const tbb::blocked_range2d< int, int >& range ) {
            for( int y = range.rows().begin(); y < range.rows().end(); y++ ) {
                for( int x = range.cols().begin(); x < range.cols().end(); x++ ) {
                    Ray ray = scene_.cameraptr_->generateRay( x + rng_.getFloat(), y + rng_.getFloat() );
                    Isect isect;
                    col3 col;
                    vec3 w;
                    float ctho, pdfw, d;
                    if( !scene_.intersect( ray, isect ) ) {
                        col = scene_.background_->radiance( ray.d, ray.o, nullptr, nullptr );
                    } else {
                        const vec3 hitpoint = ray.target( isect.dist_ );
                        BRDF brdf( ray, isect, scene_ );
                        col.r = col.g = col.b = 0.f;
                        for( int i = 0; i < iteration; i++ ) {
                            const col3 Li = light->illuminate( hitpoint, rng_.getFloat(), rng_.getFloat(), w, d, pdfw );
                            ctho = dot( w, isect.shadingnormal_ );
                            if( ctho > 0.f ) {
                                Ray shadowray;
                                shadowray.o = hitpoint;
                                shadowray.d = w;
                                if( !scene_.occlusion( shadowray ) ) {
                                    const col3 fr = brdf.evaluate( w, ctho );
                                    col += Li * fr * ctho / pdfw;
                                }
                            }
                        }
                        col /= ( float ) iteration;
                    }
                    buffer_->add( x, y, col );
                }
            }
        };
        tbb::parallel_for( tbb::blocked_range2d< int, int >( 0, resY, 0, resX ), f );
    }

    /**
     * @fn void runIterationEnvmap( int iteration )
     * 
     */
    void runIterationEnvmap( int iteration )
	{
		const int resX = buffer_->x();
		const int resY = buffer_->y();
		const int nPixel = resX * resY;

        auto f = [&]( const tbb::blocked_range2d< int, int >& range ) {
           for( int y = range.rows().begin(); y < range.rows().end(); y++ ) {
               for( int x = range.cols().begin(); x < range.cols().end(); x++ ) {
                    Ray ray = scene_.cameraptr_->generateRay( x + rng_.getFloat(), y + rng_.getFloat() );
			        Isect isect;
			        col3 col;
			        int pathLength = 1;
                    float lastpdf = 1.f;
			        col3 pathWeight( 1.f );

			        for(;; pathLength++ ) {
				        if( !scene_.intersect( ray, isect ) ) {	 //ray does not hit object
                            float pdf;
					        const col3 Li = scene_.background_->radiance( ray.d, ray.o, &pdf, nullptr );
                            if( pathLength == 1 ) {
                                col = Li;
                            } else {
                                float misweight = 1.f;
                                misweight = mis_weight( lastpdf, pdf );
                                col += pathWeight * Li * misweight;
                            }
					        break;
				        }

				        if( pathLength >= maxPathLength_ ) break;

				        const vec3 hitpoint = ray.o + isect.dist_ * ray.d;
				        BRDF brdf( ray, isect, scene_ );

				        //next event estimation
                        float ctho, lpdf, fpdf, d;
                        vec3 wi;
				        const col3 Li = scene_.background_->illuminate( hitpoint, rng_.getFloat(), rng_.getFloat(), wi, d, lpdf, nullptr, nullptr );
                        if( !is_black_or_negative( Li ) ) {
				            col3 fr = brdf.evaluate( wi, ctho, &fpdf );
				            if( !is_black_or_negative( fr ) ) {
                                col3 C = Li * fr * ctho / lpdf;
				                //shadow ray
				                Ray shadowray;
				                shadowray.o = hitpoint;
				                shadowray.d = wi;
                     
				                if( dot( wi, isect.shadingnormal_ ) > 0.f ) {
					                if( !scene_.occlusion( shadowray ) ) {
                                        float misweight = 1.f;
                                        misweight = mis_weight( lpdf, fpdf );
						               col += pathWeight * C * misweight;
					                }
				                }
                            }
                        }

				        //continue random walk;
                        {
				            const vec3 rnd3( rng_.getFloat(), rng_.getFloat(), rng_.getFloat() );
				            float pdf, cth;
				            vec3 wo;
				            const col3 fr = brdf.sample( rnd3, wo, pdf, cth );
                            if( is_black_or_negative( fr ) ) break;

				            pathWeight *= ( cth / pdf ) * fr;
                            lastpdf = pdf;
                            assert( pdf > 0.f );
                            //if( pdf == 0.f ) break;

				            ray.o = hitpoint;
				            ray.d = wo;
                        }
                    }
                   buffer_->add( x, y, col );
               }
           }
        };			    
        tbb::parallel_for( tbb::blocked_range2d< int, int >( 0, resY, 0, resX ), f );
    }

private:

	Rng rng_;
	int maxPathLength_;
	col3 intensity_;
	vec3 wi; //light direction

    float mis( const float pdf ) const
    {
        return pdf;
    }

    float mis_weight( const float sample_pdf, const float another_pdf ) const 
    {
        return mis( sample_pdf ) / ( mis( sample_pdf ) + mis( another_pdf ) );
    }

};


#endif /* defined(__VolumePathTracer__render__) */
