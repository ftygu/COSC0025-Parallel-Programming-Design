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
#include <dirt/image.h>
#include <dirt/camera.h>
#include <dirt/sphere.h>
#include <dirt/surfacegroup.h>
#include <dirt/progress.h>

Color3f vec2color(const Vec3f & dir);
Color3f rayToColor(const Ray3f & r);
Color3f intersectionToColor(const Ray3f & r, const Sphere & sphere);
Color3f recursiveColor(const Ray3f &ray, const SurfaceGroup &scene, int depth);
void functionWithJSONParameters(const json & j);
void testColorAndImage();
void testVectorsAndMatrices();
void testManualCameraImage();
void testJSON();
void testCameraClassImage();
void testTransforms();
void testXformedCameraImage();
void testRaySphereIntersection();
void testSphereImage();
void testMaterials();
void testRecursiveRaytracing();

int main(int argc, char **argv)
{
  testManualCameraImage();
	testJSON();
	testCameraClassImage();

	testTransforms();
	testXformedCameraImage();

	testRaySphereIntersection();
	testSphereImage();

	testMaterials();
	testRecursiveRaytracing();

  return 0;
}


// Generate rays by hand
void testManualCameraImage()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 1: Generating rays by hand \n";
	cout << "--------------------------------------------------------\n";

	// Setup the output image
	Image3f rayImage(200, 100);

	const Vec3f cameraOrigin(0.f, 0.f, 0.f);
	const float imagePlaneWidth = 4.f;
	const float imagePlaneHeight = 2.f;

	// loop over all pixels and generate a ray
	for (auto y : range(rayImage.height()))
	{
		for (auto x : range(rayImage.width()))
		{
			// TODO: Fill in rayOrigin so that the ray starts at the
			// cameraOrigin, and fill in rayDirection so that
			// 1) the x component of the direction varies from
			//    -imagePlaneWidth/2 for the left-most pixel to
			//    +imagePlaneWidth/2 for the right-most pixel
			// 2) the y component of the direction varies from
			//    +imagePlaneHeight/2 for the top-most pixel to
			//    -imagePlaneHeight/2 for the bottom-most pixel
			// 3) the z component is -1

			// Generate the ray through the center of the pixel (hence the +0.5)
			float u = (x + 0.5f) / rayImage.width();
			float v = (y + 0.5f) / rayImage.height();

			auto ray = Ray3f(
				cameraOrigin,
				Vec3f(
					(u - 0.5f) * imagePlaneWidth,
					(0.5f - v) * imagePlaneHeight,
					-1.f
				)
			);

			// Generate a visual color for the ray so we can debug our ray directions
			rayImage(x, y) = rayToColor(ray);
		}
	}

	string filename("scenes/01_raytrace/01_manual_ray_image.png");
	message("Saving ray image to %s....\n", filename);
	rayImage.save(filename);
}

void testJSON()
{
	// DIRT also includes a C++ library (https://github.com/nlohmann/json)
	// for parsing and manipulating JSON data.
	//
	// JSON is a human-readible data interchange format for expressing
	// attribute-value pairs. You can read more about it here:
	//      https://en.wikipedia.org/wiki/JSON
	//      https://www.json.org/
	//
	// In DIRT, we will use it for two purposes:
	//  1) As a generic way to pass named parameters to functions
	//  2) As a way to specify and load text-based scene files

	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 2: passing data using JSON \n";
	cout << "--------------------------------------------------------\n";

    float f(2.f);
    string s("a text string");
    Color3f c3f(1.0f, .25f, .5f);
    Vec3f v3f(2, 3, 4);
    Vec4f v4f(2, 3, 4, 5);
    Vec3f n3f(2, 3, 4);
    message("Original DIRT data:\nf = %f,\ns = %s,\nc3f = %s,\nv3f = %s,\nv4f "
            "= %s,\nn3f = %s.\n",
            f, s, c3f, v3f, v4f, n3f);

    // All the basic DIRT data-types can easily be stored in a JSON object
    json j;
	j["my float"]   = f;
	j["my string"]  = s;
	j["my color"]   = c3f;
	j["my vector3"] = v3f;
	j["my normal"]  = n3f;
	message("The JSON object contains:\n%s.\n", j.dump(4));

	// We can also read these structures back out of the JSON object
	float      f2 = j["my float"];
	string     s2 = j["my string"];
	Color3f c3f2 = j["my color"];	
	Vec3f v3f2 = j["my vector3"];
	Vec3f n3f2 = j["my normal"];

    message("Retrieved DIRT data:\nf2 = %f,\ns2 = %s,\nc3f2 = %s,\nv3f2 = "
            "%s,\nn3f2 = %s.\n",
            f2, s2, c3f2, v3f2, n3f2);
    // TODO: There is a bug in the code above, and c3f2 doesn't have the same
    // value as the original c3f. Fix it.

    // Now we will pass a json object in place of explicit parameters to
	// a function. Go to the function below and implement the TODO.
	json parameters = {{"radius", 2.3f}};
	functionWithJSONParameters(parameters);
}

