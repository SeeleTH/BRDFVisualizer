#include "brdfestimator.h"


/**
 * @fn void BRDFEstimator::init( void )
 * @brief
 */
void BRDFEstimator::init( void )
{
	fr_.reset( new col3 [ nth_ * nph_ * nth_ * nph_ ] );
    omega_.reset( new float [ nth_ * nph_ ] );
	memset( fr_.get(), 0, sizeof( float ) * nth_ * nph_ * nth_ * nph_ );


	ntriangle_ = 0;
	const int mesh_size = mesh_.size();
	for( int i = 0; i < mesh_size; i++ ) {
		ntriangle_ += mesh_[ i ].triangles.size();
	}

	area_.reset( new float [ ntriangle_ ] );
	geomID_.reset( new int [ ntriangle_ ] );
	triID_.reset( new int [ ntriangle_ ] );
	cdf_.reset( new float [ ntriangle_ + 1 ] );
	memset( cdf_.get(), 0, sizeof( float ) * ( ntriangle_ + 1 ) );

	//calculate area of each triangle
	int id = 0;
	totalArea_ = 0.f;
	for( int i = 0; i < mesh_size; i++ ) {
		const int size = mesh_[ i ].triangles.size();
		for( int j = 0; j < size; j++ ) {
			const ObjLoader::Vec3f v0 = mesh_[ i ].positions[ mesh_[ i ].triangles[ j ].i ];
			const ObjLoader::Vec3f v1 = mesh_[ i ].positions[ mesh_[ i ].triangles[ j ].j ];
			const ObjLoader::Vec3f v2 = mesh_[ i ].positions[ mesh_[ i ].triangles[ j ].k ];
			const vec3 tv0( v0.x, v0.y, v0.z );
			const vec3 tv1( v1.x, v1.y, v1.z );
			const vec3 tv2( v2.x, v2.y, v2.z );

			area_[ id ] = cross( tv2 - tv0, tv1 - tv0 ).norm() / 2.f;
			geomID_[ id ] = i;
			triID_[ id ] = j;
			cdf_[ id + 1 ] = cdf_[ id ] + area_[ id ]; 
			totalArea_ += area_[ id ];
			id++;
		}
	}

	for( int i = 0; i <= ntriangle_; i++ ) {
		cdf_[ i ] /= cdf_[ ntriangle_ ];
	}
	assert( cdf_[ ntriangle_ ] == 1.f );

    calculate_omega();
    init_boundary_sphere();
}

/**
 * @fn void BRDFEstimator::calculate_omega( void )
 * @brief calculate solid angle for each discretized directions
 */
void BRDFEstimator::calculate_omega( void )
{
    const float dth = pi / 2.f / ( float ) nth_;
    const float dph = 2.f * pi / ( float ) nph_;

    for( int i = 0; i < nth_; ++i ) {
        const float th0 = ( i + 0.5f ) / ( float ) nth_ * pi / 2.f;
        const float th1 = th0 + dth;
        for( int j = 0; j < nph_; ++j ) {
            omega_[ i * nph_ + j ] = dph * ( std::cosf( th0 ) - std::cosf( th1 ) ); 
        }
    }
}

/***
 * @fn void BRDFEstimator::init_boundary_sphere( void )
 * @brief calculate boundary sphere of the mesh
 */
void BRDFEstimator::init_boundary_sphere( void )
{
     center_ = ( scene_.bbmin + scene_.bbmax ) / 2.f;
     radius_ = ( scene_.bbmax - center_ ).norm();
}




/***
 * @fn void BRDFEstimator::sample_point( const float xi0, const float xi1, const float xi2, vec3& x, vec3& normal, int& geomID, int& triID ) const 
 * @brief 
 */
