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
class PathTracerMIS : public Integrator
{
public:
    PathTracerMIS(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
        m_recursive = j.value("recursive", m_recursive);
        m_power = j.value("power", m_power);
    }

    /// Power heuristic
    inline float powerHeuristic(float pdfA, float pdfB) const
    {
        pdfA = powf(pdfA, m_power);
        pdfB = powf(pdfB, m_power);
        return pdfA / (pdfA + pdfB);
    }

    /// Max heuristic
    inline float maxHeuristic(float pdfA, float pdfB) const
    {
        return pdfA > pdfB ? 1.f : 0.f;
    }

    Color3f recursiveColor(const Scene & scene, Sampler &sampler, const Ray3f& ray,
                           int moreBounces,
                           float emissionWeight) const
    {
        HitInfo hit;
        if (!scene.intersect(ray, hit))
            return scene.background();

        ScatterRecord srec;
        Color3f emitted = emissionWeight * hit.mat->emitted(ray, hit);

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
        {
            float bsdfPdf = hit.mat->pdf(ray.d, lightRay.d, hit);
            direct = bsdf * recursiveColor(scene, sampler, lightRay, 0, powerHeuristic(lightPdf, bsdfPdf)) / lightPdf;
        }

        // 3. now, get indirect illumination by sampling the BSDF
        sample = sampler.next2D();
        if (!hit.mat->sample(ray.d, hit, sample, srec))
            return emitted + direct;

        Ray3f bsdfRay(hit.p, srec.scattered);

        // if the sampled direction is specular, don't use Monte Carlo
        // and set the MIS weight to 1 since we can't sample it any other way
        if (srec.isSpecular)
            return emitted + srec.attenuation * recursiveColor(scene, sampler, bsdfRay, moreBounces-1, 1.f);

        float bsdfPdf = hit.mat->pdf(ray.d, bsdfRay.d, hit);
        lightPdf = scene.emitters().pdf(hit.p, bsdfRay.d);
        bsdf = hit.mat->eval(ray.d, bsdfRay.d, hit);

        return emitted + direct + bsdf * recursiveColor(scene, sampler, bsdfRay, moreBounces-1, powerHeuristic(bsdfPdf, lightPdf)) / bsdfPdf;
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        return recursiveColor(scene, sampler, ray_, m_maxBounces, 1.f);
    }

private:
    int m_maxBounces = 64;
    bool m_recursive = true;
    float m_power = 1.f;
};
