#include "packingsolver/rectangleguillotine/branching_scheme.hpp"

#include <gtest/gtest.h>

using namespace packingsolver;
using namespace packingsolver::rectangleguillotine;

TEST(RectangleGuillotineBranchingScheme, InsertionNoDefect)
{
    /**
     * The defect insertion is dominated by one of the item insertions.
     *
     * |--------------------------------------------------| 3210
     * |                                                  |
     * |                                                  |
     * |                                                  |
     * |                                                  |
     * |                                                  |
     * |                                                  |
     * |                            x                     |
     * |                                                  |
     * |-------------------|                              | 500
     * |         0         | 500                          |
     * |-------------------|------------------------------|
     *                   1000                           6000
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 2000, 1500, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    BranchingScheme::Node node(branching_scheme);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 1000, .y2 = 500, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };

    EXPECT_EQ(node.children(info), is);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect1)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------|---|                      | 1000
     * |   0   |   |                      |
     * |-------|   |                      | 500
     * |       | 1 |                      |
     * |   x   |   |                      | 250
     * |       |   |                      |
     * |-------|---|----------------------|
     *   500 1000 1700
     *
     */

    Info info = Info()
        //.set_log2stderr(true)
        ;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(700, 1000, -1, 1, false, false);
    instance.add_item(1700, 2000, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 500, 248, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 750, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 502, .y2 = 250, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(root, info), is);

    BranchingScheme::Insertion i0 = {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 750, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is_2 {
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 2000, .y2 = 750, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1700, .y2 = 1000, .x3 = 1700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 1450, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 1750, .x3 = 700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is_2);

    BranchingScheme::Insertion i1 = {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1700, .y2 = 1000, .x3 = 1700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is1 = branching_scheme.children(node_1, info);
    EXPECT_NE(std::find(is1.begin(), is1.end(), i1), is1.end());
    auto node_2 = branching_scheme.child(node_1, i1);

    BranchingScheme::Insertion i2 = {.j1 = 2, .j2 = -1, .df = 1, .x1 = 1700, .y2 = 3000, .x3 = 1700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is2 = branching_scheme.children(node_2, info);
    EXPECT_NE(std::find(is2.begin(), is2.end(), i2), is2.end());
    auto node_3 = branching_scheme.child(node_2, i2);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect2)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------|                          | 1400
     * |       |                          |
     * |   0   |---|                      | 1000
     * |-------|   |                      |
     * |   x   |   |                      | 900
     * |       |   |                      |
     * |       | 1 |                      |
     * |   x   |   |                      | 300
     * |       |   |                      |
     * |-------|---|----------------------|
     *   500 1000 1700
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(700, 1000, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 500, 298, 2, 2);
    instance.add_defect(0, 500, 898, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 800, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 502, .y2 = 300, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 502, .y2 = 900, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(root, info), is);

    BranchingScheme::Insertion i0 = {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 800, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is_2 {
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 2000, .y2 = 800, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1700, .y2 = 1400, .x3 = 1700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = 1, .df = 1, .x1 = 1000, .y2 = 1600, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = 1, .df = 1, .x1 = 1000, .y2 = 1900, .x3 = 700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 900, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is_2);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect3)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------|                          | 1800
     * |       |                          |
     * |   0   |                          |
     * |-------|                          |
     * |   x   |                          | 1300
     * |       |---|                      | 1000
     * |       |   |                      |
     * |   x   |   |                      | 900
     * |       |   |                      |
     * |       | 1 |                      |
     * |   x   |   |                      | 300
     * |       |   |                      |
     * |-------|---|----------------------|
     *   500 1000 1700
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(700, 1000, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 500, 298, 2, 2);
    instance.add_defect(0, 500, 898, 2, 2);
    instance.add_defect(0, 500, 1298, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 800, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 502, .y2 = 300, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 502, .y2 = 900, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
        //{.j1 = -1, .j2 = -1, .df = -1, .x1 = 502, .y2 = 1300, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(root, info), is);

    BranchingScheme::Insertion i0 = {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 800, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is_2 {
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 2000, .y2 = 800, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1700, .y2 = 1800, .x3 = 1700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = 1, .df = 1, .x1 = 1000, .y2 = 2000, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = 1, .df = 1, .x1 = 1000, .y2 = 2300, .x3 = 700, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 900, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 1300, .x3 = 502, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is_2);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect4)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |---|-------|                      | 1000
     * |   |   1   |                      |
     * |   |-------|                      | 500
     * | 0 |       |                      |
     * |   |   x   |                      | 250
     * |   |       |                      |
     * |---|-------|----------------------|
     *    500    2000
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(500, 1500, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 1000, 248, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = 1, .df = 2, .x1 = 2000, .y2 = 1000, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1000, .y2 = 1500, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = 2, .x1 = 1002, .y2 = 1000, .x3 = 1002, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 0},
        {.j1 = 1, .j2 = -1, .df = 1, .x1 = 500, .y2 = 2500, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect5)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |   |-------|                      | 1300
     * |---|   1   |                      | 1000
     * |   |-------|                      |
     * |   |   x   |                      | 800
     * |   |       |                      |
     * | 0 |       |                      |
     * |   |   x   |                      | 250
     * |   |       |                      |
     * |---|-------|----------------------|
     *    500    2000
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(500, 1500, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 1000, 248, 2, 2);
    instance.add_defect(0, 1000, 798, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = 1, .df = 2, .x1 = 2000, .y2 = 1300, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1000, .y2 = 1500, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = 2, .x1 = 1002, .y2 = 1000, .x3 = 1002, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 0},
        {.j1 = 1, .j2 = -1, .df = 1, .x1 = 500, .y2 = 2500, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect6)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------|                          | 1000
     * |   1   |   x                      | 750
     * |-------|                          | 500
     * |   0   |                          |
     * |-------|--------------------------|
     *       1000
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(500, 1000, -1, 1, false, false);
    instance.add_item(500, 1500, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 1250, 748, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = 0, .j2 = 1, .df = -1, .x1 = 1000, .y2 = 1000, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 2};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = 2, .j2 = -1, .df = 2, .x1 = 2500, .y2 = 1000, .x3 = 2500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 2},
        {.j1 = -1, .j2 = -1, .df = 2, .x1 = 1252, .y2 = 1000, .x3 = 1252, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 2},
        {.j1 = 2, .j2 = -1, .df = 1, .x1 = 1500, .y2 = 1500, .x3 = 1500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 2, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 2500, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect7)
{
    /**
     * The defect is very close to the side, therefore yj = minwaste.
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------|                          |
     * |   0   |                          |
     * |-------|                          |
     * |   x                              | 10
     * |----------------------------------|
     *   500
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 248, 8, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 520, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = 0, .df = -1, .x1 = 500, .y2 = 1020, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 250, .y2 = 20, .x3 = 250, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(root, info), is);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutDefect8)
{
    /**
     * If we add item 2, then yk = 3000 => 3210
     * However, in this case, item 0 contains a defect.
     * Therefore, it should not be possible to insert item 1.
     *
     * |----------------------------------| 3210
     * |   x                              | 3205
     * |           |---|                  | 3180
     * |       |---)   |                  | 3170
     * |       |   |   |                  |
     * |       |   |   |                  |
     * |       |   |   |                  |
     * |       |   |   |                  |
     * |       |   |   |                  |
     * |       | 1 | 2 |                  |
     * |       |   |   |                  |
     * |-------|   |   |                  |
     * |   0   |   |   |                  |
     * |-------|   |   |                  |
     * |       |   |   |                  |
     * |   x   |   |   |                  | 250
     * |       |   |   |                  |
     * |-------|---|---|------------------|
     *       1000 1200 1400
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(200, 3170, -1, 1, false, false);
    instance.add_item(200, 3180, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 498, 248, 2, 2);
    instance.add_defect(0, 498, 3205, 2, 2);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = -1, .j2 = 0, .df = -1, .x1 = 1000, .y2 = 750, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    BranchingScheme::Insertion i1 = {.j1 = 1, .j2 = -1, .df = 2, .x1 = 1200, .y2 = 3170, .x3 = 1200, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is1 = branching_scheme.children(node_1, info);
    EXPECT_NE(std::find(is1.begin(), is1.end(), i1), is1.end());
    auto node_2 = branching_scheme.child(node_1, i1);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = -1, .df = 1, .x1 = 1200, .y2 = 3210, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 2, .j2 = -1, .df = 0, .x1 = 4380, .y2 = 200, .x3 = 4380, .x1_max = 4700, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 2, .j2 = -1, .df = 0, .x1 = 1400, .y2 = 3180, .x3 = 1400, .x1_max = 4700, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_2, info), is);
}

TEST(RectangleGuillotineBranchingScheme, InsertionCutOnDefect1)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |   x   |-------|                  | 1000
     * |       |       |                  |
     * |       |       |                  |
     * |-------|   1   |                  | 500
     * |   0   |       |                  |
     * |       |       |                  |
     * |-------|-------|------------------|
     *       1000    2000
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(1000, 1000, -1, 1, false, false);
    instance.add_item(200, 3180, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 500, 995, 10, 10);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = 0, .j2 = -1, .df = -1, .x1 = 1000, .y2 = 500, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = 1, .j2 = -1, .df = 2, .x1 = 2000, .y2 = 1020, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = 1, .df = 1, .x1 = 1000, .y2 = 2005, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = -1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 1005, .x3 = 510, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is);
}

TEST(RectangleGuillotineBranchingScheme, Insertion4CutOnDefect4)
{
    /**
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------|                          | 1000
     * |       |                          |
     * |   0   |                          |
     * |       |                          |
     * |-------|                          | 500
     * |       |                          |
     * |       |                          |
     * |       |                          |
     * |-------x--------------------------|
     *       1000
     *
     */

    Info info = Info()
        //.set_log2stderr(true)
        ;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 990, 0, 20, 20);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 1010, .y2 = 20, .x3 = 1010, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(root, info), is);
}

TEST(RectangleGuillotineBranchingScheme, InsertionXMaxDefect)
{
    /**
     * This configuration is not valid since the 2-cut between items 0 and 1
     * intersects the defect. When inserting item 1, x1_max needs to be set to
     * 2000.
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |-------------------|              | 1530
     * |                   |              |
     * |        2          |              |
     * |-----------|-------|              | 1010
     * |           |                      |
     * |    1      |                      |
     * |-------|---|  x                   | 500
     * |       |                          |
     * |   0   |                          |
     * |-------|--------------------------|
     *       1000    2000
     *          1500     2500
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(500, 1000, -1, 1, false, true);
    instance.add_item(510, 1500, -1, 1, false, false);
    instance.add_item(520, 2500, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 2000, 495, 10, 10);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = 0, .j2 = -1, .df = -1, .x1 = 1000, .y2 = 500, .x3 = 1000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is {
        //{.j1 = -1, .j2 = 1, .df = 2, .x1 = 2500, .y2 = 1015, .x3 = 2500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        //{.j1 = 1, .j2 = -1, .df = 2, .x1 = 1510, .y2 = 1500, .x3 = 1510, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = 2, .x1 = 2010, .y2 = 520, .x3 = 2010, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
        {.j1 = 1, .j2 = -1, .df = 1, .x1 = 1500, .y2 = 1010, .x3 = 1500, .x1_max = 2000, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = 1, .j2 = -1, .df = 1, .x1 = 1000, .y2 = 2000, .x3 = 510, .x1_max = 2000, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is);
}

TEST(RectangleGuillotineBranchingScheme, InsertionYMaxDefect)
{
    /**
     * This configuration is not valid since the 3-cut between items 0 and 1
     * intersects the defect. When inserting item 0, y2_max needs to be set to
     * 900.
     *
     * |----------------------------------|
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |                                  |
     * |              |-----|             | 1000
     * |       x      |     |             | 900
     * |              |     |             |
     * |-------|      |  2  |             | 500
     * |   0   |------|     |             | 400
     * |       |  1   |     |             |
     * |-------|------|-----|-------------|
     *       1000    2010  3030
     *
     */

    Info info;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(1000, 500, -1, 1, false, true);
    instance.add_item(1010, 400, -1, 1, false, false);
    instance.add_item(1020, 1000, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 995, 900, 10, 10);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 1020, .y2 = 500, .x3 = 1000, .x1_max = 3500, .y2_max = 900, .z1 = 1, .z2 = 0},
        {.j1 = 0, .j2 = -1, .df = -1, .x1 = 500, .y2 = 1000, .x3 = 500, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0},
        {.j1 = -1, .j2 = -1, .df = -1, .x1 = 1005, .y2 = 910, .x3 = 1005, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
    };
    EXPECT_EQ(branching_scheme.children(root, info), is);
}

