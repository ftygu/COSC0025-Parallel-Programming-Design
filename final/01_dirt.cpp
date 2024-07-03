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
#include <dirt/argparse.h>
#include <time.h>  
#include <filesystem/resolver.h>

// runs the raytrace over all tests and saves the corresponding images
int main(int argc, char** argv)
{
    // PROGRAMMING ASSIGNMENT, PART 4/5: Ray tracing & Shading
    // =======================================================

    auto cmd = CommandlineParser({
                               "dirt",
                               {
                                   {"help", "h",  "Display this help screen and quit", typeid(bool), true, json()},
	                               {"outfile", "o",  "Specify the output image filename (extension must be one of: .png, .jpg, .hdr, .bmp, or .tga)", typeid(string), true, json()},
                                   {"format", "f",  "Specify just the output image format (png, jpg, hdr, bmp, or tga)", typeid(string), true, "png"},
                                   {"verbosity", "v",  "Specify the level of verbosity [0,1,2,3, or 4]", typeid(int), true, 3}
                               },
                               {
                                   {"scene.json", "",  "The filename of the JSON scenefile to load (or the string \"testsceneX\", where X is 0, 1, 2, or 3).", typeid(string), false, json("")},
                               }
                           });

    auto args = cmd.parse(argc, argv);

    if (!args["help"].empty())
        cmd.printUsageAndQuit();

    try
    {
        setVerbosity(args["verbosity"].get<int>());

        // check output format option
        if (!Image3f::canSave().count(args["format"].get<string>()))
            throw DirtException("Invalid output image file format: '%s'.\n",
                                args["format"].get<string>());

        // generate/load scene either by creating one of the hardcoded test
        // scenes or loading from json file
        auto sceneFile = args["scene.json"].get<string>();
        shared_ptr<Scene> scene;
        int sceneNumber = 0;
        if (sscanf(sceneFile.c_str(), "testscene%d", &sceneNumber) == 1)
            scene = createTestScene(sceneNumber);
        else
        {
            filesystem::path path(sceneFile);

            // Add the parent directory of the scene file to the file
            // resolver. That way, the scene file can reference resources
            // (OBJ files, textures) using relative paths
            getFileResolver().prepend(path.parent_path());

            scene = make_shared<Scene>(sceneFile);
        }

        auto now = time(nullptr);
        char buf[sizeof "2011-10-08-07-07-09"];
        // this will work too, if your compiler doesn't support %F or %T:
        strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M-%S", localtime(&now));

        // use the outfile if specified, otherwise take the basename from the
        // scene file and append the time.
        string outFile, outFileHdr;
        if (args["outfile"].empty()) {
            auto base = sceneFile.substr(0, sceneFile.find_last_of('.')) + "-" + buf + ".";
            outFile = base + args["format"].get<string>();
            outFileHdr = base + "hdr";
        } else {
            outFile = args["outfile"].get<string>();
        }

        message("Will save rendered image to \"%s\"\n", outFile);

        auto image = scene->raytrace();

        message("Average number of intersection tests per ray: %f \n",
                float(intersection_tests) / float(rays_traced));
        message("Writing rendered image to file \"%s\"...\n", outFile);

        image.save(outFile);
        if (!outFileHdr.empty())
            image.save(outFileHdr);

        message("done!\n");
    }
    catch (const std::exception& e)
    {
        error("%s\n", e.what());
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
