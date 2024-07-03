#pragma once

#include <dirt/fwd.h>
#include <dirt/common.h>
#include <dirt/ray.h>
#include <dirt/perlin.h>
#include <vector>

enum SamplingColor { red, green, blue };

struct AllProb {
public:
    AllProb() {
        pr_fr = 1.0f;
        pg_fr = 1.0f;
        pb_fr = 1.0f;
        pr_fg = 1.0f;
        pg_fg = 1.0f;
        pb_fg = 1.0f;
        pr_fb = 1.0f;
        pg_fb = 1.0f;
        pb_fb = 1.0f;
    }
    
    AllProb(float p) {
        pr_fr = p;
        pg_fr = p;
        pb_fr = p;
        pr_fg = p;
        pg_fg = p;
        pb_fg = p;
        pr_fb = p;
        pg_fb = p;
        pb_fb = p;
    }

    AllProb(float r1, float r2, float r3, float g1, float g2, float g3, float b1, float b2, float b3)
        : pr_fr(r1), pg_fr(r2), pb_fr(r3), pr_fg(g1), pg_fg(g2), pb_fg(g3), pr_fb(b1), pg_fb(b2), pb_fb(b3) {}

    void operator*= (const AllProb& other) {
        pr_fr *= other.pr_fr;
        pg_fr *= other.pg_fr;
        pb_fr *= other.pb_fr;
        pr_fg *= other.pr_fg;
        pg_fg *= other.pg_fg;
        pb_fg *= other.pb_fg;
        pr_fb *= other.pr_fb;
        pg_fb *= other.pg_fb;
        pb_fb *= other.pb_fb;
    }

	void operator*= (const float& f) {
        pr_fr *= f;
        pg_fr *= f;
        pb_fr *= f;
        pr_fg *= f;
        pg_fg *= f;
        pb_fg *= f;
        pr_fb *= f;
        pg_fb *= f;
        pb_fb *= f;
    }

	void operator*= (const Color3f& c) {
		//cout << *this << c;
        pr_fr *= c.r;
        pg_fr *= c.r;
        pb_fr *= c.r;
        pr_fg *= c.g;
        pg_fg *= c.g;
        pb_fg *= c.g;
        pr_fb *= c.b;
        pg_fb *= c.b;
        pb_fb *= c.b;
		// cout << *this << endl;
    }

	friend std::ostream& operator<<(std::ostream& os, const AllProb& t) {
		return os << t.pr_fr << " " << t.pg_fr << " " << t.pb_fr << " " << t.pr_fg << " " << t.pg_fg << " " << t.pb_fg << " " << t.pr_fb << " " << t.pg_fb << " " << t.pb_fb;
	}

	bool hitMedium = true;
    float pr_fr, pg_fr, pb_fr;
    float pr_fg, pg_fg, pb_fg;
    float pr_fb, pg_fb, pb_fb;
};

struct MediumInteraction
{
	Vec3f p;
	Vec3f wo;
	const Medium* medium = nullptr;

	MediumInteraction() {};

	MediumInteraction(
			const Vec3f &p,
			const Vec3f &wo,
			const Medium* medium
			)
		: p(p)
		  , wo(wo)
		  , medium(medium) {};

	bool isValid() { return medium != nullptr; };
};

inline Vec3f SphericalDirection(float sinTheta, float cosTheta, float phi)
{
	return Vec3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi), cosTheta);
}
class PhaseFunction {
	public:
		virtual ~PhaseFunction() = default;

		virtual float p(const Vec3f &wo, const Vec3f &wi) const = 0;

		virtual float sample(const Vec3f &wo, Vec3f &wi, const Vec2f &sample) const = 0;
};

class HenyeyGreenstein : public PhaseFunction
{
	public:
		HenyeyGreenstein(const json & j = json::object());

		float p(const Vec3f &wo, const Vec3f &wi) const override;

		float sample(const Vec3f &wo, Vec3f &wi, const Vec2f &sample) const override;

	private:
		float g = 0;
};

class Medium
{
	public:
		virtual ~Medium() = default;

		virtual float Tr(const Ray3f &ray, Sampler &sampler, SamplingColor color) const = 0;

		virtual AllProb Sample(const Ray3f &ray, Sampler &sampler, MediumInteraction &mi, SamplingColor color) const = 0;

		virtual float density(const Vec3f &p, SamplingColor color) const = 0;

		std::shared_ptr<PhaseFunction> phase;
};

class HomogeneousMedium: public Medium
{
	public:
		HomogeneousMedium(const json &j = json::object());

		float Tr(const Ray3f &ray, Sampler &sampler, SamplingColor color) const;

		AllProb Sample(const Ray3f &ray, Sampler &sampler, MediumInteraction &mi, SamplingColor color) const;

		float density(const Vec3f &p, SamplingColor color) const;

	private:
		Color3f sigma_a, sigma_s, sigma_t;
};

class PerlinMedium: public Medium
{
	public:
		PerlinMedium(const json &j = json::object());

		float Tr(const Ray3f &ray, Sampler &sampler, SamplingColor color) const;

		AllProb Sample(const Ray3f &ray, Sampler &sampler, MediumInteraction &mi, SamplingColor color) const;

	private:
		float density(const Vec3f &p, SamplingColor color) const;

		// params
		Color3f sigma_a, sigma_s;
		float densityScale = 1.0f;
		float densityOffset = 0.0f;
		vector<Vec3f> spatialScale;
		

		// computed
		Color3f invMaxDensity, sigma_t;

		Perlin perlin;
};

// MediumInterface Declarations
struct MediumInterface
{
	MediumInterface() : inside(nullptr), outside(nullptr) {}

	MediumInterface(const std::shared_ptr<const Medium> medium) : inside(medium), outside(medium) {}

	MediumInterface(const std::shared_ptr<const Medium> inside, const std::shared_ptr<const Medium> outside): inside(inside), outside(outside) {}

	bool IsMediumTransition() const { return inside != outside; }

	std::shared_ptr<const Medium> getMedium(const Ray3f ray, const HitInfo &hit) const;

	std::shared_ptr<const Medium> inside, outside;
};

Color3f TrL(const Scene &scene, Sampler &sampler, const Ray3f &ray_, SamplingColor color);
