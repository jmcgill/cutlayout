#include "packingsolver/rectangleguillotine/branching_scheme.hpp"

#include <string>
#include <fstream>
#include <iomanip>
#include <locale>

using namespace packingsolver;
using namespace packingsolver::rectangleguillotine;

/****************************** BranchingScheme *******************************/

BranchingScheme::BranchingScheme(const Instance& instance, const Parameters& parameters):
    instance_(instance), parameters_(parameters)
{
    if (parameters_.cut_type_1 == CutType1::TwoStagedGuillotine) {
        if (parameters_.first_stage_orientation == CutOrientation::Horinzontal) {
            parameters_.first_stage_orientation = CutOrientation::Vertical;
        } else if (parameters_.first_stage_orientation == CutOrientation::Vertical) {
            parameters_.first_stage_orientation = CutOrientation::Horinzontal;
        }
    }

    // Compute no_oriented_items_;
    if (no_item_rotation()) {
        no_oriented_items_ = false;
    } else {
        no_oriented_items_ = true;
        for (ItemTypeId j = 0; j < instance.item_type_number(); ++j) {
            if (instance.item(j).oriented) {
                no_oriented_items_ = false;
                break;
            }
        }
    }

    // Update stack_pred_
    stack_pred_ = std::vector<StackId>(instance.stack_number(), -1);
    for (StackId s = 0; s < instance.stack_number(); ++s) {
        for (StackId s0 = s - 1; s0 >= 0; --s0) {
            if (equals(s, s0)) {
                stack_pred_[s] = s0;
                break;
            }
        }
    }
}

bool BranchingScheme::oriented(ItemTypeId j) const
{
    if (instance().item(j).oriented)
        return true;
    return no_item_rotation();
}

bool BranchingScheme::equals(StackId s1, StackId s2)
{
    if (instance().stack_size(s1) != instance().stack_size(s2))
        return false;
    for (ItemPos j = 0; j < instance().stack_size(s1); ++j) {
        const Item& j1 = instance().item(s1, j);
        const Item& j2 = instance().item(s2, j);
        if (j1.oriented && j2.oriented
                && j1.rect.w == j2.rect.w
                && j1.rect.h == j2.rect.h
                && j1.profit == j2.profit
                && j1.copies == j2.copies)
            continue;
        if (!j1.oriented && !j2.oriented
                && j1.profit == j2.profit
                && ((j1.rect.w == j2.rect.w && j1.rect.h == j2.rect.h)
                    || (j1.rect.w == j2.rect.h && j1.rect.h == j2.rect.w)))
            continue;
        return false;
    }
    return true;
}

void BranchingScheme::Parameters::set_predefined(std::string str)
{
    if (str.length() != 4) {
        std::cerr << "\033[31m" << "ERROR, predefined branching scheme parameter \"" << str << "\" should contain four characters." << "\033[0m" << std::endl;
        if (str.length() < 4)
            return;
    }
    switch (str[0]) {
    case '3': {
        cut_type_1 = rectangleguillotine::CutType1::ThreeStagedGuillotine;
        break;
    } case '2': {
        cut_type_1 = rectangleguillotine::CutType1::TwoStagedGuillotine;
        break;
    } default: {
        std::cerr << "\033[31m" << "ERROR, predefined branching scheme parameter 1st character \"" << str[0] << "\" invalid." << "\033[0m" << std::endl;
    }
    }
    switch (str[1]) {
    case 'R': {
        cut_type_2 = rectangleguillotine::CutType2::Roadef2018;
        break;
    } case 'N': {
        cut_type_2 = rectangleguillotine::CutType2::NonExact;
        break;
    } case 'E': {
        cut_type_2 = rectangleguillotine::CutType2::Exact;
        break;
    } case 'H': {
        cut_type_2 = rectangleguillotine::CutType2::Homogenous;
        break;
    } default: {
        std::cerr << "\033[31m" << "ERROR, predefined branching scheme parameter 2nd character \"" << str[1] << "\" invalid." << "\033[0m" << std::endl;
    }
    }
    switch (str[2]) {
    case 'V': {
        first_stage_orientation = rectangleguillotine::CutOrientation::Vertical;
        break;
    } case 'H': {
        first_stage_orientation = rectangleguillotine::CutOrientation::Horinzontal;
        break;
    } case 'A': {
        first_stage_orientation = rectangleguillotine::CutOrientation::Any;
        break;
    } default: {
        std::cerr << "\033[31m" << "ERROR, predefined branching scheme parameter 3rd character \"" << str[2] << "\" invalid." << "\033[0m" << std::endl;
    }
    }
    switch (str[3]) {
    case 'R': {
        no_item_rotation = false;
        break;
    } case 'O': {
        no_item_rotation = true;
        break;
    } default: {
        std::cerr << "\033[31m" << "ERROR, predefined branching scheme parameter 4th character \"" << str[3] << "\" invalid." << "\033[0m" << std::endl;
    }
    }
}

std::function<bool(const std::shared_ptr<const BranchingScheme::Node>&, const std::shared_ptr<const BranchingScheme::Node>&)> BranchingScheme::compare(GuideId guide_id)
{
    switch(guide_id) {
    case 0: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->area() == 0)
                return node_2->area() != 0;
            if (node_2->area() == 0)
                return false;
            if (node_1->waste_percentage() != node_2->waste_percentage())
                return node_1->waste_percentage() < node_2->waste_percentage();
            for (StackId s = 0; s < node_1->instance().stack_number(); ++s)
                if (node_1->pos_stack(s) != node_2->pos_stack(s))
                    return node_1->pos_stack(s) < node_2->pos_stack(s);
            return false;
        };
    } case 1: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->area() == 0)
                return node_2->area() != 0;
            if (node_2->area() == 0)
                return false;
            if (node_1->item_number() == 0)
                return node_2->item_number() != 0;
            if (node_2->item_number() == 0)
                return true;
            if (node_1->waste_percentage() / node_1->mean_item_area()
                    != node_2->waste_percentage() / node_2->mean_item_area())
                return node_1->waste_percentage() / node_1->mean_item_area()
                    < node_2->waste_percentage() / node_2->mean_item_area();
            for (StackId s = 0; s < node_1->instance().stack_number(); ++s)
                if (node_1->pos_stack(s) != node_2->pos_stack(s))
                    return node_1->pos_stack(s) < node_2->pos_stack(s);
            return false;
        };
    } case 2: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->area() == 0)
                return node_2->area() != 0;
            if (node_2->area() == 0)
                return false;
            if (node_1->item_number() == 0)
                return node_2->item_number() != 0;
            if (node_2->item_number() == 0)
                return true;
            if ((0.1 + node_1->waste_percentage()) / node_1->mean_item_area()
                    != (0.1 + node_2->waste_percentage()) / node_2->mean_item_area())
                return (0.1 + node_1->waste_percentage()) / node_1->mean_item_area()
                    < (0.1 + node_2->waste_percentage()) / node_2->mean_item_area();
            for (StackId s = 0; s < node_1->instance().stack_number(); ++s)
                if (node_1->pos_stack(s) != node_2->pos_stack(s))
                    return node_1->pos_stack(s) < node_2->pos_stack(s);
            return false;
        };
    } case 3: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->area() == 0)
                return node_2->area() != 0;
            if (node_2->area() == 0)
                return false;
            if (node_1->item_number() == 0)
                return node_2->item_number() != 0;
            if (node_2->item_number() == 0)
                return true;
            if ((0.1 + node_1->waste_percentage()) / node_1->mean_squared_item_area()
                    != (0.1 + node_2->waste_percentage()) / node_2->mean_squared_item_area())
                return (0.1 + node_1->waste_percentage()) / node_1->mean_squared_item_area()
                    < (0.1 + node_2->waste_percentage()) / node_2->mean_squared_item_area();
            for (StackId s = 0; s < node_1->instance().stack_number(); ++s)
                if (node_1->pos_stack(s) != node_2->pos_stack(s))
                    return node_1->pos_stack(s) < node_2->pos_stack(s);
            return false;
        };
    } case 4: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->profit() == 0)
                return node_2->profit() != 0;
            if (node_2->profit() == 0)
                return true;
            if ((double)node_1->area() / node_1->profit() != (double)node_2->area() / node_2->profit())
                return (double)node_1->area() / node_1->profit() < (double)node_2->area() / node_2->profit();
            for (StackId s = 0; s < node_1->instance().stack_number(); ++s)
                if (node_1->pos_stack(s) != node_2->pos_stack(s))
                    return node_1->pos_stack(s) < node_2->pos_stack(s);
            return false;
        };
    } case 5: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->profit() == 0)
                return node_2->profit() != 0;
            if (node_2->profit() == 0)
                return true;
            if (node_1->item_number() == 0)
                return node_2->item_number() != 0;
            if (node_2->item_number() == 0)
                return true;
            if ((double)node_1->area() / node_1->profit() / node_1->mean_item_area()
                    != (double)node_2->area() / node_2->profit() / node_2->mean_item_area())
                return (double)node_1->area() / node_1->profit() / node_1->mean_item_area()
                    < (double)node_2->area() / node_2->profit() / node_2->mean_item_area();
            for (StackId s = 0; s < node_1->instance().stack_number(); ++s)
                if (node_1->pos_stack(s) != node_2->pos_stack(s))
                    return node_1->pos_stack(s) < node_2->pos_stack(s);
            return false;
        };
    } case 6: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            return node_1->waste() < node_2->waste();
        };
    } case 7: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            return node_1->ubkp() < node_2->ubkp();
        };
    } case 8: {
        return [](const std::shared_ptr<const BranchingScheme::Node>& node_1, const std::shared_ptr<const BranchingScheme::Node>& node_2)
        {
            if (node_1->ubkp() != node_2->ubkp())
                return node_1->ubkp() < node_2->ubkp();
            return node_1->waste() < node_2->waste();
        };
    }
    }
    return 0;
}

