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
#include <lightmetrica/property.h>
#include <lightmetrica/logger.h>
#include <lightmetrica-test/utils.h>

LM_TEST_NAMESPACE_BEGIN

struct PropertyTest : public ::testing::Test
{
    virtual auto SetUp() -> void override { Logger::Run(); }
    virtual auto TearDown() -> void override { Logger::Stop(); }
};
//
//TEST_F(PropertyTest, Scalar)
//{
//    const auto Scalar_Input = TestUtils::MultiLineLiteral(R"x(
//    | a
//    )x");
//
//    auto p = ComponentFactory::Create<PropertyTree>();
//
//    ASSERT_TRUE(p->LoadFromString(Scalar_Input));
//
//    const auto* root = p->Root();
//    ASSERT_NE(nullptr, root);
//    EXPECT_EQ(PropertyNodeType::Scalar, root->Type());
//    EXPECT_EQ("a", root->Scalar());
//}

TEST_F(PropertyTest, Map)
{
    const auto Map_Input = TestUtils::MultiLineLiteral(R"x(
    | A: a
    | B: b
    )x");

    auto p = ComponentFactory::Create<PropertyTree>();

    ASSERT_TRUE(p->LoadFromString(Map_Input));
    
    const auto* root = p->Root();
    ASSERT_NE(nullptr, root);
    EXPECT_EQ(PropertyNodeType::Map, root->Type());

    const auto* A = root->Child("A");
    ASSERT_NE(nullptr, A);
    EXPECT_EQ(PropertyNodeType::Scalar, A->Type());
    EXPECT_EQ("A", A->Key());
    EXPECT_EQ("a", A->As<std::string>());

    const auto* B = root->Child("B");
    ASSERT_NE(nullptr, B);
    EXPECT_EQ(PropertyNodeType::Scalar, B->Type());
    EXPECT_EQ("B", B->Key());
    EXPECT_EQ("b", B->As<std::string>());
}

TEST_F(PropertyTest, Sequence)
{
    const auto Sequence_Input = TestUtils::MultiLineLiteral(R"x(
    | - a
    | - b
    )x");

    auto p = ComponentFactory::Create<PropertyTree>();

    ASSERT_TRUE(p->LoadFromString(Sequence_Input));

    const auto* root = p->Root();
    ASSERT_NE(nullptr, root);
    EXPECT_EQ(PropertyNodeType::Sequence, root->Type());

    const auto* n0 = root->At(0);
    ASSERT_NE(nullptr, n0);
    EXPECT_EQ("a", n0->As<std::string>());

    const auto* n1 = root->At(1);
    ASSERT_NE(nullptr, n1);
    EXPECT_EQ("b", n1->As<std::string>());
}

TEST_F(PropertyTest, Tree)
{
    const auto Tree_Input = TestUtils::MultiLineLiteral(R"x(
    | A:
    |   - A1
    |   - A2
    | B:
    |   - B1
    |   - B2
    )x");

    auto p = ComponentFactory::Create<PropertyTree>();

    ASSERT_TRUE(p->LoadFromString(Tree_Input));

    const auto* root = p->Root();
    EXPECT_EQ("A1", root->Child("A")->At(0)->As<std::string>());
    EXPECT_EQ("A2", root->Child("A")->At(1)->As<std::string>());
    EXPECT_EQ("B1", root->Child("B")->At(0)->As<std::string>());
    EXPECT_EQ("B2", root->Child("B")->At(1)->As<std::string>());
}

TEST_F(PropertyTest, Tree_2)
{
    const auto Tree_Input_2 = TestUtils::MultiLineLiteral(R"x(
    | A: [1, 2, 3, 4]
    | B: >
    |   1 2
    |   3 4
    )x");

    auto p = ComponentFactory::Create<PropertyTree>();

    ASSERT_TRUE(p->LoadFromString(Tree_Input_2));

    const auto* root = p->Root();
    EXPECT_EQ("1", root->Child("A")->At(0)->As<std::string>());
    EXPECT_EQ("2", root->Child("A")->At(1)->As<std::string>());
    EXPECT_EQ("3", root->Child("A")->At(2)->As<std::string>());
    EXPECT_EQ("4", root->Child("A")->At(3)->As<std::string>());

    EXPECT_EQ("1 2 3 4\n", root->Child("B")->As<std::string>());
}

TEST_F(PropertyTest, TypeConversion)
{
    const auto TypeConversion_Input = TestUtils::MultiLineLiteral(R"x(
    | - hello
    | - 1
    | - 1.1
    )x");

    auto p = ComponentFactory::Create<PropertyTree>();

    ASSERT_TRUE(p->LoadFromString(TypeConversion_Input));

    const auto* root = p->Root();
    EXPECT_EQ("hello", root->At(0)->As<std::string>());
    EXPECT_EQ(1, root->At(1)->As<int>());
    EXPECT_EQ(1.1, root->At(2)->As<double>());
}

TEST_F(PropertyTest, LineColumn)
{
    const auto ErrorHandling_Input = TestUtils::MultiLineLiteral(R"x(
    | A:        # 1
    |   A1:     # 2
    |   A2:     # 3
    |     A3:   # 4
    )x");

    auto p = ComponentFactory::Create<PropertyTree>();
    ASSERT_TRUE(p->LoadFromString(ErrorHandling_Input));

    EXPECT_EQ(0, p->Root()->Line());
    EXPECT_EQ(1, p->Root()->Child("A")->Line());
    EXPECT_EQ(2, p->Root()->Child("A")->Child("A1")->Line());
    EXPECT_EQ(3, p->Root()->Child("A")->Child("A2")->Line());
    EXPECT_EQ(4, p->Root()->Child("A")->Child("A2")->Child("A3")->Line());
}

TEST_F(PropertyTest, ValueOrDefault)
{
    auto p = ComponentFactory::Create<PropertyTree>();
    ASSERT_TRUE(p->LoadFromString(TestUtils::MultiLineLiteral(R"x(
    | A: hello
    )x")));

    EXPECT_EQ("hello", p->Root()->ChildAs<std::string>("A", "world"));
    EXPECT_EQ("world", p->Root()->ChildAs<std::string>("B", "world"));
}

//TEST_F(PropertyTest, ErrorHandling)
//{
//    FAIL();
//}

LM_TEST_NAMESPACE_END
