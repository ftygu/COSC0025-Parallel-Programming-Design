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
#include <dirt/pdf.h>

/// Simple Kajiya-style path tracer
class PathTracerNEE : public Integrator
{
public:
    PathTracerNEE(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
        m_recursive = j.value("recursive", m_recursive);
    }

    Color3f recursiveColor(const Scene & scene, Sampler & sampler,
    			   const Ray3f& ray,
                           int moreBounces,
                           bool includeEmission) const
    {
        HitInfo hit;
        if (!scene.intersect(ray, hit))
            return scene.background();

        ScatterRecord srec;
        Color3f emitted = includeEmission ? hit.mat->emitted(ray, hit) : Color3f(0.f);

        // 1. if we have no more bounces, just return the emitted color
        if (!moreBounces)
            return emitted;

        // 2. compute direct illumination by sampling the lights
	Vec2f sample = sampler.next2D();
        Vec3f lightDir = normalize(scene.emitters().sample(hit.p, sample));
        Ray3f lightRay(hit.p, lightDir);
        float lightPdf = scene.emitters().pdf(hit.p, lightRay.d);
        Color3f bsdf = hit.mat->eval(ray.d, lightRay.d, hit);
        Color3f direct(0.f);
        if (lightPdf > 0.f && luminance(bsdf) > 0.f)
            direct = bsdf * recursiveColor(scene, sampler, lightRay, 0, true) / lightPdf;

        // 3. now, get indirect illumination by sampling the BSDF
	sample = sampler.next2D();
        if (!hit.mat->sample(ray.d, hit, sample, srec))
            return emitted + direct;

        Ray3f bsdfRay(hit.p, srec.scattered);

        // if the sampled direction is specular, don't use Monte Carlo
        if (srec.isSpecular)
            return emitted + srec.attenuation * recursiveColor(scene, sampler, bsdfRay, moreBounces-1, true);

        // in this case the indirect ray would just return black
        if (moreBounces == 1)
            return emitted + direct;

        float bsdfPdf = hit.mat->pdf(ray.d, bsdfRay.d, hit);
        bsdf = hit.mat->eval(ray.d, bsdfRay.d, hit);

        // exclude light sources in this indirect call since we handle them in step 2
        return emitted + direct + bsdf * recursiveColor(scene, sampler, bsdfRay, moreBounces-1, false) / bsdfPdf;
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        return recursiveColor(scene, sampler, ray_, m_maxBounces, true);
    }

private:
    int m_maxBounces = 64;
    bool m_recursive = true;
}; 
