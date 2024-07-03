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

#include <dirt/sphere.h>
#include <dirt/scene.h>
#include <dirt/onb.h>
#include <dirt/pdf.h>

Vec2f getSphereUV(const Vec3f& p)
{
    float phi = atan2(p.y, p.x);
    float theta = asin(p.z);
    float u = (phi + M_PI) / (2 * M_PI);
    float v = (theta + M_PI / 2) / M_PI;
    return Vec2f(u, v);
}

Sphere::Sphere(float radius,
               shared_ptr<const Material> material,
               const Transform & xform)
    : Surface(xform), m_radius(radius), m_material(material)
{

}

Sphere::Sphere(const Scene & scene, const json & j)
    : Surface(scene, j)
{
	m_radius = j.value("radius", m_radius);
    m_material = scene.findOrCreateMaterial(j);
    m_medium_interface = scene.findOrCreateMediumInterface(j);
}

Box3f Sphere::localBBox() const
{
    return Box3f(Vec3f(-m_radius), Vec3f(m_radius));
}

bool Sphere::intersect(const Ray3f &ray, HitInfo &hit) const
{
    INCREMENT_INTERSECTION_TESTS;
    // compute ray intersection (and ray parameter), continue if not hit
    // just grab only the first hit
    auto tray = m_xform.inverse().ray(ray);
    auto a = length2(tray.d);
    auto b = 2*dot(tray.d, tray.o);
    auto c = length2(tray.o) - m_radius*m_radius;

    // solve the quadratic equation using double precision
    double discrim = (double)b*(double)b - 4*(double)a*(double)c;
    if (discrim < 0)
        return false;

    double rootDiscrim = std::sqrt(discrim);

    double q = (b < 0) ? -.5 * (b - rootDiscrim) : -.5 * (b + rootDiscrim);

    float t1 = float(q / a);
    float t2 = float(c / q);
    if (t1 > t2)
        std::swap(t1, t2);

    // compute t
    float t = (t1 < tray.mint) ? t2 : t1;

    // check if computed param is within ray.mint and ray.maxt
    if (t < tray.mint || t > tray.maxt)
        return false;

    auto p = tray(t);
    p *= m_radius / length(p);

    Vec3f gn = normalize(m_xform.normal(p));
    Vec2f uv = getSphereUV(p/m_radius);

    // if hit, set intersection record values
    hit = HitInfo(t, m_xform.point(p), gn, gn, uv, m_material.get(), m_medium_interface.get(), this);

    return true;
}

float Sphere::pdf(const Vec3f& o, const Vec3f& v) const
{
    HitInfo hit;
    if (this->intersect(Ray3f(o, v), hit))
    {
        Vec3f center = m_xform.point(Vec3f(0));
        float radius2 = length2(m_xform.point(Vec3f(0,0,m_radius)) - center);
        float cos_theta_max = sqrt(1 - radius2/length2(center-o));
        float solid_angle = 2*M_PI*(1-cos_theta_max);
        return  1 / solid_angle;
    }
    else
        return 0.000001f;
}

Vec3f Sphere::sample(const Vec3f& o, const Vec2f &sample) const
{
    Vec3f center = m_xform.point(Vec3f(0));
    float radius = length(m_xform.point(Vec3f(0,0,m_radius)) - center);
    Vec3f direction = center-o;
    float distance_squared = length2(direction);
    ONBf onb;
    onb.build_from_w(direction);
    Vec3f ret = onb.toWorld(random_to_sphere(sample, radius, distance_squared));
    if (pdf(o, ret) == 0)
        cout << "sample: " << ret << "; " << pdf(o, ret) << endl;
    return ret;
}
