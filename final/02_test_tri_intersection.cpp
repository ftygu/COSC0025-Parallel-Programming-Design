/*
This file is part of Dirt, the Dartmouth introductory ray tracer, used in
Dartmouth's COSC 77/177 Computer Graphics course.

Copyright (c) 2018 by Wojciech Jarosz

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

#include <dirt/mesh.h>


float maxAbsVecError(const Vec3f & a, const Vec3f & b)
{
	Vec3f tmp = a-b;
	tmp.x = abs(tmp.x);
	tmp.y = abs(tmp.y);
	tmp.z = abs(tmp.z);
	return max(tmp);
}

// runs the raytrace over all tests and saves the corresponding images
int main(int argc, char** argv)
{
    // PROGRAMMING ASSIGNMENT, PART 1: Ray - Triangle Intersection
    // ===========================================================

   message("Testing triangle intersection 1...\n");

   // Test Triangle data
   const Vec3f v0(-2.0f, -5.0f, -1.0f), v1(1.0f, 3.0f, 1.0f), v2(2.0f, -2.0f, 3.0);
   const Vec3f n0(0.0f, 0.707106f, 0.707106f),
               n1(0.666666f, 0.333333f, 0.666666f),
               n2(0.0f, -0.447213f, -0.894427f);

   // Test ray.
   const Ray3f testRay(Vec3f(1.0f, -1.0f, -5.0f), Vec3f(0.0f, 0.20f, 0.50f));
   HitInfo hit;

   if (singleTriangleIntersect(testRay, v0, v1, v2, &n0, &n1, &n2, nullptr, nullptr, nullptr, hit))
   {
       float correctT = 12.520326f;
       Vec3f correctP(1.0f, 1.504065f, 1.260162f);
       Vec3f correctgN(0.744073f, -0.114473f, -0.658218f);
       Vec3f correctsN(0.762482f, 0.317441f, 0.563784f);

       message("Hit triangle! Distance is %f (should be %f)\n\n", hit.t, correctT);
       message("Intersection point is \n%s\n(should be\n%s)\n\n", hit.p, correctP);
       message("Geometric normal is \n%s\n(should be \n%s)\n\n", hit.gn, correctgN);
       message("Shading normal is \n%s\n(shoudl be \n%s)\n\n", hit.sn, correctsN);

       float error = std::max(maxAbsVecError(correctgN, hit.gn),
                     std::max(maxAbsVecError(correctP, hit.p),
                              std::abs(correctT - hit.t)));

       if (error > 1e-5f)
           warning("Result incorrect!\n\n");
       else
           success("Result correct!\n\n");
   }
   else
   {
       warning("Triangle intersection incorrect! Should hit triangle\n");
   }
    
    return 0;
}