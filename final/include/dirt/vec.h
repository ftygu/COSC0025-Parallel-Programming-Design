/** \file vec.h
    \brief Contains various classes for linear algebra: vectors, matrices, rays,
    axis-aligned bounding boxes.

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

#pragma once

#include <iostream>
#include <iomanip>
#include <cmath>
#include <stdlib.h>
#include <limits>
#include <cassert>
#include <array>
#include <type_traits>

/// An array of N values of type T for storing colors or mathematical vectors
template <size_t N, typename T>
struct Vec
{
    std::array<T, N> e;

    Vec() = default;

    explicit Vec(T e0)
    {
        for (size_t i = 0; i < N; ++i) e[i] = e0;
    }

    Vec(const std::initializer_list<T> & list)
    {
		int i = 0;
		for (auto &element : list)
		{
			e[i] = element;
			if (++i > N) break;
		}
    }

    T  operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i)       { return e[i]; }
};

template <size_t N, typename T>
inline T dot(const Vec<N,T>& v1, const Vec<N,T>& v2)
{
    T t(0); for (size_t i = 0; i < N; ++i) t += v1[i] * v2[i]; return t;
}

template<class C, size_t N, typename T>
std::basic_ostream<C> & operator<<(std::basic_ostream<C> & out, const Vec<N,T> & v)
{
    std::ios_base::fmtflags oldFlags = out.flags();
    auto width = out.precision() + 2;

    out.setf(std::ios_base::right);
    if (!(out.flags() & std::ios_base::scientific))
        out.setf(std::ios_base::fixed);
    width += 5;

    out << '{';
    for (size_t i = 0; i < N-1; ++i)
        out << std::setw(width) << v[i] << ',';
    out << std::setw(width) << v[N-1] << '}';

    out.flags(oldFlags);
    return out;
}


/// Return the geometric squared length (\f$l_2\f$-norm) of the vector
template <size_t N, typename T>
T length2(const Vec<N,T>& v) { return dot(v, v); }
/// Return the geometric length (\f$l_2\f$-norm) of the vector
template <size_t N, typename T>
T length(const Vec<N,T>& v)  { return std::sqrt(length2(v)); }
/// Return a unit-length copy of the vector
template <size_t N, typename T>
Vec<N,T> normalize(const Vec<N,T>& v) { return v / length(v); }


/// Return the index (dimension) of the largest component of the vector
template <size_t N, typename T> 
size_t maxDim(const Vec<N,T> & v)
{
    T m = v[0];
    size_t idx = 0;
    for (size_t i = 1; i < N; ++i)
    {
        if (v[i] > m)
        {
            m = v[i];
            idx = i;
        }
    }
    return idx;
}

/// Return the index (dimension) of the smallest component of the vector
template <size_t N, typename T> 
size_t minDim(const Vec<N,T> & v)
{
    T m = v[0];
    size_t idx = 0;
    for (size_t i = 1; i < N; ++i)
    {
        if (v[i] < m)
        {
            m = v[i];
            idx = i;
        }
    }
    return idx;
}


/// Compute the element-wise absolute value
template <size_t N, typename T> 
Vec<N,T> abs(const Vec<N,T> & v)
{
    Vec<N,T> v2;
    for (size_t i = 0; i < N; ++i)
        v2[i] = std::abs(v[i]);
    return v2;
}

/// Return the largest component of the vector
template <size_t N, typename T> 
T max(const Vec<N,T> & v)
{
    T m = v[0];
    for (size_t i = 1; i < N; ++i)
        if (v[i] > m)
            m = v[i];
    return m;
}

/// Return the smallest component of the vector
template <size_t N, typename T> 
T min(const Vec<N,T> & v)
{
    T m = v[0];
    for (size_t i = 1; i < N; ++i)
        if (v[i] < m)
            m = v[i];
    return m;
}

/// Return the componentwise maximum of the two vectors
template <size_t N, typename T> 
Vec<N,T> max(const Vec<N,T> & a, const Vec<N,T> & b)
{
    Vec<N,T> m;
    for (size_t i = 0; i < N; ++i)
        m[i] = (a[i] > b[i]) ? a[i] : b[i];
    return m;
}

/// Return the componentwise minimum of the two vectors
template <size_t N, typename T> 
Vec<N,T> min(const Vec<N,T> & a, const Vec<N,T> & b)
{
    Vec<N,T> m;
    for (size_t i = 0; i < N; ++i)
        m[i] = (a[i] < b[i]) ? a[i] : b[i];
    return m;
}


/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator+=(Vec<N,T>& v1, const Vec<N,S>& v2)
{
    for (size_t i = 0; i < N; ++i) v1[i] += v2[i]; return v1;
}
/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator-=(Vec<N,T>& v1, const Vec<N,S>& v2)
{
    for (size_t i = 0; i < N; ++i) v1[i] -= v2[i]; return v1;
}
/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator*=(Vec<N,T>& v1, const Vec<N,S>& v2)
{
    for (size_t i = 0; i < N; ++i) v1[i] *= v2[i]; return v1;
}
/// component-wise vector-vector assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator/=(Vec<N,T>& v1, const Vec<N,S>& v2)
{
    for (size_t i = 0; i < N; ++i) v1[i] /= v2[i]; return v1;
}

/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator+=(Vec<N,T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i) v1[i] += s; return v1;
}
/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator-=(Vec<N,T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i) v1[i] -= s; return v1;
}
/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator*=(Vec<N,T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i) v1[i] *= s; return v1;
}
/// component-wise vector-scalar assignment-arithmetic
template <size_t N, typename T, typename S>
Vec<N,T>& operator/=(Vec<N,T>& v1, S s)
{
    for (size_t i = 0; i < N; ++i) v1[i] /= s; return v1;
}

// positive and negative
template <size_t N, typename T>
inline const Vec<N,T> & operator+(const Vec<N,T>& v)  { return v; }

template <size_t N, typename T> Vec<N,T> operator-(const Vec<N,T>& v)
{
    Vec<N,T> v2; for (size_t i = 0; i < N; ++i) v2[i] = -v[i]; return v2;
}

/// component-wise vector-vector addition
template <size_t N, typename T>
Vec<N,T> operator+(const Vec<N,T>& v1, const Vec<N,T>& v2)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] + v2[i]; return v3;
}
/// component-wise vector-vector subtraction
template <size_t N, typename T>
Vec<N,T> operator-(const Vec<N,T>& v1, const Vec<N,T>& v2)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] - v2[i]; return v3;
}
/// component-wise vector-vector multiplication
template <size_t N, typename T>
Vec<N,T> operator*(const Vec<N,T>& v1, const Vec<N,T>& v2)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] * v2[i]; return v3;
}
/// component-wise vector-vector division
template <size_t N, typename T>
Vec<N,T> operator/(const Vec<N,T>& v1, const Vec<N,T>& v2)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] / v2[i]; return v3;
}

/// vector-scalar addition
template <size_t N, typename T, typename S>
Vec<N,T> operator+(const Vec<N,T>& v1, S s)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] + s; return v3;
}
/// vector-scalar subtraction
template <size_t N, typename T, typename S>
Vec<N,T> operator-(const Vec<N,T>& v1, S s)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] - s; return v3;
}
/// vector-scalar multiplication
template <size_t N, typename T, typename S>
Vec<N,T> operator*(const Vec<N,T>& v1, S s)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] * s; return v3;
}
/// vector-scalar division
template <size_t N, typename T, typename S>
Vec<N,T> operator/(const Vec<N,T>& v1, S s)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = v1[i] / s; return v3;
}

/// scalar-vector addition
template <size_t N, typename T, typename S>
Vec<N,T> operator+(S s, const Vec<N,T>& v1)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = s + v1[i]; return v3;
}
/// scalar-vector subtraction
template <size_t N, typename T, typename S>
Vec<N,T> operator-(S s, const Vec<N,T>& v1)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = s - v1[i]; return v3;
}
/// scalar-vector multiplication
template <size_t N, typename T, typename S>
Vec<N,T> operator*(S s, const Vec<N,T>& v1)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = s * v1[i]; return v3;
}
/// scalar-vector division
template <size_t N, typename T, typename S>
Vec<N,T> operator/(S s, const Vec<N,T>& v1)
{
    Vec<N,T> v3; for (size_t i = 0; i < N; ++i) v3[i] = s / v1[i]; return v3;
}


/// A mathematical 2-vector
template <typename T>
struct Vec<2,T>
{
    union
    {
        std::array<T, 2> e;
        struct { T x, y; };
        struct { T u, v; };
    };

    constexpr           Vec() = default;
    constexpr explicit  Vec(T e0) : x(e0), y(e0) { }
    constexpr           Vec(T e0, T e1) : x(e0), y(e1) { }

    T  operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i)       { return e[i]; }

    static inline Vec Zero() { return Vec(T(0)); }
    static inline Vec UnitX() { return Vec(T(1), T(0)); }
    static inline Vec UnitY() { return Vec(T(0), T(1)); }
};

/// A mathematical 3-vector
template <typename T>
struct Vec<3,T>
{
    union
    {
        std::array<T, 3> e;
        struct { T x, y, z; };
        struct { T r, g, b; };
        Vec<2,T> xy;
    };

    constexpr           Vec() = default;
    constexpr explicit  Vec(T e0) : x(e0), y(e0), z(e0) { }
    constexpr           Vec(T e0, T e1, T e2) : x(e0), y(e1), z(e2) { }
    constexpr           Vec(const Vec<2,T> xy, T _z) : x(xy.x), y(xy.y), z(_z) { }

    T  operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i)       { return e[i]; }

    static inline Vec Zero() { return Vec(T(0)); }
    static inline Vec UnitX() { return Vec(T(1), T(0), T(0)); }
    static inline Vec UnitY() { return Vec(T(0), T(1), T(0)); }
    static inline Vec UnitZ() { return Vec(T(0), T(0), T(1)); }
};

template <typename T>
inline Vec<3,T> cross(const Vec<3,T>& v1, const Vec<3,T>& v2)
{
    return Vec<3,T>( (v1.y * v2.z - v1.z * v2.y),
                    -(v1.x * v2.z - v1.z * v2.x),
                     (v1.x * v2.y - v1.y * v2.x));
}

/// A 4-vector
template <typename T>
struct Vec<4,T>
{
    union
    {
        std::array<T, 4> e;
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };
        Vec<3,T> xyz;
        Vec<3,T> rgb;
        Vec<2,T> xy;
    };

    constexpr           Vec() = default;
    constexpr explicit  Vec(T e0) : x(e0), y(e0), z(e0), w(e0) { }
    constexpr           Vec(T e0, T e1, T e2, T e3) : x(e0), y(e1), z(e2), w(e3) { }
    constexpr           Vec(const Vec<3,T> xyz, T _w) : x(xyz.x), y(xyz.y), z(xyz.z), w(_w) { }

    T  operator[](size_t i) const { return e[i]; }
    T& operator[](size_t i)       { return e[i]; }

    static inline Vec Zero() { return Vec(T(0)); }
    static inline Vec UnitX() { return Vec(T(1), T(0), T(0), T(0)); }
    static inline Vec UnitY() { return Vec(T(0), T(1), T(0), T(0)); }
    static inline Vec UnitZ() { return Vec(T(0), T(0), T(1), T(0)); }
    static inline Vec UnitW() { return Vec(T(0), T(0), T(0), T(1)); }
};


/// A \f$4 \times 4\f$ matrix for expressing affine and perspective transformations
/**
    Matrix vector multiplication is done as follows:
    
    \f$
        \begin{pmatrix}
        v'_x\\
        v'_y\\
        v'_z\\
        v'_w\\
        \end{pmatrix} =
        \begin{pmatrix}
        a_x & b_x & c_x & d_x\\
        a_y & b_y & c_y & d_y\\
        a_z & b_z & c_z & d_z\\
        a_w & b_w & c_w & d_w\\
        \end{pmatrix}
        \begin{pmatrix}
        v_x\\
        v_y\\
        v_z\\
        v_w\\
        \end{pmatrix}
    \f$
    
    An affine transformation will look like:

    \f$
    \begin{pmatrix}
        a_x & b_x & c_x & d_x\\
        a_y & b_y & c_y & d_y\\
        a_z & b_z & c_z & d_z\\
          0 &   0 &   0 &   1\\
    \end{pmatrix}
    \f$
    
    where the \f$a, b, c\f$ vectors form the linear portion, and the \f$d\f$
    vector is the translation.
    
    The matrix elements are stored in memory in column-major order:
    \code
          { { Ax, Ay, Az, Aw },
            { Bx, By, Bz, Bw },
            { Cx, Cy, Cz, Cw },
            { Dx, Dy, Dz, Dw } }
    \endcode
*/
template <typename T>
struct Mat44
{
    using Vec3 = Vec<3,T>;
    using Vec4 = Vec<4,T>;