void functionWithJSONParameters(const json & j)
{
    // Many of the constructors for ray tracing in DIRT take a JSON object. This
    // allows us to have a uniform interface for creating these structures while
    // allowing the constructors to retrieve the necessary values from the JSON
    // object. This will simplify our code for writing a parser for reading
    // scene files from disk.

    // Sometimes we may want to make a parameter optional, and take on some
    // default value if it is not specified.
    // Unfortunately, checking for a missing parameter using e.g. j["radius"]
    // will throw an exception if the parameter doesn't exist.
    // Instead, we can use j.value<type>("name", defaultValue) to extract it.
    // This is what the constructors to Camera, Sphere, Quad, and Materials do.

    // TODO: Extract the parameters radius (default=1.f), and center
    // (default={0,0,0})
	float radius = j.value<float>("radius", 1.f);
	Vec3f center = j.value<Vec3f>("center", {0,0,0});
}


// Next, we will generate the same image, but using the Camera class
void testCameraClassImage()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 3: Camera class generateRay\n";
	cout << "--------------------------------------------------------\n";

	// Setup the output image
	Image3f rayImage(200, 100);

	// Set up a camera with some reasonable parameters, using JSON
	// TODO: Look in camera.h and implement the camera constructor
	Camera camera({
		              {"vfov", 90.f},
		              {"resolution", Vec2i(rayImage.width(), rayImage.height())},
		              {"fdist", 1.f}
	              });

	// loop over all pixels and ask the camera to generate a ray
	for (auto y : range(rayImage.height()))
	{
		for (auto x : range(rayImage.width()))
		{
			// TODO: Look in camera.h|cpp and implement Camera::generateRay

			// We add 0.5 to the pixel coordinate to center the ray within the pixel
			auto ray = camera.generateRay(x + 0.5f, y + 0.5f);
			rayImage(x, y) = rayToColor(ray);
		}
	}

	string filename("scenes/01_raytrace/01_camera_ray_image.png");
	message("Saving ray image to %s....\n", filename);
	rayImage.save(filename);
}

