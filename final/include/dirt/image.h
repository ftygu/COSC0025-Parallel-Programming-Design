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

#include <dirt/array2d.h>
#include <dirt/vec.h>

/// A floating-point RGB image
class Image3f : public Array2d<Color3f>
{
    using Base = Array2d<Color3f>;

public:
    /// Default constructor (empty image)
    Image3f() : Base() {}
    /**
        Size Constructor (sets width and height)

        \param w     The width of the image
        \param h     The height of the image
     */
    Image3f(int w, int h) : Base(w, h) {}

    /**
        Construct an image of a fixed size and initialize all pixels

        \param w     The width of the image
        \param h     The height of the image
        \param v     The Color to set all pixels
     */
    Image3f(int w, int h, const Color3f &v) : Base(w,h) {reset(v);}

	/**
	    Load an image from file

	    \param filename	The filename
	    \return 		True if the file loaded successfully
	 */
    bool load(const std::string & filename);

	/**
	    Save an image to the specified filename
	    \param filename The filename to save to
	    \param gain 	The multiplicative gain to apply to pixel values before saving
	    \return 		True if the file saved successfully
	 */
    bool save(const std::string & filename, float gain = 1.0f);

    /// Set of supported formats for image loading
    static set<string> canLoad()
    {
        return {"jpg", "jpeg" "png", "bmp", "psd", "tga",
                "gif", "hdr", "pic", "ppm", "pgm"};
    }

    /// Set of supported formats for image saving
    static set<string> canSave()
    {
        return {"png", "jpg", "hdr", "bmp", "tga"};
    }
};
