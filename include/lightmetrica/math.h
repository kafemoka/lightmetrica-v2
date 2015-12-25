/*
    Lightmetrica - A modern, research-oriented renderer

    Copyright (c) 2015 Hisanari Otsu

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#pragma once

#include <lightmetrica/macros.h>
#include <string>
#include <initializer_list>

// TODO. Make configurable from cmake file 
#define LM_USE_SINGLE_PRECISION
#define LM_USE_SSE
#define LM_USE_AVX

// --------------------------------------------------------------------------------

#pragma region Precision mode

#ifdef LM_USE_SINGLE_PRECISION
	#define LM_SINGLE_PRECISION 1
#else
	#define LM_SINGLE_PRECISION 0
#endif
#ifdef LM_USE_DOUBLE_PRECISION
	#define LM_DOUBLE_PRECISION 1
#else
	#define LM_DOUBLE_PRECISION 0
#endif
#if LM_SINGLE_PRECISION + LM_DOUBLE_PRECISION != 1
	#error "Invalid precision mode"
#endif

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region SIMD support

#ifdef LM_USE_NO_SIMD
    #define LM_NO_SIMD 1
#else
    #define LM_NO_SIMD 0
#endif
#ifdef LM_USE_SSE
	#define LM_SSE 1
#else
	#define LM_SSE 0
#endif
#ifdef LM_USE_AVX
	#define LM_AVX 1
#else
	#define LM_AVX 0
#endif

#if LM_AVX
#include <immintrin.h>  // Assume AVX2
#elif LM_SSE
#include <nmmintrin.h>  // Assume SSE4.2
#endif

#pragma endregion

// --------------------------------------------------------------------------------

LM_NAMESPACE_BEGIN

// --------------------------------------------------------------------------------

#pragma region Default floating point type

// Default type
#if LM_SINGLE_PRECISION
using Float = float;
#elif LM_DOUBLE_PRECISION
using Float = double;
#endif

// Convert to default floating point type
namespace
{
    auto operator"" _f(long double v) -> Float { return Float(v); }
    auto operator"" _f(unsigned long long v) -> Float { return Float(v); }
    auto operator"" _sf(const char* v) -> Float
    {
        #if LM_SINGLE_PRECISION
        return std::stof(v);
        #elif LM_DOUBLE_PRECISION
        return std::stod(v);
        #endif
    }
}

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region SIMD flag

/*!
    SIMD flags.

    Specified SIMD optimization, which is
    utilized as a template parameter.
*/
enum class SIMD
{
    None,
    SSE,    // Requires support of SSE, SSE2, SSE3, SSE4.x
    AVX,    // Requires support of AVX, AVX2

    // Default SIMD type
    #if LM_SINGLE_PRECISION && LM_SSE
    Default = SSE,
    #elif LM_DOUBLE_PRECISION && LM_AVX
    Default = AVX,
    #endif
};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Math object types

#pragma region Math object type flag

enum class MathObjectType
{
    Vec,
    Mat,
};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Base vector type

/*!
    Base of all vector types.

    Defines required types as vector types.
*/
template <typename T, SIMD Opt, template <typename, SIMD> class VecT_, int NC_>
struct TVecBase
{

    // Math object type
    static constexpr MathObjectType ObjT = MathObjectType::Vec;

    // Value type
    using VT = T;

    // Vector type
    using VecT = VecT_<T, Opt>;

    // Number of components
    static constexpr int NC = NC_;

    // Parameter types
    template <typename U>
    using TParam = std::conditional_t<std::is_arithmetic<U>::value, U, const U&>;
    using ParamT = TParam<T>;
    using RetT   = ParamT;

};

template <typename T, SIMD Opt, template <typename, SIMD> class VecT_, int NC_>
struct SIMDTVecBase : public TVecBase<T, Opt, VecT_, NC_>
{
    // SIMD vector type
    using SIMDT = std::conditional_t<Opt == SIMD::AVX, __m256d, __m128>;
};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Vec3

/*!
    3D vector.
    
    Generic 3-dimensional vector.
*/
template <typename T, SIMD Opt = SIMD::None>
struct TVec3;

template <typename T>
struct TVec3<T, SIMD::None> : public TVecBase<T, SIMD::None, TVec3, 3>
{
    union
    {
        VT v_[NC];
        struct { VT x, y, z; };
    };

    LM_INLINE TVec3()                                   : x(VT(0)), y(VT(0)), z(VT(0)) {}
    LM_INLINE TVec3(ParamT x, ParamT y, ParamT z)       : x(x), y(y), z(z) {}
    LM_INLINE TVec3(const VecT& v)                      : x(v.x), y(v.y), z(v.z) {}
    LM_INLINE TVec3(std::initializer_list<VT> l)        { x = l.begin()[0]; y = l.begin()[1]; z = l.begin()[2]; }