void testTransforms()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 4: Transforms              \n";
	cout << "--------------------------------------------------------\n";

    // DIRT also provides you with a Transform class.
    // Transform is a helper class that helps you transform geometric primitives
    // correctly Internally, it keeps track of a transformation matrix and its
    // inverse

    // Let's create a random transformation matrix
	Mat44f transformationMatrix({-0.846852f,  0.107965f, -0.520755f, 0.0f},
								{-0.492958f, -0.526819f,  0.692427f, 0.0f},
								{-0.199586f,  0.843093f,  0.499359f, 0.0f},
								{-0.997497f,  0.127171f, -0.613392f, 1.0f});

	// Now that we have a matrix, we can create a transform from it:
	Transform transform{transformationMatrix};

    // TODO: Go to transform.h and implement all required methods there. If you
    // implement them correctly, the code below will work:

    // Let's create some random geometric objects...

    Vec3f vector{-0.997497f, 0.127171f, -0.6133920f};
	Vec3f point { 0.617481f, 0.170019f, -0.0402539f};
	Vec3f normal{-0.281208f, 0.743764f,  0.6064130f};
	Ray3f ray {{-0.997497f, 0.127171f, -0.613392f},
			   {0.962222f, 0.264941f, -0.0627278f}};

	message("vector = %s.\n", vector);
	message("point  = %s.\n", point);
	message("normal = %s.\n", normal);
	message("ray.o  = %s;\nray.d  = %s.\n", ray.o, ray.d);

    // ...and let's transform them!
    // We can transform things simply by multiplying it with the transform.
    // Let's check if you did it correctly:
    Vec3f transformedVector = transform.vector(vector);
	Vec3f transformedPoint  = transform.point(point);
	Vec3f transformedNormal = transform.normal(normal);
	Ray3f transformedRay    = transform.ray(ray);

	Vec3f correctTransformedVector( 0.904467f, -0.6918370f,  0.301205f);
	Vec3f correctTransformedPoint (-1.596190f,  0.0703303f, -0.837324f);
	Vec3f correctTransformedNormal(-0.249534f, 0.0890737f, 0.96426f);
	Vec3f correctTransformedRayPosition(-0.0930302f, -0.564666f, -0.312187f);
	Vec3f correctTransformedRayDirection(-0.932945f, -0.088575f, -0.348953f);

    float vectorError = max(abs(correctTransformedVector - transformedVector));
    float pointError = max(abs(correctTransformedPoint - transformedPoint));
    float normalError = max(abs(correctTransformedNormal - transformedNormal));
    float rayError =
        std::max(max(abs(correctTransformedRayPosition - transformedRay.o)),
                 max(abs(correctTransformedRayDirection - transformedRay.d)));

    message("The forward transform matrix is\n%s.\n", transform.m);
	message("The inverse transform matrix is\n%s.\n", transform.mInv);

	message("Result of transform*vector is:\n%s, and it should be:\n%s.\n",
		    transformedVector, correctTransformedVector);
	if (vectorError > 1e-5f)
		error("Result incorrect!\n\n");
	else
		success("Result correct!\n\n");

	message("Result of transform*point is:\n%s, and it should be:\n%s.\n",
		    transformedPoint, correctTransformedPoint);
	if (pointError > 1e-5f)
		error("Result incorrect!\n\n");
	else
		success("Result correct!\n\n");

	message("Result of transform*normal is:\n%s, and it should be:\n%s.\n",
		    transformedNormal, correctTransformedNormal);
	if (normalError > 1e-5f)
		error("Result incorrect!\n\n");
	else
		success("Result correct!\n\n");

	message("transform*ray: transformedRay.o is:\n%s, and it should be:\n%s.\n",
		    transformedRay.o, correctTransformedRayPosition);
	message("transform*ray: transformedRay.d is:\n%s, and it should be:\n%s.\n",
		    transformedRay.d, correctTransformedRayDirection);
	if (rayError > 1e-5f)
		error("Result incorrect!\n\n");
	else
		success("Result correct!\n\n");
}


// Finally, we will allow our camera to be positioned and oriented using a
// Transform
void testXformedCameraImage()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 5: Transformed camera      \n";
	cout << "--------------------------------------------------------\n";

	// Setup the output image
	Image3f rayImage(200, 100);

	// Set up a camera with some reasonable parameters
	// TODO: Look in camera.h and implement the camera constructor
	Camera camera({
		              {"vfov", 90.f},
		              {"resolution", Vec2i(rayImage.width(), rayImage.height())},
		              {"fdist", 1.f},
		              {"transform", {{"from", {5.0, 15.0, -25.0}},
		                             {"to",   {0.0,  0.0,   0.0}},
		                             {"up",   {0.0,  1.0,   0.0}}} },
	              });

	// Generate a ray for each pixel in the ray image
	for (auto y : range(rayImage.height()))
	{
		for (auto x : range(rayImage.width()))
		{
			// TODO: Look in camera.h|cpp and implement camera.generateRay

			// Make sure to take the camera transform into account!

			// We add 0.5 to the pixel coordinate to center the ray within the pixel
			auto ray = camera.generateRay(x + 0.5f, y + 0.5f);
			rayImage(x, y) = rayToColor(ray);
		}
	}

	string filename("scenes/01_raytrace/01_xformed_camera_ray_image.png");
	message("Saving ray image to %s....\n", filename);
	rayImage.save(filename);
}

