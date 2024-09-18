#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>

namespace lat {
struct All_t {};
constexpr All_t all = {};
template <typename IndexType = size_t, bool row_iter = false,
          bool col_iter = false>
struct MatrixIndex;

template <typename IndexType> struct MatrixIndex<IndexType, false, false> {
  IndexType row = 0, column = 0;
  constexpr MatrixIndex() = default;
  constexpr MatrixIndex(IndexType row, IndexType col) noexcept
      : row(row), column(col) {}
};

template <typename IndexType> struct MatrixIndex<IndexType, true, false> {
  IndexType row = 0;
  constexpr MatrixIndex() = default;
  constexpr MatrixIndex(IndexType row, All_t) noexcept : row(row) {}
};

template <typename IndexType> struct MatrixIndex<IndexType, false, true> {
  IndexType column = 0;
  constexpr MatrixIndex() = default;
  constexpr MatrixIndex(All_t, IndexType col) noexcept : column(col) {}
};

template <typename T, size_t rows, size_t columns, typename IndexType = size_t>
struct Matrix {
  // must be raw array bc std::array operator[] is not constexpr in c++14
private:
  T contents[rows * columns] = {};

public:
  constexpr Matrix() = default;
  constexpr Matrix(std::array<T, rows * columns> data) {
    std::copy(data.begin(), data.end(), contents);
  }

  constexpr Matrix<T, columns, rows, IndexType> transpose() const noexcept {
    Matrix<T, columns, rows, IndexType> result;
    for (IndexType r = 0; r != rows; ++r) {
      auto col = result[{all, r}];
      auto row = (*this)[{r, all}];
      std::copy(row.begin(), row.end(), col.begin());
    }
    return result;
  }

  constexpr Matrix(const T (&list)[rows][columns]) {
    for (IndexType r = 0; r != rows; ++r) {
      for (IndexType c = 0; c != columns; ++c) {
        contents[r * columns + c] = list[r][c];
      }
    }
  }

  constexpr operator T() const noexcept
    requires(columns == 1 && rows == 1)
  {
    return contents[0];
  }

  constexpr T &operator[](MatrixIndex<IndexType> index) noexcept {
    return contents[index.row * columns + index.column];
  }
  constexpr T const &operator[](MatrixIndex<IndexType> index) const noexcept {
    return contents[index.row * columns + index.column];
  }

  template <size_t max, size_t increment, typename DataType> struct View {
    DataType *data;
    struct Iterator {
      DataType *data;
      using difference_type = int32_t;
      using value_type = DataType;
      constexpr DataType &operator*() const noexcept { return *data; }
      constexpr DataType &operator[](IndexType i) const noexcept {
        return data[i * increment];
      }
      constexpr Iterator &operator++() noexcept {
        data += increment;
        return *this;
      }
      constexpr Iterator operator++(int) noexcept {
        auto old = *this;
        this->operator++();
        return old;
      }
      constexpr Iterator &operator--() noexcept {
        data -= increment;
        return *this;
      }
      constexpr Iterator operator--(int) noexcept {
        auto old = *this;
        this->operator--();
        return old;
      }
      constexpr friend auto operator<=>(Iterator, Iterator) noexcept = default;
      constexpr friend Iterator operator+(Iterator it, int32_t i) noexcept {
        return Iterator{.data = it.data + i * increment};
      }
      constexpr friend Iterator operator+(int32_t i, Iterator it) noexcept {
        return Iterator{.data = it.data + i * increment};
      }
      constexpr Iterator &operator+=(IndexType i) {
        *this = *this + i;
        return *this;
      }
      constexpr Iterator &operator-=(IndexType i) {
        *this = *this - i;
        return *this;
      }
      constexpr friend Iterator operator-(Iterator it, IndexType i) noexcept {
        return Iterator{.data = it.data - i * increment};
      }
      constexpr friend int32_t operator-(Iterator rhs, Iterator lhs) noexcept {
        return (rhs.data - lhs.data) / increment;
      }
    };

    constexpr DataType &operator[](IndexType i) { return data[i * increment]; }
    constexpr Iterator begin() { return Iterator{.data = data}; }
    constexpr Iterator end() {
      return Iterator{.data = data + max * increment};
    }
    constexpr Iterator begin() const { return Iterator{.data = data}; }
    constexpr Iterator end() const {
      return Iterator{.data = data + max * increment};
    }
  };

