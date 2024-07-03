/*
    This file is part of Dirt, the Dartmouth introductory ray tracer.

    Copyright (c) 2017-2019 by Wojciech Jarosz

    Dirt is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Dirt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <dirt/fwd.h>
#include <dirt/parser.h>
#include <stdlib.h>

struct ScatterRecord
{
  Color3f attenuation;
  Vec3f scattered;
  bool isSpecular = false;
};

/// A base class used to represent surface material properties.
class Material
{
public:
	virtual ~Material() = default;

	/// Return a pointer to a global default material
	static shared_ptr<const Material> defaultMaterial();

	/**
	   \brief Compute the scattered direction scattered at a surface hitpoint.

	   The base Material does not scatter any light, so it simply returns false.
	   
	   \param  ray              incoming ray
	   \param  hit              the ray's intersection with the surface
	   \param  attenuation      how much the light should be attenuated
	   \param  scattered        the direction light should be scattered
	   \return bool             True if the surface scatters light
	 */
	virtual bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
	{
		return false;
	}

	/**
	   Compute the amount of emitted light at the surface hitpoint.
	  
	   The base Material class does not emit light, so it simply returns black.
	  
	   \param  ray		the incoming ray
	   \param  hit		the ray's intersection with the surface
	   \return			the emitted color
	 */
	virtual Color3f emitted(const Ray3f &ray, const HitInfo &hit) const
	{
		return Color3f(0,0,0);
	}

  /**
	 Sample a scattered direction at the surface hitpoint \c hit.
		If it is not possible to evaluate the pdf of the material (e.g.\ it is
		specular or unknown), then set srec.isSpecular to true, and populate
		srec.scattered and srec.attenuation just like we did previously in the
		\ref scatter() function. This allows you to fall back to the way we did
		things with the \ref scatter() function, i.e.\ bypassing \ref pdf()
		evaluations needed for explicit Monte Carlo integration in your \ref
		Integrator, but this also precludes the use of MIS or mixture sampling
		since the pdf is unknown.
		\param  dirIn  The incoming ray direction
		\param  hit    The incoming ray's intersection with the surface
		\param  srec   Populate srec.scattered, srec.isSpecular, and
						srec.attenuation
		\return bool   True if the surface scatters light
	*/
	virtual bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
	{
		return false;
	}

  /**
	 Evaluate the material response for the given pair of directions.
		For non-specular materials, this should be the BSDF multiplied by the
		cosine foreshortening term.
		Specular contributions should be excluded.
		
		\param  dirIn      The incoming ray direction
		\param  scattered  The outgoing ray direction
		\param  hit        The shading hit point
		\return Color3f    The evaluated color
	*/
	virtual Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
	{
    	return Color3f(0.0f);
	}

  /**
	   Compute the probability density that \ref sample() will generate \c scattered (given \c dirIn).
	   
	   \param  dirIn      The incoming ray direction
	   \param  scattered  The outgoing ray direction
	   \param  hit        The shading hit point
	   \return float      A probability density value in solid angle measure around \c hit.p.
	 */
	virtual float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
	{
    	return 0.0f;
	}

	/**
      Return whether or not this Material is emissive.
  This is primarily used to create a global list of emitters for sampling.
    */
  virtual bool isEmissive() const {return false;}
};


/// A perfectly diffuse (Lambertian) material
class Lambertian : public Material
{
public:
	Lambertian(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

  	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	shared_ptr<const Texture> albedo; ///< The diffuse color (fraction of light that is reflected per color channel).
};


/// A metallic material that reflects light into the (potentially rough) mirror reflection direction.
class Metal : public Material
{
public:
	Metal(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

  	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override {return Color3f(0.f);}

	shared_ptr<const Texture> albedo;   ///< The reflective color (fraction of light that is reflected per color channel).
	shared_ptr<const Texture> roughness;///< A value between 0 and 1 indicating how smooth vs. rough the reflection should be.
};


/// A smooth dielectric surface that reflects and refracts light according to the specified index of refraction #ior
class Dielectric : public Material
{
public:
	Dielectric(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

  	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo &hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float ior;      ///< The (relative) index of refraction of the material
};


/// A material that emits light equally in all directions from the front side of a surface.
class DiffuseLight : public Material
{
public:
	DiffuseLight(const json & j = json::object());

	/// Returns a constant Color3f if the ray hits the surface on the front side.
	Color3f emitted(const Ray3f &ray, const HitInfo &hit) const override;

  bool isEmissive() const override {return true;}

	Color3f emit;	///< The emissive color of the light
};

class BlendMaterial : public Material
{
public:
	BlendMaterial(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

  	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	shared_ptr<const Material> a, b;
	shared_ptr<const Texture> amount;
};

class Phong : public Material
{
public:
	Phong(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	shared_ptr<const Texture> albedo;     ///< The reflective color (fraction of light that is reflected per color channel).
	float exponent = 10.f;
};

class BlinnPhong : public Material
{
public:
	BlinnPhong(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	shared_ptr<const Texture> albedo;     ///< The reflective color (fraction of light that is reflected per color channel).
	float exponent = 10.f;
};

class Beckmann : public Material
{
public:
	Beckmann(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;
	
	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float G1(const Vec3f v, const Vec3f m, const HitInfo & hit) const;

	shared_ptr<const Texture> albedo;     ///< The reflective color (fraction of light that is reflected per color channel).
	float ab = 0.5f;
	float ior = 0.05f;
	float ext = 3.9f;
};

class OrenNayar : public Material
{
public:
	OrenNayar(const json & j = json::object());

	bool scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const override;

	bool sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const override;

	Color3f eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const override;

	float G1(const Vec3f v, const Vec3f m, const HitInfo & hit) const;

	shared_ptr<const Texture> albedo;     ///< The reflective color (fraction of light that is reflected per color channel).
	float sigma = 0.35f;
};