std::vector<BranchingScheme::Insertion> BranchingScheme::children(
        const std::shared_ptr<const BranchingScheme::Node>& father,
        Info& info) const
{
    return father->children(info);
}

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const BranchingScheme::Parameters& parameters)
{
    os
        << "cut_type_1 " << parameters.cut_type_1
        << " cut_type_2 " << parameters.cut_type_2
        << " first_stage_orientation " << parameters.first_stage_orientation
        << " min1cut " << parameters.min1cut
        << " max1cut " << parameters.max1cut
        << " min2cut " << parameters.min2cut
        << " max2cut " << parameters.max2cut
        << " min_waste " << parameters.min_waste
        << " one2cut " << parameters.one2cut
        << " no_item_rotation " << parameters.no_item_rotation
        << " cut_through_defects " << parameters.cut_through_defects
        ;
    return os;
}

bool BranchingScheme::dominates(const Front& f1, const Front& f2) const
{
    if (f1.i < f2.i) return true;
    if (f1.i > f2.i) return false;
    if (f1.o != f2.o) return false;
    if (f2.y2_curr != instance().bin(f1.i).height(f1.o) && f1.x1_prev > f2.x1_prev) return false;
    if (f1.x1_curr > f2.x1_curr) return false;
    if        (f2.y2_prev <  f1.y2_prev) { if (f1.x1_curr > f2.x3_curr) return false;
    } else if (f2.y2_prev <  f1.y2_curr) { if (f1.x3_curr > f2.x3_curr) return false;
    } else  /* f2.y2_prev <= h */        { if (f1.x1_prev > f2.x3_curr) return false; }
    if        (f2.y2_curr <  f1.y2_prev) { if (f1.x1_curr > f2.x1_prev) return false;
    } else if (f2.y2_curr <  f1.y2_curr) { if (f1.x3_curr > f2.x1_prev) return false;
    } else  /* f2.y2_curr <= h */        { /* if (f1.x1_prev > f2.x1_prev) return false */; }
    return true;
}

bool BranchingScheme::dominates(const std::shared_ptr<const Node>& node_1, const std::shared_ptr<const Node>& node_2) const
{
    if (node_2->last_insertion_defect())
        return false;
    if (node_1->pos_stack() != node_2->pos_stack())
        return false;
    return dominates(node_1->front(), node_2->front());
}

/******************************** SolutionNode ********************************/

bool BranchingScheme::SolutionNode::operator==(const BranchingScheme::SolutionNode& node) const
{
    return ((f == node.f) && (p == node.p));
}

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const BranchingScheme::SolutionNode& node)
{
    os << "f " << node.f << " p " << node.p;
    return os;
}

/********************************** NodeItem **********************************/

bool BranchingScheme::NodeItem::operator==(const BranchingScheme::NodeItem& node_item) const
{
    return ((j == node_item.j) && (node == node_item.node));
}

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const BranchingScheme::NodeItem& node_item)
{
    os << "j " << node_item.j << " node " << node_item.node;
    return os;
}

/********************************* Insertion **********************************/

bool BranchingScheme::Insertion::operator==(const Insertion& insertion) const
{
    return ((j1 == insertion.j1)
            && (j2 == insertion.j2)
            && (df == insertion.df)
            && (x1 == insertion.x1)
            && (y2 == insertion.y2)
            && (x3 == insertion.x3)
            && (x1_max == insertion.x1_max)
            && (y2_max == insertion.y2_max)
            && (z1 == insertion.z1)
            && (z2 == insertion.z2)
            );
}

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const BranchingScheme::Insertion& insertion)
{
    os << "j1 " << insertion.j1
        << " j2 " << insertion.j2
        << " df " << insertion.df
        << " x1 " << insertion.x1
        << " y2 " << insertion.y2
        << " x3 " << insertion.x3
        << " x1_max " << insertion.x1_max
        << " y2_max " << insertion.y2_max
        << " z1 " << insertion.z1
        << " z2 " << insertion.z2
        ;
    return os;
}

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const std::vector<BranchingScheme::Insertion>& insertions)
{
    std::copy(insertions.begin(), insertions.end(), std::ostream_iterator<BranchingScheme::Insertion>(os, "\n"));
    return os;
}

/*********************************** Front ************************************/

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const BranchingScheme::Front& front)
{
    os << "i " << front.i
        << " o " << front.o
        << " x1_prev " << front.x1_prev
        << " x3_curr " << front.x3_curr
        << " x1_curr " << front.x1_curr
        << " y2_prev " << front.y2_prev
        << " y2_curr " << front.y2_curr
        ;
    return os;
}

/************************************ Node ************************************/

BranchingScheme::Node::Node(const BranchingScheme& branching_scheme):
    branching_scheme_(branching_scheme),
    pos_stack_(instance().stack_number(), 0)
{ }

BranchingScheme::Node::Node(const BranchingScheme::Node& node):
    branching_scheme_(node.branching_scheme_),
    father_(node.father_),
    insertion_(node.insertion_),
    pos_stack_(node.pos_stack_),
    bin_number_(node.bin_number_),
    first_stage_orientation_(node.first_stage_orientation_),
    item_number_(node.item_number_),
    item_area_(node.item_area_),
    squared_item_area_(node.squared_item_area_),
    current_area_(node.current_area_),
    waste_(node.waste_),
    profit_(node.profit_),
    x1_prev_(node.x1_prev_),
    y2_prev_(node.y2_prev_),
    subplate2curr_items_above_defect_(node.subplate2curr_items_above_defect_)
{ }

BranchingScheme::Node& BranchingScheme::Node::operator=(const BranchingScheme::Node& node)
{
    if (this != &node) {
        father_                            = node.father_;
        insertion_                         = node.insertion_;
        pos_stack_                         = node.pos_stack_;
        bin_number_                        = node.bin_number_;
        first_stage_orientation_           = node.first_stage_orientation_;
        item_number_                       = node.item_number_;
        item_area_                         = node.item_area_;
        squared_item_area_                 = node.squared_item_area_;
        current_area_                      = node.current_area_;
        waste_                             = node.waste_;
        profit_                            = node.profit_;
        x1_prev_                           = node.x1_prev_;
        y2_prev_                           = node.y2_prev_;
        subplate2curr_items_above_defect_  = node.subplate2curr_items_above_defect_;
    }
    return *this;
}

