#include <stdio.h>
#include <pthread.h>
#include "findroot.h"

// Biến toàn cục để báo hiệu khi tìm được nghiệm
volatile int solution_found = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Hàm chạy Newton-Raphson trong luồng
void *newtonThread(void *arg) {
    Token *postfix = (Token *)arg;
    long double result = newtonRaphson(postfix);
    pthread_mutex_lock(&mutex);
    if (!solution_found && !isnan(result)) {
        solution_found = 1;
        printf("Nghiệm cuối cùng từ Newton-Raphson: %.10Lf\n", result);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Hàm chạy Bisection trong luồng
void *bisectionThread(void *arg) {
    Token *postfix = (Token *)arg;
    long double result = bisectionMethod(postfix);
    pthread_mutex_lock(&mutex);
    if (!solution_found && !isnan(result)) {
        solution_found = 1;
        printf("Nghiệm cuối cùng từ Bisection: %.10Lf\n", result);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

// Hàm chạy Secant trong luồng
void *secantThread(void *arg) {
    Token *postfix = (Token *)arg;
    long double result = secantMethod(postfix);
    pthread_mutex_lock(&mutex);
    if (!solution_found && !isnan(result)) {
        solution_found = 1;
        printf("Nghiệm cuối cùng từ Secant: %.10Lf\n", result);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    // Giả sử bạn đã có biểu thức postfix (Token *postfix)
    Token *postfix = NULL;  // Thay bằng biểu thức của bạn

    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, newtonThread, postfix);
    pthread_create(&t2, NULL, bisectionThread, postfix);
    pthread_create(&t3, NULL, secantThread, postfix);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}
