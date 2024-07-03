#include <dirt/sampling.h>
#include <dirt/common.h>
#include <dirt/perlin.h>

namespace
{

static std::vector<Vec3f> generate()
{
    std::vector<Vec3f> p(256);
    for (int i = 0; i < 256; ++i)
        p[i] = normalize(randomInUnitSphere());
    return p;
}

static std::vector<int> generatePermutation()
{
    std::vector<int> perm(256);
    for (int i = 0; i < 256; i++)
        perm[i] = i;
    for (int i = 256 - 1; i > 0; i--)
        std::swap(perm[i], perm[int(randf() * (i + 1))]);
    return perm;
}

} // namespace

std::vector<Vec3f> Perlin::ranvec = generate();
std::vector<int>   Perlin::perm_x = generatePermutation();
std::vector<int>   Perlin::perm_y = generatePermutation();
std::vector<int>   Perlin::perm_z = generatePermutation();