    LM_INLINE auto operator[](int i)         -> VT&     { return (&x)[i]; }
    LM_INLINE auto operator[](int i) const   -> RetT    { return (&x)[i]; }
    LM_INLINE auto operator=(const VecT& v)  -> VecT&   { x = v.x; y = v.y; z = v.z; return *this; }
    LM_INLINE auto operator+=(const VecT& v) -> VecT&   { x += v.x; y += v.y; z += v.z; return *this; }
    LM_INLINE auto operator-=(const VecT& v) -> VecT&   { x -= v.x; y -= v.y; z -= v.z; return *this; }
    LM_INLINE auto operator*=(const VecT& v) -> VecT&   { x *= v.x; y *= v.y; z *= v.z; return *this; }
    LM_INLINE auto operator/=(const VecT& v) -> VecT&   { x /= v.x; y /= v.y; z /= v.z; return *this; }

};

template <>
struct LM_ALIGN_16 TVec3<float, SIMD::SSE> : public SIMDTVecBase<float, SIMD::SSE, TVec3, 3>
{
    union
    {
        SIMDT v_;
        struct { VT x, y, z, _; };
    };

    LM_INLINE TVec3()                                   : v_(_mm_set_ps(1.0f, 0.0f, 0.0f, 0.0f)) {}
    LM_INLINE TVec3(ParamT x, ParamT y, ParamT z)       : v_(_mm_set_ps(1.0f, z, y, x)) {}
    LM_INLINE TVec3(const VecT& v)                      : v_(v.v_) {}
    LM_INLINE TVec3(SIMDT v)                            : v_(v) {}
    LM_INLINE TVec3(std::initializer_list<VT> l)        { x = l.begin()[0]; y = l.begin()[1]; z = l.begin()[2]; }

    LM_INLINE auto operator[](int i)         -> VT&     { return (&x)[i]; }
    LM_INLINE auto operator[](int i) const   -> RetT    { return (&x)[i]; }
    LM_INLINE auto operator=(const VecT& v)  -> VecT&   { v_ = v.v_; return *this; }
    LM_INLINE auto operator+=(const VecT& v) -> VecT&   { v_ = _mm_add_ps(v_, v.v_); return *this; }
    LM_INLINE auto operator-=(const VecT& v) -> VecT&   { v_ = _mm_sub_ps(v_, v.v_); return *this; }
    LM_INLINE auto operator*=(const VecT& v) -> VecT&   { v_ = _mm_mul_ps(v_, v.v_); return *this; }
    LM_INLINE auto operator/=(const VecT& v) -> VecT&   { v_ = _mm_div_ps(v_, v.v_); return *this; }

};

template <>
struct LM_ALIGN_32 TVec3<double, SIMD::AVX> : public SIMDTVecBase<double, SIMD::AVX, TVec3, 3>
{

    union
    {
        SIMDT v_;
        struct { VT x, y, z, _; };
    };

    LM_INLINE TVec3()                                   : v_(_mm256_set_pd(1, 0, 0, 0)) {}
    LM_INLINE TVec3(ParamT x, ParamT y, ParamT z)       : v_(_mm256_set_pd(1, z, y, x)) {}
    LM_INLINE TVec3(const VecT& v)                      : v_(v.v_) {}
    LM_INLINE TVec3(SIMDT v)                            : v_(v) {}
    LM_INLINE TVec3(std::initializer_list<VT> l)        { x = l.begin()[0]; y = l.begin()[1]; z = l.begin()[2]; }

    LM_INLINE auto operator[](int i)         -> VT&     { return (&x)[i]; }
    LM_INLINE auto operator[](int i) const   -> RetT    { return (&x)[i]; }
    LM_INLINE auto operator=(const VecT& v)  -> VecT&   { v_ = v.v_; return *this; }
    LM_INLINE auto operator+=(const VecT& v) -> VecT&   { v_ = _mm256_add_pd(v_, v.v_); return *this; }
    LM_INLINE auto operator-=(const VecT& v) -> VecT&   { v_ = _mm256_sub_pd(v_, v.v_); return *this; }
    LM_INLINE auto operator*=(const VecT& v) -> VecT&   { v_ = _mm256_mul_pd(v_, v.v_); return *this; }
    LM_INLINE auto operator/=(const VecT& v) -> VecT&   { v_ = _mm256_div_pd(v_, v.v_); return *this; }

};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Vec4

/*!
    4D vector.
    
    Generic 4-dimensional vector.
*/
template <typename T, SIMD Opt = SIMD::None>
struct TVec4;

template <typename T>
struct TVec4<T, SIMD::None> : public TVecBase<T, SIMD::None, TVec4, 4>
{

    union
    {
        VT v_[NC];
        struct { VT x, y, z, w; };
    };

