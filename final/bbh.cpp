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

#include <dirt/bbh.h>

BBHNode::BBHNode(vector<shared_ptr<SurfaceBase>> primitives, Progress & progress)
{
    if (primitives.size() == 1)
    {
        m_left = m_right = primitives[0];
        progress += 1;
    }
    else if (primitives.size() == 2)
    {
        m_left = primitives[0];
        m_right = primitives[1];
        progress += 2;
    }
    else
    {
        int axis = int(randf() * 3.0f);
        std::sort(primitives.begin(), primitives.end(),
                  [&](shared_ptr<const SurfaceBase> a, shared_ptr<const SurfaceBase> b) {
                      return a->worldBBox().center()[axis] <
                             b->worldBBox().center()[axis];
                  });

        vector<shared_ptr<SurfaceBase>> leftList, rightList;
        leftList.insert(leftList.begin(), primitives.begin(),
                        primitives.begin() + primitives.size() / 2);
        rightList.insert(rightList.begin(),
                         primitives.begin() + primitives.size() / 2,
                         primitives.end());

        m_left = make_shared<BBHNode>(leftList, progress);
        m_right = make_shared<BBHNode>(rightList, progress);
    }

    (m_bounds = m_left->worldBBox()).enclose(m_right->worldBBox());
}

BBHNode::~BBHNode()
{
}

bool BBHNode::intersect(const Ray3f &ray, HitInfo &hit) const
{
    if (m_bounds.intersect(ray))
    {
        HitInfo leftHit, rightHit;
        bool hitLeft = m_left->intersect(ray, leftHit);
        bool hitRight = m_right->intersect(ray, rightHit);

        if (hitLeft && hitRight)
        {
            if (leftHit.t < rightHit.t)
                hit = leftHit;
            else
                hit = rightHit;
            return true;
        }
        else if (hitLeft)
        {
            hit = leftHit;
            return true;
        }
        else if (hitRight)
        {
            hit = rightHit;
            return true;
        }
        else
            return false;
    }
    else
        return false;
}


BBH::BBH(const Scene & scene, const json & j) : SurfaceGroup(scene, j)
{
}


void BBH::build()
{
    Progress progress("Building BVH", m_surfaces.size());
    if (!m_surfaces.empty())
        m_root = make_shared<BBHNode>(m_surfaces, progress);
    else
        m_root = nullptr;
}

bool BBH::intersect(const Ray3f &ray, HitInfo &hit) const
{
    return m_root ? m_root->intersect(ray, hit) : false;
}