std::ostream& packingsolver::rectangleguillotine::operator<<(
        std::ostream &os, const BranchingScheme::Node& node)
{
    os << "item_number " << node.item_number()
        << " bin_number " << node.bin_number()
        << std::endl;
    os << "item_area " << node.item_area()
        << " current_area " << node.area()
        << std::endl;
    os << "waste " << node.waste()
        << " waste_percentage " << node.waste_percentage()
        << " profit " << node.profit()
        << std::endl;
    os << "x1_curr " << node.x1_curr()
        << " x1_prev " << node.x1_prev()
        << " y2_curr " << node.y2_curr()
        << " y2_prev " << node.y2_prev()
        << " x3_curr " << node.x3_curr()
        << std::endl;
    os << "insertion " << node.insertion() << std::endl;

    os << "pos_stack" << std::flush;
    for (StackId s = 0; s < node.instance().stack_number(); ++s)
        os << " " << node.pos_stack(s);
    os << std::endl;

    return os;
}

bool BranchingScheme::Node::bound(const Solution& sol_best) const
{
    switch (sol_best.instance().objective()) {
    case Objective::Default: {
        if (!sol_best.full()) {
            return (ubkp() <= sol_best.profit());
        } else {
            if (ubkp() != sol_best.profit())
                return (ubkp() <= sol_best.profit());
            return waste() >= sol_best.waste();
        }
    } case Objective::BinPacking: {
        if (!sol_best.full())
            return false;
        BinPos i_pos = -1;
        Area a = instance().item_area() + waste();
        while (a > 0) {
            i_pos++;
            a -= instance().bin(i_pos).rect.area();
        }
        return (i_pos + 1 >= sol_best.bin_number());
    } case Objective::BinPackingWithLeftovers: {
        if (!sol_best.full())
            return false;
        return waste() >= sol_best.waste();
    } case Objective::Knapsack: {
        return ubkp() <= sol_best.profit();
    } case Objective::StripPackingWidth: {
        if (!sol_best.full())
            return false;
        return std::max(width(), (waste() + instance().item_area() - 1) / instance().bin(0).height(CutOrientation::Vertical) + 1) >= sol_best.width();
    } case Objective::StripPackingHeight: {
        if (!sol_best.full())
            return false;
        return std::max(height(), (waste() + instance().item_area() - 1) / instance().bin(0).height(CutOrientation::Horinzontal) + 1) >= sol_best.height();
    } default: {
        assert(false);
        std::cerr << "\033[31m" << "ERROR, branching scheme rectangle::BranchingScheme does not implement objective \"" << sol_best.instance().objective() << "\"" << "\033[0m" << std::endl;
        return false;
    }
    }
}

Profit BranchingScheme::Node::ubkp() const
{
    Area remaining_item_area     = instance().item_area() - item_area();
    Area remaining_packabla_area = instance().packable_area() - area();
    if (remaining_packabla_area >= remaining_item_area) {
        return instance().item_profit();
    } else {
        ItemTypeId j = instance().max_efficiency_item();
        return profit_ + remaining_packabla_area
            * instance().item(j).profit / instance().item(j).rect.area();
    }
}

BranchingScheme::Node::Node(const std::shared_ptr<const BranchingScheme::Node>& father, Insertion insertion):
    Node(*father)
{
    assert(insertion.df <= -1 || insertion.x1 >= x1_curr());

    father_ = father;

    // Update bin_number_
    if (insertion.df < 0) {
        bin_number_++;
        first_stage_orientation_ = last_bin_orientation(insertion.df);
    }

    BinPos i = bin_number() - 1;
    CutOrientation o = first_stage_orientation_;
    Length h = instance().bin(i).height(o);
    Length w = instance().bin(i).width(o);

    Length w_j = insertion.x3 - x3_prev(insertion.df);
    //bool rotate_j1 = (insertion.j1 == -1)? false: (instance().width(instance().item(insertion.j1), true, o) == w_j);
    bool rotate_j2 = (insertion.j2 == -1)? false: (instance().width(instance().item(insertion.j2), true, o) == w_j);
    //Length h_j1 = (insertion.j1 == -1)? -1: instance().height(instance().item(insertion.j1), rotate_j1, o);
    //Length h_j2 = (insertion.j2 == -1)? -1: instance().height(instance().item(insertion.j2), rotate_j2, o);

    // Update subplate2curr_items_above_defect_
    if (insertion.df != 2)
        subplate2curr_items_above_defect_.clear();
    if (insertion.j1 == -1 && insertion.j2 != -1) {
        JRX jrx;
        jrx.j = insertion.j2;
        jrx.rotate = rotate_j2;
        jrx.x = x3_prev(insertion.df);
        subplate2curr_items_above_defect_.push_back(jrx);
    }

    // Update previous and current cuts
    switch (insertion.df) {
    case -1: case -2: {
        x1_prev_ = 0;
        y2_prev_ = 0;
        break;
    } case 0: {
        x1_prev_ = insertion_.x1;
        y2_prev_ = 0;
        break;
    } case 1: {
        y2_prev_ = insertion_.y2;
        break;
    } case 2: {
        break;
    } default: {
        assert(false);
        break;
    }
    }
    insertion_ = insertion;

    // Update items_, pos_stack_, items_area_, squared_item_area_ and profit_
    if (insertion.j1 != -1) {
        const Item& item = instance().item(insertion.j1);
        pos_stack_[item.stack]++;
        item_number_       += 1;
        item_area_         += item.rect.area();
        squared_item_area_ += item.rect.area() * item.rect.area();
        profit_            += item.profit;
    }
    if (insertion.j2 != -1) {
        const Item& item = instance().item(insertion.j2);
        pos_stack_[item.stack]++;
        item_number_       += 1;
        item_area_         += item.rect.area();
        squared_item_area_ += item.rect.area() * item.rect.area();
        profit_            += item.profit;
    }

    // Update current_area_ and waste_
    current_area_ = instance().previous_bin_area(i);
    if (full()) {
        current_area_ += (branching_scheme().cut_type_1() == CutType1::ThreeStagedGuillotine)?
            x1_curr() * h: y2_curr() * w;
    } else {
        current_area_ += x1_prev() * h
            + (x1_curr() - x1_prev()) * y2_prev()
            + (x3_curr() - x1_prev()) * (y2_curr() - y2_prev());
    }
    waste_ = current_area_ - item_area_;
    assert(waste_ >= 0);
}

/********************************** children **********************************/

