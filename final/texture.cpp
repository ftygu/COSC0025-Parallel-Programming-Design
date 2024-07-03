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
#include <dirt/texture.h>
#include <dirt/scene.h>
#include <dirt/surface.h>
#include <filesystem/resolver.h>

namespace
{

auto g_defaultTexture = make_shared<ConstantTexture>(Color3f(0.9f, 0.1f, 0.9f));

} // namespace


shared_ptr<Texture> Texture::defaultTexture()
{
	return g_defaultTexture;
}


ConstantTexture::ConstantTexture(const json & j)
{
    try
	{
		j.get_to(color);
	}
	catch (...)
    {
        color = j.value("color", color);
    }
}


CheckerTexture::CheckerTexture(const json & j)
{
    scale = j.value("scale", scale);

    even = parseTexture(j["even"]);
    odd = parseTexture(j["odd"]);
}

Color3f CheckerTexture::value(const HitInfo & hit) const
{
    float sines = sin(scale*hit.p.x)*sin(scale*hit.p.y)*sin(scale*hit.p.z);
    return (sines < 0) ? odd->value(hit) : even->value(hit);
}


MarbleTexture::MarbleTexture(const json & j)
{
    scale = j.value("scale", scale);

    veins = parseTexture(j["veins"]);
    base = parseTexture(j["base"]);
}

Color3f MarbleTexture::value(const HitInfo & hit) const
{
    float factor = 0.5f*(1.f + sinf(scale * hit.p.z + 10 * noise.turb(hit.p)));
    return lerp(base->value(hit), veins->value(hit), factor);
}


ImageTexture::ImageTexture(const json & j)
{
    string filename;

    try
	{
		filename = j.at("filename").get<string>();
	}
	catch (...)
	{
		error("No \"filename\" specified for ImageTexture.\n", j.dump());
        tex.resize(1,1);
        tex(0,0) = Color3f(1,0,1);
	}

    string path = getFileResolver().resolve(filename).str();
    if (!tex.load(path))
    {
        error("Cannot load ImageTexture \"%s\".\n", path);
        tex.resize(1,1);
        tex(0,0) = Color3f(1,0,1);
    }
}

Color3f ImageTexture::value(const HitInfo & hit) const
{
    int i = clamp((int)round((hit.uv.u) * (tex.sizeX()-1)), 0, tex.sizeX() - 1);
    int j = clamp((int)round((1 - hit.uv.v) * (tex.sizeY()-1)), 0, tex.sizeY() - 1);

    return tex(i, j);
}
