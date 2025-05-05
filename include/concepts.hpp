#pragma once
#include "cpp_utils/types/concepts.hpp"
#include <concepts>
#include <vector>

/** data_producer concept, requires read and flush methods */
template <typename T>
concept data_producer = requires(T t) {
    { t.read(std::size_t { 1 }) } -> std::same_as<std::vector<unsigned char>>;
    { t.read(reinterpret_cast<unsigned char*>(10), std::size_t { 1 }) } -> std::same_as<int>;
    { t.flush() } -> std::same_as<void>;
};

using namespace cpp_utils::types::concepts;
