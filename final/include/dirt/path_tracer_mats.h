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
#include <dirt/scene.h>
#include <dirt/sampler.h>

/// Simple Kajiya-style path tracer
class PathTracerMats : public Integrator
{
public:
    PathTracerMats(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
        m_recursive = j.value("recursive", m_recursive);
    }

    Color3f recursiveColor(const Scene & scene, Sampler &sampler, const Ray3f& ray,
                           int moreBounces) const
    {
        HitInfo hit;
        if (!scene.intersect(ray, hit))
            return scene.background();

        ScatterRecord srec;
        Color3f emitted = hit.mat->emitted(ray, hit);

        // 1. if we have no more bounces, just return the emitted color
        if (!moreBounces)
            return emitted;

        // 2. compute illumination by sampling the BSDF
        Vec2f sample = sampler.next2D();
        if (!hit.mat->sample(ray.d, hit, sample, srec))
            return emitted;

        Ray3f bsdfRay(hit.p, srec.scattered);

        // if the sampled direction is specular, don't use Monte Carlo
        if (srec.isSpecular)
            return emitted + srec.attenuation * recursiveColor(scene, sampler, bsdfRay, moreBounces-1);

        float bsdfPdf = hit.mat->pdf(ray.d, bsdfRay.d, hit);
        if (bsdfPdf == 0.0f)
            return emitted;
        Color3f bsdf = hit.mat->eval(ray.d, bsdfRay.d, hit);

        return emitted + bsdf * recursiveColor(scene, sampler, bsdfRay, moreBounces-1) / bsdfPdf;
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        if (m_recursive)
            return recursiveColor(scene, sampler, ray_, m_maxBounces);

        HitInfo hit;
        Ray3f ray(ray_);
        Color3f throughput(1.f);
        Color3f result(0.f);
        int depth = 0;

        while (depth++ < m_maxBounces)
        {
            if (!scene.intersect(ray, hit))
            {
                result += throughput * scene.background();
                break;
            }

            ScatterRecord srec;
            result += throughput * hit.mat->emitted(ray, hit);

            Vec2f sample = sampler.next2D();
            if (hit.mat->sample(ray.d, hit, sample, srec))
            {
                if (!srec.isSpecular)
                {
                    Ray3f scat(hit.p, srec.scattered);
                    float pdf = hit.mat->pdf(ray.d, scat.d, hit);
                    if (pdf == 0.0f)
                        break;
                    Color3f value = hit.mat->eval(ray.d, scat.d, hit);
                    throughput *= value / pdf;
                    ray = scat;
                }
                else
                {
                    ray = Ray3f(hit.p, srec.scattered);
                    throughput *= srec.attenuation;
                }
            }
            else
                break;
        }

        return result;
    }

private:
    int m_maxBounces = 64;
    bool m_recursive = true;
};
