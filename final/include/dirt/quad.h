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

#include <dirt/surface.h>
#include <dirt/surfacegroup.h>

/// A quad spanning (-m_size/2, m_size/2) in the (x,y)-plane at z=0
class Quad : public Surface
{
public:
	Quad(const Vec2f & size = Vec2f(1.f),
	     shared_ptr<const Material> material = Material::defaultMaterial(),
	     const Transform & xform = Transform());
    Quad(const Scene & scene, const json & j = json::object());

    Box3f localBBox() const override;
    bool intersect(const Ray3f &ray, HitInfo &hit) const override;
    bool isEmissive() const override {return m_material && m_material->isEmissive();}

    float pdf(const Vec3f& o, const Vec3f& v) const override;
    Vec3f sample(const Vec3f& o, const Vec2f &sample) const override;

protected:
    Vec2f m_size = Vec2f(1.f);
    shared_ptr<const Material> m_material;
    shared_ptr<const MediumInterface> m_medium_interface;
};