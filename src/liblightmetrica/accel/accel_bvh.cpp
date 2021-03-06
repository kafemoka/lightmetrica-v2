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
#include <lightmetrica/accel.h>
#include <lightmetrica/scene.h>
#include <lightmetrica/trianglemesh.h>
#include <lightmetrica/triaccel.h>
#include <lightmetrica/primitive.h>
#include <lightmetrica/bound.h>
#include <lightmetrica/intersectionutils.h>

LM_NAMESPACE_BEGIN

struct BVHNode
{
    bool isleaf;
    Bound bound;

    union
    {
        struct
        {
            int begin;
            int end;
        } leaf;

        struct
        {
            int child1;
            int child2;
        } internal;
    };
};

class Accel_BVH final : public Accel
{
public:

    LM_IMPL_CLASS(Accel_BVH, Accel);

public:

    LM_IMPL_F(Initialize) = [this](const PropertyNode*) -> bool
    {
        return true;
    };

    LM_IMPL_F(Build) = [this](const Scene* scene) -> bool
    {
        std::vector<Bound> bounds_;

        // --------------------------------------------------------------------------------

        #pragma region Create triaccels

        int np = scene->NumPrimitives();
        for (int i = 0; i < np; i++)
        {
            const auto* prim = scene->PrimitiveAt(i);
            const auto* mesh = prim->mesh;
            if (mesh)
            {
                // Enumerate all triangles and create triaccels
                const auto* ps = mesh->Positions();
                const auto* faces = mesh->Faces();
                for (int j = 0; j < mesh->NumFaces(); j++)
                {
                    // Create a triaccel
                    triangles_.push_back(TriAccelTriangle());
                    triangles_.back().faceIndex = j;
                    triangles_.back().primIndex = i;
                    unsigned int i1 = faces[3 * j];
                    unsigned int i2 = faces[3 * j + 1];
                    unsigned int i3 = faces[3 * j + 2];
                    Vec3 p1(prim->transform * Vec4(ps[3 * i1], ps[3 * i1 + 1], ps[3 * i1 + 2], 1_f));
                    Vec3 p2(prim->transform * Vec4(ps[3 * i2], ps[3 * i2 + 1], ps[3 * i2 + 2], 1_f));
                    Vec3 p3(prim->transform * Vec4(ps[3 * i3], ps[3 * i3 + 1], ps[3 * i3 + 2], 1_f));
                    triangles_.back().Load(p1, p2, p3);

                    Bound bound;
                    bound = Math::Union(bound, p1);
                    bound = Math::Union(bound, p2);
                    bound = Math::Union(bound, p3);
                    bound.min -= Vec3(Math::Eps());
                    bound.max += Vec3(Math::Eps());
                    bounds_.push_back(bound);
                }
            }
        }

        #pragma endregion

        // --------------------------------------------------------------------------------

        #pragma region Build BVH

        const std::function<int(int, int)> Build_ = [&](int begin, int end) -> int
        {
            int idx = (int)(nodes_.size());
            nodes_.emplace_back(new BVHNode);
            auto* node = nodes_[idx].get();

            for (int i = begin; i < end; i++)
            {
                node->bound = Math::Union(node->bound, bounds_[i]);
            }

            // Leaf node
            const int LeafNumNodes = 10;
            if (end - begin < LeafNumNodes)
            {
                node->isleaf = true;
                node->leaf.begin = begin;
                node->leaf.end = end;
                return idx;
            }

            // Intermediate node
            const int mid = begin + (end - begin) / 2;
            node->isleaf = false;
            node->internal.child1 = Build_(begin, mid);
            node->internal.child2 = Build_(mid, end);

            return idx;
        };

        nodes_.clear();
        Build_(0, (int)(triangles_.size()));

        #pragma endregion

        // --------------------------------------------------------------------------------

        return true;
    };

    LM_IMPL_F(Intersect) = [this](const Scene* scene, const Ray& ray, Intersection& isect, Float minT, Float maxT) -> bool
    {
        int minIndex;
        Vec2 minB;

        const std::function<bool(int)> Intersect_ = [&](int idx) -> bool
        {
            const auto* node = nodes_.at(idx).get();

            // Check intersection with bound
            if (!node->bound.Intersect(ray, minT, maxT))
            {
                return false;
            }

            // Check intersection with objects in the leaf
            if (node->isleaf)
            {
                bool hit = false;
                for (int i = node->leaf.begin; i < node->leaf.end; i++)
                {
                    Float t;
                    Vec2 b;
                    if (triangles_[i].Intersect(ray, minT, maxT, b[0], b[1], t))
                    {
                        hit = true;
                        maxT = t;
                        minIndex = i;
                        minB = b;
                    }
                }
                return hit;
            }

            // Check intersection with child nodes
            bool hit = false;
            hit |= Intersect_(node->internal.child1);
            hit |= Intersect_(node->internal.child2);
            return hit;
        };

        if (!Intersect_(0))
        {
            return false;
        }

        isect = IntersectionUtils::CreateTriangleIntersection(
            scene->PrimitiveAt(triangles_[minIndex].primIndex),
            ray.o + ray.d * maxT,
            minB,
            triangles_[minIndex].faceIndex);

        return true;
    };

private:

    std::vector<TriAccelTriangle> triangles_;
    std::vector<std::unique_ptr<BVHNode>> nodes_;

};

LM_COMPONENT_REGISTER_IMPL(Accel_BVH, "accel::bvh");

LM_NAMESPACE_END
