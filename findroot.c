#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "findroot.h"

#define EPSILON 1e-10  // Độ chính xác yêu cầu, giống Casio

// Biến toàn cục để báo hiệu khi tìm được nghiệm
extern volatile int solution_found;
extern pthread_mutex_t mutex;

// Khởi tạo seed cho hàm random
void initRandom() {
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
}

// Tính đạo hàm bằng sai phân trung tâm, giống cách Casio xấp xỉ
long double derivative(Token *postfix, long double x) {
    long double h = 1e-6;  // Bước nhỏ, tương tự Casio
    long double fx_plus_h = evaluatePostfix(postfix, x + h);
    long double fx_minus_h = evaluatePostfix(postfix, x - h);
    
    if (isnan(fx_plus_h) || isnan(fx_minus_h) || isinf(fx_plus_h) || isinf(fx_minus_h)) {
        return NAN;
    }
    return (fx_plus_h - fx_minus_h) / (2 * h);
}

// Phương pháp Newton-Raphson, bắt đầu từ x = 1.0, xử lý đạo hàm nhỏ như Casio
long double newtonRaphson(Token *postfix) {
    long double x = 1.0;  // Điểm khởi tạo cố định là 1.0
    printf("Newton-Raphson: Bắt đầu với x = %.10Lf\n", x);

    while (1) {  // Lặp liên tục
        pthread_testcancel();  // Điểm kiểm tra hủy luồng

        // Kiểm tra xem đã tìm được nghiệm chưa
        pthread_mutex_lock(&mutex);
        if (solution_found) {
            pthread_mutex_unlock(&mutex);
            return NAN;  // Thoát nếu một phương pháp khác đã tìm được nghiệm
        }
        pthread_mutex_unlock(&mutex);

        // Tính f(x) và f'(x)
        long double fx = evaluatePostfix(postfix, x);
        long double dfx = derivative(postfix, x);

        // Kiểm tra lỗi
        if (isnan(fx) || isnan(dfx) || isinf(fx) || isinf(dfx)) {
            printf("Newton-Raphson: Giá trị không hợp lệ tại x = %.10Lf\n", x);
            return NAN;  // Thoát nếu gặp lỗi
        }

        // Kiểm tra nghiệm
        if (fabsl(fx) < EPSILON) {
            printf("Newton-Raphson: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x, fx);
            return x;  // Dừng khi tìm được nghiệm
        }

        // Xử lý đạo hàm quá nhỏ (theo cách của Casio)
        if (fabsl(dfx) < EPSILON) {
            printf("Newton-Raphson: Đạo hàm quá nhỏ tại x = %.10Lf, tăng x lên một chút\n", x);
            x += 1e-6;  // Tăng x lên một bước nhỏ, giống Casio
            continue;  // Tiếp tục lặp
        }

        // Tính x mới theo công thức Newton
        long double x_new = x - fx / dfx;

        // Kiểm tra x_new có hợp lệ không
        if (isnan(x_new) || isinf(x_new)) {
            printf("Newton-Raphson: Giá trị mới không hợp lệ tại x = %.10Lf\n", x);
            return NAN;
        }

        // Cập nhật x và tiếp tục lặp
        x = x_new;
        printf("Newton-Raphson: Lặp mới, x = %.10Lf\n", x);
    }

    return NAN;  // Không bao giờ đến đây do vòng lặp vô hạn
}

// Phương pháp chia đôi (giữ nguyên)
long double bisectionMethod(Token *postfix) {
    initRandom();
    while (1) {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        if (solution_found) {
            pthread_mutex_unlock(&mutex);
            return NAN;
        }
        pthread_mutex_unlock(&mutex);

        long double a = (long double)(rand() % 20001 - 10000);
        long double b = (long double)(rand() % 20001 - 10000);
        if (a > b) {
            long double temp = a;
            a = b;
            b = temp;
        }
        printf("Bisection: Thử khoảng ngẫu nhiên [%.10Lf, %.10Lf]\n", a, b);

        long double fa = evaluatePostfix(postfix, a);
        long double fb = evaluatePostfix(postfix, b);

        if (isnan(fa) || isnan(fb) || isinf(fa) || isinf(fb)) {
            printf("Bisection: Giá trị không hợp lệ tại khoảng [%.10Lf, %.10Lf]\n", a, b);
            continue;
        }

        if (fa * fb >= 0) {
            printf("Bisection: Không có nghiệm trong khoảng [%.10Lf, %.10Lf]\n", a, b);
            continue;
        }

        while (1) {
            pthread_testcancel();
            pthread_mutex_lock(&mutex);
            if (solution_found) {
                pthread_mutex_unlock(&mutex);
                return NAN;
            }
            pthread_mutex_unlock(&mutex);

            long double c = (a + b) / 2;
            long double fc = evaluatePostfix(postfix, c);

            if (isnan(fc) || isinf(fc)) {
                printf("Bisection: Giá trị không hợp lệ tại x = %.10Lf trong khoảng [%.10Lf, %.10Lf]\n", c, a, b);
                break;
            }

            if (fabsl(fc) < EPSILON || fabsl(b - a) < EPSILON) {
                printf("Bisection: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", c, fc);
                return c;
            }

            if (fa * fc < 0) {
                b = c;
                fb = fc;
            } else {
                a = c;
                fa = fc;
            }
        }
    }

    return NAN;
}

// Phương pháp dây cung (giữ nguyên)
long double secantMethod(Token *postfix) {
    long double initial_pairs[][2] = {{0.0, 1.0}, {-1.0, 1.0}, {1.0, 2.0}, {-2.0, -1.0}, {5.0, 6.0}, {-5.0, -4.0}, {10.0, 11.0}, {-10.0, -9.0}, {100.0, 101.0}, {-100.0, -99.0}};
    int num_pairs = sizeof(initial_pairs) / sizeof(initial_pairs[0]);

    for (int pair_idx = 0; pair_idx < num_pairs; pair_idx++) {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        if (solution_found) {
            pthread_mutex_unlock(&mutex);
            return NAN;
        }
        pthread_mutex_unlock(&mutex);

        long double x0 = initial_pairs[pair_idx][0];
        long double x1 = initial_pairs[pair_idx][1];
        printf("Secant: Thử cặp khởi tạo (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        while (1) {
            pthread_testcancel();
            pthread_mutex_lock(&mutex);
            if (solution_found) {
                pthread_mutex_unlock(&mutex);
                return NAN;
            }
            pthread_mutex_unlock(&mutex);

            long double f0 = evaluatePostfix(postfix, x0);
            long double f1 = evaluatePostfix(postfix, x1);

            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            if (fabsl(f1) < EPSILON) {
                printf("Secant: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x1, f1);
                return x1;
            }

            if (fabsl(f1 - f0) < EPSILON) {
                printf("Secant: Mẫu số quá nhỏ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            long double x2 = x1 - f1 * (x1 - x0) / (f1 - f0);

            if (isnan(x2) || isinf(x2)) {
                printf("Secant: Giá trị lặp mới không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            x0 = x1;
            x1 = x2;
        }
    }

    initRandom();
    while (1) {
        pthread_testcancel();
        pthread_mutex_lock(&mutex);
        if (solution_found) {
            pthread_mutex_unlock(&mutex);
            return NAN;
        }
        pthread_mutex_unlock(&mutex);

        long double x0 = (long double)(rand() % 20001 - 10000);
        long double x1 = (long double)(rand() % 20001 - 10000);
        printf("Secant: Thử cặp khởi tạo ngẫu nhiên (x0 = %.10Lf, x1 = %.10Lf)\n", x0, x1);

        while (1) {
            pthread_testcancel();
            pthread_mutex_lock(&mutex);
            if (solution_found) {
                pthread_mutex_unlock(&mutex);
                return NAN;
            }
            pthread_mutex_unlock(&mutex);

            long double f0 = evaluatePostfix(postfix, x0);
            long double f1 = evaluatePostfix(postfix, x1);

            if (isnan(f0) || isnan(f1) || isinf(f0) || isinf(f1)) {
                printf("Secant: Giá trị không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            if (fabsl(f1) < EPSILON) {
                printf("Secant: Tìm được nghiệm x = %.10Lf (f(x) = %.10Lf)\n", x1, f1);
                return x1;
            }

            if (fabsl(f1 - f0) < EPSILON) {
                printf("Secant: Mẫu số quá nhỏ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            long double x2 = x1 - f1 * (x1 - x0) / (f1 - f0);

            if (isnan(x2) || isinf(x2)) {
                printf("Secant: Giá trị lặp mới không hợp lệ tại x0 = %.10Lf, x1 = %.10Lf\n", x0, x1);
                break;
            }

            x0 = x1;
            x1 = x2;
        }
    }

    return NAN;
}