    LM_INLINE TVec4()                                       : x(VT(0)), y(VT(0)), z(VT(0)), w(VT(0)) {}
    LM_INLINE TVec4(ParamT x, ParamT y, ParamT z, ParamT w) : x(x), y(y), z(z), w(w) {}
    LM_INLINE TVec4(const VecT& v)                          : x(v.x), y(v.y), z(v.z), w(v.w) {}
    LM_INLINE TVec4(std::initializer_list<VT> l)            { x = l.begin()[0]; y = l.begin()[1]; z = l.begin()[2]; w = l.begin()[3]; }

    LM_INLINE auto operator[](int i)         -> VT&         { return (&x)[i]; }
    LM_INLINE auto operator[](int i) const   -> RetT        { return (&x)[i]; }
    LM_INLINE auto operator=(const VecT& v)  -> VecT&       { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
    LM_INLINE auto operator+=(const VecT& v) -> VecT&       { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    LM_INLINE auto operator-=(const VecT& v) -> VecT&       { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    LM_INLINE auto operator*=(const VecT& v) -> VecT&       { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
    LM_INLINE auto operator/=(const VecT& v) -> VecT&       { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }

};

template <>
struct LM_ALIGN_16 TVec4<float, SIMD::SSE> : public SIMDTVecBase<float, SIMD::SSE, TVec4, 4>
{

    union
    {
        SIMDT v_;
        struct { VT x, y, z, w; };
    };

    LM_INLINE TVec4()                                       : v_(_mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f)) {}
    LM_INLINE TVec4(ParamT x, ParamT y, ParamT z, ParamT w) : v_(_mm_set_ps(w, z, y, x)) {}
    LM_INLINE TVec4(const VecT& v)                          : v_(v.v_) {}
    LM_INLINE TVec4(SIMDT v)                                : v_(v) {}
    LM_INLINE TVec4(std::initializer_list<VT> l)            { x = l.begin()[0]; y = l.begin()[1]; z = l.begin()[2]; w = l.begin()[3]; }

    LM_INLINE auto operator[](int i)         -> VT&         { return (&x)[i]; }
    LM_INLINE auto operator[](int i) const   -> RetT        { return (&x)[i]; }
    LM_INLINE auto operator=(const VecT& v)  -> VecT&       { v_ = v.v_; return *this; }
    LM_INLINE auto operator+=(const VecT& v) -> VecT&       { v_ = _mm_add_ps(v_, v.v_); return *this; }
    LM_INLINE auto operator-=(const VecT& v) -> VecT&       { v_ = _mm_sub_ps(v_, v.v_); return *this; }
    LM_INLINE auto operator*=(const VecT& v) -> VecT&       { v_ = _mm_mul_ps(v_, v.v_); return *this; }
    LM_INLINE auto operator/=(const VecT& v) -> VecT&       { v_ = _mm_div_ps(v_, v.v_); return *this; }

};

template <>
struct LM_ALIGN_32 TVec4<double, SIMD::AVX> : public SIMDTVecBase<double, SIMD::AVX, TVec4, 4>
{

    union
    {
        SIMDT v_;
        struct { VT x, y, z, w; };
    };

    LM_INLINE TVec4()                                       : v_(_mm256_set_pd(0, 0, 0, 0)) {}
    LM_INLINE TVec4(ParamT x, ParamT y, ParamT z, ParamT w) : v_(_mm256_set_pd(w, z, y, x)) {}
    LM_INLINE TVec4(const VecT& v)                          : v_(v.v_) {}
    LM_INLINE TVec4(SIMDT v)                                : v_(v) {}
    LM_INLINE TVec4(std::initializer_list<VT> l)            { x = l.begin()[0]; y = l.begin()[1]; z = l.begin()[2]; w = l.begin()[3]; }

    LM_INLINE auto operator[](int i)         -> VT&         { return (&x)[i]; }
    LM_INLINE auto operator[](int i) const   -> RetT        { return (&x)[i]; }
    LM_INLINE auto operator=(const VecT& v)  -> VecT&       { v_ = v.v_; return *this; }
    LM_INLINE auto operator+=(const VecT& v) -> VecT&       { v_ = _mm256_add_pd(v_, v.v_); return *this; }
    LM_INLINE auto operator-=(const VecT& v) -> VecT&       { v_ = _mm256_sub_pd(v_, v.v_); return *this; }
    LM_INLINE auto operator*=(const VecT& v) -> VecT&       { v_ = _mm256_mul_pd(v_, v.v_); return *this; }
    LM_INLINE auto operator/=(const VecT& v) -> VecT&       { v_ = _mm256_div_pd(v_, v.v_); return *this; }

};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Base matrix type

template <typename T, SIMD Opt, template <typename, SIMD> class MatT_, template <typename, SIMD> class VecT_, int NC_>
struct TMatBase
{

    // Math object type
    static constexpr MathObjectType ObjT = MathObjectType::Mat;

    // Value type
    using VT = T;

    // Matrix type
    using MatT = MatT_<T, Opt>;

    // Column vector type
    template <typename T__, SIMD Opt__>
    using TVec = VecT_<T__, Opt__>;
    using VecT = VecT_<T, Opt>;

    // Number of components
    static constexpr int NC = NC_;

    // Parameter types
    using ParamT = std::conditional_t<std::is_fundamental<T>::value, T, const T&>;
    using RetT = ParamT;

};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Mat3

/*!
	3x3 matrix.

    Generic column major 3x3 matrix.
    A matrix
        v00 v01 v02
        v10 v11 v12
        v20 v21 v22
    is stored sequentially as v00, v10, ..., v22.
*/
template <typename T, SIMD Opt = SIMD::None>
struct TMat3 : public TMatBase<T, Opt, TMat3, TVec3, 3>
{

    VecT v_[NC];

    LM_INLINE TMat3() {}
    LM_INLINE TMat3(const MatT& m) : v_{m.v_[0], m.v_[1], m.v_[2]} {}
    LM_INLINE TMat3(const VecT& v0, const VecT& v1, const VecT& v2) : v_{v0, v1, v2} {}
    LM_INLINE TMat3(
        ParamT v00, ParamT v10, ParamT v20,
        ParamT v01, ParamT v11, ParamT v21,
        ParamT v02, ParamT v12, ParamT v22)
        : v_{{v00, v10, v20},
             {v01, v11, v21},
             {v02, v12, v22}}
    {}
    LM_INLINE TMat3(std::initializer_list<VT> l)
    {
        for (int i = 0; i < NC; i++)
            for (int j = 0; j < NC; j++)
                v_[i][j] = l.begin()[i*NC+j];
    }

    static auto Identity() -> MatT { return MatT{ 1,0,0, 0,1,0, 0,1,0 }; }

	LM_INLINE auto operator[](int i) -> VecT& { return v_[i]; }
	LM_INLINE auto operator[](int i) const -> const VecT& { return v_[i]; }
    LM_INLINE auto operator*=(const MatT& m) -> MatT& { *this = *this * m; return *this; }

};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Mat4

/*!
	4x4 matrix.

	Generic column major 4x4 matrix. 
	A matrix
		v00 v01 v02 v03
		v10 v11 v12 v13
		v20 v21 v22 v23
		v30 v31 v32 v33
	is stored sequentially as v00, v10, ..., v33
*/
template <typename T, SIMD Opt = SIMD::None>
struct TMat4 : public TMatBase<T, Opt, TMat4, TVec4, 4>
{

    VecT v_[NC];

    LM_INLINE TMat4() {}
    LM_INLINE TMat4(const MatT& m) : v_{m.v_[0], m.v_[1], m.v_[2], m.v_[3]} {}
    LM_INLINE TMat4(const VecT& v0, const VecT& v1, const VecT& v2, const VecT& v3) : v_{v0, v1, v2, v3} {}
    LM_INLINE TMat4(
        ParamT v00, ParamT v10, ParamT v20, ParamT v30,
        ParamT v01, ParamT v11, ParamT v21, ParamT v31,
        ParamT v02, ParamT v12, ParamT v22, ParamT v32,
        ParamT v03, ParamT v13, ParamT v23, ParamT v33)
        : v_{{v00, v10, v20, v30},
             {v01, v11, v21, v31},
             {v02, v12, v22, v32},
             {v03, v13, v23, v33}}
    {}
    LM_INLINE TMat4(std::initializer_list<VT> l)
    {
        for (int i = 0; i < NC; i++)
            for (int j = 0; j < NC; j++)
                v_[i][j] = l.begin()[i*NC+j];
    }

    static auto Identity() -> MatT { return MatT{ 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }; }
    
	LM_INLINE auto operator[](int i) -> VecT& { return v_[i]; }
    LM_INLINE auto operator[](int i) const -> const VecT& { return v_[i]; }
    LM_INLINE auto operator*=(const MatT& m) -> MatT& { *this = *this * m; return *this; }

};

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Some aliases

template <typename T, SIMD Opt, template <typename, SIMD> class MathObject>
using EnableIfVecType = std::enable_if_t<MathObject<T, Opt>::ObjT == MathObjectType::Vec>;

template <typename T, SIMD Opt, template <typename, SIMD> class MathObject>
using EnableIfMatType = std::enable_if_t<MathObject<T, Opt>::ObjT == MathObjectType::Mat>;

template <typename T, SIMD Opt>
using EnableIfSSEType = std::enable_if_t<std::is_same<T, float>::value && Opt == SIMD::SSE>;

template <typename T, SIMD Opt>
using EnableIfAVXType = std::enable_if_t<std::is_same<T, double>::value && Opt == SIMD::AVX>;

#pragma endregion

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Math operations

#pragma region operator+

template <typename T, template <typename, SIMD> class VecT>
LM_INLINE auto operator+(const VecT<T, SIMD::None>& v1, const VecT<T, SIMD::None>& v2) -> VecT<T, SIMD::None>
{
    constexpr int N = VecT<T, SIMD::None>::NC;
    VecT<T, SIMD::None> result;
    for (int i = 0; i < N; i++) result[i] = v1[i] + v2[i];
    return result;
}

template <template <typename, SIMD> class VecT>
LM_INLINE auto operator+(const VecT<float, SIMD::SSE>& v1, const VecT<float, SIMD::SSE>& v2) -> VecT<float, SIMD::SSE>
{
    return VecT<float, SIMD::SSE>(_mm_add_ps(v1.v_, v2.v_));
}

template <template <typename, SIMD> class VecT>
LM_INLINE auto operator+(const VecT<double, SIMD::AVX>& v1, const VecT<double, SIMD::AVX>& v2) -> VecT<double, SIMD::AVX>
{
    return VecT<double, SIMD::AVX>(_mm256_add_pd(v1.v_, v2.v_));
}

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region operator-

template <typename T, template <typename, SIMD> class VecT>
LM_INLINE auto operator-(const VecT<T, SIMD::None>& v1, const VecT<T, SIMD::None>& v2) -> VecT<T, SIMD::None>
{
    constexpr int N = VecT<T, SIMD::None>::NC;
    VecT<T, SIMD::None> result;
    for (int i = 0; i < N; i++) result[i] = v1[i] - v2[i];
    return result;
}

template <template <typename, SIMD> class VecT>
LM_INLINE auto operator-(const VecT<float, SIMD::SSE>& v1, const VecT<float, SIMD::SSE>& v2) -> VecT<float, SIMD::SSE>
{
    return VecT<float, SIMD::SSE>(_mm_sub_ps(v1.v_, v2.v_));
}

template <template <typename, SIMD> class VecT>
LM_INLINE auto operator-(const VecT<double, SIMD::AVX>& v1, const VecT<double, SIMD::AVX>& v2) -> VecT<double, SIMD::AVX>
{
    return VecT<double, SIMD::AVX>(_mm256_sub_pd(v1.v_, v2.v_));
}

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region operator*

template <typename T, template <typename, SIMD> class VecT, typename = EnableIfVecType<T, SIMD::None, VecT>>
LM_INLINE auto operator*(const VecT<T, SIMD::None>& v1, const VecT<T, SIMD::None>& v2) -> VecT<T, SIMD::None>
{
    constexpr int N = VecT<T, SIMD::None>::NC;
    VecT<T, SIMD::None> result;
    for (int i = 0; i < N; i++) result[i] = v1[i] * v2[i];
    return result;
}

template <typename T, template <typename, SIMD> class VecT, typename = EnableIfVecType<T, SIMD::None, VecT>>
LM_INLINE auto operator*(const VecT<T, SIMD::None>& v, const T& s) -> VecT<T, SIMD::None>
{
    constexpr int N = VecT<T, SIMD::None>::NC;
    VecT<T, SIMD::None> result;
    for (int i = 0; i < N; i++) result[i] = v[i] * s;
    return result;
}

template <template <typename, SIMD> class VecT, typename = EnableIfVecType<float, SIMD::SSE, VecT>>
LM_INLINE auto operator*(const VecT<float, SIMD::SSE>& v1, const VecT<float, SIMD::SSE>& v2) -> VecT<float, SIMD::SSE>
{
    return VecT<float, SIMD::SSE>(_mm_mul_ps(v1.v_, v2.v_));
}

template <template <typename, SIMD> class VecT, typename = EnableIfVecType<float, SIMD::SSE, VecT>>
LM_INLINE auto operator*(const VecT<float, SIMD::SSE>& v, float s) -> VecT<float, SIMD::SSE>
{
    return VecT<float, SIMD::SSE>(_mm_mul_ps(v.v_, _mm_set1_ps(s)));
}

template <template <typename, SIMD> class VecT, typename = EnableIfVecType<double, SIMD::AVX, VecT>>
LM_INLINE auto operator*(const VecT<double, SIMD::AVX>& v1, const VecT<double, SIMD::AVX>& v2) -> VecT<double, SIMD::AVX>
{
    return VecT<double, SIMD::AVX>(_mm256_mul_pd(v1.v_, v2.v_));
}

template <template <typename, SIMD> class VecT, typename = EnableIfVecType<double, SIMD::AVX, VecT>>
LM_INLINE auto operator*(const VecT<double, SIMD::AVX>& v, double s) -> VecT<double, SIMD::AVX>
{
    return VecT<double, SIMD::AVX>(_mm256_mul_pd(v.v_, _mm256_set1_pd(s)));
}

template <typename T, SIMD Opt>
LM_INLINE auto operator*(const TMat3<T, Opt>& m1, const TMat3<T, Opt>& m2) -> TMat3<T, Opt>
{
    return TMat3<T, Opt>(m1 * m2[0], m1 * m2[1], m1 * m2[2]);
}

template <typename T, SIMD Opt>
LM_INLINE auto operator*(const TMat3<T, Opt>& m, const TVec3<T, Opt>& v) -> TVec3<T, Opt>
{
    return TVec3<T, Opt>(
        m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
        m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
        m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z);
}

LM_INLINE auto operator*(const TMat3<float, SIMD::SSE>& m, const TVec3<float, SIMD::SSE>& v) -> TVec3<float, SIMD::SSE>
{
	return TVec3<float, SIMD::SSE>(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(m[0].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(0, 0, 0, 0))),
                _mm_mul_ps(m[1].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(1, 1, 1, 1)))),
                _mm_mul_ps(m[2].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(2, 2, 2, 2)))));
}