void BRDFEstimator::sample_point( const float xi0, const float xi1, const float xi2, vec3& x, vec3& normal, int& geomID, int& triID ) const
{
	//sample triangle
	int id = 0;
	for( int i = 0; i < ntriangle_; i++ ) {
		if( cdf_[ i ] <= xi0 && xi0 < cdf_[ i + 1 ] ) {
			id = i;
			break;
		}
	}

	//sample point from triangle
	const float u = xi1;
	const float v = xi2;
	geomID = geomID_[ id ];
	triID = triID_[ id ];
	const ObjLoader::Vec3f _v0 = mesh_[ geomID ].positions[ mesh_[ geomID ].triangles[ triID ].i ];
	const ObjLoader::Vec3f _v1 = mesh_[ geomID ].positions[ mesh_[ geomID ].triangles[ triID ].j ];
	const ObjLoader::Vec3f _v2 = mesh_[ geomID ].positions[ mesh_[ geomID ].triangles[ triID ].k ];
	const ObjLoader::Vec3f _n0 = mesh_[ geomID ].normals[ mesh_[ geomID ].triangles[ triID ].i ];
	const ObjLoader::Vec3f _n1 = mesh_[ geomID ].normals[ mesh_[ geomID ].triangles[ triID ].j ];
	const ObjLoader::Vec3f _n2 = mesh_[ geomID ].normals[ mesh_[ geomID ].triangles[ triID ].k ];
	const vec3 v0( _v0.x, _v0.y, _v0.z );
	const vec3 v1( _v1.x, _v1.y, _v1.z );
	const vec3 v2( _v2.x, _v2.y, _v2.z );
	const vec3 n0( _n0.x, _n0.y, _n0.z );
	const vec3 n1( _n1.x, _n1.y, _n1.z );
	const vec3 n2( _n2.x, _n2.y, _n2.z );

	x = ( 1.f - u - v ) * v0 + u * v1 + v * v2;
	normal = normalize( ( 1.f - u - v ) * n0 + u * n1 + v * n2 );

	return;
}

/**
 * @fn float BRDFEstimator::calculate_projected_area( const int N, const vec3& wo ) 
 * @brief 
 */
float BRDFEstimator::calculate_projected_area( const int N, const vec3& wo )
{
	float a = 0.f;
	float dot_wo_n;
	const float eps = 1e-3f;
	vec3 x, n;
	Ray ray;
	int geomID, triID;
	Isect isect;
	const float p = 1.f / totalArea_;

	for( int i = 0; i < N; i++ ) {
		const float xi0 = rng_.getFloat();
		const float xi1 = rng_.getFloat();
		const float xi2 = rng_.getFloat();
		sample_point( xi0, xi1, xi2, x, n, geomID, triID );
		x = x + eps * n;
		ray.o = x;
		ray.d = wo;
		dot_wo_n = dot( wo, n );
		if( dot_wo_n < 0.f ) continue;
        if( !scene_.intersect( ray, isect ) ) {
			a += dot_wo_n / p;
		}
	}
	a /= ( float ) N;
	return a;
}


/**
 * @fn col3 BRDFEstimator::calculate_throughput( const Ray& ray, const DirectionalLight& light )
 * @brief calculate throughput of a single ray illuminated by light
 */
col3 BRDFEstimator::calculate_throughput( const Ray& primary_ray, const DirectionalLight& light, bool& hit )
{
    Ray ray = primary_ray;
    Isect isect;
    col3 col;
    col3 path_weight( 1.f );
    int path_length = 1;
    float lastpdf = 1.f;
    hit = true;

    for( ;; ++path_length ) {

        // ray does not intersect scene
        if( !scene_.intersect( ray, isect ) ) {
            if( path_length == 1 ) { 
                hit = false;
                return col;
            }

            float mis_weight = mis2( lastpdf, light.pdf );
            col += light.radiance( ray.d ) * path_weight * mis_weight;
            return col;
        }

        // ray intersects scene
        if( path_length >= Config::max_path_length ) return col;

        vec3 hitpoint = ray.o + isect.dist_ * ray.d;
        BRDF brdf( ray, isect, scene_ );
    
        //next event estimation
        vec3 wi;
        col3 L, C, fr;
        float pdf, cosine, brdfpdf;
        L = light.illuminate( rng_.getFloat(), rng_.getFloat(), wi, pdf );
        fr = brdf.evaluate( wi, cosine, &brdfpdf );
        if( !is_black_or_negative( L ) && !is_black_or_negative( fr ) ) {
            C = path_weight * L * fr * cosine / pdf;
            Ray shadowray;
            shadowray.o = hitpoint;
            shadowray.d = wi;
            float weight = mis2( pdf, brdfpdf );

            if( !scene_.occlusion( shadowray ) ) {
                col += weight * C;
            }
        }

        //continue random walk
        {
            const vec3 rnd3( rng_.getFloat(), rng_.getFloat(), rng_.getFloat() );
            fr = brdf.sample( rnd3, ray.d, pdf, cosine );
            if( is_black_or_negative( fr ) ) return col;
            path_weight *= ( fr * cosine / pdf );
            ray.o = hitpoint;
            lastpdf = pdf;
        }
    }
}

