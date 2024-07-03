#include <dirt/medium.h>
#include <dirt/parser.h>
#include <dirt/onb.h>
#include <dirt/ray.h>
#include <dirt/scene.h>
#include <dirt/sampler.h>

HenyeyGreenstein::HenyeyGreenstein(const json &j)
{
	g = j.value("g", g);
}

// HenyeyGreenstein Method Definitions
float HenyeyGreenstein::p(const Vec3f &wo, const Vec3f &wi) const
{
	float cosTheta = dot(normalize(wo), normalize(wi));
	float denom = 1.0 + g * g + 2.0 * g * cosTheta;
	return INV_FOURPI * (1.0 - g * g) / (denom * std::sqrt(denom));
}

float HenyeyGreenstein::sample(const Vec3f &wo, Vec3f &wi, const Vec2f &u) const
{
	float cosTheta;
	if (std::abs(g) < 1e-3)
	{
		cosTheta = 1.0 - 2.0 * u.x;
	}
	else
	{
		float sqrTerm = (1.0 - g * g) / (1.0 + g - 2.0 * g * u.x);
		cosTheta = -(1.0 + g * g - sqrTerm * sqrTerm) / (2.0 * g);
	}

	// Compute direction _wi_ for Henyey--Greenstein sample
	float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
	float phi = 2 * M_PI * u.y;

	ONB<float> onb;
	onb.build_from_w(wo);
	wi = normalize(onb.toWorld(SphericalDirection(sinTheta, cosTheta, phi)));
	return p(wo, wi);
}

HomogeneousMedium::HomogeneousMedium(const json &j)
{
	phase = parsePhase(j.at("phase"));
	sigma_a = j.value("sigma_a", sigma_a);
	sigma_s = j.value("sigma_s", sigma_s);
	sigma_t = sigma_s + sigma_a;
}

float HomogeneousMedium::Tr(const Ray3f &ray_, Sampler &sampler, SamplingColor color) const
{
	Ray3f ray = ray_.normalizeRay();
	return std::exp(-sigma_t[color] * (ray.maxt - ray.mint));
}

AllProb HomogeneousMedium::Sample(const Ray3f &ray_, Sampler &sampler, MediumInteraction &mi, SamplingColor color) const
{
	Ray3f ray = ray_.normalizeRay();
	float dist = -std::log(1.0f - sampler.next1D()) / sigma_t[color];
	float t = std::min(dist, ray.maxt);
	bool sampledMedium = t < ray.maxt;
	if (sampledMedium)
		mi = MediumInteraction(ray(t), -ray.d, this);
	return sampledMedium ? AllProb(sigma_s[color] / sigma_t[color]) : AllProb(1.0f);
}

float HomogeneousMedium::density(const Vec3f &p, SamplingColor color) const
{
	return sigma_t[color];
}

PerlinMedium::PerlinMedium(const json &j)
{
	phase = parsePhase(j.at("phase"));
	sigma_a = j.value("sigma_a", sigma_a);
	sigma_s = j.value("sigma_s", sigma_s);
	sigma_t = sigma_s + sigma_a;

	spatialScale.push_back(j.value("spatial_scale0", Vec3f(1.0f)));
	spatialScale.push_back(j.value("spatial_scale1", Vec3f(1.0f)));
	spatialScale.push_back(j.value("spatial_scale2", Vec3f(1.0f)));
	densityScale = abs(j.value("density_scale", densityScale));
	densityOffset = j.value("density_offset", densityOffset);

	assert(densityScale + densityOffset > 0.0f);

	invMaxDensity = 1.0f / (sigma_t * (densityScale + densityOffset));
	//invMaxDensity = 1.0f / (sigma_t * 2.0f);
}

float PerlinMedium::Tr(const Ray3f &ray_, Sampler &sampler, SamplingColor color) const
{
	Ray3f ray = ray_.normalizeRay();
	float Tr = 1;
	float t = ray.mint;
	while (true) {
		t -= std::log(1.0 - sampler.next1D()) * invMaxDensity[color];
		if (t * invMaxDensity[color] >= ray.maxt) break;
		Tr *= 1.0 - std::max(0.0f,  density(ray(t), color) * invMaxDensity[color]);
	}

	return Tr;
}

float T(float sigma_t, float dist) {
	return exp(-sigma_t * abs(dist));
}