LM_INLINE auto operator*(const TMat3<double, SIMD::AVX>& m, const TVec3<double, SIMD::AVX>& v) -> TVec3<double, SIMD::AVX>
{
    return TVec3<double, SIMD::AVX>(
        _mm256_add_pd(
            _mm256_add_pd(
                _mm256_mul_pd(m[0].v_, _mm256_broadcast_sd(&(v.x))),
                _mm256_mul_pd(m[1].v_, _mm256_broadcast_sd(&(v.x) + 1))),
                _mm256_mul_pd(m[2].v_, _mm256_broadcast_sd(&(v.x) + 2))));
}

template <typename T, SIMD Opt>
LM_INLINE auto operator*(const TMat4<T, Opt>& m1, const TMat4<T, Opt>& m2) -> TMat4<T, Opt>
{
    return TMat4<T, Opt>(m1 * m2[0], m1 * m2[1], m1 * m2[2], m1 * m2[3]);
}

template <typename T, SIMD Opt>
LM_INLINE auto operator*(const TMat4<T, Opt>& m, const TVec4<T, Opt>& v) -> TVec4<T, Opt>
{
    return TVec4<T, Opt>(
        m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w,
        m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w,
        m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w,
        m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w);
}

LM_INLINE auto operator*(const TMat4<float, SIMD::SSE>& m, const TVec4<float, SIMD::SSE>& v) -> TVec4<float, SIMD::SSE>
{
    return TVec4<float, SIMD::SSE>(
        _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(m[0].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(0, 0, 0, 0))),
                _mm_mul_ps(m[1].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(1, 1, 1, 1)))),
            _mm_add_ps(
                _mm_mul_ps(m[2].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(2, 2, 2, 2))),
                _mm_mul_ps(m[3].v_, _mm_shuffle_ps(v.v_, v.v_, _MM_SHUFFLE(3, 3, 3, 3))))));
}

