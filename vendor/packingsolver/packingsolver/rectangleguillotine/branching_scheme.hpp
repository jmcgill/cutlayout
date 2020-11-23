#pragma once

#include "packingsolver/rectangleguillotine/solution.hpp"

#include <sstream>

namespace packingsolver
{
namespace rectangleguillotine
{

/****************************** BranchingScheme *******************************/

class BranchingScheme
{

public:

    /** Subclasses */
    struct Insertion;
    class Node;
    struct SolutionNode;
    struct NodeItem;
    struct Front;

    struct Parameters
    {
        CutType1 cut_type_1 = CutType1::ThreeStagedGuillotine;
        CutType2 cut_type_2 = CutType2::NonExact;
        CutOrientation first_stage_orientation = CutOrientation::Vertical;
        Length min1cut = 0;
        Length max1cut = -1;
        Length min2cut = 0;
        Length max2cut = -1;
        Length min_waste = 1;
        bool one2cut = false;
        bool no_item_rotation = false;
        bool cut_through_defects = false;

        void set_predefined(std::string str);
        void set_roadef2018()
        {
            cut_type_1 = rectangleguillotine::CutType1::ThreeStagedGuillotine;
            cut_type_2 = rectangleguillotine::CutType2::Roadef2018;
            first_stage_orientation = rectangleguillotine::CutOrientation::Vertical;
            min1cut = 100;
            max1cut = 3500;
            min2cut = 100;
            min_waste = 20;
            no_item_rotation = false;
            cut_through_defects = false;
        }
    };

    /** Constructor */
    BranchingScheme(const Instance& instance, const Parameters& parameters);

    /** Destructor */
    virtual ~BranchingScheme() { }

    /**
     * Getters
     */

    const Instance& instance() const { return instance_; }
    CutType1 cut_type_1() const { return parameters_.cut_type_1; }
    CutType2 cut_type_2() const { return parameters_.cut_type_2; }
    CutOrientation first_stage_orientation() const { return parameters_.first_stage_orientation; }
    Length min1cut() const { return parameters_.min1cut; }
    Length max1cut() const { return parameters_.max1cut; }
    Length min2cut() const { return parameters_.min2cut; }
    Length max2cut() const { return parameters_.max2cut; }
    Length min_waste() const { return parameters_.min_waste; }
    bool one2cut() const { return parameters_.one2cut; }
    bool no_item_rotation() const { return parameters_.no_item_rotation; }
    bool cut_through_defects() const { return parameters_.cut_through_defects; }

    bool oriented(ItemTypeId j) const;
    bool no_oriented_items() const { return no_oriented_items_; }
    StackId stack_pred(StackId s) const { return stack_pred_[s]; }

    /**
     * Branching scheme methods
     */

    std::shared_ptr<const Node> root() const { return std::make_shared<Node>(*this); }
    std::vector<Insertion> children(const std::shared_ptr<const Node>& father, Info& info) const;
    std::shared_ptr<const Node> child(const std::shared_ptr<const Node>& father, const Insertion& insertion) const { return std::make_shared<Node>(father, insertion); }

    std::function<bool(const std::shared_ptr<const BranchingScheme::Node>&, const std::shared_ptr<const BranchingScheme::Node>&)> compare(GuideId guide_id);

    bool dominates(const Front& front_1, const Front& front_2) const;
    bool dominates(const std::shared_ptr<const Node>& node_1, const std::shared_ptr<const Node>& node_2) const;

private:

    const Instance& instance_;
    Parameters parameters_;

    bool no_oriented_items_;

    /**
     * If stacks s1 < s2 < s3 contain identical items in the same order, then
     * stack_pred[s1] = -1, stack_pred[s2] = s1 and stack_pred[s3] = s2.
     */
    std::vector<StackId> stack_pred_;

    /**
     * Return true iff s1 and s2 contains identical objects in the same order.
     */
    bool equals(StackId s1, StackId s2);

};

std::ostream& operator<<(std::ostream &os, const BranchingScheme::Parameters& parameters);

/******************************** SolutionNode ********************************/

struct BranchingScheme::SolutionNode
{
    SolutionNodeId f; // father for 2-cuts and 3-cuts, -bin-1 for 1-cuts
    Length p;         // x      for 1-cuts and 3-cuts, y        for 2-cuts

    bool operator==(const BranchingScheme::SolutionNode& node) const;
    bool operator!=(const BranchingScheme::SolutionNode& node) const { return !(*this == node); }
};

std::ostream& operator<<(std::ostream &os, const BranchingScheme::SolutionNode& node);

/********************************** NodeItem **********************************/

struct BranchingScheme::NodeItem
{
    ItemTypeId j;
    SolutionNodeId node;
    // Note that two items may belong to the same node if cut_type_2() is set to
    // "roadef2018". On the contrary, a node may not contain any item if bins
    // contain defects.

