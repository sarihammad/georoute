#include "georoute/segment_tree.hpp"

#include <algorithm>
#include <stdexcept>

namespace georoute {

SegmentTree::SegmentTree(std::size_t size)
    : n_(size), tree_(size ? size * 4 : 0, 1.0F), lazy_(size ? size * 4 : 0, 1.0F) {}

void SegmentTree::range_multiply(std::size_t l, std::size_t r, float factor) {
    if (n_ == 0) {
        throw std::runtime_error{"SegmentTree::range_multiply called on empty tree"};
    }
    if (l > r) {
        throw std::invalid_argument{"SegmentTree::range_multiply invalid range"};
    }
    if (r >= n_) {
        throw std::out_of_range{"SegmentTree::range_multiply index out of range"};
    }
    range_multiply_impl(1, 0, n_ - 1, l, r, factor);
}

float SegmentTree::point_query(std::size_t idx) const {
    if (n_ == 0 || idx >= n_) {
        throw std::out_of_range{"SegmentTree::point_query index out of range"};
    }
    return point_query_impl(1, 0, n_ - 1, idx, 1.0F);
}

std::size_t SegmentTree::size() const noexcept {
    return n_;
}

void SegmentTree::range_multiply_impl(std::size_t node,
                                      std::size_t node_l,
                                      std::size_t node_r,
                                      std::size_t ql,
                                      std::size_t qr,
                                      float factor) {
    if (ql <= node_l && node_r <= qr) {
        apply(node, factor, node_l, node_r);
        return;
    }

    push(node, node_l, node_r);

    const auto mid = node_l + (node_r - node_l) / 2;
    const auto left = static_cast<std::size_t>(node * 2);
    const auto right = left + 1;

    if (ql <= mid) {
        range_multiply_impl(left, node_l, mid, ql, std::min(qr, mid), factor);
    }
    if (qr > mid) {
        range_multiply_impl(right, mid + 1, node_r, std::max(ql, mid + 1), qr, factor);
    }

    tree_[node] = tree_[left] * tree_[right];
}

float SegmentTree::point_query_impl(std::size_t node,
                                    std::size_t node_l,
                                    std::size_t node_r,
                                    std::size_t idx,
                                    float accumulated_factor) const {
    accumulated_factor *= lazy_[node];

    if (node_l == node_r) {
        return tree_[node] * accumulated_factor;
    }

    const auto mid = node_l + (node_r - node_l) / 2;
    const auto left = static_cast<std::size_t>(node * 2);
    const auto right = left + 1;

    if (idx <= mid) {
        return point_query_impl(left, node_l, mid, idx, accumulated_factor);
    }

    return point_query_impl(right, mid + 1, node_r, idx, accumulated_factor);
}

void SegmentTree::apply(std::size_t node, float factor, std::size_t node_l, std::size_t node_r) {
    tree_[node] *= factor;
    if (node_l != node_r) {
        lazy_[node] *= factor;
    }
}

void SegmentTree::push(std::size_t node, std::size_t node_l, std::size_t node_r) {
    if (node_l == node_r) {
        return;
    }

    const auto factor = lazy_[node];
    if (factor == 1.0F) {
        return;
    }
    const auto left = static_cast<std::size_t>(node * 2);
    const auto right = left + 1;
    const auto mid = node_l + (node_r - node_l) / 2;
    apply(left, factor, node_l, mid);
    apply(right, factor, mid + 1, node_r);
    lazy_[node] = 1.0F;
}

}  // namespace georoute

