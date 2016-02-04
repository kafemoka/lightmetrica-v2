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

#include <lightmetrica/generalizedbsdf.h>

LM_NAMESPACE_BEGIN

class PositionSampler;

/*!
    \brief An interface for Emitter.
    \ingroup asset
*/
class Emitter : public GeneralizedBSDF
{
public:

    LM_INTERFACE_CLASS(Emitter, GeneralizedBSDF, 4);

public:

    Emitter() = default;
    LM_DISABLE_COPY_AND_MOVE(Emitter);

public:

    /*!
        \brief Sample a position on the light.
        \param u Uniform random numbers in [0,1]^2.
        \param geom Surface geometry at the sampled position.
    */
    LM_INTERFACE_F(0, SamplePosition, void(const Vec2& u, SurfaceGeometry& geom));

    /*!
        \brief Evaluate positional PDF.
        \param geom Surface geometry.
        \return Evaluated PDF.
    */
    LM_INTERFACE_F(1, EvaluatePositionPDF, Float(const SurfaceGeometry& geom, bool evalDelta));

    /*!
        \brief Evaluate the positional component of the emitted quantity.
        \param geom Surface geometry.
        \return Positional component of the emitted quantity.
    */
    LM_INTERFACE_F(2, EvaluatePosition, SPD(const SurfaceGeometry& geom, bool evalDelta));

    /*!
        \brief Compute raster position from the direction and the position.

        The function calculates the raster position from the outgoing ray.
        Returns false if calculated raster position is the outside of [0, 1]^2.

        \param wo           Outgoing direction from the point on the emitter.
        \param geom         Surface geometry infromation around the point on the emitter.
        \param rasterPos    Computed raster position.
    */
    LM_INTERFACE_F(3, RasterPosition, bool(const Vec3& wo, const SurfaceGeometry& geom, Vec2& rasterPos));

};

LM_NAMESPACE_END
