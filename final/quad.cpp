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

#include <dirt/quad.h>
#include <dirt/scene.h>

Quad::Quad(const Vec2f & size,
           shared_ptr<const Material> material,
           const Transform & xform)
	: Surface(xform), m_size(size*0.5f), m_material(material)
{

}

Quad::Quad(const Scene & scene, const json & j)
    : Surface(scene, j)
{
    m_size = j.value("size", m_size);
	m_size /= 2.f;
    
    m_material = scene.findOrCreateMaterial(j);
    m_medium_interface = scene.findOrCreateMediumInterface(j);
}

bool Quad::intersect(const Ray3f &ray, HitInfo &hit) const
{
    INCREMENT_INTERSECTION_TESTS;

    // compute ray intersection (and ray parameter), continue if not hit
    auto tray = m_xform.inverse().ray(ray);
    if (tray.d.z == 0)
        return false;
    auto t = -tray.o.z / tray.d.z;
    auto p = tray(t);

    if (m_size.x < p.x || -m_size.x > p.x || m_size.y < p.y || -m_size.y > p.y)
        return false;

    // check if computed param is within ray.mint and ray.maxt
    if (t < tray.mint || t > tray.maxt)
        return false;

	// project hitpoint onto plane to reduce floating-point error
	p.z = 0;

    Vec3f gn = normalize(m_xform.normal({0,0,1}));

    Vec2f uv = Vec2f(p.x / (2 * m_size.x) + 0.5f, p.y / (2 * m_size.y) + 0.5f);

    // if hit, set intersection record values
    hit = HitInfo(t, m_xform.point(p), gn, gn, uv, m_material.get(), m_medium_interface.get(), this);
    return true;
}


Box3f Quad::localBBox() const
{
    return Box3f(-Vec3f(m_size.x,m_size.y,0) - Vec3f(1e-4f), Vec3f(m_size.x,m_size.y,0) + Vec3f(1e-4f));
}

float Quad::pdf(const Vec3f& o, const Vec3f& v) const
{
    HitInfo rec;
    if (this->intersect(Ray3f(o, v), rec))
    {
        float area = 4 * length(cross(m_xform.vector({m_size.x, 0, 0}), m_xform.vector({0, m_size.y, 0})));
        float distance_squared = rec.t * rec.t * length2(v);
        float cosine = std::abs(dot(v, rec.gn) / length(v));
        return distance_squared / (cosine * area);
    }
    else
        return 0;
}

Vec3f Quad::sample(const Vec3f& o, const Vec2f &sample) const
{
    Vec3f p {(2 * sample.x - 1) * m_size.x, (2 * sample.y - 1) * m_size.y, 0};
    return m_xform.point(p) - o;
}

