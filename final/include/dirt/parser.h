
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

#include <dirt/transform.h>

/**
   Parse a Surface from the json object \ref j and add it as a child
   to \ref parent by calling parent->addChild().

   \param  scene  The scene, passed to the Surface constructor
   \param  parent The parent to add this surface to as a child
   \param  j      The json object to parse
 */
void parseSurface(const Scene & scene, SurfaceBase * parent, const json & j);

/**
   Return a newly constructed Material by parsing the json object \ref j.
   
   \param  j            The json object to parse
   \return Material*    The resulting Material
 */
shared_ptr<Material> parseMaterial(const json & j);

/**
   Return a newly constructed Medium by parsing the json object \ref j.
   
   \param  j            The json object to parse
   \return Medium*    The resulting Medium
 */
shared_ptr<Medium> parseMedium(const json & j);

/**
   Return a newly constructed Phase function by parsing the json object \ref j.
   
   \param  j            The json object to parse
   \return PhaseFunction*    The resulting phase function
 */
shared_ptr<PhaseFunction> parsePhase(const json &j);

/**
   Return a newly constructed Texture by parsing the json object \ref j.
   
   \param  j            The json object to parse
   \return Texture*     The resulting Texture
 */
shared_ptr<Texture> parseTexture(const json & j);

/**
    Return a newly constructed Sampler by parsing the json object \ref j.
*/
shared_ptr<Sampler> parseSampler(const json &j);

/// parse a Mat44<T> from json
template <typename T>
void from_json(const json & j, Mat44<T> & m)
{
    if (j.count("from") || j.count("at") || j.count("to") || j.count("up"))
    {
        Vec3<T> from(0,0,1), at(0,0,0), up(0,1,0);
        from = j.value("from", from);
	    at = j.value("at", at) + j.value("to", at);
	    up = j.value("up", up);

        Vec3<T> dir = normalize(from - at);
        Vec3<T> left = normalize(cross(up, dir));
        Vec3<T> newUp = normalize(cross(dir, left));

        m = Mat44<T>(left, newUp, dir, from);
    }
    else if (j.count("o") || j.count("x") || j.count("y") || j.count("z"))
    {
        Vec3<T> o(0,0,0), x(1, 0, 0), y(0, 1, 0), z(0, 0, 1);
        o = j.value("o", o);
        x = j.value("x", x);
        y = j.value("y", y);
        z = j.value("z", z);
        m = Mat44<T>(x, y, z, o);
    }
    else if (j.count("translate"))
    {
        Vec3<T> v = j.value("translate", Vec3<T>(0));
        m = Mat44<T>(1);
        m[3].xyz = v;
    }
    else if (j.count("scale"))
    {
        Vec3<T> v = j.value("scale", Vec3<T>(1));

        m = Mat44<T>(1);
        m(0,0) = v[0];
        m(1,1) = v[1];
        m(2,2) = v[2];
    }
    else if (j.count("axis") || j.count("angle"))
    {
        float angle = 0.0f;
        Vec3<T> axis(1,0,0);
        axis = j.value("axis", axis);
        angle = j.value("angle", angle);
        m = axisAngle<float>(axis, deg2rad(angle));
    }
    else if (j.count("matrix"))
    {
        json jm = j["matrix"];
        if (jm.size() == 1)
        {
            m = Mat44<T>(jm.get<T>());
            warning("Incorrect array size when trying to parse an Eigen::Matrix. "
                    "Expecting 4 x 4 = 16 values but only found a single scalar. "
                    "Creating a 4 x 4 scaling matrix with '%f's along the diagonal.\n", jm.get<float>());
            return;
        }
        else if (16 != jm.size())
        {
            throw DirtException("Incorrect array size when trying to parse a Matrix. "
                                "Expecting 4 x 4 = 16 values but found %d, here:\n%s.", jm.size(), jm.dump(4));
        }

        // jm.size() == 16
        for (auto row : range(4))
            for (auto col : range(4))
                jm.at(row*4 + col).get_to(m(row,col));
    }
    else
        throw DirtException("Unrecognized 'transform' command:\n%s.", j.dump(4));
}

/// parse a Vec<N,T> from json
template <size_t N, typename T>
inline void from_json(const json & j, Vec<N,T> & v)
{
    if (j.is_object())
        throw DirtException("Can't parse a Vec%d. Expecting a json array, but got a json object.", N);

    if (j.size() == 1)
    {
        if (j.is_array())
            message("Incorrect array size when trying to parse a Vec3. "
                    "Expecting %d values but only found 1. "
                    "Creating a Vec of all '%f's.\n", N, j.get<T>());
        v = Vec<N,T>(j.get<T>());
        return;
    }
    else if (N != j.size())
    {
        throw DirtException("Incorrect array size when trying to parse a Vec. "
                            "Expecting %d values but found %d here:\n%s", N, (int)j.size(), j.dump(4));
    }

    // j.size() == N
    for (auto i : range((int)j.size()))
        j.at(i).get_to(v[i]);
}

/// Parse a Dirt Transform from json
void from_json(const json & j, Transform & t);


/// Serialize a Mat44<T> to json
template <typename T>
inline void to_json(json & j, const Mat44<T> & v)
{
    j["matrix"] = vector<T>(reinterpret_cast<const T*>(&v.m00), reinterpret_cast<const T*>(&v.m00) + 16);
}

/// Serialize a Vec3<N,T> to json
template <size_t N, typename T>
inline void to_json(json & j, const Vec<N,T> & v)
{
    j = vector<T>(&v.e[0], &v.e[0] + N);
}

/// Serialize a dirt Transform to json
inline void to_json(json & j, const Transform & t)
{
	to_json(j, t.m);
}