std::vector<BranchingScheme::Insertion> BranchingScheme::Node::children(Info& info) const
{
    LOG_FOLD_START(info, "children" << std::endl);

    std::vector<Insertion> insertions;

    if (full())
        return insertions;

    // Compute df_min
    Depth df_min = -2;
    if (bin_number() == instance().bin_number()) {
        df_min = 0;
    } else if (branching_scheme().first_stage_orientation() == CutOrientation::Vertical) {
        df_min = -1;
    } else if (branching_scheme().first_stage_orientation() == CutOrientation::Any
            && instance().bin(bin_number()).defects.size() == 0                           // Next bin has no defects,
            && instance().bin(bin_number()).rect.w == instance().bin(bin_number()).rect.h // is a square,
            && branching_scheme().no_oriented_items()) {                                  // and items can be rotated
        df_min = -1;
    }

    // Compute df_max
    Depth df_max = 2;
    if (father_ == nullptr)
        df_max = -1;

    LOG(info, "df_max " << df_max << " df_min " << df_min << std::endl);
    for (Depth df = df_max; df >= df_min; --df) {
        LOG(info, "df " << df << std::endl);
        if (df == -1 && branching_scheme().first_stage_orientation() == CutOrientation::Horinzontal)
            continue;

        // Simple dominance rule
        bool stop = false;
        for (const Insertion& insertion: insertions) {
            if (insertion.j1 == -1 && insertion.j2 == -1)
                continue;
            if        (df == 1 && insertion.x1 == x1_curr() && insertion.y2 == y2_curr()) {
                stop = true;
                break;
            } else if (df == 0 && insertion.x1 == x1_curr()) {
                stop = true;
                break;
            } else if (df < 0 && insertion.df >= 0) {
                stop = true;
                break;
            }
        }
        if (stop)
            break;

        CutOrientation o = last_bin_orientation(df);
        Length x = x3_prev(df);
        Length y = y2_prev(df);

        // Try adding an item
        for (StackId s = 0; s < instance().stack_number(); ++s) {
            if (pos_stack_[s] == instance().stack_size(s))
                continue;
            StackId sp = branching_scheme().stack_pred(s);
            if (sp != -1 && pos_stack_[sp] <= pos_stack_[s])
                continue;

            ItemTypeId j = instance().item(s, pos_stack_[s]).id;

            if (!branching_scheme().oriented(j)) {
                bool b = instance().item(j).rect.w > instance().item(j).rect.h;
                insertion_1_item(insertions, j, !b, df, info);
                insertion_1_item(insertions, j,  b, df, info);
                //insertion_1_item(insertions, j, false, df, df_min, info);
                //insertion_1_item(insertions, j, true, df, df_min, info);
            } else {
                insertion_1_item(insertions, j, false, df, info);
            }

            // Try adding it with a second item
            if (branching_scheme().cut_type_2() == CutType2::Roadef2018) {
                LOG(info, "try adding with a second item" << std::endl);
                for (StackId s2 = s; s2 < instance().stack_number(); ++s2) {
                    ItemTypeId j2 = -1;
                    if (s2 == s) {
                        if (pos_stack_[s2] + 1 == instance().stack_size(s2))
                            continue;
                        StackId sp2 = branching_scheme().stack_pred(s2);
                        if (sp2 != -1 && pos_stack_[sp2] <= pos_stack_[s2])
                            continue;
                        j2 = instance().item(s2, pos_stack_[s2] + 1).id;
                    } else {
                        if (pos_stack_[s2] == instance().stack_size(s2))
                            continue;
                        StackId sp2 = branching_scheme().stack_pred(s2);
                        if (                    (sp2 == s && pos_stack_[sp2] + 1 <= pos_stack_[s2])
                                || (sp2 != -1 && sp2 != s && pos_stack_[sp2]     <= pos_stack_[s2]))
                            continue;
                        j2 = instance().item(s2, pos_stack_[s2]).id;
                    }

                    // To break symmetries, the item with the smallest id is always
                    // placed at the bottom.
                    // This doesn't create precedency issues since all the
                    // predecessors of an item have smaller id.
                    if (j2 < j) {
                        ItemTypeId tmp = j;
                        j = j2;
                        j2 = tmp;
                    }
                    const Item& item1 = instance().item(j);
                    const Item& item2 = instance().item(j2);
                    if (instance().width(item1, false, o) == instance().width(item2, false, o))
                        insertion_2_items(insertions, j, false, j2, false, df, info);

                    if (!branching_scheme().oriented(j2))
                        if (instance().width(item1, false, o) == instance().width(item2, true, o))
                            insertion_2_items(insertions, j, false, j2, true,  df, info);
                    if (!branching_scheme().oriented(j))
                        if (instance().width(item1, true, o) == instance().width(item2, false, o))
                            insertion_2_items(insertions, j, true,  j2, false, df, info);
                    if (!branching_scheme().oriented(j2) && !branching_scheme().oriented(j))
                        if (instance().width(item1, true, o) == instance().width(item2, true, o))
                            insertion_2_items(insertions, j, true,  j2, true,  df, info);
                }
            }
        }

        if (father_ == nullptr || insertion_.j1 != -1 || insertion_.j2 != -1) {
            const std::vector<Defect>& defects = instance().bin(last_bin(df)).defects;
            for (const Defect& defect: defects)
                if (instance().left(defect, o) >= x && instance().bottom(defect, o) >= y)
                    insertion_defect(insertions, defect, df, info);
        }
    }

    DBG(
        LOG_FOLD_START(info, "insertions" << std::endl);
        for (const Insertion& insertion: insertions)
            LOG(info, insertion << std::endl);
        LOG_FOLD_END(info, "");
    );

    LOG_FOLD_END(info, "children");
    return insertions;
}

Area BranchingScheme::Node::waste(const Insertion& insertion) const
{
    BinPos i = last_bin(insertion.df);
    CutOrientation o = last_bin_orientation(insertion.df);
    Length h = instance().bin(i).height(o);
    Front f = front(insertion);
    ItemPos n = item_number();
    Area item_area = item_area_;
    if (insertion.j1 != -1) {
        n++;
        item_area += instance().item(insertion.j1).rect.area();
    }
    if (insertion.j2 != -1) {
        n++;
        item_area += instance().item(insertion.j2).rect.area();
    }
    Area current_area = (n == instance().item_number())?
        instance().previous_bin_area(i)
        + (f.x1_curr * h):
        instance().previous_bin_area(i)
        + f.x1_prev * h
        + (f.x1_curr - f.x1_prev) * f.y2_prev
        + (f.x3_curr - f.x1_prev) * (f.y2_curr - f.y2_prev);
    return current_area - item_area;
}

BinPos BranchingScheme::Node::last_bin(Depth df) const
{
    if (df <= -1) {
        return (bin_number() == 0)? 0: bin_number();
    } else {
        return bin_number() - 1;
    }
}

CutOrientation BranchingScheme::Node::last_bin_orientation(Depth df) const
{
    switch (df) {
    case -1: {
        return CutOrientation::Vertical;
    } case -2: {
        return CutOrientation::Horinzontal;
    } default: {
        return first_stage_orientation_;
    }
    }
}

BranchingScheme::Front BranchingScheme::Node::front(const Insertion& insertion) const
{
    switch (insertion.df) {
    case -1: case -2: {
        return {.i = last_bin(insertion.df), .o = last_bin_orientation(insertion.df),
            .x1_prev = 0, .x3_curr = insertion.x3, .x1_curr = insertion.x1,
            .y2_prev = 0, .y2_curr = insertion.y2};
    } case 0: {
        return {.i = last_bin(insertion.df), .o = last_bin_orientation(insertion.df),
            .x1_prev = x1_curr(), .x3_curr = insertion.x3, .x1_curr = insertion.x1,
            .y2_prev = 0, .y2_curr = insertion.y2};
    } case 1: {
        return {.i = last_bin(insertion.df), .o = last_bin_orientation(insertion.df),
            .x1_prev = x1_prev(), .x3_curr = insertion.x3, .x1_curr = insertion.x1,
            .y2_prev = y2_curr(), .y2_curr = insertion.y2};
    } case 2: {
        return {.i = last_bin(insertion.df), .o = last_bin_orientation(insertion.df),
            .x1_prev = x1_prev(), .x3_curr = insertion.x3, .x1_curr = insertion.x1,
            .y2_prev = y2_prev(), .y2_curr = insertion.y2};
    } default: {
        assert(false);
        return {.i = -1, .o = CutOrientation::Vertical,
            .x1_prev = -1, .x3_curr = -1, .x1_curr = -1,
            .y2_prev = -1, .y2_curr = -1};
    }
    }
}

Length BranchingScheme::Node::x1_prev(Depth df) const
{
    switch (df) {
    case -1: case -2: {
        return 0;
    } case 0: {
        return x1_curr();
    } case 1: {
        return x1_prev();
    } case 2: {
        return x1_prev();
    } default: {
        assert(false);
        return -1;
    }
    }
}

Length BranchingScheme::Node::x3_prev(Depth df) const
{
    switch (df) {
    case -1: case -2: {
        return 0;
    } case 0: {
        return x1_curr();
    } case 1: {
        return x1_prev();
    } case 2: {
        return x3_curr();
    } default: {
        assert(false);
        return -1;
    }
    }
}

Length BranchingScheme::Node::x1_max(Depth df) const
{
    switch (df) {
    case -1: case -2: {
        BinPos i = last_bin(df);
        CutOrientation o = last_bin_orientation(df);
        Length x = instance().bin(i).width(o);
        if (branching_scheme().max1cut() != -1)
            if (x > x1_prev(df) + branching_scheme().max1cut())
                x = x1_prev(df) + branching_scheme().max1cut();
        return x;
    } case 0: {
        BinPos i = last_bin(df);
        CutOrientation o = last_bin_orientation(df);
        Length x = instance().bin(i).width(o);
        if (branching_scheme().max1cut() != -1)
            if (x > x1_prev(df) + branching_scheme().max1cut())
                x = x1_prev(df) + branching_scheme().max1cut();
        return x;
    } case 1: {
        BinPos i = last_bin(df);
        CutOrientation po = last_bin_orientation(df);
        Length x = x1_max();
        if (!branching_scheme().cut_through_defects())
            for (const Defect& k: instance().bin(i).defects)
                if (instance().bottom(k, po) < y2_curr() && instance().top(k, po) > y2_curr())
                    if (instance().left(k, po) > x1_prev())
                        if (x > instance().left(k, po))
                            x = instance().left(k, po);
        return x;
    } case 2: {
        return x1_max();
    } default: {
        return -1;
    }
    }
}