/**
 * @fn void BRDFEstimator::estimate( const int nsample )
 * @brief 
 */
void BRDFEstimator::estimate( const int nsample )
{
    const int size = nth_ * nph_;
	bool isEnegyConserv = true;

    //omega_i 
	for (int i = 0; i < size; i++) {
		//set light 
		const int thidx = i / nph_;
		const int phidx = i - thidx * nph_;
#if 0
		const float theta = ((float)thidx + 0.5f) / ( float ) nth_ * pi / 2.f;
		const float phi = ((float)phidx + 0.5f) / (float)nph_ * 2.f * pi;
#else
		const float theta = thidx / (float)nth_ * pi / 2.f;
		const float phi = phidx / (float)nph_ * 2.f * pi;
#endif
		const float dth = 1.f / (float)nth_ * pi / 2.f;
		const float dph = 1.f / (float)nph_ * 2.f * pi;
		DirectionalLight light(theta, dth, phi, dph);

		const float cosine = cosf(theta + dth / 2.f);

		col3 eneCheck = col3(0.f, 0.f, 0.f);

		//omega_o
		for (int j = 0; j < size; j++) {
			//calculate wo
			const int thoidx = j / nph_;
			const int phoidx = j - thoidx * nph_;
			col3 fr;
			int N = 0;
#if 0
			N = 1;
			const float tho = ((float)thoidx + 0.5f) / (float)nth_ * pi / 2.f;
			const float pho = ((float)phoidx + 0.5f) / (float)nph_ * 2.f * pi;
			vec3 wo;
			wo.x = sinf( tho ) * cosf( pho );
			wo.y = cosf( tho );
			wo.z = sinf( tho ) * sinf( pho );
			vec3 wi;
			wi.x = sinf( theta ) * cosf( phi );
			wi.y = cosf( theta );
			wi.z = sinf( theta ) * sinf( phi );
			vec3 halfVec = normalize(wo + wi);
			float shininess = 25.f;
			float energyConvervation = (8.0f + shininess) / (8.0 * M_PI);
			fr = energyConvervation * pow(clamp(dot(vec3(0.f, 1.f, 0.f), halfVec), 0.f, 1.f), shininess);
#else
			for (int k = 0; k < nsample; k++) {
				const float tho = (thoidx + rng_.getFloat()) / (float)nth_ * pi / 2.f;
				const float pho = (phoidx + rng_.getFloat()) / (float)nph_ * 2.f * pi;
				vec3 wo;
				wo.x = sinf(tho) * cosf(pho);
				wo.y = cosf(tho);
				wo.z = sinf(tho) * sinf(pho);

				//generate ray by sampling a disk perpendicular to wo 
				const vec3 disk = sampleConcentricDisc(rng_.getFloat(), rng_.getFloat());
				Frame frame;
				frame.set(wo);
				Ray primary;
				primary.o = center_ + radius_ * disk.x * frame.tangent() + radius_ * disk.y * frame.binormal() + radius_ * wo;
				primary.d = -wo;
				bool hit = false;
				const col3 col = calculate_throughput(primary, light, hit);
				if (hit) {
					fr += col;
					N++;
				}
			}
#endif
			fr /= (float)N;
			eneCheck += fr;
			fr /= (cosine * light.solid_angle()); //definition of BRDF f_r(\omega_i,\omega_o)= dL(x,\omega_o)/L(x,\omega_i)cos\theta_i d\omega_i 
			fr_[i * size + j] = fr;
			//std::cout << fr << "\n";
		}

		if (eneCheck.r > 1.f || eneCheck.g > 1.f || eneCheck.b > 1.f)
		{
			isEnegyConserv = false;
			std::cout << "[" << i << "/" << size << "]" << "Energy Conservation FAILED!! with " << eneCheck << std::endl;
		}
		else
		{
			std::cout << "[" << i << "/" << size << "]" << "Energy Conservation OK with" << eneCheck << std::endl;
		}
    }
	if (!isEnegyConserv)
	{
		std::cout << "[ERROR] Result >> Energy Conservation FAILED!!!" << std::endl;
	}
}


/**
 *  @fn void BRDFEstimator::visualize( const EnvMap& map, const char* filename ) const
 *  @brief simply render sphere under EnvMap 
 */
