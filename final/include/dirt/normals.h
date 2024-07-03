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

class NormalIntegrator : public Integrator
{
public:
    NormalIntegrator(const json& j = json::object())
    {
        /* No parameters this time */
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray) const override
    {
        // Find the surface that is visible in the requested direction
        HitInfo hit;
        if (!scene.intersect(ray, hit))
            return Color3f(0.0f);

        // Return the component-wise absolute value of the normal as a color
        return Color3f(fabs(hit.sn.x), fabs(hit.sn.y), fabs(hit.sn.z));
    }
};