    /// Direct access to matrix elements
    union
    {
        /// Access to individual matrix elements,
        /// where the first digit is the row, and
        /// second is the column
        struct
        {
            T m00, m10, m20, m30;
            T m01, m11, m21, m31;
            T m02, m12, m22, m32;
            T m03, m13, m23, m33;
        };
        /// Array access to matrix columns
        std::array<Vec4,4> m;
        /// Access to columns as named 4-vectors x, y, z, w
        struct { Vec4 x, y, z, w; };
        /// Access to columns as named 4-vectors a, b, c, d
        struct { Vec4 a, b, c, d; };
    };
    
    ///\{ \name Constructors and assignment.
    explicit Mat44(T s);
    Mat44() : Mat44(1) { }
    Mat44(const Vec4 &A, const Vec4 &B, const Vec4 &C, const Vec4 &D);
    Mat44(const Vec3 &A, const Vec3 &B, const Vec3 &C, const Vec3 &D);
    Mat44(const T a[4][4]);
    ///\}

    ///\{ \name Element access.
    const Vec<4,T> & operator[](int col) const  {return m[col];}
          Vec<4,T> & operator[](int col)        {return m[col];}
    T & operator()(int row, int col)            {return m[col][row];}
    T   operator()(int row, int col) const      {return m[col][row];}
    ///\}
};