  // row view
  constexpr View<columns, 1, T>
  operator[](MatrixIndex<IndexType, true, false> index) noexcept {
    return View<columns, 1, T>{.data = contents + index.row * columns};
  }

  constexpr View<columns, 1, const T>
  operator[](MatrixIndex<IndexType, true, false> index) const noexcept {
    return View<columns, 1, const T>{.data = contents + index.row * columns};
  }

  // column view
  constexpr View<rows, columns, T>
  operator[](MatrixIndex<IndexType, false, true> index) noexcept {
    return View<rows, columns, T>{.data = contents + index.column};
  }

  constexpr View<rows, columns, const T>
  operator[](MatrixIndex<IndexType, false, true> index) const noexcept {
    return View<rows, columns, const T>{.data = contents + index.column};
  }

  constexpr friend Matrix operator*(Matrix mat, T a) {
    Matrix result = mat;
    for (auto &i : result.contents) {
      i *= a;
    }
    return result;
  }

  constexpr friend Matrix operator*(T a, Matrix mat) { return mat * a; }
  constexpr Matrix &operator+=(Matrix rhs) {
    for (IndexType i = 0; i != rows * columns; ++i) {
      contents[i] += rhs.contents[i];
    }
  }
  constexpr friend Matrix operator+(Matrix lhs, Matrix rhs) {
    return lhs += rhs;
  }
  constexpr Matrix &operator-=(Matrix rhs) {
    for (IndexType i = 0; i != rows * columns; ++i) {
      contents[i] -= rhs.contents[i];
    }
  }
  constexpr friend Matrix operator-(Matrix lhs, Matrix rhs) {
    return lhs -= rhs;
  }
};

template <typename T1, typename T2, size_t out_row, size_t mid, size_t mid2,
          size_t out_col>
constexpr auto operator*(Matrix<T1, out_row, mid> lhs,
                         Matrix<T2, mid2, out_col> rhs) {
  Matrix<decltype(std::declval<T1>() * std::declval<T2>()), out_row, out_col>
      result;
  static_assert(mid == mid2, "Matrix dimentions do not match!");
  for (size_t r = 0; r < out_row; ++r) {
    for (size_t c = 0; c < out_col; ++c) {
      auto row = lhs[{r, all}];
      auto col = rhs[{all, c}];
      result[{r, c}] =
          std::inner_product(row.begin(), row.end(), col.begin(), 0);
    }
  }
  return result;
}

template <typename T, size_t len> using RowVec = Matrix<T, 1, len>;
template <typename T, size_t len> using ColVec = Matrix<T, len, 1>;
template <typename T, size_t len> using SquareMatrix = Matrix<T, len, len>;

template <typename T, size_t len> constexpr auto make_identity() {
  SquareMatrix<T, len> matrix;
  for (size_t i = 0; i != len; ++i) {
    matrix[{i, i}] = 1;
  }
  return matrix;
}

template <typename T, size_t len>
constexpr SquareMatrix<T, len> identity = make_identity<T, len>();

template <size_t rows, size_t columns>
using IMatrix = Matrix<int32_t, rows, columns>;

template <size_t len> using IRowVec = RowVec<int32_t, len>;
template <size_t len> using IColVec = ColVec<int32_t, len>;

template <size_t rows, size_t columns>
using FMatrix = Matrix<float, rows, columns>;

template <size_t len> using FRowVec = RowVec<float, len>;
template <size_t len> using FColVec = ColVec<float, len>;

} // namespace lat