LM_INLINE auto operator*(const TMat4<double, SIMD::AVX>& m, const TVec4<double, SIMD::AVX>& v) -> TVec4<double, SIMD::AVX>
{
    return TVec4<double, SIMD::AVX>(
        _mm256_add_pd(
            _mm256_add_pd(
                _mm256_mul_pd(m[0].v_, _mm256_broadcast_sd(&(v.x))),
                _mm256_mul_pd(m[1].v_, _mm256_broadcast_sd(&(v.x) + 1))),
            _mm256_add_pd(
                _mm256_mul_pd(m[2].v_, _mm256_broadcast_sd(&(v.x) + 2)),
                _mm256_mul_pd(m[3].v_, _mm256_broadcast_sd(&(v.x) + 3)))));
}

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region operator/

template <typename T, template <typename, SIMD> class VecT>
LM_INLINE auto operator/(const VecT<T, SIMD::None>& v1, const VecT<T, SIMD::None>& v2) -> VecT<T, SIMD::None>
{
    constexpr int N = VecT<T, SIMD::None>::NC;
    VecT<T, SIMD::None> result;
    for (int i = 0; i < N; i++) result[i] = v1[i] / v2[i];
    return result;
}

template <template <typename, SIMD> class VecT>
LM_INLINE auto operator/(const VecT<float, SIMD::SSE>& v1, const VecT<float, SIMD::SSE>& v2) -> VecT<float, SIMD::SSE>
{
    return VecT<float, SIMD::SSE>(_mm_div_ps(v1.v_, v2.v_));
}

