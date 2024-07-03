#pragma once

#include <dirt/common.h>
#include <dirt/onb.h>

inline Vec3f random_to_sphere(const Vec2f &sample, float radius, float distance_squared)
{
    float s = sample.x;
    float t = sample.y;
    float z = 1 + t * (sqrt(1 - radius * radius / distance_squared) - 1);
    float phi = 2 * M_PI * s;
    float x = cos(phi) * sqrt(1 - z * z);
    float y = sin(phi) * sqrt(1 - z * z);
    return {x, y, z};
}
