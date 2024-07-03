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
#include <dirt/transform.h>
#include <cmath>
#include <iomanip>
#include <filesystem/resolver.h>

uint64_t intersection_tests = 0;
uint64_t rays_traced = 0;

Verbosity g_verbosity = Verbosity::Debug;

Verbosity verbosity()
{
    return g_verbosity;
}

void setVerbosity(int v)
{
    if (v < (int)Verbosity::None || v >= (int)Verbosity::Size)
        throw DirtException("Verbosity value must be between %s and %s.\n",
                            (int)Verbosity::None, (int)Verbosity::Size - 1);

    g_verbosity = static_cast<Verbosity>(v);
}


/*
    Taken from Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob, Romain Prévost

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
std::string timeString(double time, int precision)
{
    if (std::isnan(time) || std::isinf(time))
        return "inf";

    std::string suffix = "ms";
    if (time > 1000)
    {
        time /= 1000; suffix = "s";
        if (time > 60)
        {
            time /= 60; suffix = "m";
            if (time > 60)
            {
                time /= 60; suffix = "h";
                if (time > 12)
                {
                    time /= 12; suffix = "d";
                }
            }
        }
    }

    std::ostringstream os;
    os << std::setprecision(precision) << std::fixed << time << suffix;

    return os.str();
}


std::string memString(size_t size, int precision)
{
    double value = (double) size;
    const char *suffixes[] =
    {
        "B", "KiB", "MiB", "GiB", "TiB", "PiB"
    };
    int suffix = 0;
    while (suffix < 5 && value > 1024.0f)
    {
        value /= 1024.0f; ++suffix;
    }

    std::ostringstream os;
    os << std::setprecision(suffix == 0 ? 0 : precision)
       << std::fixed << value << " " << suffixes[suffix];

    return os.str();
}


filesystem::resolver & getFileResolver()
{
    static filesystem::resolver resolver;
    return resolver;
}