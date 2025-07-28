#pragma once

#include <array>
#include <vector>

struct Index2D
{
    std::size_t row;
    std::size_t col;
    Index2D(std::size_t r, std::size_t c) : row(r), col(c) { }
};


template <std::size_t Rows, std::size_t Cols, typename T>
struct Array2D
{
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr size_type rows() { return Rows; }
    static constexpr size_type cols() { return Cols; }
    static constexpr size_type row_size = Cols;
    static constexpr size_type col_size = Rows;


    Array2D() = default;
    Array2D(const Array2D&) = default;
    Array2D(Array2D&&) = default;
    Array2D& operator=(const Array2D&) = default;
    Array2D& operator=(Array2D&&) = default;
    ~Array2D() = default;


    reference operator()(std::size_t row, std::size_t col) { return _data[row * Cols + col]; }

    const_reference operator()(std::size_t row, std::size_t col) const
    {
        return _data[row * Cols + col];
    }

    reference operator[](Index2D index) { return _data[index.row * Cols + index.col]; }

    const_reference operator[](Index2D index) const { return _data[index.row * Cols + index.col]; }

    constexpr std::size_t size() const { return Rows * Cols; }
    value_type* data() { return _data.data(); }
    const value_type* data() const { return _data.data(); }


private:
    std::array<T, Rows * Cols> _data;
};

namespace std
{
template <std::size_t Rows, std::size_t Cols, typename T>
constexpr size_t size(const Array2D<Rows, Cols, T>& arr)
{
    return arr.size();
}

template <std::size_t Rows, std::size_t Cols, typename T>
T* data(Array2D<Rows, Cols, T>& arr)
{
    return arr.data();
}

template <std::size_t Rows, std::size_t Cols, typename T>
const T* data(const Array2D<Rows, Cols, T>& arr)
{
    return arr.data();
}


}


template <typename T>
struct DynamicArray2D
{
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    DynamicArray2D() = delete;

    DynamicArray2D(size_type rows, size_type cols) : _rows(rows), _cols(cols)
    {
        _data.resize(rows * cols);
    }

    DynamicArray2D(DynamicArray2D&& other) noexcept
        : _rows(other._rows), _cols(other._cols), _data(std::move(other._data))
    {
        other._rows = 0;
        other._cols = 0;
    }

    DynamicArray2D& operator=(DynamicArray2D&& other) noexcept
    {
        if (this != &other)
        {
            _rows = other._rows;
            _cols = other._cols;
            _data = std::move(other._data);
            other._rows = 0;
            other._cols = 0;
        }
        return *this;
    }

    reference operator()(size_type row, size_type col) { return _data[row * _cols + col]; }

    const_reference operator()(size_type row, size_type col) const
    {
        return _data[row * _cols + col];
    }
    reference operator[](Index2D index) { return _data[index.row * _cols + index.col]; }
    const_reference operator[](Index2D index) const { return _data[index.row * _cols + index.col]; }
    value_type* data() { return _data.data(); }
    const value_type* data() const { return _data.data(); }

    size_type rows() const { return _rows; }
    size_type cols() const { return _cols; }
    size_type size() const { return _rows * _cols; }


private:
    size_type _rows;
    size_type _cols;
    std::vector<T> _data;
};

namespace std
{
template <typename T>
constexpr size_t size(const DynamicArray2D<T>& arr)
{
    return arr.size();
}

template <typename T>
T* data(DynamicArray2D<T>& arr)
{
    return arr.data();
}

template <typename T>
const T* data(const DynamicArray2D<T>& arr)
{
    return arr.data();
}

}
