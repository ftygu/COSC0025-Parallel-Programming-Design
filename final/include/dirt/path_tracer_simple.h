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
class PathTracerSimple : public Integrator
{
public:
    PathTracerSimple(const json& j = json::object())
    {
        m_maxBounces = j.value("max_bounces", m_maxBounces);
    }

    virtual Color3f Li(const Scene & scene, Sampler &sampler, const Ray3f& ray_) const override
    {
        HitInfo hit;
        Ray3f ray(ray_);
        Ray3f scattered;
        Color3f attenuation;
        Color3f throughput(1.f);
        Color3f result(0.f);
        int depth = 0;

        while (depth++ < m_maxBounces+1)
        {
            if (!scene.intersect(ray, hit))
            {
                result += throughput * scene.background();
                break;
            }

            result += throughput * hit.mat->emitted(ray, hit);
            Vec2f sample = sampler.next2D();
            if (hit.mat->scatter(ray, hit, sample, attenuation, scattered))
            {
                ray = scattered;
                throughput *= attenuation;
            }
            else
                break;
        }

        return result;
    }

private:
    int m_maxBounces = 64;
};
