#pragma once

#include "common.h"

inline Vec2f randomInUnitDisk()
{
	Vec2f p;
	do
	{
        float a = randf();
        float b = randf();
		p = 2.0f * Vec2f(a, b) - Vec2f(1);
	} while (length2(p) >= 1.f);

	return p;
}

inline Vec3f randomInUnitSphere()
{
	Vec3f p;
	do
	{
        float a = randf();
        float b = randf();
        float c = randf(); 
		p = 2.0f * Vec3f(a, b, c) - Vec3f(1);
	} while (length2(p) >= 1.0f);

	return p;
}

inline Vec3f randomOnUnitSphere(const Vec2f &sample)
{
    float phi = sample.x*(2*M_PI);
    float cosTheta = 2*sample.y - 1;
    float sinTheta = sqrt(1 - cosTheta*cosTheta);
    float x = cos(phi)*sinTheta;
    float y = sin(phi)*sinTheta;
    float z = cosTheta;
    return Vec3f(x, y, z);
}

inline Vec3f randomOnUnitHemisphere(const Vec2f &sample)
{
    float phi = sample.x*(2*M_PI);
    float cosTheta = sample.y;
    float sinTheta = sqrt(1 - cosTheta*cosTheta);
    float x = cos(phi)*sinTheta;
    float y = sin(phi)*sinTheta;
    float z = cosTheta;
    return Vec3f(x, y, z);
}

inline Vec3f randomCosineHemisphere(const Vec2f &sample)
{
    float phi = sample.x*(2*M_PI);
    float cosTheta = sqrt(sample.y);
    float sinTheta = sqrt(1 - cosTheta*cosTheta);
    float x = cos(phi)*sinTheta;
    float y = sin(phi)*sinTheta;
    float z = cosTheta;
    return Vec3f(x, y, z);
}

inline Vec3f randomCosinePowerHemisphere(float exponent, const Vec2f &sample)
{
    float phi = sample.x*(2*M_PI);
    float cosTheta = powf(sample.y, 1.0f / (exponent + 1.0f));
    float sinTheta = sqrt(1 - cosTheta*cosTheta);
    float x = cos(phi)*sinTheta;
    float y = sin(phi)*sinTheta;
    float z = cosTheta;
    return Vec3f(x, y, z);
}

inline Vec3f randomBeckmannNormal(float ab, const Vec2f &sample)
{
    float phi = sample.x*(2*M_PI);
    float tan2Theta = -ab*ab*log(1-sample.y);
    float cosTheta = 1/sqrt(1+tan2Theta);
    float sinTheta = sqrt(max(0.f, 1.f - cosTheta * cosTheta));
    float x = cos(phi)*sinTheta;
    float y = sin(phi)*sinTheta;
    float z = cosTheta;
    return Vec3f(x,y,z);
}
