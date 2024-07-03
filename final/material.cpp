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

#include <dirt/material.h>
#include <dirt/texture.h>
#include <dirt/parser.h>
#include <dirt/scene.h>
#include <dirt/surface.h>
#include <dirt/onb.h>

shared_ptr<const Material> Material::defaultMaterial()
{
	return nullptr;
}


inline bool refract(const Vec3f &v, const Vec3f &n, float iorIOverT, Vec3f &refracted, float &cosTheta2)
{
	Vec3f uv = normalize(v);
	float dt = dot(uv,n);
	float discrim = 1.0f - iorIOverT * iorIOverT * (1.0f - dt * dt);
	if (discrim > 0)
	{
		cosTheta2 = std::sqrt(discrim);
		refracted = iorIOverT * (uv - n * dt) - n * cosTheta2;
		return true;
	}
	else
	{
		return false;
	}
}

inline Vec3f reflect(const Vec3f &v, const Vec3f &n)
{
	return v - 2 * dot(v, n) * n;
}

inline float conductorFresnel(float cosine, float ior, float ext) 
{
	float tmp = (ior*ior + ext*ext);
	float R_s = (tmp - 2*ior*cosine + cosine*cosine) / (tmp + 2*ior*cosine + cosine*cosine);
	float R_p = (tmp*cosine*cosine - 2*ior*cosine + 1)/(tmp*cosine*cosine + 2*ior*cosine + 1);
	return (R_s + R_p)/2.0f;
}

Lambertian::Lambertian(const json & j)
{
	albedo = parseTexture(j.at("albedo"));
}

bool Lambertian::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	// TODO: Implement Lambertian reflection
	//       You should assign the albedo to ``attenuation'', and
	//       you should assign the scattered ray to ``scattered''
	//       The origin of the scattered ray should be at the hit point,
	//       and the scattered direction is the shading normal plus a random
	//       point on a sphere (please look at the text book for this)
	//       You can get the hit point using hit.p, and the shading normal using hit.sn

	//       Hint: You can use the function randomInUnitSphere() to get a random
	//       point in a sphere. IMPORTANT: You want to add a random point *on*
	//       a sphere, not *in* the sphere (the text book gets this wrong)
	//       If you normalize the point, you can force it to be on the sphere always, so
	//       add normalize(randomInUnitSphere()) to your shading normal
	scattered = Ray3f(hit.p, hit.sn + randomOnUnitSphere(sample));
	attenuation = albedo->value(hit);
	return true;
}

bool Lambertian::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	ONBf onb;
	onb.build_from_w(hit.sn);
    srec.scattered = onb.toWorld(randomCosineHemisphere(sample));
	srec.isSpecular = false;
    srec.attenuation = albedo->value(hit);
	return true;
}

Color3f Lambertian::eval(const Vec3f & dirIn, const Vec3f &scattered, const HitInfo & hit) const 
{
	return albedo->value(hit) * max(0.0f, dot(scattered, hit.sn) / M_PI);
}

float Lambertian::pdf(const Vec3f & dirIn, const Vec3f &scattered, const HitInfo & hit) const 
{
	return max(0.0f, dot(scattered, hit.sn) / M_PI);
}

Metal::Metal(const json & j)
{
	albedo = parseTexture(j.at("albedo"));
	roughness = parseTexture(j.at("roughness"));
}

bool Metal::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	// TODO: Implement metal reflection
	//       This function proceeds similar to the lambertian material, except that the
	//       scattered direction is different.
	//       Instead of adding a point on a sphere to the normal as before, you should add the point
	//       to the *reflected ray direction*.
	//       You can reflect a vector by the normal using reflect(vector, hit.sn); make sure the vector is normalized.
	//       Different to before you can't just use randomInUnitSphere directly; the sphere should be scaled by roughness.
	//       (see text book). In other words, if roughness is 0, the scattered direction should just be the reflected direction.
	//       
	//       This procedure could produce directions below the surface. Handle this by returning false if the scattered direction and the shading normal
	//       point in different directions (i.e. their dot product is negative)
	Vec3f reflected = reflect(normalize(ray.d), hit.sn);
	scattered = Ray3f(hit.p, reflected + luminance(roughness->value(hit)) * randomOnUnitSphere(sample));
	attenuation = albedo->value(hit);
	return (dot(scattered.d, hit.sn) > 0);
}