// Now, we will implement ray-sphere intersection
void testRaySphereIntersection()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 6: Ray-Sphere intersection \n";
	cout << "--------------------------------------------------------\n";

	// TODO: Go to sphere.cpp and implement Sphere::intersect

	// Let's check if your implementation was correct:
	Sphere testSphere(1.f);

	message("Testing untransformed sphere intersection\n");
	Ray3f testRay(Vec3f(-0.25f, 0.5f, 4.0f), Vec3f(0.0f, 0.0f, -1.0f));
	HitInfo hit;
	if (testSphere.intersect(testRay, hit))
	{
		float correctT = 3.170844f;
		Vec3f correctP(-0.25f, 0.5f, 0.829156f);
		Vec3f correctN(-0.25f, 0.5f, 0.829156f);

		message("Hit sphere! Distance is:\n%f, and it should be:\n%f.\n", hit.t, correctT);
		message("Intersection point is:\n%s, and it should be:\n%s.\n", hit.p, correctP);
		message("Intersection normal is:\n%s, and it should be:\n%s.\n", hit.sn, correctN);

		float sphereError = std::max(std::max(max(abs(correctP-hit.p)),
		                                      max(abs(correctN-hit.sn))),
		                             std::abs(correctT - hit.t));

		if (sphereError > 1e-5f)
			error("Result incorrect!\n");
		else
			success("Result correct!\n");
	}
	else
		error("Sphere intersection incorrect! Should hit sphere\n");

	// Now, let's check if you implemented sphere transforms correctly!
	auto transform = Transform::axisOffset(
		Vec3f(2.0,  0.0,   0.0), // x-axis
		Vec3f(0.0,  1.0,   0.0), // y-axis
		Vec3f(0.0,  0.0,   0.5), // z-axis
		Vec3f(0.0, 0.25,   5.0)  // translation
	);
	Sphere transformedSphere(1.0f, nullptr, transform);
	testRay = Ray3f(Vec3f(1.0f, 0.5f, 8.0f), Vec3f(0.0f, 0.0f, -1.0f));
	hit = HitInfo();

	message("Testing transformed sphere intersection\n");
	if (transformedSphere.intersect(testRay, hit))
	{
		float correctT = 2.585422f;
		Vec3f correctP(1.0f, 0.5f, 5.41458f);
		Vec3f correctN(0.147442f, 0.147442f, 0.978019f);

        message("Hit sphere! Distance is:\n%f, and it should be:\n%f.\n", hit.t,
                correctT);
        message("Intersection point is:\n%s, and it should be:\n%s.\n", hit.p,
                correctP);
        message("Intersection normal is:\n%s, and it should be:\n%s.\n", hit.sn,
                correctN);

        float sphereError = std::max(std::max(max(abs(correctP-hit.p)),
		                                      max(abs(correctN-hit.sn))),
		                             std::abs(correctT - hit.t));

		if (sphereError > 1e-5f)
			error("Result incorrect!\n");
		else
			success("Result correct!\n");
	}
	else
		error("Transformed sphere intersection incorrect! Should hit sphere\n");
}

// Now: Let's allow our camera to be positioned and oriented using a Transform,
// and will use it to raytrace a sphere
void testSphereImage()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 7: False-color sphere image\n";
	cout << "--------------------------------------------------------\n";

	// Setup the output image
	Image3f rayImage(200, 100);

	// Set up a camera with some reasonable parameters
	Camera camera({
		              {"vfov", 90.f},
		              {"resolution", Vec2i(rayImage.sizeX(), rayImage.sizeY())},
		              {"fdist", 1.f},
		              {"transform", {{"from", {5.0, 15.0, -25.0}},
		                             {"to",   {0.0,  0.0,   0.0}},
		                             {"up",   {0.0,  1.0,   0.0}}} },
	              });

	Sphere sphere(20.f);

	// Generate a ray for each pixel in the ray image
	for (auto y : range(rayImage.height()))
	{
		for (auto x : range(rayImage.width()))
		{
			// TODO: Look in camera.h and implement camera.generateRay

			// Make sure to take the camera transform into account!

            // We add 0.5 to the pixel coordinate to center the ray within the
            // pixel
            auto ray = camera.generateRay(x + 0.5f, y + 0.5f);

            // If we hit the sphere, output the sphere normal; otherwise,
            // convert the ray direction into a color so we can have some visual
            // debugging
            rayImage(x, y) = intersectionToColor(ray, sphere);
		}
	}

	string filename("scenes/01_raytrace/01_xformed_camera_sphere_image.png");
    message("Saving ray image to %s....\n", filename);
    rayImage.save(filename);
}