template <template <typename, SIMD> class VecT>
LM_INLINE auto operator/(const VecT<double, SIMD::AVX>& v1, const VecT<double, SIMD::AVX>& v2) -> VecT<double, SIMD::AVX>
{
    return VecT<double, SIMD::AVX>(_mm256_div_pd(v1.v_, v2.v_));
}

#pragma endregion

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Default types

using Vec3 = TVec3<Float, SIMD::Default>;
using Vec4 = TVec4<Float, SIMD::Default>;
using Mat3 = TMat3<Float, SIMD::Default>;
using Mat4 = TMat4<Float, SIMD::Default>;

#pragma endregion

// --------------------------------------------------------------------------------

#pragma region Math utility

namespace Math
{
    #pragma region Constants

    template <typename T> constexpr auto Pi() -> T { return T(3.14159265358979323846); }

    #pragma endregion

    // --------------------------------------------------------------------------------

    #pragma region Basic functions

    template <typename T> constexpr auto Radians(const T& v) -> T { return v * Pi<T>() / T(180); }
    template <typename T> constexpr auto Degrees(const T& v) -> T { return v * T(180) / Pi<T>(); }
    template <typename T> LM_INLINE auto Cos(const T& v)     -> T { return std::cos(v); }
    template <typename T> LM_INLINE auto Sin(const T& v)     -> T { return std::sin(v); }
    template <typename T> LM_INLINE auto Sqrt(const T& v)    -> T { return std::sqrt(v); }