template <typename T>
inline Mat44<T>::Mat44(T s)
{
    m00 = s;    m01 = 0;    m02 = 0;    m03 = 0;
    m10 = 0;    m11 = s;    m12 = 0;    m13 = 0;
    m20 = 0;    m21 = 0;    m22 = s;    m23 = 0;
    m30 = 0;    m31 = 0;    m32 = 0;    m33 = s;
}


template <typename T>
inline Mat44<T>::Mat44(const T mat[4][4])
{
    memcpy(m, mat, 16 * sizeof(T));
}


template <typename T>
inline Mat44<T>::Mat44(const Vec<4,T> &a,
                       const Vec<4,T> &b,
                       const Vec<4,T> &c,
                       const Vec<4,T> &d) :
    x(a), y(b), z(c), w(d)
{

}


template <typename T>
inline Mat44<T>::Mat44(const Vec<3,T> &a,
                       const Vec<3,T> &b,
                       const Vec<3,T> &c,
                       const Vec<3,T> &d) :
    Mat44({a, 0}, {b, 0}, {c, 0}, {d, 1})
{
    
}


template <typename T> 
inline Mat44<T> multiply(const Mat44<T> &a, const Mat44<T> &b)
{
    return {{a.x*b.x.x + a.y*b.x.y + a.z*b.x.z + a.w*b.x.w},
            {a.x*b.y.x + a.y*b.y.y + a.z*b.y.z + a.w*b.y.w},
            {a.x*b.z.x + a.y*b.z.y + a.z*b.z.z + a.w*b.z.w},
            {a.x*b.w.x + a.y*b.w.y + a.z*b.w.z + a.w*b.w.w}};
}