bool Metal::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	srec.scattered = reflect(normalize(dirIn), hit.sn);
	srec.attenuation = albedo->value(hit);
	srec.isSpecular = true;
	return true;
}

Dielectric::Dielectric(const json & j)
{
	ior = j.value("ior", ior);
}

bool Dielectric::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	Vec3f normal;
	float eta1, eta2;

	// ensure ior and normal are correctly oriented for computing reflection and refraction
	if (dot(ray.d, hit.sn) > 0)
	{
		normal = -hit.sn;	
		eta1 = ior;
		eta2 = 1.0f;
	}
	else
	{
		normal = hit.sn;
		eta1 = 1.0f;
		eta2 = ior;
	}

	attenuation = Color3f(1);

	// compute reflected + refracted ray
	float cosTheta2, cosTheta1 = dot(ray.d, -normal) / length(ray.d);
	Vec3f refracted, reflected = reflect(ray.d, hit.sn);
	if (!refract(ray.d, normal, eta1 / eta2, refracted, cosTheta2))
	{
		// no refraction, only reflection
		scattered = Ray3f(hit.p, reflected);
		return true;
	}

	// compute fresnel solutions
	float rho_parallel = ((eta2 * cosTheta1) - (eta1 * cosTheta2)) / ((eta2 * cosTheta1) + (eta1 * cosTheta2));
	float rho_perp = ((eta1 * cosTheta1) - (eta2 * cosTheta2)) / ((eta1 * cosTheta1) + (eta2 * cosTheta2));
	float Freflected = (rho_parallel * rho_parallel + rho_perp * rho_perp) / 2.0f;

	// sample scattered or reflected ray
	scattered = sample.x < Freflected 
		? Ray3f(hit.p, reflected)
		: Ray3f(hit.p, refracted);

	return true;
}

bool Dielectric::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	Vec3f normal;
	float eta1, eta2;

	// ensure ior and normal are correctly oriented for computing reflection and refraction
	if (dot(dirIn, hit.sn) > 0)
	{
		normal = -hit.sn;	
		eta1 = ior;
		eta2 = 1.0f;
	}
	else
	{
		normal = hit.sn;
		eta1 = 1.0f;
		eta2 = ior;
	}
	
	srec.isSpecular = true;
	srec.attenuation = Color3f(1.f);

	// compute reflected + refracted ray
	float cosTheta2, cosTheta1 = dot(dirIn, -normal) / length(dirIn);
	Vec3f refracted, reflected = reflect(dirIn, hit.sn);
	if (!refract(dirIn, normal, eta1 / eta2, refracted, cosTheta2))
	{
    // no refraction, only reflection
    srec.scattered = reflected;
    return true;
	}

  // compute fresnel solutions
  float rho_parallel = ((eta2 * cosTheta1) - (eta1 * cosTheta2)) / ((eta2 * cosTheta1) + (eta1 * cosTheta2));
  float rho_perp = ((eta1 * cosTheta1) - (eta2 * cosTheta2)) / ((eta1 * cosTheta1) + (eta2 * cosTheta2));
  float Freflected = (rho_parallel * rho_parallel + rho_perp * rho_perp) / 2.0f;

  // sample scattered or reflected ray
  srec.scattered = sample.x < Freflected ? reflected : refracted;

  return true;
}

Color3f Dielectric::eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
  return Color3f(0.f);
}

float Dielectric::pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
  return 1.f;
}

DiffuseLight::DiffuseLight(const json & j)
{
	emit = j.value("emit", emit);
}

Color3f DiffuseLight::emitted(const Ray3f &ray, const HitInfo &hit) const
{
	// only emit from the normal-facing side
	if (dot(ray.d, hit.sn) > 0)
		return Color3f(0,0,0);
	else
		return emit;
}

BlendMaterial::BlendMaterial(const json & j)
{
	a = parseMaterial(j.at("a"));
	b = parseMaterial(j.at("b"));
	amount = parseTexture(j.at("amount"));
}

bool BlendMaterial::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	float t = luminance(amount->value(hit));
	if (randf() < t)
		return b->scatter(ray, hit, sample, attenuation, scattered);
	else
		return a->scatter(ray, hit, sample, attenuation, scattered);
}

bool BlendMaterial::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	float t = luminance(amount->value(hit));
	if (randf() < t)
		return b->sample(dirIn, hit, sample, srec);
	else
		return a->sample(dirIn, hit, sample, srec);
}

