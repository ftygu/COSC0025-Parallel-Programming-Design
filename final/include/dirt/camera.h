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
#include <dirt/vec.h>
#include <dirt/parser.h>
#include <dirt/sampling.h>
#include <dirt/medium.h>

/**
    This class represents a virtual pinhole camera.
   
    The camera is responsible for generating primary rays. It is positioned
    using a Transform and points along the -z axis of the local coordinate
    system. It has an image plane positioned a z = -dist with size
    (width, height).
   
    We currently only support pinhole perspective cameras. This class could
    be made into a virtual base class to support other types of cameras
    (e.g. an orthographic camera, or omni-directional camera).
 */
class Camera
{
public:
    /// Construct a camera from json parameters.
    Camera(const json & j = json())
    {
		m_xform = j.value("transform", m_xform);
	    m_resolution = j.value("resolution", m_resolution);
	    m_focalDistance = j.value("fdist", m_focalDistance);
	    m_apertureRadius = j.value("aperture", m_apertureRadius);
        if (j.contains("medium"))
        {
            m_medium = parseMedium(j.at("medium"));
        }

		float vfov = 90.f; // Default vfov value. Override this with the value from json
        // TODO: Assignment 1: read the vertical field-of-view from j ("vfov"),
        // and compute the width and height of the image plane. Remember that
        // the "vfov" parameter is specified in degrees, but C++ math functions
        // expect it in radians. You can use deg2rad() from common.h to convert
        // from one to the other
        vfov = deg2rad(j.value("vfov", vfov));	
	    m_size.y = 2*tan(vfov/2) * m_focalDistance;
	    m_size.x = float(m_resolution.x)/m_resolution.y * m_size.y;
    }

	/// Return the camera's image resolution
	Vec2i resolution() const {return m_resolution;}

    /**
        Generate a ray going through image-plane location (u,v).

        (\c u,\c v) range from 0 to m_resolution.x() and m_resolution.y() along
       	the x- and y-axis of the rendered image, respectively

        \param u 	The horizontal position within the image
        \param v  	The vertical position within the image
        \return 	The \ref Ray3f data structure filled with the
       				appropriate position and direction
     */
    Ray3f generateRay(float u, float v) const
    {
        u /= m_resolution.x;
		v /= m_resolution.y;

		Vec2f disk = m_apertureRadius*randomInUnitDisk();
		Vec3f origin(disk.x, disk.y, 0.f);
        return m_xform.ray(
            Ray3f(origin,
        	Vec3f(
                (u - 0.5f) * m_size.x,
        	    (0.5f - v) * m_size.y,
        	    -m_focalDistance) - origin
            )
        ).withMedium(m_medium);
    }

private:
	//
	// The camera setup looks something like this, where the
	// up vector points out of the screen:
	//
	//         top view                         side view
	//            ^                    up
	//            |                     ^
	//            |                     |             _,-'
	//          width                   |         _,-'   |
	//       +----|----+     +          |     _,-'       | h
	//        \   |   /    d |        e | _,-'           | e
	//         \  |  /     i |        y +'---------------+-i----->
	//          \ | /      s |        e  '-,_   dist     | g
	//           \|/       t |               '-,_        | h
	//            +          +                   '-,_    | t
	//           eye                                 '-,_|
	//


	Transform m_xform = Transform();      ///< Local coordinate system
	Vec2f m_size = Vec2f(1,1);            ///< Physical size of the image plane
	float m_focalDistance = 1.f;          ///< Distance to image plane along local z axis
	Vec2i m_resolution = Vec2i(512,512);  ///< Image resolution
	float m_apertureRadius = 0.f;         ///< The size of the aperture for depth of field
    std::shared_ptr<const Medium> m_medium;
};
