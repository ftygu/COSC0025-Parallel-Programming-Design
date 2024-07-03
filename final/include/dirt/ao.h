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
#include <dirt/sampler.h>
#include <dirt/sampling.h>
#include <dirt/scene.h>

/**
 * \brief Ambient occlusion: a very simple rendering technique that
 * approximates ambient lighting by accounting for local shadowing.
 */
class AmbientOcclusion : public Integrator
{
public:
    AmbientOcclusion(const json& j = json::object())
    {
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray) const override
    {
        HitInfo hit;
        if (!scene.intersect(ray, hit))
            return Color3f(0.0f);

        ScatterRecord record;
        Vec2f sample = sampler.next2D();
        if (!hit.mat->sample(ray.d, hit, sample, record))
            return Color3f(0.0f);

        Ray3f shadowRay(hit.p, record.scattered, Epsilon, 1e30f);
        if (scene.intersect(shadowRay, hit))
            return Color3f(0.0f);

        return Color3f(1.0f);
    }
};
