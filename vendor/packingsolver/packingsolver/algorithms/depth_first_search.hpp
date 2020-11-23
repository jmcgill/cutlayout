#pragma once

#include "packingsolver/algorithms/common.hpp"

namespace packingsolver
{

template <typename Solution, typename BranchingScheme>
class DepthFirstSearch
{

public:

    DepthFirstSearch(
            Solution& sol_best,
            BranchingScheme& branching_scheme,
            Counter thread_id_,
            GuideId guide_id,
            Info info):
        thread_id_(thread_id_),
        sol_best_(sol_best),
        branching_scheme_(branching_scheme),
        guide_id_(guide_id),
        info_(info) { }

    void run();

private:

    Counter thread_id_;
    Solution& sol_best_;
    BranchingScheme& branching_scheme_;
    GuideId guide_id_ = 0;
    Info info_ = Info();

    void rec(const std::shared_ptr<const typename BranchingScheme::Node>& node_cur);

};

/************************** Template implementation ***************************/

template <typename Solution, typename BranchingScheme>
void DepthFirstSearch<Solution, BranchingScheme>::rec(const std::shared_ptr<const typename BranchingScheme::Node>& node_cur)
{
    typedef typename BranchingScheme::Node Node;
    typedef typename BranchingScheme::Insertion Insertion;

    LOG_FOLD_START(info_, "rec" << std::endl);
    LOG_FOLD(info_, "node_cur" << std::endl << *node_cur);

    // Check time
    if (!info_.check_time()) {
        LOG_FOLD_END(info_, "");
        return;
    }

    // Bound
    if (node_cur->bound(sol_best_)) {
        LOG(info_, " bound ×" << std::endl);
        return;
    }

    std::vector<std::shared_ptr<const Node>> children;
    for (const Insertion& insertion: branching_scheme_.children(node_cur, info_)) {
        LOG(info_, insertion << std::endl);
        auto child = branching_scheme_.child(node_cur, insertion);
        LOG_FOLD(info_, "child" << std::endl << *child);

        // Bound
        if (child->bound(sol_best_)) {
            LOG(info_, " bound ×" << std::endl);
            continue;
        }

        // Update best solution
        if (sol_best_ < *child) {
            std::stringstream ss;
            ss << "A* (thread " << thread_id_ << ")";
            sol_best_.update(child->convert(sol_best_), ss, info_);
        }

        // Add child to the queue
        if (!child->full())
            children.push_back(child);
    }

    auto comp = branching_scheme_.compare(guide_id_);
    sort(children.begin(), children.end(), comp);

    for (const auto& child: children)
        rec(child);

    LOG_FOLD_END(info_, "");
}

template <typename Solution, typename BranchingScheme>
void DepthFirstSearch<Solution, BranchingScheme>::run()
{
    typedef typename BranchingScheme::Node Node;

    std::shared_ptr<const Node> root = branching_scheme_.root();
    rec(root);
}

}

