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

#include <dirt/scene.h>

shared_ptr<Scene> createSphereScene()
{
    std::string test = R"(
    {
        "camera":
        {
            "transform": { "o": [0,0,2.5] },
            "resolution": [ 512, 512 ]
        },
        "surfaces": [
            {
                "type": "sphere",
                "material": { "type": "lambertian", "albedo": [0.6,0.6,0.6] }
            }
        ],
        "image_samples": 10,
        "background": [0.1, 0.1, 0.1]
    }
    )";
    return make_shared<Scene>(json::parse(test));
}

shared_ptr<Scene> createSpherePlaneScene()
{
    std::string test = R"(
    {
        "camera":
        {
            "transform": { "o": [0,0,4] },
            "resolution": [ 512, 512 ]
        },
        "surfaces": [
            {
                "type": "sphere",
                "radius": 1,
                "material": { "type": "lambertian", "albedo": [0.6,0.4,0.4] }
            },
            {
                "type": "quad",
                "transform": { "o": [0,-1,0], "x": [1,0,0], "y": [0,0,-1], "z": [0,1,0] },
                "size": [ 100, 100 ],
                "material": { "type": "lambertian", "albedo": [1,1,1] }
            }
        ],
        "image_samples": 100,
        "background": [1, 1, 1]
    }
    )";
    return make_shared<Scene>(json::parse(test));
}

shared_ptr<Scene> createSteinbachScene()
{
    json jobj;

    // Compose the camera
    jobj["camera"] =
	    {
		    {"transform", {{"from", {5.0, 15.0, -25.0}},
		                   {"to",   {0.0,  0.0,   0.0}},
		                   {"up",   {0.0,  1.0,   0.0}}}},
		    {"vfov", 22},
            {"resolution", { 512, 512 }}
	    };

    // compose the image properties
    jobj["image_samples"] = 100;
    jobj["background"] = {1, 1, 1};

    jobj["accelerator"] =
       {
           {"type", "bbh"}
       };

    Vec3f objectCenter(0.0f, 0.0f, 0.0f);
    float radius = 0.5f;
    int numI = 40;
    int numJ = 40;
    for (auto i : range(numI))
    {
        for (auto j : range(numJ))
        {
            float s = (i + 0.5f) / numI;
            float t = (j + 0.5f) / numJ;
            float u = s * (8) - 4.0f;
            float v = t * (6.25f);
            Vec3f center(-u * cos(v), v * cos(u) * 0.75f, u * sin(v));
            Color3f kd = 0.35f*lerp(lerp(Color3f(0.9f, 0.0f, 0.0f), Color3f(0.0f, 0.9f, 0.0f), t),
                                    lerp(Color3f(0.0f, 0.0f, 0.9f), Color3f(0.0f, 0.0f, 0.0f), t), s);

	        jobj["surfaces"] +=
	            {
	                {"type", "sphere"},
	                {"radius", radius},
	                {"transform", {{ "o", objectCenter + center },
	                               { "x", { 1.0, 0.0, 0.0 } },
	                               { "y", { 0.0, 1.0, 0.0 } },
	                               { "z", { 0.0, 0.0, 1.0 } }}},
	                {"material", {{"type", "lambertian"},
                                  {"albedo", kd}}}
				};
        }
    }

    jobj["surfaces"] +=
        {
            {"type", "quad"},
            {"size", {100, 100}},
            {"transform", {{ "o", { 0.0,-5.0, 0.0} },
                           { "x", { 1.0, 0.0, 0.0 } },
                           { "y", { 0.0, 0.0,-1.0 } },
                           { "z", { 0.0, 1.0, 0.0 } }}},
            {"material", {{"type", "lambertian"}, {"albedo", 0.9}}}
        };

    return make_shared<Scene>(jobj);
}

shared_ptr<Scene> createShirleyScene()
{
    json jobj;

    // Compose the camera
    jobj["camera"] =
        {
            {"transform", {{"from", {13,2,3}},
                           {"to",   {0,0,0}},
                           {"up",   {0,1,0}}}},
            {"vfov", 20},
            {"fdist", 10},
            {"aperture", 0.1},
            {"resolution", { 600, 400 }}
        };

    // compose the image properties
    jobj["image_samples"] = 100;

    jobj["background"] = {1,1,1};

    jobj["accelerator"] =
       {
           {"type", "bbh"},
           {"maxPrimsInNode", 3},
           {"splitMethod", "sah"}
       };


    // ground plane
    jobj["surfaces"] +=
        {
            {"type", "quad"},
            {"size", {100, 100}},
            {"transform", {{ "o", { 0.0, 0.0, 0.0} },
                           { "x", { 1.0, 0.0, 0.0 } },
                           { "y", { 0.0, 0.0,-1.0 } },
                           { "z", { 0.0, 1.0, 0.0 } }}},
            {"material", {{"type", "lambertian"},
                          {"albedo", {0.5, 0.5, 0.5} }}}
        };


    for (int a = -11; a < 11; a++)
    {
        for (int b = -11; b < 11; b++)
        {
            float choose_mat = randf();
            Vec3f center(a+0.9f*randf(),0.2f,b+0.9f*randf());
            if (length(center-Vec3f(4.0f,0.2f,0.0f)) > 0.9f)
            {
                json sphere =
                {
                    {"type", "sphere"},
                    {"radius", 0.2f},
                    {"transform", {{ "translate", center }}}
                };

                if (choose_mat < 0.8)
                {  // diffuse
                    Color3f albedo(randf()*randf(), randf()*randf(), randf()*randf());
                    sphere["material"] =
                        {
                            {"type", "lambertian"},
                            {"albedo", albedo}
                        };
                }
                else if (choose_mat < 0.95)
                { // metal
                    Color3f albedo(0.5f*(1 + randf()), 0.5f*(1.0f + randf()), 0.5f*(1.0f + randf()));
                    float rough = 0.5f*randf();
                    sphere["material"] =
                        {
                            {"type", "metal"},
                            {"albedo", albedo},
                            {"roughness", rough}
                        };
                }
                else
                {  // glass
                    sphere["material"] =
                        {
                            {"type", "dielectric"},
                            {"ior", 1.5}
                        };
                }

                jobj["surfaces"] += sphere;
            }
        }
    }


    jobj["surfaces"] +=
        {
            {"type", "sphere"},
            {"radius", 1.f},
            {"transform", {{ "translate", {0,1,0} }}},
            {"material", {{"type", "dielectric"},
                          {"ior", 1.5}}}
        };
    jobj["surfaces"] +=
        {
            {"type", "sphere"},
            {"radius", 1.f},
            {"transform", {{ "translate", {-4,1,0} }}},
            {"material", {{"type", "lambertian"},
                          {"albedo", {0.4,0.2,0.1} }}}
        };
    jobj["surfaces"] +=
        {
            {"type", "sphere"},
            {"radius", 1.f},
            {"transform", {{ "translate", {4,1,0} }}},
            {"material", {{"type", "metal"},
                          {"albedo", {0.7, 0.6, 0.5} },
                          {"roughness", 0.0 }}}
        };

    return make_shared<Scene>(jobj);
}


shared_ptr<Scene> createTestScene(int sceneNumber)
{
    switch (sceneNumber)
    {
        case 0: return createSphereScene();
        case 1: return createSpherePlaneScene();
        case 2: return createSteinbachScene();
        case 3: return createShirleyScene();
        default:
            throw DirtException("Invalid hardcoded scene number %d. Must be 0..3.", sceneNumber);
    }
}