Color3f BlendMaterial::eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const
{
    float t = luminance(amount->value(hit));
    return lerp(a->eval(dirIn, scattered, hit),
                b->eval(dirIn, scattered, hit), t);
}

float BlendMaterial::pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
	float t = luminance(amount->value(hit));
    return lerp(a->pdf(dirIn, scattered, hit),
                b->pdf(dirIn, scattered, hit), t);
}


Phong::Phong(const json & j)
{
	albedo = parseTexture(j.at("albedo"));
	exponent = j.value("exponent", exponent);
}

bool Phong::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	ScatterRecord srec;
	if (!this->sample(ray.d, hit, sample, srec))
		return false;
	attenuation = srec.attenuation;
	scattered = Ray3f(hit.p, srec.scattered);
	return true;
}

bool Phong::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	ONBf onb;
	onb.build_from_w(reflect(dirIn, hit.sn));
  	srec.scattered = onb.toWorld(randomCosinePowerHemisphere(exponent, sample));
	srec.isSpecular = false;
  	srec.attenuation = albedo->value(hit);
	return dot(hit.sn, srec.scattered) > 0;
}

Color3f Phong::eval(const Vec3f & dirIn, const Vec3f &scattered, const HitInfo & hit) const 
{
	return albedo->value(hit) * pdf(dirIn, scattered, hit);
}

float Phong::pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
  Vec3f mirrorDir = normalize(reflect(dirIn, hit.sn));
	float cosine = max(dot(normalize(scattered), mirrorDir), 0.0f);
	return (exponent + 1.0f)/(2.0f*M_PI) * powf(cosine, exponent);
}

BlinnPhong::BlinnPhong(const json & j)
{
	albedo = parseTexture(j.at("albedo"));
	exponent = j.value("exponent", exponent);
}

bool BlinnPhong::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	ScatterRecord srec;
	if (!this->sample(ray.d, hit, sample, srec))
		return false;
	attenuation = srec.attenuation;
	scattered = Ray3f(hit.p, srec.scattered);
	return true;
}

bool BlinnPhong::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	ONBf onb;
	onb.build_from_w(hit.sn);
	Vec3f normal = onb.toWorld(randomCosinePowerHemisphere(exponent, sample));
	srec.scattered = normalize(reflect(dirIn, normal));
	srec.isSpecular = false;
	srec.attenuation = albedo->value(hit);
	return dot(hit.sn, srec.scattered) > 0;
}

Color3f BlinnPhong::eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
	return albedo->value(hit) * pdf(dirIn, scattered, hit);
}

float BlinnPhong::pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
	float iDotN = dot(-dirIn, hit.sn);
	if (iDotN <= 0.0f || dot(scattered, hit.sn) <= 0.0f)
		return 0.0f;
	Vec3f normal = normalize(-normalize(dirIn) + normalize(scattered));
	float cosine = max(dot(normal, hit.sn), 0.0f);
	float normalPdf = (exponent + 1.0f)/(2.0f*M_PI)*powf(cosine, exponent);
  return normalPdf/(4.0f*dot(-dirIn, normal));
}


Beckmann::Beckmann(const json & j)
{
	albedo = parseTexture(j.at("albedo"));
	ab = j.value("ab", ab);
	ior = j.value("ior", ior);
	ext = j.value("ext", ext);
}

bool Beckmann::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
	ScatterRecord srec;
	if (!this->sample(ray.d, hit, sample, srec))
		return false;
	attenuation = srec.attenuation;
	scattered = Ray3f(hit.p, srec.scattered);
	return true;
}

bool Beckmann::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const 
{
	ONBf onb;
	onb.build_from_w(hit.sn);
	Vec3f hn = randomBeckmannNormal(ab, sample);
	Vec3f normal = onb.toWorld(hn);
	srec.scattered = normalize(reflect(dirIn, normal));
	srec.isSpecular = false;
	srec.attenuation = albedo->value(hit);
	return dot(hit.sn, srec.scattered) > 0;
}


