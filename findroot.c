#include <stdio.h>
#include <math.h>
#include "findroot.h"

#define EPSILON 1e-6
#define MAX_ITER 1000

float derivative(Token *postfix, float x) {
    float h = 1e-4;
    return (evaluatePostfix(postfix, x + h) - evaluatePostfix(postfix, x - h)) / (2 * h);
}

float newtonRaphson(Token *postfix) {
    float x = 1.0; // Initial guess
    for (int i = 0; i < MAX_ITER; i++) {
        float fx = evaluatePostfix(postfix, x);
        float dfx = derivative(postfix, x);
        if (fabs(dfx) < EPSILON) break;
        float x1 = x - fx / dfx;
        if (fabs(x1 - x) < EPSILON) return x1;
        x = x1;
    }
    return x;
}

float bisectionMethod(Token *postfix) {
    float a = -10.0, b = 10.0; // Initial interval
    float fa = evaluatePostfix(postfix, a);
    float fb = evaluatePostfix(postfix, b);

    if (fa * fb >= 0) return NAN; // No root in this interval

    for (int i = 0; i < MAX_ITER; i++) {
        float c = (a + b) / 2;
        float fc = evaluatePostfix(postfix, c);
        if (fabs(fc) < EPSILON || fabs(b - a) < EPSILON) return c;
        if (fa * fc < 0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }
    return (a + b) / 2;
}

float secantMethod(Token *postfix) {
    float x0 = 0.0, x1 = 1.0; // Initial guesses
    for (int i = 0; i < MAX_ITER; i++) {
        float f0 = evaluatePostfix(postfix, x0);
        float f1 = evaluatePostfix(postfix, x1);
        if (fabs(f1 - f0) < EPSILON) break;
        float x2 = x1 - f1 * (x1 - x0) / (f1 - f0);
        if (fabs(x2 - x1) < EPSILON) return x2;
        x0 = x1;
        x1 = x2;
    }
    return x1;
}