template <typename T> 
inline const Mat44<T> & operator*=(Mat44<T>& lhs, const Mat44<T>& rhs)
{
    return (lhs = multiply(lhs, rhs));
}

template <typename T> 
inline const Mat44<T> & operator/=(Mat44<T>& m, T a)
{
    return (m *= T(1)/a);
}

template <typename T> 
inline Mat44<T> operator*(const Mat44<T>& lhs, const Mat44<T>& rhs)
{
    return multiply(lhs, rhs);
}

template <typename T> 
inline Mat44<T> operator*(T a, const Mat44<T> & m)
{
    return m * a;
}

template <typename T> 
inline Vec<4, T> operator*(const Mat44<T>& m, const Vec<4, T> &a)
{
    return Vec<4, T>(
        m(0, 0)*a[0] + m(0, 1)*a[1] + m(0, 2)*a[2] + m(0, 3)*a[3],
        m(1, 0)*a[0] + m(1, 1)*a[1] + m(1, 2)*a[2] + m(1, 3)*a[3],
        m(2, 0)*a[0] + m(2, 1)*a[1] + m(2, 2)*a[2] + m(2, 3)*a[3],
        m(3, 0)*a[0] + m(3, 1)*a[1] + m(3, 2)*a[2] + m(3, 3)*a[3]
    );
}

