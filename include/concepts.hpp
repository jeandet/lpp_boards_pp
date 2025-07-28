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

template <typename T>
concept array2d = requires(T t) {
    { t.rows() } -> std::convertible_to<std::size_t>;
    { t.cols() } -> std::convertible_to<std::size_t>;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.data() } -> std::same_as<typename T::value_type*>;
    { t(0, 0) } -> std::same_as<typename T::reference>;
    { t(0, 0) } -> std::same_as<typename T::const_reference>;
    { t[{0, 0}] } -> std::same_as<typename T::reference>;
    { t[{0, 0}] } -> std::same_as<typename T::const_reference>;
};

using namespace cpp_utils::types::concepts;
