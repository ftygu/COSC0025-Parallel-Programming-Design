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

// function declarations; definitions are below main
void testVectorsAndMatrices();
void testColorAndImage();


int main(int argc, char **argv)
{
	testVectorsAndMatrices();
	testColorAndImage();

    return 0;
}


void testVectorsAndMatrices()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT 0, PART 1: Vectors and Matrices  \n";
	cout << "--------------------------------------------------------\n";

    // The DIRT basecode provides a number of useful classes and functions
    // We will walk through the basic functionality of the most important
    // ones.

    // Almost all graphics programs have some data structures for storing and
    // manipulating geometric vectors and colors. DIRT already provides this for
    // you in vec.h. The Vec<N,T> struct is a template parametrized by the
    // dimensionality N, and the type T. This just means that we can use the
    // same generic code to represent vectors containing different types
    // (floats, doubles, integers, etc) and dimensionalities (2D: 2 floats, 3D:
    // 3 floats, etc).
    //
    // In addition to storing N values of type T, the Vec class provides a
    // number of useful operations to manipulate vectors in bulk or access their
    // elements.
    //
    // We can access the elements of a vector using the [] operator
    // (e.g. myvec[0]), but for 2D, 3D, and 4D vectors you can access the
    // individual elements directly as data *member variables* by name (e.g.
    // myvec.x). Note that this is different than in the Shirley book (Shirley
    // defines member *functions* e.g. x(), to access individual elements in his
    // vec3 class.
    //
    // Most arithmetic operations are overloaded so you can perform element-wise
    // addition, multiplication, etc. You can compute the length, dot product,
    // etc. Take a look at vec.h or the Doxygen documentation.
    //
    // Most of the time we will be dealing with the 3D float
    // version of this structure, and to avoid typing out the full template
    // typename "Vec<3,float>" we define a bunch of type aliases at the bottom
    // of vec.h, so we can use the shorthand "Vec3f" for a 3D vector of floats,
    // etc. There are similar ones for 2D and 4D vectors. We will also sometimes
	// use integer versions of these (e.g. Vec2i)
	//
	// We will use the Vec<T,N> type to represent geometric points,
	// direction vectors, and normals.
	//

	// Let's create some 3D vectors
	// We can initialize them using:
	// 1) the 3-element constructor,
	// 2) from a single scalar, or
	// 3) using the array initializer
	Vec3f v1(-0.1f, 0.20f, -0.300f);
	Vec3f v2(1.f);	     			 // This is the same as Vec3f(1,1,1);
	Vec3f v3{ 0.5f, 0.25f, -0.123f};// Same as Vec3f(0.5f, 0.25f, -0.123f)

	// we can also print out vectors to the console:
	// We can do this with cout, etc:
	cout << "v1 = " << v1 << endl;

    // But Dirt also provides some output functions (message, success, warning,
    // error). These operate like printf, but colorize the output based on the
    // type of message. These functions also have better support for Dirt's
    // built-in types, so we can output a Vec like so:

	message("v2 = %s.\n", v2);
	message("v3 = %s.\n\n", v3);

	message("You can access specific components using x, y, and z.\n");
	// TODO: Output the z coordinate of the normal
	message("The z coordinate of v3 is %s.\n\n", "TODO");

	message("We can also element-wise add, subtract, and multiply vectors:\n");
	message("v1 + v2:\n   %s\n + %s\n = %s\n",
			v1, v2, v1 + v2);
	// TODO: divide vector 1 by vector 3
	message("v1 / v3:\n   %s\n / %s\n = %s\n\n",
			v1, v3, "TODO");

	message("or perform mixed vector-scalar arithmetic\n");
	message("scalar * v2:\n   %s\n * %s\n = %s\n",
			2.0f, v2, 2.0f * v2);

    message("We can compute the length of a vector, or normalize it, or take "
            "the dot product or cross product of two vectors:\n");

	message("The length of v2 is: %s\n", length(v2));
	message("The squared length of v2 is: %s\n", length2(v2));
	Vec3f normalized2 = normalize(v2);
	message("A normalized copy of v2 is: %s\n", normalized2);
	message("Let's confirm that its length is 1: %s\n\n", length(normalized2));

	// TODO: look in vec.h to find an appropriate function to call to compute
	// the dot product and cross product between two vectors.
	message("The dot product of v1 and v3 is: %s\n", "TODO");
	message("The cross product of v1 and v2 is: %s\n", "TODO");

    // TODO: compute the angle between v1 and v3 (in degrees) using
    // either the dot or cross product. Use the rad2deg function from common.h.
	float degrees = 0.0f;
    message("The angle between v1 and v3 is: %s\n", degrees);
	if (std::abs(degrees-80.0787f) < 1e-4f)
		success("Result correct!\n\n");
	else
		error("Result incorrect!\n\n");

    // We will also make use of rays, which represent an origin and a direction:
	Ray3f ray{{ 0.5f,   2.0f, -3.0f},
		      {-0.25f, -0.5f,  0.3f}};

	// Let's print some info about our ray
	message("The origin of ray is    %s.\n", ray.o);
	message("The direction of ray is %s.\n", ray.d);

    // We also provide a 4x4 matrix class Mat44<T>, again templated by type T.
    // The Mat44 class includes a number of constructors. One way to fill a
    // matrix is to pass in its four *column* vectors.
	// Note that because we pass in columns, visually the matrix below looks
	// transposed: the elements {4, 5, 6, 1} appear in the 4th column, not row.
    Mat44f matrix{{1, 0, 0, 0},
				  {0, 2, 0, 0},
				  {0, 0, 3, 0},
				  {4, 5, 6, 1}};

	// We also provide the ability to compute matrix products and inverses.
	message("The matrix is\n%s.\n", matrix);
	message("The inverse is\n%s.\n", inverse(matrix));
	message("mat*inv should be the identity\n%s.\n", matrix * inverse(matrix));
}

