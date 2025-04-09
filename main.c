#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <wiringPi.h>
#include "postfix.h"
#include "findroot.h"

#define NUM_THREADS 3
#define ROWS 7    // 7 hàng
#define COLS 4    // 4 cột

// Định nghĩa các chân GPIO cho hàng và cột (theo chuẩn BCM)
int rowPins[ROWS] = {17, 18, 27, 22, 23, 24, 25}; // 7 hàng
int colPins[COLS] = {8, 7, 1, 4};                 // 4 cột

// Ánh xạ phím 7x4 (28 phím) thành các ký tự
char keymap[ROWS][COLS] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '*'},
    {'0', '.', '/', '^'},
    {'(', ')', 'x', 'E'}, // E là Enter
    {'\0', '\0', '\0', '\0'}, // Dòng trống
    {'\0', '\0', '\0', '\0'}  // Dòng trống
};

// Biến toàn cục cho luồng
int found = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
float best_result = 0.0;

typedef struct {
    Token *postfix;
    float result;
} ThreadData;

// Hàm quét bàn phím 7x4
char scanKeypad() {
    for (int r = 0; r < ROWS; r++) {
        digitalWrite(rowPins[r], LOW); // Đặt hàng hiện tại thành LOW
        for (int c = 0; c < COLS; c++) {
            if (digitalRead(colPins[c]) == LOW) { // Phím được nhấn
                while (digitalRead(colPins[c]) == LOW) delay(10); // Chờ thả phím
                digitalWrite(rowPins[r], HIGH);
                return keymap[r][c];
            }
        }
        digitalWrite(rowPins[r], HIGH); // Đặt lại hàng thành HIGH
    }
    return '\0'; // Không có phím nào được nhấn
}

void *findrootNewton(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = newtonRaphson(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(data->result)) {
        float fx = evaluatePostfix(data->postfix, data->result);
        if (fabs(fx) < 1e-4) {
            best_result = data->result;
            found = 1;
            printf("Newton-Raphson tìm được nghiệm: %f\n", best_result);
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootBisection(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = bisectionMethod(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(data->result)) {
        float fx = evaluatePostfix(data->postfix, data->result);
        if (fabs(fx) < 1e-4) {
            best_result = data->result;
            found = 1;
            printf("Bisection tìm được nghiệm: %f\n", best_result);
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

void *findrootSecant(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    data->result = secantMethod(data->postfix);

    pthread_mutex_lock(&mutex);
    if (!found && !isnan(data->result)) {
        float fx = evaluatePostfix(data->postfix, data->result);
        if (fabs(fx) < 1e-4) {
            best_result = data->result;
            found = 1;
            printf("Secant tìm được nghiệm: %f\n", best_result);
        }
    }
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main() {
    struct timespec start, end;
    Token *output;
    char str[MAX];
    int idx = 0;
    char ch;

    // Khởi tạo wiringPi
    if (wiringPiSetupGpio() == -1) {
        printf("Lỗi: Không thể khởi tạo wiringPi!\n");
        return 1;
    }

    // Cấu hình GPIO cho hàng và cột
    for (int i = 0; i < ROWS; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH);
    }
    for (int i = 0; i < COLS; i++) {
        pinMode(colPins[i], INPUT);
        pullUpDnControl(colPins[i], PUD_UP);
    }

    printf("Nhập biểu thức bằng bàn phím 7x4 (ấn 'E' để xác nhận và tính toán):\n");
    printf("Hiện tại: ");

    // Nhập biểu thức từ bàn phím 7x4
    while (1) {
        ch = scanKeypad();
        if (ch == '\0') continue; // Không có phím nào được nhấn

        if (ch == 'E') { // Enter để xác nhận và tính toán
            str[idx] = '\0';
            printf("\n");
            break;
        } else if (idx < MAX - 1) { // Nhập ký tự vào biểu thức
            str[idx++] = ch;
            printf("\rHiện tại: %s", str);
            fflush(stdout);
        }
    }

    output = infixToPostfix(str);
    if (output != NULL) {
        printTokens(output);

        pthread_t threads[NUM_THREADS];
        ThreadData threadData[NUM_THREADS];

        clock_gettime(CLOCK_MONOTONIC, &start);

        // Chạy tất cả các phương pháp tìm nghiệm
        threadData[0].postfix = output;
        pthread_create(&threads[0], NULL, findrootNewton, (void *)&threadData[0]);

        threadData[1].postfix = output;
        pthread_create(&threads[1], NULL, findrootBisection, (void *)&threadData[1]);

        threadData[2].postfix = output;
        pthread_create(&threads[2], NULL, findrootSecant, (void *)&threadData[2]);

        // Chờ các luồng hoàn thành
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("Thời gian tìm nghiệm: %f giây\n", elapsed);

        if (found) {
            float fx = evaluatePostfix(output, best_result);
            printf("Kết quả với nghiệm %.4f là: %.4f\n", best_result, fx);
            if (fabs(fx) > 1e-4) {
                printf("Cảnh báo: Giá trị tại nghiệm không đủ gần 0 (có thể không phải nghiệm chính xác)!\n");
            }
        } else {
            printf("Không tìm được nghiệm hợp lệ!\n");
        }

        free(output);
    } else {
        printf("Lỗi khi chuyển đổi biểu thức!\n");
    }
    return 0;
}