template <typename T> 
inline Mat44<T> operator*(const Mat44<T>& m, T a)
{
    return Mat44<T>(m[0] * a, m[1] * a, m[2] * a, m[3] * a);
}


template <typename T> 
inline const Mat44<T> & operator*=(Mat44<T> & m, T a)
{
    m[0] *= a;
    m[1] *= a;
    m[2] *= a;
    m[3] *= a;
    return m;
}


template <typename T> 
inline Mat44<T> operator/(const Mat44<T>& m, T a)
{
    return m * (T(1)/a);
}


//----------------------------------
// Transpose and inverse
//----------------------------------

template <typename T> 
inline Mat44<T> transpose(const Mat44<T> & m)
{
    return {{m.m00, m.m01, m.m02, m.m03},
            {m.m10, m.m11, m.m12, m.m13},
            {m.m20, m.m21, m.m22, m.m23},
            {m.m30, m.m31, m.m32, m.m33}};
}


template <typename T> 
Mat44<T> adjugate(const Mat44<T> & m)
{
    return {{m.m11*m.m22*m.m33 + m.m13*m.m21*m.m32 + m.m12*m.m23*m.m31 - m.m11*m.m23*m.m32 - m.m12*m.m21*m.m33 - m.m13*m.m22*m.m31,
             m.m10*m.m23*m.m32 + m.m12*m.m20*m.m33 + m.m13*m.m22*m.m30 - m.m13*m.m20*m.m32 - m.m12*m.m23*m.m30 - m.m10*m.m22*m.m33,
             m.m10*m.m21*m.m33 + m.m13*m.m20*m.m31 + m.m11*m.m23*m.m30 - m.m10*m.m23*m.m31 - m.m11*m.m20*m.m33 - m.m13*m.m21*m.m30,
             m.m10*m.m22*m.m31 + m.m11*m.m20*m.m32 + m.m12*m.m21*m.m30 - m.m10*m.m21*m.m32 - m.m12*m.m20*m.m31 - m.m11*m.m22*m.m30},
            {m.m21*m.m33*m.m02 + m.m22*m.m31*m.m03 + m.m23*m.m32*m.m01 - m.m21*m.m32*m.m03 - m.m23*m.m31*m.m02 - m.m22*m.m33*m.m01,
             m.m20*m.m32*m.m03 + m.m23*m.m30*m.m02 + m.m22*m.m33*m.m00 - m.m20*m.m33*m.m02 - m.m22*m.m30*m.m03 - m.m23*m.m32*m.m00,
             m.m20*m.m33*m.m01 + m.m21*m.m30*m.m03 + m.m23*m.m31*m.m00 - m.m20*m.m31*m.m03 - m.m23*m.m30*m.m01 - m.m21*m.m33*m.m00,
             m.m20*m.m31*m.m02 + m.m22*m.m30*m.m01 + m.m21*m.m32*m.m00 - m.m20*m.m32*m.m01 - m.m21*m.m30*m.m02 - m.m22*m.m31*m.m00},
            {m.m31*m.m02*m.m13 + m.m33*m.m01*m.m12 + m.m32*m.m03*m.m11 - m.m31*m.m03*m.m12 - m.m32*m.m01*m.m13 - m.m33*m.m02*m.m11,
             m.m30*m.m03*m.m12 + m.m32*m.m00*m.m13 + m.m33*m.m02*m.m10 - m.m30*m.m02*m.m13 - m.m33*m.m00*m.m12 - m.m32*m.m03*m.m10,
             m.m30*m.m01*m.m13 + m.m33*m.m00*m.m11 + m.m31*m.m03*m.m10 - m.m30*m.m03*m.m11 - m.m31*m.m00*m.m13 - m.m33*m.m01*m.m10,
             m.m30*m.m02*m.m11 + m.m31*m.m00*m.m12 + m.m32*m.m01*m.m10 - m.m30*m.m01*m.m12 - m.m32*m.m00*m.m11 - m.m31*m.m02*m.m10},
            {m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 + m.m03*m.m12*m.m21 - m.m01*m.m12*m.m23 - m.m03*m.m11*m.m22 - m.m02*m.m13*m.m21,
             m.m00*m.m12*m.m23 + m.m03*m.m10*m.m22 + m.m02*m.m13*m.m20 - m.m00*m.m13*m.m22 - m.m02*m.m10*m.m23 - m.m03*m.m12*m.m20,
             m.m00*m.m13*m.m21 + m.m01*m.m10*m.m23 + m.m03*m.m11*m.m20 - m.m00*m.m11*m.m23 - m.m03*m.m10*m.m21 - m.m01*m.m13*m.m20,
             m.m00*m.m11*m.m22 + m.m02*m.m10*m.m21 + m.m01*m.m12*m.m20 - m.m00*m.m12*m.m21 - m.m01*m.m10*m.m22 - m.m02*m.m11*m.m20}}; 
}