AllProb PerlinMedium::Sample(const Ray3f &ray_, Sampler &sampler, MediumInteraction &mi, SamplingColor color) const
{
	Ray3f ray = ray_.normalizeRay();
	float t = ray.mint, dist = 0.0f;
    AllProb probs(1.0f);

	while (true)
	{
		dist = -std::log(1.0f - sampler.next1D()) * invMaxDensity[color];
		t += dist;
		if (ray.maxt <= t) break;
		// cout <<  density(ray(t), color) * invMaxDensity[color] << endl;

		float t_r = T(1.0f / invMaxDensity[red], dist);
		float t_g = T(1.0f / invMaxDensity[green], dist);
		float t_b = T(1.0f / invMaxDensity[blue], dist);

		// Real collision, multiply by p(real)/f(real)
		if (sampler.next1D() < density(ray(t), color) * invMaxDensity[color])
		{
			float f_r_real = density(ray(t), red);
			float f_g_real = density(ray(t), green);
			float f_b_real = density(ray(t), blue);

			float p_r_real = f_r_real * invMaxDensity[red];
			float p_g_real = f_g_real * invMaxDensity[green];
			float p_b_real = f_b_real * invMaxDensity[blue];

			//cout << density(ray(t), red) << " " << 1.0f / invMaxDensity[red] << " " << p_r_real << " " << f_r_real << " " << p_r_real / f_r_real << endl;
			
			probs.pr_fr *= p_r_real / f_r_real;
			probs.pr_fg *= p_r_real / f_g_real;
			probs.pr_fb *= p_r_real / f_b_real;
			probs.pg_fr *= p_g_real / f_r_real;
			probs.pg_fg *= p_g_real / f_g_real;
			probs.pg_fb *= p_g_real / f_b_real;
			probs.pb_fr *= p_b_real / f_r_real;
			probs.pb_fg *= p_b_real / f_g_real;
			probs.pb_fb *= p_b_real / f_b_real;

			// cout << probs << endl;

			mi = MediumInteraction(ray(t), -ray.d, this);
			probs.pr_fr *= sigma_t[red] / sigma_s[red];
			probs.pr_fg *= sigma_t[green] / sigma_s[green];
			probs.pr_fb *= sigma_t[blue] / sigma_s[blue];
			probs.pg_fr *= sigma_t[red] / sigma_s[red];
			probs.pg_fg *= sigma_t[green] / sigma_s[green];
			probs.pg_fb *= sigma_t[blue] / sigma_s[blue];
			probs.pb_fr *= sigma_t[red] / sigma_s[red];
			probs.pb_fg *= sigma_t[green] / sigma_s[green];
			probs.pb_fb *= sigma_t[blue] / sigma_s[blue];

			probs.pr_fr *= (t_r / invMaxDensity[red]) / t_r;
			probs.pr_fg *= (t_r / invMaxDensity[red]) / t_g;
			probs.pr_fb *= (t_r / invMaxDensity[red]) / t_b;
			probs.pg_fr *= (t_g / invMaxDensity[green]) / t_r;
			probs.pg_fg *= (t_g / invMaxDensity[green]) / t_g;
			probs.pg_fb *= (t_g / invMaxDensity[green]) / t_b;
			probs.pb_fr *= (t_b / invMaxDensity[blue]) / t_r;
			probs.pb_fg *= (t_b / invMaxDensity[blue]) / t_g;
			probs.pb_fb *= (t_b / invMaxDensity[blue]) / t_b;
			return probs;
		}

		//Null collision, multiply by p(t)/f(t) * p(null)/f(null)

		//Null density is total density - real density
		float null_r = 1.0f/invMaxDensity[red] - density(ray(t), red);
		float null_g = 1.0f/invMaxDensity[green] - density(ray(t), green);
		float null_b = 1.0f/invMaxDensity[blue] - density(ray(t), blue);

		probs.pr_fr *= 1.0f; //Perfectly importance sampled
		probs.pr_fg *= ((t_r / invMaxDensity[red]) / t_g) * ((null_r * invMaxDensity[red]) / null_g);
		probs.pr_fb *= ((t_r / invMaxDensity[red]) / t_b) * ((null_r * invMaxDensity[red]) / null_b);

		probs.pg_fr *= ((t_g / invMaxDensity[green]) / t_r) * ((null_g * invMaxDensity[green]) / null_r);
		probs.pg_fg *= 1.0f; //Perfectly importance sampled
		probs.pg_fb *= ((t_g / invMaxDensity[green]) / t_b) * ((null_g * invMaxDensity[green]) / null_b);

		probs.pb_fr *= ((t_b / invMaxDensity[blue]) / t_r) * ((null_b * invMaxDensity[blue]) / null_r);
		probs.pb_fg *= ((t_b / invMaxDensity[blue]) / t_g) * ((null_b * invMaxDensity[blue]) / null_g);
		probs.pb_fb *= 1.0f; //Perfectly importance sampled
	}
	
	return AllProb(1.0f);
}

float PerlinMedium::density(const Vec3f &p, SamplingColor color) const
{
	Vec3f pScaled(p.x * spatialScale[color].x, p.y * spatialScale[color].y, p.z * spatialScale[color].z);
  	return sigma_t[color] * std::max(0.0f, densityScale * perlin.noise(pScaled) + densityOffset);
}

std::shared_ptr<const Medium> MediumInterface::getMedium(const Ray3f ray, const HitInfo &hit) const
{
	if (dot(hit.sn, ray.d) < 0)
		return inside;
	else
		return outside;
}

Color3f TrL(const Scene &scene, Sampler &sampler, const Ray3f &ray_, SamplingColor color)
{
	Ray3f ray = ray_.normalizeRay();
	float Tr = 1.0;
	while (true)
	{
		HitInfo hit;
		bool hitSurface = scene.intersect(ray, hit);

		if (hitSurface) ray.maxt = length(hit.p - ray.o) + 2.0 * Epsilon;

		// hit an emitter
		if (hitSurface && hit.mat != nullptr)
			return hit.mat->isEmissive() ? Tr * hit.mat->emitted(ray, hit) : Color3f(0.0f);

		// stil in medium
		if (ray.medium) Tr *= ray.medium->Tr(ray, sampler, color);

		// if transmittance below threshold exit
		if (Tr < Epsilon) break;

		// escaped scene (assume no infinite lights)
		if (!hitSurface)
			return Tr * scene.background();

		// set medium based on whether we are entering or exiting the surface
		if (hit.mi->IsMediumTransition())
			ray.medium = hit.mi->getMedium(ray, hit);

		// update ray origin
		ray.o = hit.p;
	}

	return Color3f(0.0f);
}
