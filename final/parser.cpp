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

#include <dirt/parser.h>
#include <dirt/obj.h>
#include <dirt/bbh.h>
#include <dirt/sphere.h>
#include <dirt/quad.h>
#include <dirt/scene.h>
#include <dirt/texture.h>
#include <iostream>
#include <filesystem/resolver.h>
#include <dirt/integrator.h>
#include <dirt/ao.h>
#include <dirt/path_tracer_simple.h>
#include <dirt/path_tracer_mats.h>
#include <dirt/path_tracer_mis.h>
#include <dirt/path_tracer_mixture.h>
#include <dirt/path_tracer_nee.h>
#include <dirt/volpath_tracer_nee.h>
#include <dirt/volpath_tracer_uni.h>
#include <dirt/original.h>
#include <dirt/normals.h>

void from_json(const json & j, Transform & v)
{
    Mat44f m = v.m;
    if (j.is_array())
    {
        // multiple transformation commands listed in order
        for (auto & element : j)
            m = element.get<Mat44f>() * m;
    }
    else if (j.is_object())
    {
        // a single transformation
        j.get_to(m);
    }
    else
        throw DirtException("'transform' must be either an array or an object here:\n%s", j.dump(4));

    v = Transform(m);
}

// helper function to check for a required string key within a json object
static string getKey(const string & key, const string & parent, const json & j)
{
    try
    {
        return j.at(key).get<string>();
    }
    catch (...)
    {
        throw DirtException("Missing '%s' on '%s' specification:\n%s", key, parent, j.dump(4));
    }
}

shared_ptr<SurfaceGroup> parseAccelerator(const Scene & scene, const json & j)
{
    string type = getKey("type", "accelerator", j);

    if (type == "bbh" || type == "bvh")
        return make_shared<BBH>(scene, j);
    else if (type == "group")
        return make_shared<SurfaceGroup>(scene, j);
    else
        throw DirtException("Unknown 'accelerator' type '%s' here:\n%s.", type, j.dump(4));
}

shared_ptr<Integrator> parseIntegrator(const json & j)
{
    string type = getKey("type", "integrator", j);

    if (type == "normals")
        return make_shared<NormalIntegrator>(j);
    else if (type == "path_tracer_simple")
        return make_shared<PathTracerSimple>(j);
    else if (type == "path_tracer_mats")
        return make_shared<PathTracerMats>(j);
    else if (type == "path_tracer_mixture")
        return make_shared<PathTracerMixture>(j);
    else if (type == "path_tracer_mis")
        return make_shared<PathTracerMIS>(j);
    else if (type == "path_tracer_nee")
        return make_shared<PathTracerNEE>(j);
    else if (type == "volpath_tracer_nee")
        return make_shared<VolpathTracerNEE>(j);
    else if (type == "volpath_tracer_uni")
        return make_shared<VolpathTracerUni>(j);
	else if (type == "original")
        return make_shared<Original>(j);
    else if (type == "ao")
        return make_shared<AmbientOcclusion>(j);
    else
        throw DirtException("Unknown 'integrator' type '%s' here:\n%s.", type, j.dump(4));
}

shared_ptr<Material> parseMaterial(const json & j)
{
    string type = getKey("type", "material", j);

	if (type == "lambertian")
		return make_shared<Lambertian>(j);
	else if (type == "metal")
		return make_shared<Metal>(j);
    else if (type == "dielectric")
        return make_shared<Dielectric>(j);
    else if (type == "diffuse light")
        return make_shared<DiffuseLight>(j);
    else if (type == "blend")
        return make_shared<BlendMaterial>(j);
    else if (type == "phong")
		return make_shared<Phong>(j);
	else if (type == "blinnphong")
        return make_shared<BlinnPhong>(j);
    else if (type == "beckmann")
        return make_shared<Beckmann>(j);
    else if (type == "oren-nayar")
        return make_shared<OrenNayar>(j);
	else
		throw DirtException("Unknown 'material' type '%s' here:\n%s.", type, j.dump(4));
}

shared_ptr<PhaseFunction> parsePhase(const json &j)
{
    string type = getKey("type", "phase", j);
    if (type == "hg")
        return make_shared<HenyeyGreenstein>(j);
    else
        throw DirtException("Unknown 'phase' type '%s' here:\n%s.", type, j.dump(4));
}

shared_ptr<Medium> parseMedium(const json &j)
{
    string type = getKey("type", "medium", j);
    if (type == "homogeneous")
        return make_shared<HomogeneousMedium>(j);
    else if (type == "perlin")
        return make_shared<PerlinMedium>(j);
    else
        throw DirtException("Unknown 'medium' type '%s' here:\n%s.", type, j.dump(4));
}