template <typename T> 
T determinant(const Mat44<T> & m)
{ 
    return m.m00*(m.m11*m.m22*m.m33 + m.m13*m.m21*m.m32 + m.m12*m.m23*m.m31 - m.m11*m.m23*m.m32 - m.m12*m.m21*m.m33 - m.m13*m.m22*m.m31)
         + m.m10*(m.m21*m.m33*m.m02 + m.m22*m.m31*m.m03 + m.m23*m.m32*m.m01 - m.m21*m.m32*m.m03 - m.m23*m.m31*m.m02 - m.m22*m.m33*m.m01)
         + m.m20*(m.m31*m.m02*m.m13 + m.m33*m.m01*m.m12 + m.m32*m.m03*m.m11 - m.m31*m.m03*m.m12 - m.m32*m.m01*m.m13 - m.m33*m.m02*m.m11)
         + m.m30*(m.m01*m.m13*m.m22 + m.m02*m.m11*m.m23 + m.m03*m.m12*m.m21 - m.m01*m.m12*m.m23 - m.m03*m.m11*m.m22 - m.m02*m.m13*m.m21); 
}

template <typename T> 
inline Mat44<T> inverse(const Mat44<T> & m)
{
    return adjugate(m)/determinant(m);
}


template <typename T, typename S>
Mat44<T> axisAngle(const Vec<3,S> & v, S a)
{
    S ct  = std::cos(a);
    S st  = std::sin(a);
    S omc = S(1) - ct;

    return {{omc*v.x*v.x + ct    , omc*v.x*v.y + st*v.z, omc*v.x*v.z - st*v.y, 0},
            {omc*v.x*v.y - st*v.z, omc*v.y*v.y + ct    , omc*v.y*v.z + st*v.x, 0},
            {omc*v.x*v.z + st*v.y, omc*v.y*v.z - st*v.x, omc*v.z*v.z + ct    , 0},
            {0                   , 0                   , 0                   , 1}};
}


template<class C, typename T>
std::basic_ostream<C> & operator<<(std::basic_ostream<C> & s, const Mat44<T> & m)
{
    return s << "{" << m[0] << ",\n "
                    << m[1] << ",\n "
                    << m[2] << ",\n "
                    << m[3] << "}";
}

template <typename T> using Vec2 = Vec<2, T>;
template <typename T> using Vec3 = Vec<3, T>;
template <typename T> using Vec4 = Vec<4, T>;

