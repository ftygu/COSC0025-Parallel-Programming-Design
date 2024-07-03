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

#include <dirt/mesh.h>
#include <dirt/scene.h>

// Ray-Triangle intersection
// p0, p1, p2 - Triangle vertices
// n0, n1, n2 - optional per vertex normal data
// t0, t1, t2 - optional per vertex texture coordinates
bool singleTriangleIntersect(const Ray3f& ray,
	                         const Vec3f& p0, const Vec3f& p1, const Vec3f& p2,
	                         const Vec3f* n0, const Vec3f* n1, const Vec3f* n2,
                             const Vec2f* t0, const Vec2f* t1, const Vec2f* t2,
	                         HitInfo& hit,
	                         const Material * material,
                             const MediumInterface *medium_interface,
	                         const SurfaceBase * surface)
{
   // Find vectors for two edges sharing v[0]
    Vec3f edge1 = p1 - p0,
          edge2 = p2 - p0;

    // Begin calculating determinant - also used to calculate U parameter
    Vec3f pvec = cross(ray.d, edge2);

    // If determinant is near zero, ray lies in plane of triangle
    float det = dot(edge1, pvec);

    if (det > -1e-8f && det < 1e-8f)
        return false;
    float inv_det = 1.0f / det;

    // Calculate distance from v[0] to ray origin
    Vec3f tvec = ray.o - p0;

    // Calculate U parameter and test bounds
    float u = dot(tvec,pvec) * inv_det;
    if (u < 0.0 || u > 1.0)
        return false;

    // Prepare to test V parameter
    Vec3f qvec = cross(tvec, edge1);

    // Calculate V parameter and test bounds
    float v = dot(ray.d,qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0)
        return false;

    // Ray intersects triangle -> compute t
    float t = dot(edge2, qvec) * inv_det;

    if (!(t >= ray.mint && t <= ray.maxt))
        return false;

    Vec3f gn = normalize(cross(p1 - p0, p2 - p0));

    Vec3f bary(1 - (u + v), u, v);

    Vec3f sn;
    if (n0 && n1 && n2)
        sn = normalize(bary.x * (*n0) +
                       bary.y * (*n1) +
                       bary.z * (*n2));
    else
        sn = gn;

    Vec2f uv;
    if (t0 && t1 && t2)
        uv = bary.x * (*t0) + bary.y * (*t1) + bary.z * (*t2);
    else
        uv = {u, v};

    // Compute the intersection positon accurately using barycentric coordinates
    Vec3f p = bary.x * p0 + bary.y * p1 + bary.z * p2;

    // if hit, set intersection record values
    hit = HitInfo(t, p, gn, sn, uv, material, medium_interface, surface);
    return true; 
}

Triangle::Triangle(const Scene & scene, const json & j, shared_ptr<const Mesh> mesh, uint32_t triNumber)
    : m_mesh(mesh), m_faceIdx(triNumber)
{
    
}

bool Triangle::intersect(const Ray3f &ray, HitInfo &hit) const
{
    INCREMENT_INTERSECTION_TESTS;

    auto i0 = m_mesh->F[m_faceIdx].x,
         i1 = m_mesh->F[m_faceIdx].y,
         i2 = m_mesh->F[m_faceIdx].z;
    auto p0 = m_mesh->V[i0],
         p1 = m_mesh->V[i1],
         p2 = m_mesh->V[i2];
    const Vec3f * n0 = nullptr, *n1 = nullptr, *n2 = nullptr;
    if (!m_mesh->N.empty())
    {
        n0 = &m_mesh->N[i0];
        n1 = &m_mesh->N[i1];
        n2 = &m_mesh->N[i2];
    }
    const Vec2f * t0 = nullptr, *t1 = nullptr, *t2 = nullptr;
    if (!m_mesh->UV.empty())
    {
        t0 = &m_mesh->UV[i0];
        t1 = &m_mesh->UV[i1];
        t2 = &m_mesh->UV[i2];
    }

    return singleTriangleIntersect(ray,
                                   p0, p1, p2,
                                   n0, n1, n2,
                                   t0, t1, t2,
                                   hit,
                                   m_mesh->material.get(), 
                                   m_mesh->medium_interface.get(),
                                   this);
}

Box3f Triangle::localBBox() const
{
	// all mesh vertices have already been transformed to world space,
	// so we need to transform back to get the local space bounds
    Box3f result;
    result.enclose(m_mesh->m_xform.inverse().point(vertex(0)));
    result.enclose(m_mesh->m_xform.inverse().point(vertex(1)));
    result.enclose(m_mesh->m_xform.inverse().point(vertex(2)));
    
    // if the triangle lies in an axis-aligned plane, expand the box a bit
    auto diag = result.diagonal();
    for (int i = 0; i < 3; ++i)
    {
        if (diag[i] < 1e-4f)
        {
            result.pMin[i] -= 5e-5f;
            result.pMax[i] += 5e-5f;
        }
    }
    return result;
}

Box3f Triangle::worldBBox() const
{
    // all mesh vertices have already been transformed to world space,
    // so just bound the triangle vertices
    Box3f result;
    result.enclose(vertex(0));
    result.enclose(vertex(1));
    result.enclose(vertex(2));

    // if the triangle lies in an axis-aligned plane, expand the box a bit
    auto diag = result.diagonal();
    for (int i = 0; i < 3; ++i)
    {
        if (diag[i] < 1e-4f)
        {
            result.pMin[i] -= 5e-5f;
            result.pMax[i] += 5e-5f;
        }
    }
    return result;
}

Vec3f Triangle::sample(const Vec3f &o, const Vec2f &sample) const
{
    // get triangle vertices
    Vec3f p0 = m_mesh->m_xform.point(vertex(0));
    Vec3f p1 = m_mesh->m_xform.point(vertex(1));
    Vec3f p2 = m_mesh->m_xform.point(vertex(2));

    // compute baycentric coordinates
    float u = std::sqrt(sample.x);
    float b0 = 1.0f - u;
    float b1 = sample.y * u;

    // calculate point based on barycentric coordinates
    Vec3f p = b0 * p0 + b1 * p1 + (1.0f - b0 - b1) * p2;
    return normalize(p - o);
}

float Triangle::pdf(const Vec3f &o, const Vec3f &dir) const
{
    // check that ray intersects triangle
    Ray3f r(o, dir);
    HitInfo hit;
    if (!intersect(r, hit)) return 0.0f;

    // get triangle vertices
    Vec3f p0 = m_mesh->m_xform.point(vertex(0));
    Vec3f p1 = m_mesh->m_xform.point(vertex(1));
    Vec3f p2 = m_mesh->m_xform.point(vertex(2));   

    // compute the pdf
    float areaPdf = 2.0f / length(cross((p1 - p0), (p2 - p0)));
    float geometryTerm = length2(hit.p - o) / abs(dot(dir, hit.gn));
    return areaPdf * geometryTerm;
}
