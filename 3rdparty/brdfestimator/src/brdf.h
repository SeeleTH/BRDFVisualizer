#ifndef _BRDF_H_
#define _BRDF_H_

#include <iostream>
#include <cmath>
#include "scene.h"
#include "frame.h"
#include "utility.h"


/**
 * @class BRDF
 *
 */
class BRDF {

public:

	enum Events 
	{
		kNONE = 0,
		kDIFFUSE = 1,
		kGLOSSY  = 2,
		kREFLECT = 4,
		kSPECULAR = kREFLECT,
		kNONSPECULAR = ( kDIFFUSE | kGLOSSY ),
		kALL = ( kSPECULAR | kNONSPECULAR )
	};

	BRDF() {
	}

	BRDF( const Ray& ray, const Isect& isect, const Scene& scene )
	{
		init( ray, isect, scene );
	}

	/**
	 * @brief initialize function with overrided material
	 */
	BRDF(const Ray& ray, const Isect& isect, const Material& mat)
	{
		mat_ = mat;
		frame_.set(isect.shadingnormal_);
		local_ = frame_.toLocal(-ray.d);
		getComponentProbability(mat_);
	}

	/**
	 * @fn void init( const Ray& ray, const Isect& isect, const Scene& scene )
	 * @brief initialize function 
	 */
	void init( const Ray& ray, const Isect& isect, const Scene& scene )
	{
		scene.setMaterial( isect, mat_ );
		//frame_.set( isect.normal_ );
		frame_.set( isect.shadingnormal_ );
        local_ = frame_.toLocal( - ray.d );
		getComponentProbability( mat_ );
	}

	/**
	 * @fn void getComponentProbability( const Material& mat )
	 * @brief 
	 */
	void getComponentProbability( const Material& mat )
	{
		const float ad = albedoDiffuse( mat );
		const float ag = albedoGlossy( mat );
		const float ar = albedoReflect( mat );
		const float total = ad + ag + ar;
		if( total < 1e-9f )
		{
			diffuseProb() = 0.f;
			glossyProb() = 0.f;
			reflectProb() = 0.f;
		} else {
			diffuseProb() = ad / total;
			glossyProb()  = ag / total;
			reflectProb() = ar / total;
		}
	}

	/**
	 * @fn col3 evaluate( const vec3& wo, float& cosine, float *pdfw = nullptr ) const 
	 * @brief 
	 */
	col3 evaluate( const vec3& wo, float& cosine, float *pdfw = nullptr ) const
	{
		col3 col;
		if( pdfw != nullptr ) *pdfw = 0.f;

		const vec3 lwo = frame_.toLocal( wo );
		const float ctho = lwo.z;
		cosine = std::fabsf( ctho );
		
		if( cosine < Config::EPS_COSINE ) return col;
		

		col += evaluateDiffuse( lwo, pdfw );
		col += evaluateGlossy( lwo, pdfw );

		return col;
	}
	
	/**
	 * @fn col3 evaluateDiffuse( const vec3& lwo, float *pdfw = nullptr ) const
	 * @brief 
	 */
	col3 evaluateDiffuse( const vec3& lwo, float *pdfw = nullptr )	const
	{
		col3 col;
		if( diffuseProb() == 0.f ) return col;
		if( lwo.z < Config::EPS_COSINE || local_.z < Config::EPS_COSINE ) return col;
		if( pdfw != nullptr ) *pdfw += diffuseProb() * std::max( 0.f, lwo.z / ( float ) M_PI );
		return mat_.diffuse / ( float ) M_PI;
	}

	/**
     * @fn col3 evaluateGlossy( const vec3& lwo, float *pdfw = nullptr ) const
	 * @brief 
	 */
	col3 evaluateGlossy( const vec3& lwo, float *pdfw = nullptr ) const
	{
		col3 col;
		if( glossyProb() == 0.f ) return col;
		if( lwo.z < Config::EPS_COSINE || local_.z < Config::EPS_COSINE ) return col;
        const vec3 reflocal = reflectLocal( local_ );
        const float dot_r_wi = dot( reflocal, lwo );
        if( dot_r_wi < Config::EPS_PHONG ) return col;
        if( pdfw != nullptr ) {
            *pdfw += glossyProb() * powerCosHemispherePDF( dot_r_wi, mat_.glossy.a );
        }
        return mat_.glossy * ( mat_.glossy.a + 2.f ) * 0.5f * invpi * powf( dot_r_wi, mat_.glossy.a );
        /*
		const vec3 half = normalize( lwo + local_ );
		if( half.z < Config::EPS_PHONG ) return col;
		if( pdfw != nullptr ) {
			*pdfw += glossyProb() * powerCosHemispherePDF( half.z, mat_.glossy.a );
		}
		col = mat_.glossy * ( mat_.glossy.a + 2.f ) / ( 2.f * ( float ) M_PI ) * powf( half.z, mat_.glossy.a );
		return col;
        */
	}

