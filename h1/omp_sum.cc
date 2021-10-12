#include <omp.h>

#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
using std::cout;

/**
 * Sum is used to compute the sum of a vector using OpenMP.
 * @param x: the input vector.
 * @param n: the length of x.
 * @param result: the pointer to the output result.
 * @param k: thread count.
 */
template <typename T>
T Sum(const T* const x, const size_t n, const size_t k);

// input: x = [1, ..., 1000], n = 1000, k = 20
// expect: result = (1 + 1000) / 2 * 1000
void TestSumBasic() {
    cout << "-- TestSumBasic\n";
    int x[1000];
    for (int i = 0; i < 1000; i++) {
        x[i] = i + 1;
    }
    int result = 0;
    int k = 20;
    int expected = (1 + 1000) * 1000 / 2;
    result = Sum(x, 1000, k);

    cout << "expected: " << expected << ", actual: " << result << std::endl;
    if (result != expected) {
        cout << "wrong" << std::endl;
    } else {
        cout << "success" << std::endl;
    }
}

void TestSumRand() {
    cout << "-- TestSumRand\n";
    constexpr size_t n = 10000;
    constexpr int MAX_VALUE = 2000;
    constexpr int MIN_VALUE = 10;
    int x[n];
    for (size_t i = 0; i < n; i++) {
        std::srand(std::time(nullptr));
        x[i] = (rand() % (MAX_VALUE - MIN_VALUE + 1)) + MIN_VALUE;
    }
    int result = 0;
    int k = 20;

    int expected = 0;
    for (size_t i = 0; i < n; i++) {
        expected += x[i];
    }

    result = Sum(x, n, k);
    cout << "expected: " << expected << ", actual: " << result << std::endl;
    if (result != expected) {
        cout << "wrong" << std::endl;
    } else {
        cout << "success" << std::endl;
    }
}

int main() {
    TestSumBasic();
    TestSumRand();
}

template <typename T>
T Sum(const T* const x, const size_t n, const size_t k) {
    T sum(0);
#pragma omp parallel for num_threads(k) reduction(+ : sum)
    for (size_t i = 0; i < n; i++) {
        sum += x[i];
    }

    return sum;
}