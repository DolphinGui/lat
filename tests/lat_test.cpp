#include <cstddef>
#include <cstdint>
#include <iostream>
#include <lat.hpp>

template <size_t rows, size_t cols> void print(lat::IMatrix<rows, cols> mat) {
  for (uint64_t i = 0; i != rows; i++) {
    for (uint64_t j = 0; j != cols; j++) {
      std::cout << mat[{i, j}] << " ";
    }
    std::cout << "\n";
  }
}

int main() {
    lat::IMatrix<2, 2> mat = {{{1, 2}, {3, 4}}};
    auto id = lat::identity<int32_t, 2>;
    std::cout << "mat:\n";
    print(mat);

    auto transpose = mat.transpose();
    std::cout << "transpose:\n";
    print(transpose);

    std::cout << "id:\n";
    print(id);
    auto result = mat * id;

    std::cout << "result:\n";
    print(result);

    auto row = result[{0, lat::all}];
    std::cout << "rows:\n";
    for (auto i : row) {
      std::cout << i << " ";
    }
    std::cout << '\n';

    auto col = result[{lat::all, 1}];
    std::cout << "columns:\n";
    for (auto i : col) {
      std::cout << i << " ";
    }
    std::cout << '\n';

    std::cout << "vector:\n";
    lat::IColVec<2> vector = {{2, 3}};
    print(vector);

    std::cout << "transformed:\n";
    auto transformed = result * vector;
    print(transformed);

    
    std::cout << "scaled:\n";
    auto scaled = transformed * 2;
    print(scaled);

    std::cout << "Dot product\n";
    int dot = transformed.transpose() * transformed;
    std::cout << dot << '\n';
}
