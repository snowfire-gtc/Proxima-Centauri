#include "Math.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include "utils/Logger.h"

namespace proxima {
namespace stdlib {

// ============================================================================
// Базовые математические функции
// ============================================================================

double Math::abs(double x) {
    return std::abs(x);
}

float Math::abs(float x) {
    return std::abs(x);
}

int32_t Math::abs(int32_t x) {
    return std::abs(x);
}

int64_t Math::abs(int64_t x) {
    return std::abs(x);
}

double Math::sqrt(double x) {
    return std::sqrt(x);
}

float Math::sqrt(float x) {
    return std::sqrt(x);
}

double Math::exp(double x) {
    return std::exp(x);
}

float Math::exp(float x) {
    return static_cast<float>(std::exp(x));
}

double Math::log(double x) {
    return std::log(x);
}

float Math::log(float x) {
    return static_cast<float>(std::log(x));
}

double Math::log10(double x) {
    return std::log10(x);
}

float Math::log10(float x) {
    return static_cast<float>(std::log10(x));
}

double Math::pow(double base, double exponent) {
    return std::pow(base, exponent);
}

float Math::pow(float base, float exponent) {
    return static_cast<float>(std::pow(base, exponent));
}

// ============================================================================
// Тригонометрические функции
// ============================================================================

double Math::sin(double x) {
    return std::sin(x);
}

float Math::sin(float x) {
    return static_cast<float>(std::sin(x));
}

double Math::cos(double x) {
    return std::cos(x);
}

float Math::cos(float x) {
    return static_cast<float>(std::cos(x));
}

double Math::tan(double x) {
    return std::tan(x);
}

float Math::tan(float x) {
    return static_cast<float>(std::tan(x));
}

double Math::asin(double x) {
    return std::asin(x);
}

double Math::acos(double x) {
    return std::acos(x);
}

double Math::atan(double x) {
    return std::atan(x);
}

double Math::atan2(double y, double x) {
    return std::atan2(y, x);
}

// ============================================================================
// Гиперболические функции
// ============================================================================

double Math::sinh(double x) {
    return std::sinh(x);
}

double Math::cosh(double x) {
    return std::cosh(x);
}

double Math::tanh(double x) {
    return std::tanh(x);
}

// ============================================================================
// Округление
// ============================================================================

double Math::floor(double x) {
    return std::floor(x);
}

double Math::ceil(double x) {
    return std::ceil(x);
}

double Math::round(double x) {
    return std::round(x);
}

int32_t Math::toInt32(double x) {
    return static_cast<int32_t>(x);
}

int64_t Math::toInt64(double x) {
    return static_cast<int64_t>(x);
}

// ============================================================================
// Минимум/Максимум
// ============================================================================

double Math::min(double a, double b) {
    return std::min(a, b);
}

float Math::min(float a, float b) {
    return std::min(a, b);
}

int32_t Math::min(int32_t a, int32_t b) {
    return std::min(a, b);
}

double Math::max(double a, double b) {
    return std::max(a, b);
}

float Math::max(float a, float b) {
    return std::max(a, b);
}

int32_t Math::max(int32_t a, int32_t b) {
    return std::max(a, b);
}

// ============================================================================
// Векторные операции
// ============================================================================

double Math::sum(const std::vector<double>& vec) {
    return std::accumulate(vec.begin(), vec.end(), 0.0);
}

float Math::sum(const std::vector<float>& vec) {
    return std::accumulate(vec.begin(), vec.end(), 0.0f);
}

double Math::mean(const std::vector<double>& vec) {
    if (vec.empty()) return 0.0;
    return sum(vec) / vec.size();
}

float Math::mean(const std::vector<float>& vec) {
    if (vec.empty()) return 0.0f;
    return sum(vec) / vec.size();
}

double Math::variance(const std::vector<double>& vec) {
    if (vec.size() < 2) return 0.0;
    double m = mean(vec);
    double sum_sq = 0.0;
    for (double x : vec) {
        sum_sq += (x - m) * (x - m);
    }
    return sum_sq / (vec.size() - 1);
}

double Math::stddev(const std::vector<double>& vec) {
    return std::sqrt(variance(vec));
}

double Math::min(const std::vector<double>& vec) {
    if (vec.empty()) return 0.0;
    return *std::min_element(vec.begin(), vec.end());
}

double Math::max(const std::vector<double>& vec) {
    if (vec.empty()) return 0.0;
    return *std::max_element(vec.begin(), vec.end());
}

size_t Math::argmin(const std::vector<double>& vec) {
    if (vec.empty()) return 0;
    return std::distance(vec.begin(), std::min_element(vec.begin(), vec.end()));
}

size_t Math::argmax(const std::vector<double>& vec) {
    if (vec.empty()) return 0;
    return std::distance(vec.begin(), std::max_element(vec.begin(), vec.end()));
}

// ============================================================================
// Матричные операции
// ============================================================================

std::vector<std::vector<double>> Math::matrixMultiply(
    const std::vector<std::vector<double>>& A,
    const std::vector<std::vector<double>>& B) {
    
    size_t m = A.size();
    size_t n = B[0].size();
    size_t k = B.size();
    
    std::vector<std::vector<double>> C(m, std::vector<double>(n, 0.0));
    
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            for (size_t l = 0; l < k; l++) {
                C[i][j] += A[i][l] * B[l][j];
            }
        }
    }
    