Color3f Beckmann::eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
	ONBf onb;
	onb.build_from_w(hit.sn);
	Vec3f normal = normalize(-normalize(dirIn) + normalize(scattered));
	Vec3f v1 = -normalize(dirIn);
	Vec3f v2 = normalize(scattered);
	float G2 = G1(-v1, normal, hit) * G1(v2, normal, hit);
	float F = conductorFresnel(abs(dot(v2, normal)), ior, ext);

	float cosTheta = abs(dot(normal, hit.sn));
	float sinTheta = sqrt(1 - cosTheta*cosTheta);
	float tanTheta = sinTheta/cosTheta;
	float nPdf = (exp(-tanTheta*tanTheta / (ab*ab))) / (M_PI*ab*ab*cosTheta*cosTheta*cosTheta*cosTheta);

	return F * albedo->value(hit) * G2 * pdf(dirIn, scattered, hit);
}

float Beckmann::pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const 
{
	float iDotN = dot(-dirIn, hit.sn);
	if (iDotN <= 0.0f || dot(scattered, hit.sn) <= 0.0f)
		return 0.0f;
	Vec3f normal = normalize(-normalize(dirIn) + normalize(scattered));
	
	float cosTheta = abs(dot(normal, hit.sn));
	float sinTheta = sqrt(1 - cosTheta*cosTheta);
	float tanTheta = sinTheta/cosTheta;
	float nPdf = (exp(-tanTheta*tanTheta / (ab*ab))) / (M_PI*ab*ab*cosTheta*cosTheta*cosTheta*cosTheta);
	return nPdf/(4.0f*abs(dot(normalize(dirIn), normalize(normal))));
	return abs(dot(normalize(hit.sn), normal)) * nPdf/(4.0f*abs(dot(normalize(dirIn), normalize(normal))));
}

float Beckmann::G1(Vec3f v1, Vec3f v2, const HitInfo & hit) const
{
	float cosTheta1 = abs(dot(v1, hit.sn));
	float sinTheta1 = sqrt(1 - cosTheta1*cosTheta1);
	float tanTheta1 = sinTheta1/cosTheta1;
	float alpha = 1/(ab*tanTheta1);

	if (alpha < 1.6f) {
		return (3.535f*alpha + 2.181f*alpha*alpha)/(1.0f+2.276f*alpha+2.577f*alpha*alpha);
	} else {
		return 1.0f;
	}
}


OrenNayar::OrenNayar(const json & j)
{
   albedo = parseTexture(j.at("albedo"));
   sigma = j.value("sigma", sigma);
}
 
bool OrenNayar::scatter(const Ray3f &ray, const HitInfo &hit, const Vec2f &sample, Color3f &attenuation, Ray3f &scattered) const
{
   scattered = Ray3f(hit.p, hit.sn + normalize(randomInUnitSphere()));
   attenuation = albedo->value(hit);
   return true;
}
 
bool OrenNayar::sample(const Vec3f & dirIn, const HitInfo &hit, const Vec2f &sample, ScatterRecord &srec) const
{
   ONBf onb;
   onb.build_from_w(hit.sn);
   srec.scattered = onb.toWorld(randomCosineHemisphere(sample));
   srec.isSpecular = false;
   srec.attenuation = albedo->value(hit);
   return true;
}
 
Color3f OrenNayar::eval(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const
{
   float A = 1.0f - (sigma*sigma)/(2.0f*(sigma*sigma + 0.33f));
   float B = (0.45f*sigma*sigma)/(sigma*sigma + 0.09f);
  
   Vec3f inPar = dot(-dirIn, hit.sn)*hit.sn;
   Vec3f inOrth = normalize(-dirIn - inPar);
   Vec3f outPar = dot(scattered, hit.sn)*hit.sn;
   Vec3f outOrth = normalize(scattered - outPar);
 
   float cosTerm = max(-10.0f, dot(inOrth, outOrth));
 
   float sinTanTerm = 0.f;
   float cosI = abs(dot(-normalize(dirIn), normalize(hit.sn)));
   float cosO = abs(dot(normalize(scattered), normalize(hit.sn)));
   if (cosI > cosO) {
       sinTanTerm = sqrt(1 - cosO*cosO) * (sqrt(1 - cosI*cosI))/cosI;
   } else {
       sinTanTerm = sqrt(1 - cosI*cosI) * (sqrt(1 - cosO*cosO))/cosO;
   }

   return albedo->value(hit) * (A + B*cosTerm * sinTanTerm) * max(0.0f, dot(scattered, hit.sn) / M_PI);
}
 
float OrenNayar::pdf(const Vec3f & dirIn, const Vec3f & scattered, const HitInfo & hit) const
{
   return max(0.0f, dot(scattered, hit.sn) / M_PI);
}