Length BranchingScheme::Node::y2_prev(Depth df) const
{
    switch (df) {
    case -1: case -2: {
        return 0;
    } case 0: {
        return 0;
    } case 1: {
        return y2_curr();
    } case 2: {
        return y2_prev();
    } default: {
        assert(false);
        return -1;
    }
    }
}

Length BranchingScheme::Node::y2_max(Depth df, Length x3) const
{
    BinPos i = last_bin(df);
    CutOrientation o = last_bin_orientation(df);
    Length y = (df == 2)? y2_max(): instance().bin(i).height(o);
    if (!branching_scheme().cut_through_defects())
        for (const Defect& k: instance().bin(i).defects)
            if (instance().left(k, o) < x3 && instance().right(k, o) > x3)
                if (instance().bottom(k, o) >= y2_prev(df))
                    if (y > instance().bottom(k, o))
                        y = instance().bottom(k, o);
    return y;
}

void BranchingScheme::Node::insertion_1_item(std::vector<Insertion>& insertions,
        ItemTypeId j, bool rotate, Depth df, Info& info) const
{
    LOG_FOLD_START(info, "insertion_1_item"
            << " j " << j << " rotate " << rotate << " df " << df << std::endl);
    assert(-2 <= df); assert(df <= 3);

    // Check defect intersection
    BinPos i = last_bin(df);
    CutOrientation o = last_bin_orientation(df);
    const rectangleguillotine::Item& item = instance().item(j);
    Length x = x3_prev(df) + instance().width(item, rotate, o);
    Length y = y2_prev(df) + instance().height(item, rotate, o);
    Length w = instance().bin(i).width(o);
    Length h = instance().bin(i).height(o);
    LOG(info, "x3_prev(df) " << x3_prev(df) << " y2_prev(df) " << y2_prev(df)
            << " i " << i
            << " w " << instance().width(item, rotate, o)
            << " h " << instance().height(item, rotate, o)
            << std::endl);
    if (x > w) {
        LOG_FOLD_END(info, "too wide x " << x << " > w " << w);
        return;
    }
    if (y > h) {
        LOG_FOLD_END(info, "too high y " << y << " > h " << h);
        return;
    }

    // Homogenous
    if (df == 2 && branching_scheme().cut_type_2() == CutType2::Homogenous
            && insertion_.j1 != j) {
        LOG_FOLD_END(info, "homogenous father_->insertion_.j1 " << father_->insertion_.j1 << " j " << j);
        return;
    }

    Insertion insertion {
        .j1 = j, .j2 = -1, .df = df,
        .x1 = x, .y2 = y, .x3 = x,
        .x1_max = x1_max(df), .y2_max = y2_max(df, x), .z1 = 0, .z2 = 0};
    LOG(info, insertion << std::endl);

    // Check defect intersection
    DefectId k = instance().item_intersects_defect(x3_prev(df), y2_prev(df), item, rotate, i, o);
    if (k >= 0) {
        if (branching_scheme().cut_type_2() == CutType2::Roadef2018
                || branching_scheme().cut_type_2() == CutType2::NonExact) {
            // Place the item on top of its third-level sub-plate
            insertion.j1 = -1;
            insertion.j2 = j;
        } else {
            LOG_FOLD_END(info, "intersects defect");
            return;
        }
    }

    // Update insertion.z2 with respect to cut_type_2()
    if (branching_scheme().cut_type_2() == CutType2::Exact
            || branching_scheme().cut_type_2() == CutType2::Homogenous)
        insertion.z2 = 2;

    update(insertions, insertion, info);
}

void BranchingScheme::Node::insertion_2_items(std::vector<Insertion>& insertions,
        ItemTypeId j1, bool rotate1, ItemTypeId j2, bool rotate2, Depth df, Info& info) const
{
    LOG_FOLD_START(info, "insertion_2_items"
            << " j1 " << j1 << " rotate1 " << rotate1
            << " j2 " << j2 << " rotate2 " << rotate2
            << " df " << df << std::endl);
    assert(-2 <= df); assert(df <= 3);

    // Check defect intersection
    BinPos i = last_bin(df);
    CutOrientation o = last_bin_orientation(df);
    const Item& item1 = instance().item(j1);
    const Item& item2 = instance().item(j2);
    Length w = instance().bin(i).width(o);
    Length h = instance().bin(i).height(o);
    Length h_j1 = instance().height(item1, rotate1, o);
    Length x = x3_prev(df) + instance().width(item1, rotate1, o);
    Length y = y2_prev(df) + h_j1
                           + instance().height(item2, rotate2, o);
    if (x > w || y > h) {
        LOG_FOLD_END(info, "too wide/high");
        return;
    }
    if (instance().item_intersects_defect(x3_prev(df), y2_prev(df), item1, rotate1, i, o) >= 0
            || instance().item_intersects_defect(x3_prev(df), y2_prev(df) + h_j1, item2, rotate2, i, o) >= 0) {
        LOG_FOLD_END(info, "intersects defect");
        return;
    }

    Insertion insertion {
        .j1 = j1, .j2 = j2, .df = df,
        .x1 = x, .y2 = y, .x3 = x,
        .x1_max = x1_max(df), .y2_max = y2_max(df, x), .z1 = 0, .z2 = 2};
    LOG(info, insertion << std::endl);

    update(insertions, insertion, info);
}

void BranchingScheme::Node::insertion_defect(std::vector<Insertion>& insertions,
            const Defect& k, Depth df, Info& info) const
{
    LOG_FOLD_START(info, "insertion_defect"
            << " k " << k.id << " df " << df << std::endl);
    assert(-2 <= df);
    assert(df <= 3);

    // Check defect intersection
    BinPos i = last_bin(df);
    CutOrientation o = last_bin_orientation(df);
    Length w = instance().bin(i).width(o);
    Length h = instance().bin(i).height(o);
    Length min_waste = branching_scheme().min_waste();
    Length x = std::max(instance().right(k, o), x3_prev(df) + min_waste);
    Length y = std::max(instance().top(k, o),   y2_prev(df) + min_waste);
    if (x > w || y > h) {
        LOG_FOLD_END(info, "too wide/high");
        return;
    }

    Insertion insertion {
        .j1 = -1, .j2 = -1, .df = df,
        .x1 = x, .y2 = y, .x3 = x,
        .x1_max = x1_max(df), .y2_max = y2_max(df, x), .z1 = 1, .z2 = 1};
    LOG(info, insertion << std::endl);

    update(insertions, insertion, info);
}

