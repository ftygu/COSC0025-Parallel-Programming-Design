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

    This file is based on the WavefrontOBJ mesh class from Nori:

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <dirt/mesh.h>

/// Loader for Wavefront OBJ triangle meshes
Mesh loadWavefrontOBJ(const std::string & filename, const Transform & xform);