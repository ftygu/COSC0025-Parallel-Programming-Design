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

    ------------------------------------------------------------------------

    This file is based on the Transform class from Nori:

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <dirt/common.h>
#include <dirt/box.h>

/**
    Homogeneous coordinate transformation

    This class stores a general homogeneous coordinate transformation, such as
    rotation, translation, uniform or non-uniform scaling, and perspective
    transformations. The inverse of this transformation is also recorded
    here, since it is required when transforming normal vectors.
*/
struct Transform
{
    Mat44f m;
    Mat44f mInv;

    /// Create the identity transform
    Transform() : m(), mInv() {}

    /// Create a new transform instance for the given matrix
    Transform(const Mat44f &m) : m(m), mInv(::inverse(m)) {}

    /// Create a new transform instance for the given matrix and its inverse
    Transform(const Mat44f &trafo, const Mat44f &inv) : m(trafo), mInv(inv) {}

    /// Return the inverse transformation
    Transform inverse() const
    {
        return Transform(mInv, m);
    }

    /// Concatenate with another transform
    Transform operator*(const Transform &t) const
    {
        return Transform(m * t.m, t.mInv * mInv);
    }

    /// Apply the homogeneous transformation to a 3D vector
    Vec3f vector(const Vec3f &v) const
    {
        // TODO: Implement the transformation of a 3D vector by the transform
        // matrix m. A regular vector should be transformed without any
        // translation applied; You can do this by considering only the top-left
        // 3x3 submatrix of m and multiplying the vector with that.
        //
        // You can access the entries of the matrix m either with:
        //      m(row, col),
        // or
        //      m[col][row].
        return (m*Vec4f(v, 0.0f)).xyz;
    }

    /// Apply the homogeneous transformation to a 3D normal
    Vec3f normal(const Vec3f &n) const
    {
        // TODO: Implement the transformation of a normal by the transform
        // matrix m. Note that normals need to be transformed differently than
        // vectors; you need to multiply by the transpose of the inverse matrix.
        // The inverse transformation matrix has already been computed for you:
        // mInv. Similar to vectors, you should not apply translation - only use
        // the top left 3x3 entries of the matrix.
        return normalize((transpose(mInv)*Vec4f(n, 0.0f)).xyz);
    }

    /// Transform a point by an arbitrary matrix in homogeneous coordinates
    Vec3f point(const Vec3f &p) const
    {
        // TODO: Implement the transformation of a point by the transform matrix
        // m. Here we want to apply translation - meaning you should interpret
        // the point p to be a 4-vector with a 1 in the 4th component.
        //
        // The result of the transform is another 4-vector, (x, y, z, w).
        // You should return the first 3 elements of this vector, divided by the
        // 4th coordinate
        Vec4f tform = m*Vec4f(p, 1.0f);
        return tform.xyz/tform.w; 
    }

    /// Apply the homogeneous transformation to a ray
    Ray3f ray(const Ray3f &r) const
    {
        // TODO: Transform a ray by this transform.
        // A ray consists of an origin, the point r.o, and a direction, r.d.
        // Transform these, and return a new ray with the transformed
        // coordinates.
        //
        // IMPORTANT: The ray you return should have the same mint
        // and maxt as the original ray
        return Ray3f(point(r.o), vector(r.d), r.mint, r.maxt);
    }

    /// Transform the axis-aligned Box and return the bounding box of the result
    Box3f box(const Box3f & box) const
    {
        // a transformed empty box is still empty
        if (box.isEmpty())
            return box;

        // Just in case this is a projection matrix, do things the naive way.
        Vec3f pts[8];

        // Set up the eight points at the corners of the extent
        pts[0].x = pts[1].x = pts[2].x = pts[3].x = box.pMin.x;
        pts[4].x = pts[5].x = pts[6].x = pts[7].x = box.pMax.x;

        pts[0].y = pts[1].y = pts[4].y = pts[5].y = box.pMin.y;
        pts[2].y = pts[3].y = pts[6].y = pts[7].y = box.pMax.y;

        pts[0].z = pts[2].z = pts[4].z = pts[6].z = box.pMin.z;
        pts[1].z = pts[3].z = pts[5].z = pts[7].z = box.pMax.z;

        Box3f newBox(point(pts[0]));
        for (size_t i = 1; i < 8; ++i)
            newBox.enclose(point(pts[i]));

        return newBox;
    }

    static Transform translate(const Vec3f & t)
    {
        return Transform(Mat44f({1.f, 0.f, 0.f, 0.f},
                                {0.f, 1.f, 0.f, 0.f},
                                {0.f, 0.f, 1.f, 0.f},
                                {t.x, t.y, t.z, 1.f}));
    }

    static Transform axisOffset(const Vec3f& x, const Vec3f& y, const Vec3f& z,
                                const Vec3f& o)
    {
        return Transform(Mat44f({x, 0},
                                {y, 0},
                                {z, 0},
                                {o, 1}));
    }
};