template <typename T> using Color3 = Vec<3, T>;
template <typename T> using Color4 = Vec<4, T>;

using Vec2f   = Vec2<float>;
using Vec2d   = Vec2<double>;
using Vec2i   = Vec2<std::int32_t>;
using Vec2u   = Vec2<std::uint32_t>;
using Vec2c   = Vec2<std::uint8_t>;

using Vec3f   = Vec3<float>;
using Vec3d   = Vec3<double>;
using Vec3i   = Vec3<std::int32_t>;
using Vec3u   = Vec3<std::uint32_t>;
using Vec3c   = Vec3<std::uint8_t>;
using Color3f = Vec3<float>;
using Color3d = Vec3<double>;
using Color3u = Vec3<std::uint32_t>;
using Color3c = Vec3<std::uint8_t>;

using Vec4f   = Vec4<float>;
using Vec4d   = Vec4<double>;
using Vec4i   = Vec4<std::int32_t>;
using Vec4u   = Vec4<std::uint32_t>;
using Vec4c   = Vec4<std::uint8_t>;
using Color4f = Vec4<float>;
using Color4d = Vec4<double>;
using Color4u = Vec4<std::uint32_t>;
using Color4c = Vec4<std::uint8_t>;

using Mat44f = Mat44<float>;
using Mat44d = Mat44<double>;

static_assert(std::is_pod<Vec4d>::value, "Vec4d is not a pod!");
static_assert(std::is_pod<Vec3d>::value, "Vec3d is not a pod!");
static_assert(std::is_pod<Vec2d>::value, "Vec2d is not a pod!");

static_assert(std::is_pod<Vec4f>::value, "Vec4f is not a pod!");
static_assert(std::is_pod<Vec3f>::value, "Vec3f is not a pod!");
static_assert(std::is_pod<Vec2f>::value, "Vec2f is not a pod!");

static_assert(std::is_pod<Vec4u>::value, "Vec4u is not a pod!");
static_assert(std::is_pod<Vec3u>::value, "Vec3u is not a pod!");
static_assert(std::is_pod<Vec2u>::value, "Vec2u is not a pod!");

static_assert(std::is_pod<Vec4i>::value, "Vec4i is not a pod!");
static_assert(std::is_pod<Vec3i>::value, "Vec3i is not a pod!");
static_assert(std::is_pod<Vec2i>::value, "Vec2i is not a pod!");

static_assert(std::is_pod<Vec4c>::value, "Vec4c is not a pod!");
static_assert(std::is_pod<Vec3c>::value, "Vec3c is not a pod!");
static_assert(std::is_pod<Vec2c>::value, "Vec2c is not a pod!");

/// Clamp to the positive range
inline Color3f clampColor(const Color3f & c)
{
    return Color3f(std::max(c.r, 0.0f),
                   std::max(c.g, 0.0f),
                   std::max(c.b, 0.0f));
}

/// Check if the color vector contains a NaN/Inf/negative value
inline Color3f toSRGB(const Color3f & c)
{
    Color3f result;

    for (int i=0; i<3; ++i)
    {
        float value = c[i];
        if (value <= 0.0031308f)
            result[i] = 12.92f * value;
        else
            result[i] = (1.0f + 0.055f) * std::pow(value, 1.0f/2.4f) -  0.055f;
    }

    return result;
}

/// Convert from sRGB to linear RGB
inline Color3f toLinearRGB(const Color3f & c)
{
    Color3f result;

    for (int i=0; i<3; ++i)
    {
        float value = c[i];

        if (value <= 0.04045f)
            result[i] = value * (1.0f / 12.92f);
        else
            result[i] = std::pow((value + 0.055f) * (1.0f / 1.055f), 2.4f);
    }

    return result;
}

/// Convert from linear RGB to sRGB
inline bool isValidColor(const Color3f & c)
{
    for (int i=0; i<3; ++i)
    {
        float value = c[i];
        if (value < 0 || !std::isfinite(value))
            return false;
    }
    return true;
}

/// Return the associated luminance
inline float luminance(const Color3f & c)
{
    return c[0] * 0.212671f + c[1] * 0.715160f + c[2] * 0.072169f;
}