    #pragma endregion

    // --------------------------------------------------------------------------------

    #pragma region Vector functions

    #pragma region Dot

    template <typename T, SIMD Opt>
    LM_INLINE auto Dot(const TVec3<T, Opt>& v1, const TVec3<T, Opt>& v2) -> T
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    template <>
    LM_INLINE auto Dot<float, SIMD::SSE>(const TVec3<float, SIMD::SSE>& v1, const TVec3<float, SIMD::SSE>& v2) -> float
    {
        return _mm_cvtss_f32(_mm_dp_ps(v1.v_, v2.v_, 0x71));
    }

    template <>
    LM_INLINE auto Dot<double, SIMD::AVX>(const TVec3<double, SIMD::AVX>& v1, const TVec3<double, SIMD::AVX>& v2) -> double
    {
        __m256d z = _mm256_setzero_pd();
        __m256d tv1 = _mm256_blend_pd(v1.v_, z, 0x8);	// = ( 0, z1, y1, x1 )
        __m256d tv2 = _mm256_blend_pd(v2.v_, z, 0x8);	// = ( 0, z2, y2, x2 )
        __m256d t1 = _mm256_mul_pd(tv1, tv2);			// = ( 0, z1 * z2, y1 * y2, x1 * x2 )
        __m256d t2 = _mm256_hadd_pd(t1, t1);			// = ( z1 * z2, z1 * z2, x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
        __m128d t3 = _mm256_extractf128_pd(t2, 1);		// = ( z1 * z2, z1 * z2 )
        __m128d t4 = _mm256_castpd256_pd128(t2);		// = ( x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
        __m128d result = _mm_add_pd(t3, t4);			// = ( x1 * x2 + y1 * y2 + z1 * z2, x1 * x2 + y1 * y2 + z1 * z2 )
        return _mm_cvtsd_f64(result);
    }

    template <typename T, SIMD Opt>
    LM_INLINE auto Dot(const TVec4<T, Opt>& v1, const TVec4<T, Opt>& v2) -> T
    {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
    }

    template <>
    LM_INLINE auto Dot<float, SIMD::SSE>(const TVec4<float, SIMD::SSE>& v1, const TVec4<float, SIMD::SSE>& v2) -> float
    {
        return _mm_cvtss_f32(_mm_dp_ps(v1.v_, v2.v_, 0xf1));
    }

    template <>
    LM_INLINE auto Dot<double, SIMD::AVX>(const TVec4<double, SIMD::AVX>& v1, const TVec4<double, SIMD::AVX>& v2) -> double
    {
        __m256d t1 = _mm256_mul_pd(v1.v_, v2.v_);   // = ( w1 * w2, z1 * z2, y1 * y2, x1 * x2 )
        __m256d t2 = _mm256_hadd_pd(t1, t1);		// = ( z1 * z2 + w1 * w2, z1 * z2 + w1 * w2, x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
        __m128d t3 = _mm256_extractf128_pd(t2, 1);	// = ( z1 * z2 + w1 * w2, z1 * z2 + w1 * w2 )
        __m128d t4 = _mm256_castpd256_pd128(t2);	// = ( x1 * x2 + y1 * y2, x1 * x2 + y1 * y2 )
        __m128d result = _mm_add_pd(t3, t4);		// = ( _, v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w )
        return _mm_cvtsd_f64(result);
    }

    #pragma endregion

    // --------------------------------------------------------------------------------

    #pragma region Length

