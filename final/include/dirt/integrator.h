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

#include <dirt/fwd.h>
#include <dirt/common.h>
#include <dirt/vec.h>
#include <dirt/scene.h>

/**
    An abstract class encapsulating an integrator (i.e.\ a rendering technique)
   
    In Dirt, the different rendering techniques are collectively referred to as 
    integrators, since they perform integration over a high-dimensional
    space. Each integrator represents a specific approach for solving
    the light transport equation.
 */
class Integrator
{
public:
    /// Release all memory
    virtual ~Integrator() { }

    /// Perform an (optional) preprocess step
    virtual void preprocess(const Scene *scene) { }

    /**
        Sample the incident radiance along a ray
       
        \param scene
           A pointer to the underlying scene
        \param sampler
            A pointer to a sampler
        \param ray
           The ray in question
        \return
           An estimate of the radiance in this direction
     */
    virtual Color3f Li(const Scene &scene, Sampler &sampler, const Ray3f &ray) const;
};
