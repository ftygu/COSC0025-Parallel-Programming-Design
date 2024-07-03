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

    This file is based on the Mesh class from Nori:

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <dirt/surface.h>

/**
    A triangle mesh.

    This class stores a triangle mesh object and provides numerous functions
    for querying the individual triangles. Subclasses of \c Mesh implement
    the specifics of how to create its contents (e.g. by loading from an
    external file)
 */
struct Mesh
{
public:

	bool empty() const {return F.empty() || V.empty();}

    vector<Vec3f> V;                        ///< Vertex positions
    vector<Vec3f> N;                        ///< Vertex normals
    vector<Vec2f> UV;                       ///< Vertex texture coordinates
    vector<Vec3i> F;                        ///< Faces

    Transform m_xform = Transform();        ///< Local-to-world Transformation
    shared_ptr<const Material> material;     ///< One material for all faces
    shared_ptr<const MediumInterface> medium_interface;
};


/// An instance of a triangle for a given face in a mesh
class Triangle : public SurfaceBase
{
public:
	Triangle(const Scene & scene, const json & j, shared_ptr<const Mesh> mesh, uint32_t triNumber);
	Box3f localBBox() const override;
	Box3f worldBBox() const override;
	bool intersect(const Ray3f &ray, HitInfo &hit) const override;

    bool isEmissive() const override {return m_mesh && m_mesh->material && m_mesh->material->isEmissive();}
    
    Vec3f sample(const Vec3f &o, const Vec2f &sample) const override;
    
    float pdf(const Vec3f &o, const Vec3f &v) const override;

protected:
    // convenience function to access the i-th vertex (i must be 0, 1, or 2)
    Vec3f vertex(size_t i) const {return m_mesh->V[m_mesh->F[m_faceIdx][i]];}

	shared_ptr<const Mesh> m_mesh;
	uint32_t m_faceIdx;
};


/// ray - single triangle intersection routine
bool singleTriangleIntersect(const Ray3f& ray,
                             const Vec3f& v0, const Vec3f& v1, const Vec3f& v2,
                             const Vec3f* n0, const Vec3f* n1, const Vec3f* n2,
                             const Vec2f* t0, const Vec2f* t1, const Vec2f* t2,
                             HitInfo& isect,
                             const Material * material = nullptr,
                             const SurfaceBase * surface = nullptr);

