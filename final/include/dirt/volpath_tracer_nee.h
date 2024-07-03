#pragma once

#include <dirt/integrator.h>
#include <dirt/medium.h>
#include <dirt/scene.h>
#include <dirt/pdf.h>

class VolpathTracerNEE: public Integrator
{
public:
    VolpathTracerNEE(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
        m_power = j.value("mis_power", m_power);
    }

    float misWeight(float pdfA, float pdfB) const
    {
        pdfA = std::pow(pdfA, m_power);
        pdfB = std::pow(pdfB, m_power);
        return pdfA / (pdfA + pdfB);
    }

    void attachMedium(const HitInfo &hit, Ray3f &ray) const
    {
        if (hit.mat == nullptr && hit.mi->IsMediumTransition())
            ray.medium = hit.mi->getMedium(ray, hit);
        ray.maxt = length(hit.p - ray.o) / length(ray.d) + Epsilon; 
    }

    Color3f surfaceNEE(const Scene &scene, Sampler &sampler, const HitInfo &hit, const Ray3f &ray, const Color3f &throughput) const
    {
        Color3f result(0.0f);

        float epsilon = sampler.next1D();
        SamplingColor color;
        if(epsilon < 1.0f / 3.0f)
            color = red;
        else if(epsilon < 2.0f / 3.0f)
            color = green;
        else
            color = blue;

        Vec3f lightSample = scene.emitters().sample(hit.p, sampler.next2D());
        Ray3f scat = Ray3f(hit.p, normalize(lightSample));
        float lightPdf = scene.emitters().pdf(hit.p, scat.d);
        if (lightPdf > 0.0f)
        {
            float bsdfPdf = hit.mat->pdf(ray.d, scat.d, hit);
            result += misWeight(lightPdf, bsdfPdf) * throughput * hit.mat->eval(ray.d, scat.d, hit) * TrL(scene, sampler, scat, color) / lightPdf;
        }

        ScatterRecord srec;
        if (hit.mat->sample(ray.d, hit, sampler.next2D(), srec))
        {
            Ray3f scat = Ray3f(hit.p, srec.scattered).withMedium(ray.medium);
            float bsdfPdf = hit.mat->pdf(ray.d, scat.d, hit);
            if (bsdfPdf > 0.0f)
            {
                float lightPdf = scene.emitters().pdf(hit.p, scat.d);
                result += misWeight(bsdfPdf, lightPdf) * throughput * hit.mat->eval(ray.d, scat.d, hit) * TrL(scene, sampler, scat, color) / bsdfPdf;
            }
        }
        return result;
    }

    Color3f mediumNEE(const Scene &scene, Sampler &sampler, const MediumInteraction &mi, const Ray3f &ray, const Color3f &throughput) const
    {
        Color3f result(0.0f);

        float epsilon = sampler.next1D();
        SamplingColor color;
        if(epsilon < 1.0f / 3.0f)
            color = red;
        else if(epsilon < 2.0f / 3.0f)
            color = green;
        else
            color = blue;

        Vec3f lightSample = scene.emitters().sample(mi.p, sampler.next2D());
        Ray3f scat = Ray3f(mi.p, normalize(lightSample)).withMedium(ray.medium);
        float lightPdf = scene.emitters().pdf(mi.p, scat.d);
        if (lightPdf > 0.0f)
        {
            float phasePdf = mi.medium->phase->p(mi.wo, scat.d);
            result += misWeight(lightPdf, phasePdf) * throughput * mi.medium->phase->p(mi.wo, scat.d) * TrL(scene, sampler, scat, color) / lightPdf;
        }

        Vec3f wi;
        float phasePdf = mi.medium->phase->sample(mi.wo, wi, sampler.next2D());
        if (phasePdf > 0.0f)
        {
            Ray3f scat = Ray3f(mi.p, wi).withMedium(ray.medium);
            float lightPdf = scene.emitters().pdf(mi.p, scat.d);
            result += misWeight(phasePdf, lightPdf) * throughput * mi.medium->phase->p(mi.wo, scat.d) * TrL(scene, sampler, scat, color) / phasePdf;
        }

        return result;
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        Ray3f ray(ray_);
        Color3f throughput(1.f);
        Color3f result(0.f);
        Vec3f wi;

        float epsilon = sampler.next1D();
        SamplingColor color;
        if(epsilon < 1.0f / 3.0f)
            color = red;
        else if(epsilon < 2.0f / 3.0f)
            color = green;
        else
            color = blue;
        AllProb probs;

        // first compute primary ray connection to light source
        HitInfo hit;
        Ray3f primaryRay(ray_);
        if (scene.intersect(primaryRay, hit)) attachMedium(hit, primaryRay);
        result += TrL(scene, sampler, primaryRay, color);

        int bounces = 0;

        while (bounces < m_maxBounces)
        {
            HitInfo hit;
            bool foundIntersection = scene.intersect(ray, hit);
            if (foundIntersection)
                attachMedium(hit, ray);

            MediumInteraction mi;
            if (ray.medium)
                probs *= ray.medium->Sample(ray, sampler, mi, color);

            if (mi.isValid())
            {
                result += mediumNEE(scene, sampler, mi, ray, throughput);
                float phasePdf = mi.medium->phase->sample(mi.wo, wi, sampler.next2D());
                throughput *= mi.medium->phase->p(mi.wo, wi) / phasePdf;
                ray = Ray3f(mi.p, wi).withMedium(ray.medium);
                bounces ++;
            }
            else
            {
                if (!foundIntersection)
                {
                    result += throughput * scene.background();
                    break;
                }

                if (hit.mat == nullptr)
                {
                    ray = Ray3f(hit.p, normalize(ray.d));
                    continue;
                }

                ScatterRecord srec;
                if (!hit.mat->sample(ray.d, hit, sampler.next2D(), srec)) break;
                Ray3f scat = Ray3f(hit.p, srec.scattered).withMedium(ray.medium);

                if (srec.isSpecular)
                {
                    throughput *= srec.attenuation;
                    result += throughput * TrL(scene, sampler, scat, color);
                }
                else
                {
                    result += surfaceNEE(scene, sampler, hit, ray, throughput);
                    float bsdfPdf = hit.mat->pdf(ray.d, scat.d, hit);
                    if (bsdfPdf == 0.0f) break;
                    throughput *= hit.mat->eval(ray.d, scat.d, hit) / bsdfPdf;
                }

                ray = scat;
                bounces ++;
            }
	
            float lum = luminance(throughput);
            const float rrThreshold = .1;

	        // TODO: Part 1
            float q = max(0.05f, lum);
            if(q < 1.0f) {
                if(sampler.next1D() < 1 - q) {
                    switch(color) {
                        case red:
                            result.r *= 3;
                            result.g = 0;
                            result.b = 0;
                            return result;
                        case green:
                            result.r = 0;
                            result.g *= 3;
                            result.b = 0;
                            return result;
                        case blue:
                            result.r = 0;
                            result.g = 0;
                            result.b *= 3;
                            return result;
                    } 
                }
                else {
                    throughput = 1.0f / q * throughput;
                }
            }
        }
        return result;
    }

private:
    int m_maxBounces = 64;
    float m_power = 2;
};
