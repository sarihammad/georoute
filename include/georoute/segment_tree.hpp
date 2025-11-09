#pragma once

#include <cstddef>
#include <vector>

#include <cstdint>

namespace georoute {

class SegmentTree {
public:
    explicit SegmentTree(std::size_t size = 0);

    void range_multiply(std::size_t l, std::size_t r, float factor);
    [[nodiscard]] float point_query(std::size_t idx) const;

    [[nodiscard]] std::size_t size() const noexcept;

private:
    void range_multiply_impl(std::size_t node,
                             std::size_t node_l,
                             std::size_t node_r,
                             std::size_t ql,
                             std::size_t qr,
                             float factor);
    [[nodiscard]] float point_query_impl(std::size_t node,
                                         std::size_t node_l,
                                         std::size_t node_r,
                                         std::size_t idx,
                                         float accumulated_factor) const;
    void apply(std::size_t node, float factor, std::size_t node_l, std::size_t node_r);
    void push(std::size_t node, std::size_t node_l, std::size_t node_r);

    std::size_t n_{0};
    std::vector<float> tree_{};
    std::vector<float> lazy_{};
};

}  // namespace georoute