void BRDFEstimator::visualize( const EnvMap& map, const char* filename ) const
{
    Framebuffer buffer;
    const int resx = buffer.x();
    const int resy = buffer.y();
    const int map_width  = map.img->w();
    const int map_height = map.img->h();
    const float dth = pi / 2.f / ( float ) nth_;
    const float dph = 2.f * pi / ( float ) nph_;
    const int size = nth_ * nph_;
    const float du = 1.f / ( float ) map_width;
    const float dv = 1.f / ( float ) map_height;
    const float scale = du * dv * 2.f * pi * pi;

    for( int w = 0; w < resx; ++w ) {
        for( int h = 0; h < resy; ++h ) {

            const float x = ( w - resx / 2.f ) / ( float ) resx * 2.f;
            const float y = ( h - resy / 2.f ) / ( float ) resy * 2.f;
            const float zz = x * x + y * y;
            if( zz >= 1.f ) continue;
            const float z = sqrtf( 1.f - zz );
            const vec3 normal( x, y, z );
            const vec3 wo( 0.f, 0.f, 1.f );
            Frame frame;
            frame.set( normal );
            const vec3 lwo = frame.toLocal( wo );
            col3 col( 0.f );

            for( int i = 0; i < map_width; ++i ) {
                for( int j = 0; j < map_height; ++j ) {
                     const float u = ( i + 0.5f ) / ( float ) map_width;
                     const float v = ( j + 0.5f ) / ( float ) map_height;
                     const float sth = sinf( v * pi );                
                     const vec3 wi = map.latitude_longitude_to_vec( u, v );                     
                     const float cosine = dot( wi, normal );
                     
                     if( cosine > 0.f ) {
                          const col3 Li = map.lookup( wi ) * sth;
                          const vec3 lwi = frame.toLocal( wi );

                          float thi = acosf( clamp( lwi.z, 0.f, 1.f ) );
                          float phi = atan2f( lwi.y, lwi.x ); if( phi < 0.f ) phi += 2.f * pi;
                          float tho = acosf( clamp( lwo.z, 0.f, 1.f ) );
                          float pho = atan2f( lwo.y, lwo.x ); if( pho < 0.f ) pho += 2.f * pi;

                          const int thi_idx0 = clamp( ( int ) floor( thi / dth ), 0, nth_ - 1 );
                          const int thi_idx1 = clamp( thi_idx0 + 1, 0, nth_ - 1 );
                          const int phi_idx0 = clamp( ( int ) floor( phi / dph ), 0, nph_ - 1 );
                          const int phi_idx1 = clamp( phi_idx0 + 1, 0, nph_ - 1 );

                          const int tho_idx0 = clamp( ( int ) floor( tho / dth ), 0, nth_ - 1 );
                          const int tho_idx1 = clamp( tho_idx0 + 1, 0, nth_ - 1 );
                          const int pho_idx0 = clamp( ( int ) floor( pho / dph ), 0, nph_ - 1 );
                          const int pho_idx1 = clamp( pho_idx0 + 1, 0, nph_ - 1 );

                          int iidx[ 4 ], oidx[ 4 ];
                          float tw[ 2 ], pw[ 2 ];
                          float weighti[ 4 ], weighto[ 4 ];

                          tw[ 0 ] = clamp( ( thi - thi_idx0 * dth ) / dth, 0.f, 1.f );
                          pw[ 0 ] = clamp( ( phi - phi_idx0 * dph ) / dph, 0.f, 1.f );
                          tw[ 1 ] = clamp( ( tho - tho_idx0 * dth ) / dth, 0.f, 1.f );
                          pw[ 1 ] = clamp( ( pho - pho_idx0 * dph ) / dph, 0.f, 1.f );

                          weighti[ 0 ] = ( 1.f - tw[ 0 ] ) * ( 1.f - pw[ 0 ] );
                          weighti[ 1 ] = ( 1.f - tw[ 0 ] ) *         pw[ 0 ]  ;
                          weighti[ 2 ] =         tw[ 0 ]   * ( 1.f - pw[ 0 ] );
                          weighti[ 3 ] =         tw[ 0 ]   *         pw[ 0 ]  ;

                          weighto[ 0 ] = ( 1.f - tw[ 1 ] ) * ( 1.f - pw[ 1 ] );
                          weighto[ 1 ] = ( 1.f - tw[ 1 ] ) *         pw[ 1 ]  ;
                          weighto[ 2 ] =         tw[ 1 ]   * ( 1.f - pw[ 1 ] );
                          weighto[ 3 ] =         tw[ 1 ]   *         pw[ 1 ]  ;

                          iidx[ 0 ] = thi_idx0 * nph_ + phi_idx0;
                          iidx[ 1 ] = thi_idx0 * nph_ + phi_idx1;
                          iidx[ 2 ] = thi_idx1 * nph_ + phi_idx0;
                          iidx[ 3 ] = thi_idx1 * nph_ + phi_idx1;

                          oidx[ 0 ] = tho_idx0 * nph_ + pho_idx0;
                          oidx[ 1 ] = tho_idx0 * nph_ + pho_idx1;
                          oidx[ 2 ] = tho_idx1 * nph_ + pho_idx0;
                          oidx[ 3 ] = tho_idx1 * nph_ + pho_idx1;

                          col3 fr;
                          for( int ii = 0; ii < 4; ++ii ) {
                              for( int jj = 0; jj < 4; ++jj ) {
                                  if( iidx[ ii ] * size + oidx[ jj ] > size * size ) {
                                      std::cout << iidx[ ii ] << " : " << oidx[ jj ] << "\n";
                                      std::cout << tho_idx0 << " : " << tho_idx1 << " : " << pho_idx0 << " : " << pho_idx1 << "\n";
                                  }
                                  fr += weighti[ ii ] * weighto[ jj ] * fr_[ iidx[ ii ] * size + oidx[ jj ] ];
                              }
                          }

                          col += Li * fr * cosine;
                     }
                }
            }
            buffer.set( w, h, col * scale * Config::envmap_scale );
        }
    }
	buffer.saveBMP(filename);
}