    return C;
}

std::vector<std::vector<double>> Math::matrixTranspose(
    const std::vector<std::vector<double>>& A) {
    
    if (A.empty()) return {};
    
    size_t m = A.size();
    size_t n = A[0].size();
    
    std::vector<std::vector<double>> T(n, std::vector<double>(m));
    
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            T[j][i] = A[i][j];
        }
    }
    
    return T;
}

std::vector<std::vector<double>> Math::matrixIdentity(size_t n) {
    std::vector<std::vector<double>> I(n, std::vector<double>(n, 0.0));
    for (size_t i = 0; i < n; i++) {
        I[i][i] = 1.0;
    }
    return I;
}

std::vector<std::vector<double>> Math::matrixZeros(size_t rows, size_t cols) {
    return std::vector<std::vector<double>>(rows, std::vector<double>(cols, 0.0));
}

std::vector<std::vector<double>> Math::matrixOnes(size_t rows, size_t cols) {
    return std::vector<std::vector<double>>(rows, std::vector<double>(cols, 1.0));
}

std::vector<std::vector<double>> Math::matrixRand(size_t rows, size_t cols, 
                                                   double min, double max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    
    std::vector<std::vector<double>> M(rows, std::vector<double>(cols));
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            M[i][j] = dis(gen);
        }
    }
    return M;
}

double Math::matrixDeterminant(const std::vector<std::vector<double>>& A) {
    size_t n = A.size();
    if (n == 0 || A[0].size() != n) return 0.0;
    
    if (n == 1) return A[0][0];
    if (n == 2) return A[0][0] * A[1][1] - A[0][1] * A[1][0];
    
    double det = 0.0;
    for (size_t j = 0; j < n; j++) {
        std::vector<std::vector<double>> submatrix(n - 1, std::vector<double>(n - 1));
        for (size_t row = 1; row < n; row++) {
            size_t subcol = 0;
            for (size_t col = 0; col < n; col++) {
                if (col != j) {
                    submatrix[row - 1][subcol] = A[row][col];
                    subcol++;
                }
            }
        }
        det += (j % 2 == 0 ? 1 : -1) * A[0][j] * matrixDeterminant(submatrix);
    }
    return det;
}

// ============================================================================
// Элемент-по-элементу операции
// ============================================================================

std::vector<double> Math::elementWiseAdd(const std::vector<double>& a, 
                                         const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> result(n);
    for (size_t i = 0; i < n; i++) {
        result[i] = a[i] + b[i];
    }
    return result;
}

std::vector<double> Math::elementWiseMultiply(const std::vector<double>& a, 
                                              const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> result(n);
    for (size_t i = 0; i < n; i++) {
        result[i] = a[i] * b[i];
    }
    return result;
}

std::vector<double> Math::elementWiseDivide(const std::vector<double>& a, 
                                            const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> result(n);
    for (size_t i = 0; i < n; i++) {
        result[i] = b[i] != 0 ? a[i] / b[i] : 0.0;
    }
    return result;
}