void testColorAndImage()
{
	cout << endl;
	cout << "--------------------------------------------------------\n";
	cout << "PROGRAMMING ASSIGNMENT 0, PART 2: Color & image tutorial\n";
	cout << "--------------------------------------------------------\n";

    //
    // We will use the same Vec class to represent 3D locations, directions,
    // offsets, but also colors (since the space of colors humans can see are
    // well represented with 3 numbers). We introduce the convenience typedef
    // Color3f to represent RGB colors.
    //

    // A Color3f stores three floating-point values, one for red, green and
	// blue.
	Color3f red(1, 0, 0);
	Color3f blue(0, 0, 1);
	Color3f white = Color3f(1);     // This is the same as Color3f(1,1,1);

	// We can perform basic element-wise arithmatic on Colors:
	Color3f magenta = red + blue;
	Color3f stillRed = red * white;

	// TODO: Initialize the color pinkish to the average of white and red
    Color3f pinkish;

	message("white    = %s.\n", white);
    message("red      = %s.\n", red);
	message("blue     = %s.\n", blue);
	message("magenta  = %s.\n", magenta);
	message("pinkish  = %s.\n", pinkish);
	message("stillRed = %s.\n", stillRed);

	// We can also access the individual elements of the color by channel index:
	message("Red channel of pinkish is: %f\n", pinkish[0]);

	// or by channel name: pinkish.r, pinkish.g, pinkish.b

	// TODO: Print out the green channel of pinkish using the named accessor
	message("Green channel of pinkish is: %f\n", 0.0f);

	message("Blue channel of stillRed is: %f\n", stillRed[1]);

	pinkish.r *= 2.f;

	message("After scaling by 2, red channel of pinkish is: %f\n", pinkish.r);

    // The Color3f class provides a few additional operations which are useful
    // specifically for manipulating colors, see the bottom of the vec.h file.

    // TODO: Compute and print the luminance of pinkish. Look at vec.h to see
    // what method you might need
	message("The luminance of pinkish is: %f\n", 0.0f);

	// DIRT also provides the Image3f class (see image.h|cpp) to load, store,
	// manipulate, and write images.

	// Image3f is just a dynamically allocated 2D array of pixels. It
	// derives from the Array2D class, which is a generic 2D array
	// container of arbitrary size.

	// Here we construct an empty image that is 200 pixels across, and
	// 100 pixels tall:
	auto image1 = Image3f(200, 100);

	// In the case of Image3f, each array element (pixel) is a Color3f, which,
	// as we saw before, is itself a 3-element array.

	// We can access individual pixels of an Image3f using the (x,y) operator:
	image1(5,10) = white;       // This sets the pixel to white

    // The file common.h defines a simple linear interpolation function: lerp
    // which allows us to specify two values, a and b, and an interpolation
    // parameter t. This function is a template, which means it will work with
    // any type as long as (in this case) we can add them and multiply by a
    // scalar. Just as we could interpolate between two scalar values, we can
    // also use it to interpolate between two colors:

    message("25%% of the way from blue to red is: %s.\n",
            lerp(blue, red, 0.25f));

    // Now, let's populate the colors of an entire image, and write it to a PNG
	// file.

	Image3f gradient(200, 100);

	// TODO: Populate and output the gradient image
	// First, loop over all rows, and then columns of an image.
	// Set the red component of a pixel's color to vary linearly from 0 at the
	// leftmost pixel to 1 at the rightmost pixel; and the green component to
	// vary from 0 at the topmost pixel to 1 at the bottommost pixel. The blue
	// component should be 0 for all pixels.

    // After populating the pixel colors, look at the member functions of
    // Image3f, and call a function to save the gradient image out to the file
    // "gradient.png".

    message("Creating gradient image.\n");

    putYourCodeHere(
        "Populate an image with a color gradient and write to disk.");
    message("Saving image \"gradient.png\" ...\n");

    // Now, we will load an image, modify it, and save it back to disk.
    Image3f image;

    // TODO: Load the image scenes/00_preliminaries/cornellbox.png into the
    // ``image'' variable
    message("Loading image cornellbox.png ...\n");
    putYourCodeHere(
        "Load the image \"scenes/00_preliminaries/cornellbox.png\".");
    // Hint: Take a look at Image3f::load
    // Keep in mind filenames are interpreted relative to your current
    // working directory

    // TODO: Convert the image to grayscale. Loop over every pixel and convert
	// it to grayscale by replacing every pixel with its luminance
	message("Converting image to grayscale....\n");
	putYourCodeHere("Convert the image to grayscale.");

	// TODO: Save the image to scenes/00_preliminaries/cornell_grayscale.png
	// Hint: Take a look at Image3f::save
	message("Saving image cornell_grayscale.png....\n");
	putYourCodeHere("Save the image to \"scenes/00_preliminaries/cornell_grayscale.png\".");

	success("Done!\n");
}