	/***
	 * @fn col3 sample( const vec3 rnd, vec3& lwo, float& pdfw, float& cth ) const 
	 * @brief sample new direction
	 */
	col3 sample( const vec3& rnd3, vec3& wo, float& pdfw, float& cth ) const
	{
		int sampleEvent;
		col3 result;
		vec3 lwo;
		pdfw = 0.f;

		if( rnd3.z < diffuseProb() ) {
			sampleEvent = kDIFFUSE;
		} else if( rnd3.z < diffuseProb() + glossyProb() ) {
			sampleEvent = kGLOSSY;
		} else {
			sampleEvent = kNONE;
		}

		if( sampleEvent == kDIFFUSE ) {
			result += sampleDiffuse( rnd3.x, rnd3.y, lwo, pdfw );
			result += evaluateGlossy( lwo, &pdfw );
		} else if( sampleEvent == kGLOSSY ) {
			result += sampleGlossy( rnd3.x, rnd3.y, lwo, pdfw );
			result += evaluateDiffuse( lwo, &pdfw );
		}

		cth = lwo.z;
		wo = frame_.toWorld( lwo );

		if( cth < Config::EPS_COSINE ) {
			return col3( 0.f );
		}
		return result;
	}

	/**
	 * @fn col3 sampleDiffuse( const float xi0, const float xi1, vec3& lwo, float& pdfw ) const
	 * @brief sample direction (lwo) and evaluate BRDF
	 */
	col3 sampleDiffuse( const float xi0, const float xi1, vec3& lwo, float& pdfw ) const
	{
		col3 fr;
		if( local_.z < Config::EPS_COSINE ) return fr;
		float pdf;
		lwo = sampleCosHemisphere( xi0, xi1, &pdf );
		pdfw += pdf * diffuseProb();
		return mat_.diffuse * invpi; // kd / pi
	}

	/**
	 * @fn col3 sampleGlossy( const float xi0, const float xi1, vec3& lwo, float& pdfw ) const
	 * @brief sample direction (lwo) and evaluate BRDF
	 */
	col3 sampleGlossy( const float xi0, const float xi1, vec3& lwo, float& pdfw ) const
	{
        col3 fr;
        const float power = mat_.glossy.a;
		lwo = samplePowerCosHemisphere( xi0, xi1, power, nullptr );
        const vec3 reflocal = reflectLocal( local_ );
        {
            Frame frame;
            frame.set( reflocal );
            lwo = frame.toWorld( lwo );
        }
        const float dot_r_wi = dot( reflocal, lwo );
        if( dot_r_wi < Config::EPS_PHONG ) return fr;
        glossyPDF( power, dot_r_wi, &pdfw );
        return mat_.glossy * ( power + 2.f ) * 0.5f * invpi * std::powf( dot_r_wi, power );

	}

	void diffusePDF( const vec3& lwo, float *pdfw = nullptr ) const
	{
		if( diffuseProb() == 0.f ) return;
		if( pdfw != nullptr ) {
			*pdfw += diffuseProb() * std::max( 0.f, lwo.z * invpi );
		}
	}

	void glossyPDF( const float power, const float cosine,  float *pdfw = nullptr ) const
	{
		if( glossyProb() == 0.f ) return;
		if( cosine < Config::EPS_COSINE ) return;
		if( pdfw != nullptr ) {
			const float pdf = powerCosHemispherePDF( cosine, power ) * glossyProb();
			*pdfw += pdf;
		}
	}


	float albedoDiffuse( const Material& mat ) const 
	{
		return luminance( mat.diffuse );
	}

	float albedoGlossy( const Material& mat ) const
	{
		return luminance( mat.glossy );
	}

	float albedoReflect( const Material& mat ) const
	{
		return luminance( mat.specular );
	}

private:

	Frame frame_;
	vec3 local_;
	col3 coef_;
	Material mat_;


	inline const float diffuseProb( void ) const { return coef_.r; }
	inline float& diffuseProb( void )            { return coef_.r; }
	inline const float glossyProb( void )  const { return coef_.g; }
	inline float& glossyProb( void )             { return coef_.g; }
	inline const float reflectProb( void ) const { return coef_.b; }
	inline float& reflectProb( void )            { return coef_.b; }


};




#endif