std::vector<double> Math::elementWisePower(const std::vector<double>& a, 
                                           const std::vector<double>& b) {
    size_t n = std::min(a.size(), b.size());
    std::vector<double> result(n);
    for (size_t i = 0; i < n; i++) {
        result[i] = std::pow(a[i], b[i]);
    }
    return result;
}

// ============================================================================
// Специальные значения
// ============================================================================

double Math::nan() {
    return std::numeric_limits<double>::quiet_NaN();
}

double Math::inf() {
    return std::numeric_limits<double>::infinity();
}

double Math::ninf() {
    return -std::numeric_limits<double>::infinity();
}

double Math::pi() {
    return 3.14159265358979323846;
}

double Math::pi2() {
    return 2.0 * pi();
}

double Math::exp() {
    return 2.71828182845904523536;
}

bool Math::isNaN(double x) {
    return std::isnan(x);
}

bool Math::isInf(double x) {
    return std::isinf(x);
}

bool Math::isFinite(double x) {
    return std::isfinite(x);
}

// ============================================================================
// Валидация аргументов
// ============================================================================

bool Math::isPositive(double x) {
    return x > 0;
}

bool Math::isNegative(double x) {
    return x < 0;
}

bool Math::isZero(double x) {
    return x == 0;
}

bool Math::isInteger(double x) {
    return std::floor(x) == x;
}

bool Math::isEven(int32_t x) {
    return x % 2 == 0;
}

bool Math::isOdd(int32_t x) {
    return x % 2 != 0;
}

// ============================================================================
// Комбинаторика
// ============================================================================

int64_t Math::factorial(int32_t n) {
    if (n < 0) return 0;
    if (n <= 1) return 1;
    int64_t result = 1;
    for (int32_t i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

int64_t Math::combinations(int32_t n, int32_t k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n / 2) k = n - k;
    
    int64_t result = 1;
    for (int32_t i = 1; i <= k; i++) {
        result = result * (n - i + 1) / i;
    }
    return result;
}

int64_t Math::permutations(int32_t n, int32_t k) {
    if (k < 0 || k > n) return 0;
    int64_t result = 1;
    for (int32_t i = 0; i < k; i++) {
        result *= (n - i);
    }
    return result;
}

// ============================================================================
// НОД и НОК
// ============================================================================

int64_t Math::gcd(int64_t a, int64_t b) {
    a = std::abs(a);
    b = std::abs(b);
    while (b != 0) {
        int64_t t = b;
        b = a % b;
        a = t;
    }
    return a;
}

int64_t Math::lcm(int64_t a, int64_t b) {
    if (a == 0 || b == 0) return 0;
    return std::abs(a * b) / gcd(a, b);
}

// ============================================================================
// Решение линейных уравнений
// ============================================================================

std::vector<double> Math::solveLinearSystem(
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b) {
    
    // Gaussian elimination with partial pivoting
    size_t n = A.size();
    std::vector<std::vector<double>> augmented(n, std::vector<double>(n + 1));
    
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            augmented[i][j] = A[i][j];
        }
        augmented[i][n] = b[i];
    }
    
    // Forward elimination
    for (size_t i = 0; i < n; i++) {
        // Find pivot
        size_t maxRow = i;
        for (size_t k = i + 1; k < n; k++) {
            if (std::abs(augmented[k][i]) > std::abs(augmented[maxRow][i])) {
                maxRow = k;
            }
        }
        
        // Swap rows
        std::swap(augmented[i], augmented[maxRow]);
        
        // Eliminate column
        for (size_t k = i + 1; k < n; k++) {
            double c = -augmented[k][i] / augmented[i][i];
            for (size_t j = i; j <= n; j++) {
                if (i == j) {
                    augmented[k][j] = 0;
                } else {
                    augmented[k][j] += c * augmented[i][j];
                }
            }
        }
    }
    
    // Back substitution
    std::vector<double> x(n);
    for (int32_t i = n - 1; i >= 0; i--) {
        x[i] = augmented[i][n];
        for (size_t j = i + 1; j < n; j++) {
            x[i] -= augmented[i][j] * x[j];
        }
        x[i] /= augmented[i][i];
    }
    
    return x;
}

} // namespace stdlib
} // namespace proxima