void BranchingScheme::Node::update(
        std::vector<Insertion>& insertions,
        Insertion& insertion, Info& info) const
{
    Length min_waste = branching_scheme().min_waste();
    BinPos i = last_bin(insertion.df);
    CutOrientation o = last_bin_orientation(insertion.df);
    Length w = instance().bin(i).width(o);
    Length h = instance().bin(i).height(o);

    // Update insertion.x1 and insertion.z1 with respect to min1cut()
    if ((insertion.j1 != -1 || insertion.j2 != -1)
            && insertion.x1 - x1_prev(insertion.df) < branching_scheme().min1cut()) {
        if (insertion.z1 == 0) {
            insertion.x1 = std::max(
                    insertion.x1 + branching_scheme().min_waste(),
                    x1_prev(insertion.df) + branching_scheme().min1cut());
            insertion.z1 = 1;
        } else { // insertion.z1 = 1
            insertion.x1 = x1_prev(insertion.df) + branching_scheme().min1cut();
        }
    }

    // Update insertion.y2 and insertion.z2 with respect to min2cut()
    if ((insertion.j1 != -1 || insertion.j2 != -1)
            && insertion.y2 - y2_prev(insertion.df) < branching_scheme().min2cut()) {
        if (insertion.z2 == 0) {
            insertion.y2 = std::max(
                    insertion.y2 + branching_scheme().min_waste(),
                    y2_prev(insertion.df) + branching_scheme().min2cut());
            insertion.z2 = 1;
        } else if (insertion.z2 == 1) {
            insertion.y2 = y2_prev(insertion.df) + branching_scheme().min2cut();
        } else { // insertion.z2 == 2
            return;
        }
    }

    // Update insertion.y2 and insertion.z2 with respect to one2cut()
    if (branching_scheme().one2cut() && insertion.df == 1
            && y2_prev(insertion.df) != 0 && insertion.y2 != h) {
        if (insertion.z2 == 0) {
            if (insertion.y2 + branching_scheme().min_waste() > h)
                return;
            insertion.y2 = h;
        } else if (insertion.z2 == 1) {
            insertion.y2 = h;
        } else { // insertion.z2 == 2
            return;
        }
    }

    // Update insertion.x1 if 2-staged
    if (branching_scheme().cut_type_1() == CutType1::TwoStagedGuillotine && insertion.x1 != w) {
        if (insertion.z1 == 0) {
            if (insertion.x1 + branching_scheme().min_waste() > w)
                return;
            insertion.x1 = w;
        } else { // insertion.z1 == 1
            insertion.x1 = w;
        }
    }

    // Update insertion.x1 and insertion.z1 with respect to x1_curr() and z1().
    if (insertion.df >= 1) {
        LOG(info, "i.x3 " << insertion.x3 << " x1_curr() " << x1_curr() << std::endl);
        if (insertion.z1 == 0) {
            if (insertion.x1 + min_waste <= x1_curr()) {
                insertion.x1 = x1_curr();
                insertion.z1 = z1();
            } else if (insertion.x1 < x1_curr()) { // x - min_waste < insertion.x1 < x
                if (z1() == 0) {
                    insertion.x1 = x1_curr() + min_waste;
                    insertion.z1 = 1;
                } else {
                    insertion.x1 = insertion.x1 + min_waste;
                    insertion.z1 = 1;
                }
            } else if (insertion.x1 == x1_curr()) {
            } else { // x1_curr() < insertion.x1
                if (z1() == 0 && insertion.x1 < x1_curr() + min_waste) {
                    insertion.x1 = insertion.x1 + min_waste;
                    insertion.z1 = 1;
                }
            }
        } else { // insertion.z1 == 1
            if (insertion.x1 <= x1_curr()) {
                insertion.x1 = x1_curr();
                insertion.z1 = z1();
            } else { // x1_curr() < insertion.x1
                if (z1() == 0 && x1_curr() + min_waste > insertion.x1)
                    insertion.x1 = x1_curr() + min_waste;
            }
        }
    }

    // Update insertion.y2 and insertion.z2 with respect to y2_curr() and z1().
    if (insertion.df == 2) {
        LOG(info, "i.y2 " << insertion.y2 << " y2_curr() " << y2_curr() << std::endl);
        if (insertion.z2 == 0) {
            if (insertion.y2 + min_waste <= y2_curr()) {
                insertion.y2 = y2_curr();
                insertion.z2 = z2();
            } else if (insertion.y2 < y2_curr()) { // y_curr() - min_waste < insertion.y4 < y_curr()
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                } else if (z2() == 0) {
                    insertion.y2 = y2_curr() + min_waste;
                    insertion.z2 = 1;
                } else { // z2() == 1
                    insertion.y2 = insertion.y2 + min_waste;
                    insertion.z2 = 1;
                }
            } else if (insertion.y2 == y2_curr()) {
                if (z2() == 2)
                    insertion.z2 = 2;
            } else if (y2_curr() < insertion.y2 && insertion.y2 < y2_curr() + min_waste) {
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                } else if (z2() == 0) {
                    insertion.y2 = insertion.y2 + min_waste;
                    insertion.z2 = 1;
                } else { // z2() == 1
                }
            } else { // y2_curr() + min_waste <= insertion.y2
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                }
            }
        } else if (insertion.z2 == 1) {
            if (insertion.y2 <= y2_curr()) {
                insertion.y2 = y2_curr();
                insertion.z2 = z2();
            } else if (y2_curr() < insertion.y2 && insertion.y2 < y2_curr() + min_waste) {
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                } else if (z2() == 0) {
                    insertion.y2 = y2_curr() + min_waste;
                } else { // z2() == 1
                }
            } else {
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                }
            }
        } else { // insertion.z2 == 2
            if (insertion.y2 < y2_curr()) {
                LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                return;
            } else if (insertion.y2 == y2_curr()) {
            } else if (y2_curr() < insertion.y2 && insertion.y2 < y2_curr() + min_waste) {
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                } else if (z2() == 0) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                } else { // z2() == 1
                }
            } else { // y2_curr() + min_waste <= insertion.y2
                if (z2() == 2) {
                    LOG_FOLD_END(info, "too high, y2_curr() " << y2_curr() << " insertion.y2 " << insertion.y2 << " insertion.z2 " << insertion.z2);
                    return;
                }
            }
        }
    }

    // Update insertion.x1 and insertion.z1 with respect to defect intersections.
    for (;;) {
        DefectId k = instance().x_intersects_defect(insertion.x1, i, o);
        if (k == -1)
            break;
        const Defect& defect = instance().defect(k);
        insertion.x1 = (insertion.z1 == 0)?
            std::max(instance().right(defect, o), insertion.x1 + min_waste):
            instance().right(defect, o);
        insertion.z1 = 1;
    }

    // Increase width if too close from border
    if (insertion.x1 < w && insertion.x1 + min_waste > w) {
        if (insertion.z1 == 1) {
            insertion.x1 = w;
            insertion.z1 = 0;
        } else { // insertion.z1 == 0
            LOG(info, insertion << std::endl);
            LOG_FOLD_END(info, "too long w - min_waste < insertion.x1 < w and insertion.z1 == 0");
            return;
        }
    }

    // Check max width
    if (insertion.x1 > insertion.x1_max) {
        LOG(info, insertion << std::endl);
        LOG_FOLD_END(info, "too long insertion.x1 > insertion.x1_max");
        return;
    }
    LOG(info, "width OK" << std::endl);

    // Update insertion.y2 and insertion.z2 with respect to defect intersections.
    bool y2_fixed = (insertion.z2 == 2 || (insertion.df == 2 && z2() == 2));

    for (;;) {
        bool found = false;

        // Increase y2 if it intersects a defect.
        DefectId k = instance().y_intersects_defect(x1_prev(insertion.df), insertion.x1, insertion.y2, i, o);
        if (k != -1) {
            const Defect& defect = instance().defect(k);
            if (y2_fixed) {
                LOG_FOLD_END(info, "y2_fixed");
                return;
            }
            insertion.y2 = (insertion.z2 == 0)?
                std::max(instance().top(defect, o), insertion.y2 + min_waste):
                instance().top(defect, o);
            insertion.z2 = 1;
            found = true;
        }

        // Increase y2 if an item on top of its third-level sub-plate
        // intersects a defect.
        if (insertion.df == 2) {
            for (auto jrx: subplate2curr_items_above_defect_) {
                const Item& item = instance().item(jrx.j);
                Length h_j2 = instance().height(item, jrx.rotate, o);
                Length l = jrx.x;
                DefectId k = instance().item_intersects_defect(l, insertion.y2 - h_j2, item, jrx.rotate, i, o);
                if (k >= 0) {
                    const Defect& defect = instance().defect(k);
                    if (y2_fixed) {
                        LOG_FOLD_END(info, "y2_fixed");
                        return;
                    }
                    insertion.y2 = (insertion.z2 == 0)?
                        std::max(instance().top(defect, o) + h_j2,
                                insertion.y2 + min_waste):
                        instance().top(defect, o) + h_j2;
                    insertion.z2 = 1;
                    found = true;
                }
            }
        }
        if (insertion.j1 == -1 && insertion.j2 != -1) {
            const Item& item = instance().item(insertion.j2);
            Length w_j = insertion.x3 - x3_prev(insertion.df);
            bool rotate_j2 = (instance().width(item, true, o) == w_j);
            Length h_j2 = instance().height(item, rotate_j2, o);
            Length l = x3_prev(insertion.df);
            DefectId k = instance().item_intersects_defect(l, insertion.y2 - h_j2, item, rotate_j2, i, o);
            if (k >= 0) {
                const Defect& defect = instance().defect(k);
                if (y2_fixed) {
                    LOG_FOLD_END(info, "y2_fixed");
                    return;
                }
                insertion.y2 = (insertion.z2 == 0)?
                    std::max(instance().top(defect, o) + h_j2, insertion.y2 + min_waste):
                    instance().top(defect, o) + h_j2;
                insertion.z2 = 1;
                found = true;
            }
        }
        LOG(info, "found " << found << std::endl);
        if (!found)
            break;
    }

    // Now check bin's height
    if (insertion.y2 < h && insertion.y2 + min_waste > h) {
        if (insertion.z2 == 1) {
            insertion.y2 = h;
            insertion.z2 = 0;

            if (insertion.df == 2) {
                for (auto jrx: subplate2curr_items_above_defect_) {
                    const Item& item = instance().item(jrx.j);
                    Length l = jrx.x;
                    Length h_j2 = instance().height(item, jrx.rotate, o);
                    DefectId k = instance().item_intersects_defect(l, insertion.y2 - h_j2, item, jrx.rotate, i, o);
                    if (k >= 0) {
                        LOG_FOLD_END(info, "too high");
                        return;
                    }
                }
            }

            if (insertion.j1 == -1 && insertion.j2 != -1) {
                const Item& item = instance().item(insertion.j2);
                Length w_j = insertion.x3 - x3_prev(insertion.df);
                bool rotate_j2 = (instance().width(item, true, o) == w_j);
                Length h_j2 = instance().height(item, rotate_j2, o);
                Length l = x3_prev(insertion.df);
                DefectId k = instance().item_intersects_defect(l, insertion.y2 - h_j2, item, rotate_j2, i, o);
                if (k >= 0) {
                    LOG_FOLD_END(info, "too high");
                    return;
                }
            }

        } else { // insertion.z2 == 0 or insertion.z2 == 2
            LOG(info, insertion << std::endl);
            LOG_FOLD_END(info, "too high");
            return;
        }
    }

    // Check max height
    if (insertion.y2 > insertion.y2_max) {
        LOG(info, insertion << std::endl);
        LOG_FOLD_END(info, "too high");
        return;
    }
    LOG(info, "height OK" << std::endl);

    // Check dominance
    for (auto it = insertions.begin(); it != insertions.end();) {
        bool b = true;
        LOG(info, "f_i  " << front(insertion) << std::endl);
        LOG(info, "f_it " << front(*it) << std::endl);
        if (insertion.j1 == -1 && insertion.j2 == -1
                && it->j1 == -1 && it->j2 == -1) {
            if (insertion.df != -1 && insertion.x1 == it->x1 && insertion.y2 == it->y2 && insertion.x3 == it->x3) {
                LOG_FOLD_END(info, "dominated by " << *it);
                return;
            }
        }
        if ((it->j1 != -1 || it->j2 != -1)
                && (insertion.j1 == -1 || insertion.j1 == it->j1 || insertion.j1 == it->j2)
                && (insertion.j2 == -1 || insertion.j2 == it->j2 || insertion.j2 == it->j2)) {
            if (branching_scheme().dominates(front(*it), front(insertion))) {
                LOG_FOLD_END(info, "dominated by " << *it);
                return;
            }
        }
        if ((insertion.j1 != -1 || insertion.j2 != -1)
                && (it->j1 == insertion.j1 || it->j1 == insertion.j2)
                && (it->j2 == insertion.j2 || it->j2 == insertion.j1)) {
            if (branching_scheme().dominates(front(insertion), front(*it))) {
                LOG(info, "dominates " << *it << std::endl);
                if (std::next(it) != insertions.end()) {
                    *it = insertions.back();
                    insertions.pop_back();
                    b = false;
                } else {
                    insertions.pop_back();
                    break;
                }
            }
        }
        if (b)
            ++it;
    }

    insertions.push_back(insertion);
    LOG_FOLD_END(info, "ok");
}

