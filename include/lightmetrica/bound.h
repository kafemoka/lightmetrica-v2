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

#include <lightmetrica/math.h>

LM_NAMESPACE_BEGIN

/*!
    \addtogroup math
    \{
*/

//! Axis-aligned bounding box.
struct Bound
{
    Vec3 min{  Math::Inf() };
    Vec3 max{ -Math::Inf() };
};

namespace Math
{
    //! Merge two bounds
    LM_INLINE auto Union(const Bound& a, const Bound& b) -> Bound
    {
        Bound r;
        r.min = Math::Min(a.min, b.min);
        r.max = Math::Max(a.max, b.max);
        return r;
    }

    //! Merge one bound and a point
    LM_INLINE auto Union(const Bound& a, const Vec3& p) -> Bound
    {
        Bound r;
        r.min = Math::Min(a.min, p);
        r.max = Math::Max(a.max, p);
        return r;
    }
}

//! \}

LM_NAMESPACE_END