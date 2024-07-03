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

#include <dirt/parser.h>
#include <dirt/common.h>
#include <dirt/image.h>
#include <dirt/camera.h>
#include <dirt/material.h>
#include <dirt/surfacegroup.h>
#include <dirt/texture.h>
#include <dirt/integrator.h>
#include <dirt/medium.h>

/**
    Main scene data structure.
   
    This class aggregates all the surfaces and materials along with the camera.
    It provides convenience functions for querying materials by name.
 */
class Scene : public SurfaceBase
{
public:
    Scene() = default;
    /// Construct a new scene from a json object
    Scene(const json & j);
    /// Read a scene from a json file
    Scene(const string & filename);

    /// Parser a scene from a json object
    void parseFromJSON(const json & j);

    /// Release all memory
    virtual ~Scene();

    void addChild(shared_ptr<SurfaceBase> surface) override
    {
        m_surfaces->addChild(surface);
        if (surface->isEmissive())
            m_emitters.addChild(surface);
    }

    bool intersect(const Ray3f & ray, HitInfo & hit) const override
    {
        return m_surfaces->intersect(ray, hit);
    }

    Box3f localBBox() const override {return m_surfaces->localBBox();}

    /**
        Find/create a material.
       
        Return a Material pointer by parsing the "material" specification.
        If \c j is a string "material": "material-name", then try to find
        a material with name "material-name" in the pre-declared map of scene
        materials. If \c j is a json object "material": {}, then create a new
        material with the specified parameters.
     */
    shared_ptr<const Material> findOrCreateMaterial(const json & j, const string & key = "material") const;

    /**
        Find/create a medium.
       
        Return a Medium pointer by parsing the "medium" specification.
        If \c j is a string "medium": "medium-name", then try to find
        a medium with name "medium-name" in the pre-declared map of scene
        media. If \c j is a json object "medium": {}, then create a new
        medium with the specified parameters.
     */
    shared_ptr<const Medium> findOrCreateMedium(const json & j, const string & key) const;

    /**
        Find/create a medium interface.
     */
    shared_ptr<const MediumInterface> findOrCreateMediumInterface(const json & j, const string & key = "medium_interface") const;

    /// Return a const reference to the emitters
    const SurfaceBase & emitters() const {return m_emitters;}

    /// Return the background color
    Color3f background() const {return m_background;}

    /**
        Sample the color along a ray

        \param ray  The ray in question
        \return     An estimate of the color from this direction
     */
    Color3f recursiveColor(Sampler &sampler, const Ray3f &ray, int depth) const;

    /// Generate the entire image by ray tracing.
    Image3f raytrace() const;

    /// Generate the entire image by ray tracing.
    Image3f integrateImage() const;

private:
    shared_ptr<Camera> m_camera;
    map<string, shared_ptr<const Material>> m_materials;
    map<string, shared_ptr<const Medium>> m_media;
    shared_ptr<SurfaceGroup> m_surfaces;
    SurfaceGroup m_emitters {*this};
    Color3f m_background = Color3f(0.2f);
    shared_ptr<Integrator> m_integrator;
    shared_ptr<Sampler> m_sampler;

    int m_imageSamples = 1;                      ///< samples per pixels in each direction
};

// create test scenes that do not need to be loaded from a file
shared_ptr<Scene> createTestScene(int sceneNumber);