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
#include <lightmetrica/component.h>
#include <lightmetrica/texture.h>
#include <lightmetrica/logger.h>
#include <lightmetrica-test/mathutils.h>

LM_TEST_NAMESPACE_BEGIN

struct PluginTest : public ::testing::Test
{
    virtual auto SetUp() -> void override { Logger::SetVerboseLevel(2); Logger::Run(); }
    virtual auto TearDown() -> void override { Logger::Stop(); }
};

TEST_F(PluginTest, LoadPlugin)
{
    // Load plugins
    ASSERT_TRUE(ComponentFactory::LoadPlugin("./plugin/texture_white"));

    {
        // Create instance from plugin
        const auto p = ComponentFactory::Create<Texture>("texture::white");
        EXPECT_TRUE(ExpectVecNear(Vec3(1_f), p->Evaluate(Vec2())));
    }

    // Unload
    ComponentFactory::UnloadPlugins();
}

TEST_F(PluginTest, LoadPlugins)
{
    // Load plugins
    ComponentFactory::LoadPlugins("./plugin");

    {
        // Create instance from plugin
        const auto p = ComponentFactory::Create<Texture>("texture::white");
        EXPECT_TRUE(ExpectVecNear(Vec3(1_f), p->Evaluate(Vec2())));
    }

    // Unload
    ComponentFactory::UnloadPlugins();
}

LM_TEST_NAMESPACE_END
