/** \file ray.h
    \brief Contains the definition of a generic, N-dimension Ray class.

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

#include <dirt/vec.h>
#include <dirt/fwd.h>

/// "Ray epsilon": relative error threshold for ray intersection computations
#define Epsilon 0.001f

/**
    Simple ray segment data structure.

    Along with the ray origin and direction, this data structure additionally
    stores the segment interval [mint, maxt] (whose entries may include
    positive/ negative infinity).
 */
template <size_t N, typename T>
struct Ray
{
    Vec<N,T> o;     ///< The origin of the ray
    Vec<N,T> d;     ///< The direction of the ray
    T mint;         ///< Minimum distance along the ray segment
    T maxt;         ///< Maximum distance along the ray segment
    std::shared_ptr<const Medium> medium = nullptr;

    /// Construct a new ray
    Ray() : mint(Epsilon),
        maxt(std::numeric_limits<T>::infinity()) { }

    /// Construct a new ray
    Ray(const Vec<N,T> &o, const Vec<N,T> &d) : o(o), d(d),
            mint(Epsilon), maxt(std::numeric_limits<T>::infinity()) { }

    /// Construct a new ray
    Ray(const Vec<N,T> &o, const Vec<N,T> &d,
        T mint, T maxt) : o(o), d(d), mint(mint), maxt(maxt) { }

    /// Copy a ray, but change the covered segment of the copy
    Ray(const Ray &ray, T mint, T maxt)
     : o(ray.o), d(ray.d), mint(mint), maxt(maxt), medium(ray.medium) { }

    /// Return the position of a point along the ray
    Vec<N,T> operator() (T t) const { return o + t * d; }
    
    Ray<N, T> normalizeRay() const
    {
        float rayLength = length(d);

        Vec3f normalizedRayDir = d / rayLength;
        float normalizedMint = mint * rayLength;
        float normalizedMaxt = maxt * rayLength;

        return Ray(o, normalizedRayDir, normalizedMint, normalizedMaxt);
    }

    Ray<N, T> withMedium(std::shared_ptr<const Medium> medium)
    {
        this->medium = medium;
        return *this;
    }
};

template <typename T> using Ray2 = Ray<2, T>;
template <typename T> using Ray3 = Ray<3, T>;

using Ray2f   = Ray2<float>;
using Ray2d   = Ray2<double>;

using Ray3f   = Ray3<float>;
using Ray3d   = Ray3<double>;