// It's time to test materials!
void testMaterials()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 8: Materials               \n";
	cout << "--------------------------------------------------------\n";

	// TODO: Go to material.cpp and implement the lambertian and metal material

	// Let's create a red lambertian surface
	Color3f surfaceColor = Color3f(1.0f, 0.25f, 0.25f);
	Lambertian lambertMaterial(json{{"albedo", surfaceColor}});

	// And a slightly shiny metal surface
	Metal metalMaterial(json{{"albedo", surfaceColor}, {"roughness", 0.3f}});

	// Let's create a fictitious hitpoint
	Vec3f surfacePoint(1.0f, 2.0f, 0.0f);
	Vec3f normal = normalize(Vec3f(1.0f, 2.0f, -1.0f));
	HitInfo hit(0.0f, surfacePoint, normal, normal, Vec2f(0.0f, 0.0f));
	// And a fictitious ray
	Ray3f ray(Vec3f(2.0f, 3.0f, -1.0f), Vec3f(-1.0f, -1.0f, 1.0f));

	// Now, let's test your implementation!
	Ray3f lambertScattered;
	Color3f lambertAttenuation;
	message("Testing lambert scatter\n");
	if (lambertMaterial.scatter(ray, hit, Vec2f(randf(), randf()), lambertAttenuation, lambertScattered))
	{
		Vec3f correctOrigin = surfacePoint;
		Color3f correctAttenuation = surfaceColor;
		Vec3f correctDirection(1.206627e+00f, 3.683379e-01f,-8.104229e-01f);

        message("Scattered ray origin is:\n%s, and it should be:\n%s.\n",
                lambertScattered.o, correctOrigin);
        message("Attenuation is:\n%s, and it should be:\n%s.\n",
                lambertAttenuation, correctAttenuation);
        message("Ray direction is:\n%s, and it should be:\n%s.\n",
                lambertScattered.d, correctDirection);

        float lambertError = std::max(
            std::max(max(abs(correctOrigin - lambertScattered.o)),
                     max(abs(lambertAttenuation - correctAttenuation))),
            max(abs(correctDirection - lambertScattered.d)));

        if (lambertError > 1e-5f)
			error("Result incorrect!\n");
		else
			success("Result correct!\n");
	}
	else
        error("Lambert scatter incorrect! Scattering should have been "
              "successful\n");

    Ray3f metalScattered;
	Color3f metalAttenuation;
	message("Testing metal scatter\n");
	if (metalMaterial.scatter(ray, hit, Vec2f(randf(), randf()), metalAttenuation, metalScattered))
	{
		Vec3f correctOrigin = surfacePoint;
		Color3f correctAttenuation = surfaceColor;
		Vec3f correctDirection(2.697650e-01, 9.322242e-01,-2.421507e-01);

        message("Scattered! Ray origin is:\n%s, and it should be:\n%s.\n",
                metalScattered.o, correctOrigin);
        message("Attenuation is:\n%s, and it should be:\n%s.\n",
                metalAttenuation, correctAttenuation);
        message("Ray direction is:\n%s, and it should be:\n%s.\n",
                metalScattered.d, correctDirection);

        float metalError =
            std::max(std::max(max(abs(correctOrigin - metalScattered.o)),
                              max(abs(metalAttenuation - correctAttenuation))),
                     max(abs(correctDirection - metalScattered.d)));

        if (metalError > 1e-5f)
            error("Result incorrect!\n");
        else
            success("Result correct!\n");
    }
    else
        error("Metal scatter incorrect! Scattering should have been "
              "successful\n");
}

