/** \file box.h
    \brief Contains the implementation of a generic, N-dimensional axis-aligned
    Box class.

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

#include <dirt/ray.h>

/// An N-D axis-aligned bounding box consisting of two N-D points pMin and pMax
template <size_t N, typename T>
struct Box
{
    Vec<N,T> pMin;                  ///< The lower-bound of the interval
    Vec<N,T> pMax;                  ///< The upper-bound of the interval

    Box() :
        pMin(std::numeric_limits<T>::max()),
        pMax(std::numeric_limits<T>::lowest()) {}
    explicit Box(const Vec<N,T> &p) : pMin(p), pMax(p) {}
    Box(const Vec<N,T> &p1, const Vec<N,T> &p2)
        : pMin(min(p1, p2)), pMax(max(p1, p2)) {}

    bool isEmpty() const
    {
        for (size_t i = 0; i < N; ++i)
            if (pMin[i] > pMax[i])
                return true;
        return false;
    }

    void enclose(const Box & box2)
    {
        pMin = min(pMin, box2.pMin);
        pMax = max(pMax, box2.pMax);
    }

    void enclose(const Vec<N,T> & p)
    {
        pMin = min(pMin, p);
        pMax = max(pMax, p);
    }

    Vec<N,T> center() const { return (pMin + pMax) / T(2);}
    Vec<N,T> diagonal() const { return pMax - pMin; }

    /**
        Compute the intersection of a Ray with an Box

        \param bounds	The Box to intersect
        \param ray 		The ray along which to check for intersection
        \return 		\c true if there is an intersection
    */
    bool intersect(const Ray<N,T> &ray) const
    {
        T minT = ray.mint;
        T maxT = ray.maxt;

        for (size_t i = 0; i < N; ++i)
        {
            T invD = T(1) / ray.d[i];
            T t0 = (pMin[i] - ray.o[i]) * invD;
            T t1 = (pMax[i] - ray.o[i]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);

            minT = t0 > minT ? t0 : minT;
            maxT = t1 < maxT ? t1 : maxT;
            if (maxT < minT)
                return false;
        }
        return true;
    }
};

template <typename T> using Box2 = Box<2, T>;
template <typename T> using Box3 = Box<3, T>;
template <typename T> using Box4 = Box<4, T>;

using Box2f = Box2<float>;
using Box2d = Box2<double>;
using Box2i = Box2<int32_t>;
using Box2u = Box2<uint32_t>;

using Box3f = Box3<float>;
using Box3d = Box3<double>;
using Box3i = Box3<int32_t>;
using Box3u = Box3<uint32_t>;

using Box4f = Box4<float>;
using Box4d = Box4<double>;
using Box4i = Box4<int32_t>;
using Box4u = Box4<uint32_t>;