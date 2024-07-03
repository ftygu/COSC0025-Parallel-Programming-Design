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

#include <dirt/sampling.h>
#include <dirt/common.h>

int main(int argc, char** argv)
{
    std::cout << "x,y,z" << std::endl;
    std::cout << "0,0,0" << std::endl;
    for (int i = 0; i < 500; ++i) {
        Vec3f p = randomInUnitSphere();
        std::cout << p.x << "," << p.y << "," << p.z << std::endl;
    }
    
    return 0;
}
