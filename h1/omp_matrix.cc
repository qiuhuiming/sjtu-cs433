#include <omp.h>

#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <string>
using std::cout;
using std::endl;
using std::string;

/**
 * MatrixMultiplication computes matrix multiplication in OpenMP.
 * matrixC = matrixA x matrixB
 * @param matrixA: input matrix A. shape = (r1, c2).
 * @param matrixB: input matrix B. shape = (r2, c2).
 * @param matrixC: output matrix C. shape = (r1, c2).
 * @param r1: the number of rows of A.
 * @param c1: the number of columns of A.
 * @param r2: the number of rows of B.
 * @param c2: the number of columns of B.
 * @return code: 0 = ok, -1 = error.
 */
template <typename T>
int MatrixMultiplication(const T* const matrixA, const T* const matrixB,
                         T* const matrixC, const int r1, const int c1,
                         const int r2, const int c2) {
    if (c1 != r2) {
        return 1;
    }
    int size = r1 * c2;
#pragma omp parallel for num_threads(size)
    for (int index = 0; index < size; index++) {
        int i = index / c2;
        int j = index % c2;
        T sub_result(0);
        for (int k = 0; k < c1; k++) {
            sub_result += matrixA[i * c1 + k] * matrixB[k * r2 + j];
        }
        matrixC[index] = sub_result;
        cout << std::to_string(omp_get_thread_num()) + ": " + std::to_string(sub_result) + "\n";
    }
    return 0;
}

template <typename T>
int MatrixMultiplicationSerial(const T* const matrixA, const T* const matrixB,
                               T* const matrixC, const int r1, const int c1,
                               const int r2, const int c2) {
    if (c1 != r2) {
        return 1;
    }
    int size = r1 * c2;
    for (int index = 0; index < size; index++) {
        int i = index / c2;
        int j = index % c2;
        T sub_result(0);
        for (int k = 0; k < c1; k++) {
            sub_result += matrixA[i * c1 + k] * matrixB[k * r2 + j];
        }
        matrixC[index] = sub_result;
    }
    return 0;
}

template <typename T>
void PrintMatrix(const T* const matrix, const int m, const int n) {
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            int index = j + i * n;
            cout << matrix[index] << '\t';
        }
        cout << '\n';
    }
}

template <typename T>
bool CompareMatrix(const T* const matrixA, const T* const matrixB,
                   const int size) {
    for (int i = 0; i < size; i++)
        if (matrixA[i] != matrixB[i]) {
            return false;
        }
    return true;
}

void TestMatrixMultiplicationBasic() {
    cout << "-- TestMatrixMultiplicationBasic\n";
    // |1 2; 3 4| X |1 2; 3 4| == |7 10; 15 22|
    int r1 = 2;
    int c1 = 2;

    int ma[] = {
        1,
        2,
        3,
        4,
    };

    int r2 = 2;
    int c2 = 2;

    int mb[] = {
        1,
        2,
        3,
        4,
    };

    int expected[] = {
        7,
        10,
        15,
        22,
    };
    int mc[4];
    if (MatrixMultiplication(ma, mb, mc, r1, c1, r2, c2) < 0) {
        cout << "error" << std::endl;
        return;
    }

    cout << "expected: \n";
    PrintMatrix(expected, 2, 2);

    cout << "actual: \n";
    PrintMatrix(mc, r1, c2);

    if (CompareMatrix(expected, mc, 4)) {
        cout << "ok";
    } else {
        cout << "wrong";
    }
}

int* newRandomMatrix(const int r, const int c) {
    int* result = new int[r * c];
    constexpr int MAX_VALUE = 2000;
    constexpr int MIN_VALUE = 10;
    std::srand(std::time(nullptr));
    for (int index = 0; index < r * c; index++) {
        result[index] = (rand() % (MAX_VALUE - MIN_VALUE + 1)) + MIN_VALUE;
    }
    return result;
}

void TestMatrixMultiplicationManyRandom() {
    cout << "-- TestMatrixMultiplicationBasicManyRandom\n";
    constexpr int r1 = 20;
    constexpr int c1 = 20;
    constexpr int r2 = 20;
    constexpr int c2 = 20;

    int* ma = newRandomMatrix(r1, c1);
    int* mb = newRandomMatrix(r2, c2);
    int* mc = newRandomMatrix(r1, c2);
    int* expected = newRandomMatrix(r1, c2);

    MatrixMultiplicationSerial(ma, mb, expected, r1, c1, r2, c2);
    MatrixMultiplication(ma, mb, mc, r1, c1, r2, c2);
    if (!CompareMatrix(mc, expected, r1 * c2)) {
        cout << "wrong\n";
    } else {
        cout << "ok";
    }

    delete[] ma;
    delete[] mb;
    delete[] mc;
    delete[] expected;
}

int main() {
    TestMatrixMultiplicationBasic();
    TestMatrixMultiplicationManyRandom();
}