    template <typename T, SIMD Opt, template <typename, SIMD> class VecT>
    LM_INLINE auto Length(const VecT<T, Opt>& v) -> T
    {
        return Math::Sqrt(Math::Length2(v));
    }

    template <>
    LM_INLINE auto Length<float, SIMD::SSE, TVec3<float, SIMD::SSE>>(const TVec3<float, SIMD::SSE>& v) -> float
    {
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.v, v.v, 0x71)));
    }

    template <>
    LM_INLINE auto Length<float, SIMD::SSE, TVec4<float, SIMD::SSE>>(const TVec4<float, SIMD::SSE>& v) -> float
    {
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(v.v, v.v, 0xf1)));
    }

    #pragma endregion

    // --------------------------------------------------------------------------------

    #pragma region Length2

    template <typename T, SIMD Opt, template <typename, SIMD> class VecT>
    LM_INLINE T Length2(const VecT<T, Opt>& v)
    {
        return Dot(v, v);
    }

    template <>
    LM_INLINE auto Length2<float, SIMD::SSE, TVec3<float, SIMD::SSE>>(const TVec3<float, SIMD::SSE>& v) -> float
    {
        return _mm_cvtss_f32(_mm_dp_ps(v.v_, v.v_, 0x71));
    }

    template <>
    LM_INLINE auto Length2<float, SIMD::SSE, TVec4<float, SIMD::SSE>>(const TVec4<float, SIMD::SSE>& v) -> float
    {
        return _mm_cvtss_f32(_mm_dp_ps(v.v_, v.v_, 0xf1));
    }
 
    #pragma endregion

    // --------------------------------------------------------------------------------

    #pragma region Normalize

    template <typename T, SIMD Opt>
    LM_INLINE TVec4<T> Normalize(const TVec4<T>& v)
    {
        return v / Length(v);
    }

    #pragma endregion

    #pragma endregion

    // --------------------------------------------------------------------------------

    #pragma region Transform

    template <typename T, SIMD Opt>
    LM_INLINE TMat4<T, Opt> Translate(const TMat4<T, Opt>& m, const TVec3<T, Opt>& v)
    {
        TMat4<T, Opt> r(m);
        r[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
        return r;
    }

    template <typename T, SIMD Opt>
    LM_INLINE TMat4<T, Opt> Translate(const TVec3<T, Opt>& v)
    {
        return Translate<T>(TMat4<T, Opt>::Identity(), v);
    }

    template <typename T, SIMD Opt>
    LM_INLINE TMat4<T, Opt> Rotate(const TMat4<T, Opt>& m, const T& angle, const TVec3<T, Opt>& axis)
    {
	    T c = Cos(angle);
	    T s = Sin(angle);

	    TVec3<T, Opt> a = Normalize(axis);
	    TVec3<T, Opt> t = T(T(1) - c) * a;	// For expression template, (T(1) - c) * a generates compile errors

	    TMat4<T, Opt> rot;
	    rot[0][0] = c + t[0] * a[0];
	    rot[0][1] =     t[0] * a[1] + s * a[2];
	    rot[0][2] =     t[0] * a[2] - s * a[1];
	    rot[1][0] =     t[1] * a[0] - s * a[2];
	    rot[1][1] = c + t[1] * a[1];
	    rot[1][2] =     t[1] * a[2] + s * a[0];
	    rot[2][0] =     t[2] * a[0] + s * a[1];
	    rot[2][1] =     t[2] * a[1] - s * a[0];
	    rot[2][2] = c + t[2] * a[2];

	    TMat4<T, Opt> r;
	    r[0] = m[0] * rot[0][0] + m[1] * rot[0][1] + m[2] * rot[0][2];
	    r[1] = m[0] * rot[1][0] + m[1] * rot[1][1] + m[2] * rot[1][2];
	    r[2] = m[0] * rot[2][0] + m[1] * rot[2][1] + m[2] * rot[2][2];
	    r[3] = m[3];

	    return r;
    }

    template <typename T, SIMD Opt>
    LM_INLINE TMat4<T, Opt> Rotate(const T& angle, const TVec3<T, Opt>& axis)
    {
	    return Rotate(TMat4<T, Opt>::Identity(), angle, axis);
    }

    template <typename T, SIMD Opt>
    LM_INLINE TMat4<T, Opt> Scale(const TMat4<T, Opt>& m, const TVec3<T, Opt>& v)
    {
        return TMat4<T, Opt>(m[0] * v[0], m[1] * v[1], m[2] * v[2], m[3]);
    }

    template <typename T, SIMD Opt>
    LM_INLINE TMat4<T, Opt> Scale(const TVec3<T, Opt>& v)
    {
        return Scale(TMat4<T, Opt>::Identity(), v);
    }

    #pragma endregion   
}

#pragma endregion

LM_NAMESPACE_END
