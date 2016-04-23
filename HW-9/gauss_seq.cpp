#include <iostream>
#include <vector>
#include <cmath>

#include "timer.h"

const size_t MAX_N = 3000;

const double EPS = 1e-5;

size_t n;
double A[MAX_N][MAX_N], _A[MAX_N][MAX_N];
double b[MAX_N], _b[MAX_N];
double x[MAX_N];

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output_seq.txt", "w", stdout);

    // input number of equations

    std::cin >> n;

    // input coefficients matrix A

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            std::cin >> A[i][j];
            _A[i][j] = A[i][j];
        }
    }

    // input vector b

    for (size_t j = 0; j < n; ++j) {
        std::cin >> b[j];
        _b[j] = b[j];
    }

    steady_timer timer;

    // gaussian elimination

    for (size_t i = 0; i + 1 < n; ++i) {
        size_t best_i = i;
        for (size_t k = i + 1; k < n; ++k) {
            if (fabs(A[k][i]) > fabs(A[best_i][i])) {
                best_i = k;
            }
        }

        if (fabs(A[best_i][i]) < EPS) {
            printf("no solution");
            return 0;
        }

        if (i != best_i) {
            for (size_t j = i; j < n; ++j) {
                std::swap(A[i][j], A[best_i][j]);
            }
            std::swap(b[i], b[best_i]);
        }

        for (size_t k = i + 1; k < n; ++k) {
            double q = A[k][i] / A[i][i];
            for (size_t j = i; j < n; ++j) {
                A[k][j] = A[k][j] - A[i][j] * q;
            }
            b[k] = b[k] - b[i] * q;
        }
    }
    // backward substitution

    for (int i = n - 1; i >= 0; --i) {
        x[i] = b[i] / A[i][i];
        for (int j = i - 1; j >= 0; --j) {
            b[j] -= A[j][i] * x[i];
        }
    }

    // measure work time

    const double work_time = timer.seconds_elapsed();


    // output solution

    for (size_t i = 0; i < n; ++i) {
        printf("%14.8lf ", x[i]);
    }
    printf("\n");

    // compute norm of residuals vector

    double err = 0;
    for (size_t i = 0; i < n; ++i) {
        double lhs = 0;
        for (size_t j = 0; j < n; ++j) {
            lhs += _A[i][j] * x[j];
        }
        double r = _b[i] - lhs;
        err += r * r;
    }

    printf("err = %.10lf\n", err);

    printf("work time = %.5lf\n", work_time); 

    return 0;
}