    bool operator==(const BranchingScheme::NodeItem& node_item) const;
    bool operator!=(const BranchingScheme::NodeItem& node_item) const { return !(*this == node_item); }
};

std::ostream& operator<<(std::ostream &os, const BranchingScheme::NodeItem& node);

/********************************* Insertion **********************************/

struct BranchingScheme::Insertion
{
    /** Id of the item at the bottom of the third-level sub-plate, -1 if none. */
    ItemTypeId j1;
    /** Id of the item at the top of the third-level sub-plate, -1 if none. */
    ItemTypeId j2;

    /**
     * Depth of the father in the tree representation of the solution:
     * * 2: same second-level sub-plate
     * * 1: same first-level sub-plate, new second-level sub-plate
     * * 0: same bin, new first-level sub-plate
     * * -1: new bin, first stage veritical
     * * -2: new bin, first stage horizontal
     */
    Depth  df;

    /** Position of the current 1-cut. */
    Length x1;
    /** Position of the current 2-cut. */
    Length y2;
    /** Position of the current 3-cut. */
    Length x3;

    /**
     * x1_max_ is the maximum position of the current 1-cut.
     * It is used when otherwise, a 2-cut of the current 1-level sub-plate
     * would intersect a defect.
     */
    Length x1_max;

    /**
     * y2_max_ is the maximum position of the current 2-cut.
     * It is used when otherwise, a 3-cut of the current 2-level sub-plate
     * would intersect a defect.
     */
    Length y2_max;

    /**
     * z1_
     * * 0: to increase the width of the last 1-cut, it is necessary to add at
     * least the minimum waste.
     * * 1: the width of the last 1-cut can be increased by any value.
     */
    Counter z1 = 0;

    /**
     * z2_
     * * 0: to increase the height of the last 2-cut, it is necessary to add at
     * least the minimum waste.
     * * 1: the height of the last 2-cut can be increased by any value.
     * * 2: the height of the last 2-cut cannot be increased (case where it
     * contains of 4-cut with 2 items).
     */
    Counter z2 = 0;

    bool operator==(const Insertion& insertion) const;
    bool operator!=(const Insertion& insertion) const { return !(*this == insertion); }
};

std::ostream& operator<<(std::ostream &os, const BranchingScheme::Insertion& insertion);
std::ostream& operator<<(std::ostream &os, const std::vector<BranchingScheme::Insertion>& insertions);

/*********************************** Front ************************************/

struct BranchingScheme::Front
{
    BinPos i;
    CutOrientation o;
    Length x1_prev, x3_curr, x1_curr;
    Length y2_prev, y2_curr;
};

std::ostream& operator<<(std::ostream &os, const BranchingScheme::Front& front);

/************************************ JRX *************************************/

struct JRX
{
    ItemTypeId j;
    bool rotate;
    Length x;
};

/************************************ Node ************************************/

class BranchingScheme::Node
{

public:

    /** Standard constructor, branching scheme root node. */
    Node(const BranchingScheme& branching_scheme);

    /** Copy constructor. */
    Node(const BranchingScheme::Node& solution);
    /** Assignment operator. */
    Node& operator=(const Node& solution);

    /** Children. */
    std::vector<Insertion> children(Info& info) const;
    /** Constructor from a father and an insertion. */
    Node(const std::shared_ptr<const BranchingScheme::Node>& father, Insertion insertion);

    /** Desctructor. */
    ~Node() { };

    /** Front. */
    inline Front front() const { return {.i = static_cast<BinPos>(bin_number() - 1), .o = first_stage_orientation_, .x1_prev = x1_prev(), .x3_curr = x3_curr(), .x1_curr = x1_curr(), .y2_prev = y2_prev(), .y2_curr = y2_curr()}; }
    /** Bound. */
    bool bound(const Solution& sol_best) const;
    /** Export */
    Solution convert(const Solution&) const;

    /**
     * Getters
     */

    inline const BranchingScheme& branching_scheme() const { return branching_scheme_; }
    inline const Instance& instance() const { return branching_scheme_.instance(); }
    inline const std::shared_ptr<const Node>& father() const { return father_; }
    inline const Insertion& insertion() const { return insertion_; }

