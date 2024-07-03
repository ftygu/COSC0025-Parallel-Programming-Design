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

#include <dirt/integrator.h>
#include <dirt/medium.h>
#include <dirt/scene.h>
#include <dirt/pdf.h>

class Original: public Integrator
{
public:
    Original(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
    }

    void attachMedium(const HitInfo &hit, Ray3f &ray) const
    {
        if (hit.mat == nullptr && hit.mi->IsMediumTransition())
            ray.medium = hit.mi->getMedium(ray, hit);
        ray.maxt = length(hit.p - ray.o) / length(ray.d) + Epsilon; 
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        Ray3f ray(ray_);
        Color3f throughput(1.f);
        Color3f result(0.f);

        int bounces = 0;

        float epsilon = sampler.next1D();
        SamplingColor color;
        if(epsilon < 1.0f / 3.0f)
            color = red;
        else if(epsilon < 2.0f / 3.0f)
            color = green;
        else
            color = blue;

        while (bounces <= m_maxBounces)
        {
            HitInfo hit;
            bool foundIntersection = scene.intersect(ray, hit);
            if (foundIntersection) attachMedium(hit, ray);

            MediumInteraction mi;
            if (ray.medium) {
                AllProb probs = ray.medium->Sample(ray, sampler, mi, color);
				throughput *= Vec3f(probs.pr_fr, probs.pg_fg, probs.pb_fb);
                // cout << throughput << endl;
            }

            if (mi.isValid())
            {
                // sample direction (from phase)
                Vec3f wi;
                float phasePdf = mi.medium->phase->sample(mi.wo, wi, sampler.next2D());
                throughput *= mi.medium->phase->p(mi.wo, wi) / phasePdf;

                // spawn new ray
                ray = Ray3f(mi.p, wi).withMedium(ray.medium);
                bounces++;
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

                result += throughput * hit.mat->emitted(ray, hit);

                // sample direction (from BSDF)
                ScatterRecord srec;
                if (!hit.mat->sample(ray.d, hit, sampler.next2D(), srec)) break;

                Ray3f scat(hit.p, srec.scattered);
                if (!srec.isSpecular) 
                {
                    float bsdfPdf = hit.mat->pdf(ray.d, scat.d, hit);
                    if (bsdfPdf == 0.0f) break;
                    throughput *= hit.mat->eval(ray.d, scat.d, hit) / bsdfPdf;
                }
                else
                {
                    throughput *= srec.attenuation;
                }

                ray = scat;
                bounces++;
            }

            float lum = luminance(throughput);
            const float rrThreshold = .1;
	        
            // TODO: Part 1
            float q = max(0.05f, lum);
            if(q < 1.0f) {
                if(sampler.next1D() < 1 - q) {
                    // cout << result <<"SAD"<< endl;
                    break;
                }
                else {
                    throughput = 1.0f / q * throughput;
                }
            }
        }

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

private:
    int m_maxBounces = 64;
};