/*********************************** export ***********************************/

bool empty(const std::vector<Solution::Node>& v, SolutionNodeId f_v)
{
    if (v[f_v].children.size() == 0)
        return (v[f_v].j < 0);
    for (SolutionNodeId c: v[f_v].children)
        if (!empty(v, c))
            return false;
    return true;
}

SolutionNodeId sort(std::vector<Solution::Node>& res,
        const std::vector<Solution::Node>& v,
        SolutionNodeId gf_res, SolutionNodeId f_v)
{
    SolutionNodeId id_res = res.size();
    res.push_back(v[f_v]);
    res[id_res].id = id_res;
    res[id_res].f  = gf_res;
    res[id_res].children = {};
    if (empty(v, f_v)) {
        res[id_res].j = -1;
    } else {
        for (SolutionNodeId c_v: v[f_v].children) {
            SolutionNodeId c_id_res = sort(res, v, id_res, c_v);
            res[id_res].children.push_back(c_id_res);
        }
    }
    return id_res;
}

Solution BranchingScheme::Node::convert(const Solution&) const
{
    // Get nodes, items and bins
    std::vector<SolutionNode> nodes;
    std::vector<NodeItem> items;
    std::vector<CutOrientation> first_stage_orientations;
    std::vector<const BranchingScheme::Node*> descendents {this};
    while (descendents.back()->father() != nullptr)
        descendents.push_back(descendents.back()->father().get());
    descendents.pop_back();

    std::vector<SolutionNodeId> nodes_curr(4, -1);
    for (auto it = descendents.rbegin(); it != descendents.rend(); ++it) {
        const Node* n = *it;
        auto insertion = n->insertion();

        if (insertion.df < 0)
            first_stage_orientations.push_back(last_bin_orientation(insertion.df));

        SolutionNodeId id = (insertion.df >= 0)? nodes.size() + 2 - insertion.df: nodes.size() + 2;
        if (insertion.j1 != -1)
            items.push_back({.j = insertion.j1, .node = id});
        if (insertion.j2 != -1)
            items.push_back({.j = insertion.j2, .node = id});

        // Add new solution nodes
        SolutionNodeId f = (insertion.df <= 0)? -first_stage_orientations.size(): nodes_curr[insertion.df];
        Depth          d = (insertion.df < 0)? 0: insertion.df;
        SolutionNodeId c = nodes.size() - 1;
        do {
            c++;
            nodes.push_back({.f = f});
            f = c;
            d++;
            nodes_curr[d] = nodes.size() - 1;
        } while (d != 3);

        nodes[nodes_curr[1]].p = insertion.x1;
        nodes[nodes_curr[2]].p = insertion.y2;
        nodes[nodes_curr[3]].p = insertion.x3;
    }

    std::vector<SolutionNodeId> bins;
    std::vector<Solution::Node> res(nodes.size());
    for (BinPos i = 0; i < bin_number(); ++i) {
        CutOrientation o = first_stage_orientations[i];
        Length w = instance().bin(i).width(o);
        Length h = instance().bin(i).height(o);
        SolutionNodeId id = res.size();
        bins.push_back(id);
        res.push_back({.id = id, .f = -1, .d = 0, .i = i,
                .l = 0, .r = w, .b = 0, .t = h, .children = {}, .j = -1});
    }
    for (SolutionNodeId id = 0; id < (SolutionNodeId)nodes.size(); ++id) {
        SolutionNodeId f = (nodes[id].f >= 0)? nodes[id].f: bins[(-nodes[id].f)-1];
        Depth d = res[f].d + 1;
        res[id].id = id;
        res[id].f  = f;
        res[id].d  = d;
        res[id].i  = res[f].i;
        if (d == 1 || d == 3) {
            res[id].r  = nodes[id].p;
            res[id].l  = (res[f].children.size() == 0)?
                res[f].l:
                res[res[f].children.back()].r;
            res[id].b  = res[f].b;
            res[id].t  = res[f].t;
        } else { // d == 2
            res[id].t  = nodes[id].p;
            res[id].b  = (res[f].children.size() == 0)?
                res[f].b:
                res[res[f].children.back()].t;
            res[id].l  = res[f].l;
            res[id].r  = res[f].r;
        }
        res[f].children.push_back(id);
    }

    for (SolutionNodeId f = 0; f < (SolutionNodeId)(nodes.size()+bins.size()); ++f) {
        SolutionNodeId nb = res[f].children.size();
        if (nb == 0)
            continue;
        SolutionNodeId c_last = res[f].children.back();
        if ((res[f].d == 0 || res[f].d == 2) && res[f].r != res[c_last].r) {
            if (res[f].r - res[c_last].r < branching_scheme().min_waste()) {
                res[c_last].r = res[f].r;
            } else {
                SolutionNodeId id = res.size();
                res.push_back({.id = id, .f = f,
                        .d = static_cast<Depth>(res[f].d+1), .i = res[f].i,
                        .l = res[c_last].r, .r = res[f].r,
                        .b = res[f].b,      .t = res[f].t,
                        .children = {}, .j = -1});
                res[f].children.push_back(id);
            }
        } else if ((res[f].d == 1 || res[f].d == 3) && res[f].t != res[c_last].t) {
            if (res[f].t - res[c_last].t < branching_scheme().min_waste()) {
                res[c_last].t = res[f].t;
            } else {
                SolutionNodeId id = res.size();
                res.push_back({.id = id, .f = f,
                        .d = static_cast<Depth>(res[f].d+1), .i = res[f].i,
                        .l = res[f].l,      .r = res[f].r,
                        .b = res[c_last].t, .t = res[f].t,
                        .children = {}, .j = -1});
                res[f].children.push_back(id);
            }
        }
    }

    for (SolutionNodeId id = 0; id < (SolutionNodeId)res.size(); ++id)
        res[id].j  = -1;

    for (Counter j_pos = 0; j_pos < item_number(); ++j_pos) {
        ItemTypeId     j  = items[j_pos].j;
        SolutionNodeId id = items[j_pos].node;
        Length         wj = instance().item(j).rect.w;
        Length         hj = instance().item(j).rect.h;
        if (res[id].children.size() > 0) { // Second item of the third-level sub-plate
            res[res[id].children[1]].j = j; // Alone in its third-level sub-plate
        } else if ((res[id].t - res[id].b == hj && res[id].r - res[id].l == wj)
                || (res[id].t - res[id].b == wj && res[id].r - res[id].l == hj)) {
            res[id].j = j;
            continue;
        } else {
            Length t = (res[id].r - res[id].l == wj)? hj: wj;
            BinPos i = res[id].i;
            CutOrientation o = first_stage_orientations[i];
            DefectId k = instance().rect_intersects_defect(
                    res[id].l, res[id].r, res[id].b, res[id].b + t, i, o);
            if (k == -1) { // First item of the third-level sub-plate
                SolutionNodeId c1 = res.size();
                res.push_back({.id = c1, .f = id,
                        .d = static_cast<Depth>(res[id].d + 1), .i = res[id].i,
                        .l = res[id].l, .r = res[id].r,
                        .b = res[id].b, .t = res[id].b + t,
                        .children = {}, .j = j});
                res[id].children.push_back(c1);
                SolutionNodeId c2 = res.size();
                res.push_back({.id = c2, .f = id,
                        .d = static_cast<Depth>(res[id].d+1), .i = res[id].i,
                        .l = res[id].l, .r = res[id].r,
                        .b = res[id].b + t, .t = res[id].t,
                        .children = {}, .j = -1});
                res[id].children.push_back(c2);
            } else {
                SolutionNodeId c1 = res.size();
                res.push_back({.id = c1, .f = id,
                        .d = static_cast<Depth>(res[id].d+1), .i = res[id].i,
                        .l = res[id].l, .r = res[id].r,
                        .b = res[id].b, .t = res[id].t - t,
                        .children = {}, .j = -1});
                res[id].children.push_back(c1);
                SolutionNodeId c2 = res.size();
                res.push_back({.id = c2, .f = id,
                        .d = static_cast<Depth>(res[id].d+1), .i = res[id].i,
                        .l = res[id].l, .r = res[id].r,
                        .b = res[id].t - t, .t = res[id].t,
                        .children = {}, .j = j});
                res[id].children.push_back(c2);
            }
        }
    }

    // Set j to -2 for intermediate nodes and to -3 for residual
    for (Solution::Node& n: res)
        if (n.j == -1 && n.children.size() != 0)
            n.j = -2;

    if (branching_scheme().cut_type_1() == CutType1::TwoStagedGuillotine) {
        for (Solution::Node& n: res) {
            if (n.d == 0) {
                assert(n.children.size() == 1);
                n.children = res[n.children[0]].children;
            }
            if (n.d == 2)
                n.f = res[n.f].f;
            if (n.d >= 2)
                n.d--;
        }
    }

    // Sort nodes
    std::vector<Solution::Node> res2;
    for (SolutionNodeId c: bins)
        sort(res2, res, -1, c);

    if (res2.rbegin()->j == -1 && res2.rbegin()->d == 1)
        res2.rbegin()->j = -3;

    for (Solution::Node& n: res2) {
        CutOrientation o = first_stage_orientations[n.i];
        if (o == CutOrientation::Horinzontal) {
            Length tmp_1 = n.l;
            Length tmp_2 = n.r;
            n.l = n.b;
            n.r = n.t;
            n.b = tmp_1;
            n.t = tmp_2;
        }
    }

    if (!check(res2))
        return Solution(instance(), {});
    return Solution(instance(), res2);
}

