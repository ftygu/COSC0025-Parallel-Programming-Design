#pragma once

#include <dirt/vec.h>
#include <vector>

class Perlin
{
public:
    float noise(const Vec3f& p) const
    {
        float u = p.x - floor(p.x);
        float v = p.y - floor(p.y);
        float w = p.z - floor(p.z);
        int i = int(floor(p.x));
        int j = int(floor(p.y));
        int k = int(floor(p.z));
        Vec3f c[2][2][2];
        for (int di = 0; di < 2; di++)
            for (int dj = 0; dj < 2; dj++)
                for (int dk = 0; dk < 2; dk++)
                    c[di][dj][dk] =
                        ranvec[perm_x[(i + di) & 255] ^ perm_y[(j + dj) & 255] ^
                               perm_z[(k + dk) & 255]];
        return interp(c, u, v, w);
    }
    float turb(const Vec3f& p, int depth = 7) const
    {
        float accum = 0;
        Vec3f temp_p = p;
        float weight = 1.0;
        for (int i = 0; i < depth; i++)
        {
            accum += weight * noise(temp_p);
            weight *= 0.5f;
            temp_p *= 2.f;
        }
        return fabs(accum);
    }

private:
    static std::vector<Vec3f> ranvec;
    static std::vector<int> perm_x;
    static std::vector<int> perm_y;
    static std::vector<int> perm_z;

    static float interp(Vec3f c[2][2][2], float u, float v, float w)
    {
        float uu = u * u * (3 - 2 * u);
        float vv = v * v * (3 - 2 * v);
        float ww = w * w * (3 - 2 * w);
        float accum = 0;
        for (int i = 0; i < 2; i++)
            for (int j = 0; j < 2; j++)
                for (int k = 0; k < 2; k++)
                {
                    Vec3f weight_v(u - i, v - j, w - k);
                    accum += (i * uu + (1 - i) * (1 - uu)) *
                            (j * vv + (1 - j) * (1 - vv)) *
                            (k * ww + (1 - k) * (1 - ww)) *
                            dot(c[i][j][k], weight_v);
                }
        return accum;
    }
};
