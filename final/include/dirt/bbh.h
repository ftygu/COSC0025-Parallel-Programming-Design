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

#include <dirt/surfacegroup.h>
#include <dirt/progress.h>

class BBHNode : public Surface
{
    Box3f m_bounds;
    shared_ptr<SurfaceBase> m_left;
    shared_ptr<SurfaceBase> m_right;

public:
    BBHNode(vector<shared_ptr<SurfaceBase>> primitives, Progress & progress);
    ~BBHNode();

    bool intersect(const Ray3f &ray, HitInfo &hit) const override;

    Box3f localBBox() const override { return m_bounds; }
    Box3f worldBBox() const override { return m_bounds; }
};

/// An axis-aligned bounding box hierarchy acceleration structure
class BBH: public SurfaceGroup
{
    shared_ptr<BBHNode> m_root;

public:
    BBH(const Scene & scene, const json & j = json::object());

    /// Construct the BBH (must be called before @ref intersect)
    void build() override;

    /// Intersect a ray against all surfaces registered with the Accelerator
    bool intersect(const Ray3f &ray, HitInfo &hit) const override;
};
