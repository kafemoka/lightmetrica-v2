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

#include <pch.h>
#include <lightmetrica/random.h>
#include <dSFMT.h>

LM_NAMESPACE_BEGIN

class Random::Impl
{
public:
    dsfmt_t dsfmt;
};

auto Random_Constructor(Random* p) -> void
{
    p->p_ = new Random::Impl;
}

auto Random_Destructor(Random* p) -> void
{
    LM_SAFE_DELETE(p->p_);
}

auto Random_SetSeed(Random* p, unsigned int seed) -> void
{
    dsfmt_init_gen_rand(&p->p_->dsfmt, seed);
}

auto Random_NextUInt(Random* p) -> unsigned int
{
    return dsfmt_genrand_uint32(&p->p_->dsfmt);
}

auto Random_Next(Random* p) -> double
{
    return dsfmt_genrand_close_open(&p->p_->dsfmt);
}

LM_NAMESPACE_END
