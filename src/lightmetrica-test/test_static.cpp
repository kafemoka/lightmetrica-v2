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

#include <pch_test.h>
#include <lightmetrica/statictest.h>

LM_TEST_NAMESPACE_BEGIN

/*
    Check if the lightmetrica library is loaded in
    the static initialization phase.
*/
TEST (StaticTest, CheckLoaded)
{
    EXPECT_TRUE(StaticInit<ExternalPolicy>::Instance().Library() != nullptr);
}

/*
    Call static member function and call.
    The static member function is exported with c linkage
    and loaded without any automatic loading feature.
*/
TEST(StaticTest, LoadAndEvaluate)
{
    EXPECT_EQ(42, detail::StaticTest::Func1());
    EXPECT_EQ(3,  detail::StaticTest::Func2(1, 2));
}

LM_TEST_NAMESPACE_END
