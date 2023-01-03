#pragma once

#include <Vector2.hpp>
#include <cstdint>
#include <vector>

template <typename T>
struct Matrix {
    std::vector<T>       v;
    Vector2<std::size_t> size{};

    Matrix(const Vector2<std::size_t>& size_) : v(size_.x * size_.y, T{}), size(size_) {}

    Matrix(std::size_t x_, std::size_t y_) : v(x_ * y_, T{}), size(x_, y_) {}

    T& operator()(std::size_t x_, std::size_t y_) { return v[x_ + y_ * size.x]; }

    T& operator()(const Vector2<std::size_t>& pos) { return v[pos.x + pos.y * size.x]; }

    const T& operator()(std::size_t x_, std::size_t y_) const { return v[x_ + y_ * size.x]; }

    const T& operator()(const Vector2<std::size_t>& pos) const { return v[pos.x + pos.y * size.x]; }
};