shared_ptr<Texture> parseTexture(const json & j)
{
    if (j.is_object())
    {
	    // create a new texture
        string type = getKey("type", "texture", j);

        if (type == "constant")
            return make_shared<ConstantTexture>(j);
        else if (type == "checker")
            return make_shared<CheckerTexture>(j);
        else if (type == "marble")
            return make_shared<MarbleTexture>(j);
        if (type == "image")
            return make_shared<ImageTexture>(j);
        else
            throw DirtException("Unknown texture type '%s' here:\n%s", type.c_str(), j.dump(4));
    }
    else if (j.is_array() || j.is_number())
        return make_shared<ConstantTexture>(j);
    else
        return Texture::defaultTexture();
}

shared_ptr<Sampler> parseSampler(const json &j)
{
    if (j.is_object())
    {
        string type = getKey("type", "sampler", j);
        if (type == "independent")
            return make_shared<IndependentSampler>(j);
        else if (type == "stratified")
            return make_shared<StratifiedSampler>(j);
        else if (type == "halton")
            return make_shared<HaltonSampler>(j);
        else
            throw DirtException("Unknown sampler type '%s' here:\n%s", type.c_str(), j.dump(4));
    }
    return Sampler::defaultSampler();
}

void parseSurface(const Scene & scene, SurfaceBase * parent, const json & j)
{
    string type = getKey("type", "surface", j);

    if (type == "quad")
        parent->addChild(make_shared<Quad>(scene, j));
    else if (type == "sphere")
        parent->addChild(make_shared<Sphere>(scene, j));
    else if (type == "mesh")
    {
        auto xform = Transform();
        xform = j.value("transform", xform);
        std::string filename = j["filename"];

        auto mesh = make_shared<Mesh>(loadWavefrontOBJ(getFileResolver().resolve(filename).str(), xform));

        if (mesh->empty())
            return;

        mesh->material = scene.findOrCreateMaterial(j);
        mesh->medium_interface = scene.findOrCreateMediumInterface(j);
        for (auto index : range(mesh->F.size()))
            parent->addChild(make_shared<Triangle>(scene, j, mesh, int(index)));
    }
    else
        throw DirtException("Unknown surface type '%s' here:\n%s",
                            type.c_str(), j.dump(5));
}


void Scene::parseFromJSON(const json & j)
{
    message("parsing...\n");

    // first create the scene-wide acceleration structure
    if (j.contains("accelerator"))
        m_surfaces = parseAccelerator(*this, j["accelerator"]);
    else
        // default to a naive accelerator
        m_surfaces = make_shared<SurfaceGroup>(*this, j["accelerator"]);

    if (j.contains("sampler"))
        m_sampler = parseSampler(j["sampler"]);
    else
        m_sampler = std::make_shared<IndependentSampler>(json::object());

    // now loop through all keys in the json file and take the appropriate action
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        if (it.key() == "accelerator")
        {
            // already handled above
        }
        else if (it.key() == "sampler")
        {
            // already handled above
        }
        else if (it.key() == "camera")
        {
            if (m_camera)
                throw DirtException("There can only be one camera per scene!");
            m_camera = make_shared<Camera>(it.value());
        } 
        else if (it.key() == "image_samples")
        {
            m_imageSamples = it.value();
        }
        else if (it.key() == "integrator")
        {
            if (m_integrator)
                throw DirtException("There can only be one integrator per scene!");
            m_integrator = parseIntegrator(it.value());
        }
        else if (it.key() == "background")
        {
            m_background = it.value();
        }
        else if (it.key() == "materials")
        {
            for (auto & m : it.value())
            {
                auto material = parseMaterial(m);
                m_materials[getKey("name", "material", m)] = material;
            }
        }
        else if (it.key() == "media")
        {
            for (auto & m : it.value())
            {
                auto medium = parseMedium(m);
                m_media[getKey("name", "media", m)] = medium;
            }
        }
        else if (it.key() == "surfaces")
        {
            for (auto & s : it.value())
                parseSurface(*this, this, s);
        }
        else
            throw DirtException("Unsupported key '%s' here:\n%s", it.key(), it.value().dump(4));
    }
    
    // if (!m_integrator)
    //     throw DirtException("No integrator specified in scene!");

    if (!m_camera)
        throw DirtException("No camera specified in scene!");

    m_surfaces->build();
    message("done parsing scene.\n");
}