#pragma once

#include <dirt/vec.h>

/// Three vectors forming an orthonormal basis in 3D
template <typename T>
struct ONB
{
    using Vec3 = Vec<3,T>;

    ONB() = default;

    Vec3 operator[](int i) const { return axis[i]; }
    Vec3 toWorld(const Vec3& a) const { return a.x*u + a.y*v + a.z*w; }
    void build_from_w(const Vec3&);

    union
    {
        Vec3 axis[3];               ///< Array access to the 3 axes
        struct { Vec3 u, v, w; };   ///< Named access to the 3 individual axes
    };
};


template <typename T>
void ONB<T>::build_from_w(const Vec<3,T>& n)
{
    w = normalize(n);
    Vec3 a;
    if (std::abs(w.x) > 0.9)
        a = Vec3(0, 1, 0);
    else
        a = Vec3(1, 0, 0);
    v = normalize(cross(w, a));
    u = cross(w, v);
}


using ONBf = ONB<float>;
using ONBd = ONB<double>;