void BRDFEstimator::write_result(const char* filename) const
{
	const int size = nth_ * nph_;
	std::cout << "Writing result to " << filename << " in Bitmap " << size << "x" << size << std::endl;
	Framebuffer buffer(size, size);
	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			buffer.set(i, j, fr_[i * size + j]);
		}
	}
	buffer.saveHDR(filename);

	std::cout << "Writing Completed!" << std::endl;
}



/**
 * @fn col3 BRDFEstimator::estimate_throughput( const vec3 x, const vec3 wo, const int nsample )
 * @brief estimate throughput (f_r*cos/p) at x towards w
 */
/*
col3 BRDFEstimator::estimate_throughput( const vec3& x, const vec3& normal, const vec3& wo, const int index, const int nsample )
{
    Ray ray, shadowray;
    col3 col;
    Isect isect;
    col3 path_weight( 1.f );
    int path_length = 1;
    const col3 L( 1.f ); 

    const int thidx = index / nph_;
    const int phidx = index - thidx * nph_;    
    
    shadowray.o = x;
    shadowray.d = wo;

    if( scene_.occlusion( shadowray ) ) return col;

    ray.o = x;
    //ray.d = 

    for(;; ++path_length ) {
        if( !scene_.intersect( ray, isect ) ) {

            if( inside( ray.d, thidx, phidx ) ) {
                col += path_weight * L;
            }
            return col;
        }

        vec3 hitpoint = ray.o + isect.dist_ * ray.d;
        BRDF brdf( ray, isect, scene_ );

        if( path_length >= Config::max_path_length ) return col;

        //next event estimation
        //sample light
        vec3 wi;
        col3 L, C, fr;
        float pdf, cosine;
        L = illuminate( rng_.getFloat(), rng_.getFloat(), thidx, phidx, wi, pdf );
        fr = brdf.evaluate( wi, cosine );

        if( !is_black_or_negative( L ) && !is_black_or_negative( fr ) ) {
            C = path_weight * fr * cosine * L / pdf;

            shadowray.o = hitpoint;
            shadowray.d = wi;

            if( !scene_.occlusion( shadowray ) ) {
                col += C;
            }
        }

        //continue random walk
        {
            const vec3 rnd3( rng_.getFloat(), rng_.getFloat(), rng_.getFloat() );
            fr = brdf.sample( rnd3, ray.d, pdf, cosine );
            if( is_black_or_negative( fr ) ) return col;
            path_weight *= fr * cosine / pdf;
            ray.o = hitpoint;
        }
    }
}
*/

