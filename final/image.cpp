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

#include <dirt/common.h>
#include <dirt/image.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <cctype>

#define STB_IMAGE_IMPLEMENTATION

// since NanoVG includes an old version of stb_image, we declare it static here
#define STB_IMAGE_STATIC

// these pragmas ignore warnings about unused static functions
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#elif defined(_MSC_VER)
#pragma warning (push, 0)
#endif

#include "stb_image.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning (pop)
#endif


#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;

// local functions
namespace
{

string getFileExtension(const string& filename)
{
    if (filename.find_last_of(".") != string::npos)
        return filename.substr(filename.find_last_of(".")+1);
    return "";
}

} // namespace


bool Image3f::load(const string & filename)
{
    stbi_ldr_to_hdr_scale(1.0f);
    stbi_ldr_to_hdr_gamma(1.0f);
    bool isHdr = stbi_is_hdr(filename.c_str());

    int n, w, h;
    float * float_data = stbi_loadf(filename.c_str(), &w, &h, &n, 3);
    if (float_data)
    {
        resize(w, h);
        for (auto y : range(h))
        {
            for (auto x : range(w))
            {
                auto c = Color3f(float_data[3*(x + y*w) + 0],
                                 float_data[3*(x + y*w) + 1],
                                 float_data[3*(x + y*w) + 2]);
                if (!isHdr)
                    c = toLinearRGB(c);
                (*this)(x,y) = c;
            }
        }
        return true;
    }

    return false;
}

bool Image3f::save(const string & filename, float gain)
{
    string extension = getFileExtension(filename);

    transform(extension.begin(),
              extension.end(),
              extension.begin(),
              [](char c){return static_cast<char>(std::tolower(c));});

    if (extension == "hdr")
        return stbi_write_hdr(filename.c_str(), width(), height(), 3, (const float *) &m_data[0]) != 0;
    else
    {
        // convert floating-point image to 8-bit per channel
        vector<unsigned char> data(width()*height()*3, 0);
        for (auto y : range(height()))
            for (auto x : range(width()))
            {
                Color3f c = (*this)(x,y) * gain;
                c = toSRGB(c);

                // convert to [0-255] range
                c.r = clamp(c.r * 255.0f, 0.0f, 255.0f);
                c.g = clamp(c.g * 255.0f, 0.0f, 255.0f);
                c.b = clamp(c.b * 255.0f, 0.0f, 255.0f);

                data[3*x + 3*y*width() + 0] = (unsigned char) c[0];
                data[3*x + 3*y*width() + 1] = (unsigned char) c[1];
                data[3*x + 3*y*width() + 2] = (unsigned char) c[2];
            }

        if (extension == "png")
            return stbi_write_png(filename.c_str(), width(), height(),
                                  3, &data[0], sizeof(unsigned char)*width()*3) != 0;
        else if (extension == "jpg" || extension == "jpeg")
            return stbi_write_jpg(filename.c_str(), width(), height(), 3, &data[0], 100) != 0;
        else if (extension == "bmp")
            return stbi_write_bmp(filename.c_str(), width(), height(), 3, &data[0]) != 0;
        else if (extension == "tga")
            return stbi_write_tga(filename.c_str(), width(), height(), 3, &data[0]) != 0;
        else
            throw DirtException("Could not determine desired file type from extension.");
    }
}