TEST(RectangleGuillotineBranchingScheme, InsertionDefect)
{
    /**
     *
     * |------------------------|-------------------------| 3210
     * |                        |                         |
     * |           2            |                         |
     * |                        |                         |
     * |------------------------|                         | 2210
     * |                        |                         |
     * |           1            |           3             |
     * |                        |                         |
     * |------------------------|                         | 1210
     * |                     x  |                         | 1200
     * |-------------------|    |                         | 1000
     * |                   |    |                         |
     * |         0         |    |                         |
     * |                   |    |                         |
     * |-------------------|----|-------------------------|
     *                   2000  3000                     6000
     *                     2500
     */

    Info info = Info()
        //.set_log2stderr(true)
        ;

    Instance instance(Objective::BinPackingWithLeftovers);
    instance.add_item(2000, 1000, -1, 1, false, true);
    instance.add_item(3000, 1000, -1, 1, false, false);
    instance.add_item(3000, 1000, -1, 1, false, false);
    instance.add_item(3000, 3210, -1, 1, false, false);
    instance.add_bin(6000, 3210);
    instance.add_defect(0, 2490, 1200, 10, 10);

    BranchingScheme::Parameters p;
    p.set_roadef2018();
    BranchingScheme branching_scheme(instance, p);
    auto root = branching_scheme.root();

    BranchingScheme::Insertion i0 = {.j1 = 0, .j2 = -1, .df = -1, .x1 = 2000, .y2 = 1000, .x3 = 2000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 0};
    std::vector<BranchingScheme::Insertion> is0 = branching_scheme.children(root, info);
    EXPECT_NE(std::find(is0.begin(), is0.end(), i0), is0.end());
    auto node_1 = branching_scheme.child(root, i0);

    std::vector<BranchingScheme::Insertion> is {
        {.j1 = -1, .j2 = -1, .df = 2, .x1 = 2500, .y2 = 1210, .x3 = 2500, .x1_max = 3500, .y2_max = 3210, .z1 = 1, .z2 = 1},
        {.j1 = -1, .j2 = 1, .df = 1, .x1 = 3000, .y2 = 2210, .x3 = 3000, .x1_max = 3500, .y2_max = 3210, .z1 = 0, .z2 = 1},
        {.j1 = 1, .j2 = -1, .df = 0, .x1 = 5000, .y2 = 1000, .x3 = 5000, .x1_max = 5500, .y2_max = 3210, .z1 = 0, .z2 = 0},
    };
    EXPECT_EQ(branching_scheme.children(node_1, info), is);
}