bool BranchingScheme::Node::check(const std::vector<Solution::Node>& nodes) const
{
    std::vector<ItemPos> items(instance().item_number(), 0);

    for (const Solution::Node& node: nodes) {
        Length w = instance().bin(node.i).rect.w;
        Length h = instance().bin(node.i).rect.h;

        // TODO Check tree consistency

        // Check defect intersection
        if (!branching_scheme().cut_through_defects()) {
            for (Defect defect: instance().bin(node.i).defects) {
                Length l = defect.pos.x;
                Length r = defect.pos.x + defect.rect.w;
                Length b = defect.pos.y;
                Length t = defect.pos.y + defect.rect.h;
                if (
                           (node.l > l && node.l < r && node.b < t && node.t > b)
                        || (node.r > l && node.r < r && node.b < t && node.t > b)
                        || (node.b > b && node.b < t && node.l < r && node.r > l)
                        || (node.t > b && node.t < t && node.l < r && node.r > l)) {
                    std::cerr << "\033[31m" << "ERROR, "
                        "Node " << node << " cut intersects defect " << defect
                        << "\033[0m" << std::endl;
                    assert(false);
                    return false;
                }
            }
        }

        if (node.j >= 0) {
            // TODO Check item order

            // Check item type copy number
            items[node.j]++;
            if (items[node.j] > instance().item(node.j).copies) {
                std::cerr << "\033[31m" << "ERROR, "
                    "item " << node.j << " produced more that one time"
                    << "\033[0m" << std::endl;
                return false;
            }

            // Check if item j contains a defect
            for (Defect defect: instance().bin(node.i).defects) {
                Coord c1;
                c1.x = node.l;
                c1.y = node.b;
                Rectangle r1;
                r1.w = node.r - node.l;
                r1.h = node.t - node.b;
                if (rect_intersection(c1, r1, defect.pos, defect.rect)) {
                    std::cerr << "\033[31m" << "ERROR, "
                        "Node " << node << " intersects defect " << defect
                        << "\033[0m" << std::endl;
                    assert(false);
                    return false;
                }
            }

        } else if (node.j == -1) {
            // Check minimum waste constraint
            if (
                       node.r - node.l < branching_scheme().min_waste()
                    || node.t - node.b < branching_scheme().min_waste()) {
                std::cerr << "\033[31m" << "ERROR, "
                    "Node " << node << " violates min_waste constraint"
                    << "\033[0m" << std::endl;
                return false;
            }
        }

        if (node.d == 0) {
             if (node.l != 0 || node.r != w || node.b != 0 || node.t != h) {
                 std::cerr << "\033[31m" << "ERROR, "
                     "Node " << node << " incorrect dimensions"
                     << "\033[0m" << std::endl;
                 return false;
             }
         } else if (node.d == 1 && node.j != -1 && node.j != -3) {
             if (node.r - node.l < branching_scheme().min1cut()) {
                 std::cerr << "\033[31m" << "ERROR, "
                     "Node " << node << " violates min1cut constraint"
                     << "\033[0m" << std::endl;
                 return false;
             }
             if (branching_scheme().max1cut() >= 0
                     && node.r - node.l > branching_scheme().max1cut()) {
                 std::cerr << "\033[31m" << "ERROR, "
                     "Node " << node << " violates max1cut constraint"
                     << "\033[0m" << std::endl;
                 return false;
             }
         } else if (node.d == 2 && node.j != -1) {
             if (node.t - node.b < branching_scheme().min2cut()) {
                 std::cerr << "\033[31m" << "ERROR, "
                     "Node " << node << " violates min2cut constraint"
                     << "\033[0m" << std::endl;
                 return false;
             }
             if (branching_scheme().max2cut() >= 0
                     && node.t - node.b > branching_scheme().max2cut()) {
                 std::cerr << "\033[31m" << "ERROR, "
                     "Node " << node << " violates max2cut constraint"
                     << "\033[0m" << std::endl;
                 return false;
             }
         }
    }

    return true;
}