// Now that we can scatter off of surfaces, let's try this out and render a scene with different materials
void testRecursiveRaytracing()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT, PART 9: Recursive Ray Tracing   \n";
	cout << "--------------------------------------------------------\n";

	// Setup the output image
	Image3f rayImage(300, 150);

	// We want to average over several rays to get a more pleasing result
	const int NumSamples = 64;

	// Set up a camera with some reasonable parameters
	Camera camera({
		              {"vfov", 45.f},
		              {"resolution", Vec2i(rayImage.sizeX(), rayImage.sizeY())},
		              {"fdist", 1.f},
		              {"transform", {{"from", {1.9, 0.8, -3.5}},
		                             {"to",   {1.9, 0.8,  0.0}},
		                             {"up",   {0.0, 1.0,  0.0}}} },
	              });

    auto groundMaterial = make_shared<Lambertian>(json{{"albedo", 0.5f}});
    auto matteMaterial =
        make_shared<Lambertian>(json{{"albedo", Vec3f(1.0f, 0.25f, 0.25f)}});
    auto shinyMaterial = make_shared<Metal>(
        json{{"albedo", Vec3f(1.0f, 1.0f, 1.0f)}, {"roughness", 0.3f}});

    auto matteSphere = make_shared<Sphere>(
        1.f, matteMaterial, Transform::translate({3.f, 1.f, 0.f}));
    auto shinySphere = make_shared<Sphere>(
        1.f, shinyMaterial, Transform::translate({0.f, 1.f, 1.f}));
    auto groundSphere = make_shared<Sphere>(
        1000.f, groundMaterial, Transform::translate({0.f, -1000.f, 0.f}));

    // To raytrace more than one object at a time, we can put them into a group
	SurfaceGroup scene;
	scene.addChild(matteSphere);
	scene.addChild(shinySphere);
	scene.addChild(groundSphere);

	{
		Progress progress("Rendering", rayImage.size());
		// Generate a ray for each pixel in the ray image
		for (auto y : range(rayImage.height()))
		{
			for (auto x : range(rayImage.width()))
			{
				INCREMENT_TRACED_RAYS;
				auto ray = camera.generateRay(x + 0.5f, y + 0.5f);

				Color3f color = Color3f(0.0f);
				for (int i = 0; i < NumSamples; ++i)
				{
					color += recursiveColor(ray, scene, 0);
				}
				color /= float(NumSamples);	

				rayImage(x, y) = color;
				++progress;
			}
		}
	}	// progress reporter goes out of scope here


    message("Average number of intersection tests per ray: %f \n", float(intersection_tests) / float(rays_traced));

	string filename("scenes/01_raytrace/01_recursive_raytracing.png");
	message("Saving rendered image to %s....\n", filename);
	rayImage.save(filename);
}

Color3f recursiveColor(const Ray3f &ray, const SurfaceGroup &scene, int depth)
{
	const int MaxDepth = 64;

	// TODO: Implement this function
	// Pseudo-code:
	//
	// if scene.intersect:
	// 		if depth < MaxDepth and hit_material.scatter(....) is successful:
	//			recursive_color = call this function recursively with the scattered ray and increased depth
	//          return attenuation * recursive_color
	//		else
	//			return black;
	// else:
	// 		return white
	HitInfo hit;
	if (scene.intersect(ray, hit))
	{
		Ray3f scattered;
		Color3f attenuation;
		if (depth < MaxDepth && hit.mat->scatter(ray, hit, Vec2f(randf(), randf()), attenuation, scattered))
		{
			return attenuation * recursiveColor(scattered, scene, depth + 1);
		}
		else
		{
			return Color3f(0.0f, 0.0f, 0.0f);
		}
	}
	else
	{
		// We hit the background. Let's simulate a white sky
		return Color3f(1.0f, 1.0f, 1.0f);
	}
	return Color3f(0.0f);

}

Color3f vec2color(const Vec3f & dir)
{
    return 0.5f * (dir + 1.f);
}

Color3f rayToColor(const Ray3f & r)
{
    return vec2color(normalize(r.d));
}

Color3f intersectionToColor(const Ray3f & r, const Sphere & sphere)
{
    HitInfo hit;
    if (sphere.intersect(r, hit))
        return vec2color(normalize(hit.sn));
    else
        return vec2color(normalize(r.d));
}