    inline ItemPos item_number()              const { return item_number_; }
    inline bool    full()                     const { return item_number() == instance().item_number(); }
    inline double  item_percentage()          const { return (double)item_number() / instance().item_number(); }
    inline BinPos  bin_number()               const { return bin_number_; }
    inline Area    area()                     const { return current_area_; }
    inline double  mean_area()                const { return (double)area() / item_number(); }
    inline double  mean_item_area()           const { return (double)item_area() / item_number(); }
    inline double  mean_squared_item_area()   const { return (double)squared_item_area() / item_number(); }
    inline double  mean_remaining_item_area() const { return (double)remaining_item_area() / (instance().item_number() - item_number()); }
    inline double  remaining_item_area()      const { return instance().item_area() - item_area(); }
    inline Area    item_area()                const { return item_area_; }
    inline Area    squared_item_area()        const { return squared_item_area_; }
    inline Profit  profit()                   const { return profit_; }
    inline Area    waste()                    const { return waste_; }
    inline double  waste_percentage()         const { return (double)waste() / area(); }
    inline double  waste_ratio()              const { return (double)waste() / item_area(); }
    inline Length  width()                    const { return (branching_scheme().cut_type_1() == CutType1::ThreeStagedGuillotine)? x1_curr(): y2_curr(); }
    inline Length  height()                   const { return (branching_scheme().cut_type_1() == CutType1::ThreeStagedGuillotine)? x1_curr(): y2_curr(); }

    Profit ubkp() const;
    inline CutOrientation last_bin_orientation() const { return first_stage_orientation_; }
    inline bool last_insertion_defect() const { return bin_number_ > 0 && insertion_.j1 == -1 && insertion_.j2 == -1; }

    /** Previous and current cuts. */
    inline Length x1_curr() const { return insertion_.x1; }
    inline Length y2_curr() const { return insertion_.y2; }
    inline Length x3_curr() const { return insertion_.x3; }
    inline Length x1_prev() const { return x1_prev_; }
    inline Length y2_prev() const { return y2_prev_; }

    inline Length x1_max() const { return insertion_.x1_max; }
    inline Length y2_max() const { return insertion_.y2_max; }
    inline Counter    z1() const { return insertion_.z1; }
    inline Counter    z2() const { return insertion_.z2; }

    /** Getters for unit tests. */
    ItemPos pos_stack(StackId s) const { return pos_stack_[s]; }
    std::vector<ItemPos> pos_stack() const { return pos_stack_; }

private:

    /**
     * Attributes
     */

    const BranchingScheme& branching_scheme_;
    std::shared_ptr<const Node> father_ = nullptr;
    Insertion insertion_ = {.j1 = -1, .j2 = -1, .df = -1, .x1 = 0, .y2 = 0, .x3 = 0, .x1_max = -1, .y2_max = -1};

    /**
     * pos_stack_[s] == k iff the solution contains items 0 to k - 1 in the
     * sequence of stack s.
     */
    std::vector<ItemPos> pos_stack_ = {};

    BinPos bin_number_ = 0;
    CutOrientation first_stage_orientation_;

    ItemPos item_number_    = 0;
    Area item_area_         = 0;
    Area squared_item_area_ = 0;
    Area current_area_      = 0;
    Area waste_             = 0;
    Profit profit_          = 0;

    Length x1_prev_ = 0;
    Length y2_prev_ = 0;

    /**
     * Contains the list of items (id, rotate, left cut position) inserted
     * above a defect in the current 2-level sub-plate.
     */
    std::vector<JRX> subplate2curr_items_above_defect_ = {};


    /**
     * Private methods
     */

    /**
     * children
     */

    /** Insertion of one item. */
    void insertion_1_item(std::vector<Insertion>& insertions,
            ItemTypeId j, bool rotate, Depth df, Info& info) const;
    /** Insertion of two items. */
    void insertion_2_items(std::vector<Insertion>& insertions,
            ItemTypeId j1, bool rotate1, ItemTypeId j2, bool rotate2, Depth df, Info& info) const;
    /** Insertion of a defect. */
    void insertion_defect(std::vector<Insertion>& insertions,
            const Defect& k, Depth df, Info& info) const;

    /** Attributes of the new node if an insertion is performed at depth df.  */
    BinPos last_bin(Depth df) const;
    CutOrientation last_bin_orientation(Depth df) const;
    Length x1_prev(Depth df) const;
    Length y2_prev(Depth df) const;
    Length x3_prev(Depth df) const;
    Length x1_max(Depth df) const;
    Length y2_max(Depth df, Length x3) const;
    Front front(const Insertion& insertion) const;
    Area waste(const Insertion& insertion) const;

    /** Update insertion (x1, z1, y2, z2) and add insertion to insertions. */
    void update(std::vector<Insertion>& insertions, Insertion& insertion, Info& info) const;

    bool check(const std::vector<Solution::Node>& nodes) const;

};

std::ostream& operator<<(std::ostream &os, const BranchingScheme::Node& node);

}
}

