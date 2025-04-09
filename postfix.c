#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "postfix.h"
#include "findroot.h"

int isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

int precedence(char op) {
    switch (op) {
        case '+':
        case '-': return 1;
        case '*':
        case '/': return 2;
        case '^': return 3;
        default: return 0;
    }
}

Token *infixToPostfix(char *myFunction) {
    state_t current_state = S_START;
    Token *output = (Token *)malloc(MAX * sizeof(Token));
    int outputIndex = 0;
    char stack[MAX];
    int stackTop = -1;
    char *ptr = myFunction;
    while (1) {
        switch (current_state) {
            case S_START:
                if (isdigit(*ptr) || *ptr == '.') {
                    current_state = S_OPERAND;
                } else if (*ptr == 'x') {
                    current_state = S_OPERAND;
                } else if (*ptr == '(') {
                    current_state = S_OPEN;
                } else if (*ptr == '\0') {
                    current_state = S_END;
                } else if (isOperator(*ptr)) {
                    current_state = S_OPERATOR;
                } else {
                    current_state = S_ERROR;
                }
                break;

            case S_OPERAND: {
                float operand = 0.0;
                int decimal_flag = 0;
                float decimal_divisor = 1.0;

                if (isdigit(*ptr) || *ptr == '.') {
                    while (isdigit(*ptr) || *ptr == '.') {
                        if (*ptr == '.') {
                            decimal_flag = 1;
                        } else if (decimal_flag == 0) {
                            operand = operand * 10 + (*ptr - '0');
                        } else {
                            decimal_divisor *= 10;
                            operand = operand + (*ptr - '0') / decimal_divisor;
                        }
                        ptr++;
                    }
                    output[outputIndex].type = OPERAND;
                    output[outputIndex].value.operand = operand;
                    outputIndex++;
                } else if (*ptr == 'x') {
                    output[outputIndex].type = VARIABLE;
                    output[outputIndex].value.variable = 0.0;
                    outputIndex++;
                    ptr++;
                }

                if (isOperator(*ptr)) {
                    current_state = S_OPERATOR;
                } else if (*ptr == ')') {
                    current_state = S_CLOSE;
                } else if (*ptr == '\0') {
                    current_state = S_END;
                } else {
                    current_state = S_ERROR;
                }
                break;
            }

            case S_OPERATOR:
                while (stackTop >= 0 && precedence(stack[stackTop]) >= precedence(*ptr)) {
                    output[outputIndex].type = OPERATOR;
                    output[outputIndex].value.operator = stack[stackTop];
                    outputIndex++;
                    stackTop--;
                }
                stack[++stackTop] = *ptr;
                ptr++;
                current_state = S_START;
                break;

            case S_OPEN:
                stack[++stackTop] = *ptr;
                ptr++;
                current_state = S_START;
                break;

            case S_CLOSE:
                while (stackTop >= 0 && stack[stackTop] != '(') {
                    output[outputIndex].type = OPERATOR;
                    output[outputIndex].value.operator = stack[stackTop];
                    outputIndex++;
                    stackTop--;
                }
                if (stackTop >= 0) stackTop--; // Bỏ '('
                ptr++;
                if (isOperator(*ptr)) {
                    current_state = S_OPERATOR;
                } else if (*ptr == ')') {
                    current_state = S_CLOSE;
                } else if (*ptr == '\0') {
                    current_state = S_END;
                } else if (isdigit(*ptr) || *ptr == '.' || *ptr == 'x') {
                    current_state = S_OPERAND;
                } else {
                    current_state = S_ERROR;
                }
                break;

            case S_END:
                while (stackTop >= 0) {
                    if (stack[stackTop] != '(') {
                        output[outputIndex].type = OPERATOR;
                        output[outputIndex].value.operator = stack[stackTop];
                        outputIndex++;
                    }
                    stackTop--;
                }
                output[outputIndex].type = OPERATOR;
                output[outputIndex].value.operator = 'E';
                outputIndex++;
                return output;
                break;

            case S_ERROR:
            default:
                printf("Lỗi: Biểu thức đầu vào không hợp lệ!!!\n");
                free(output);
                return NULL;
        }
    }
}

float evaluatePostfix(Token *postfix, float x_value) {
    int stackTop = -1;
    float stack[MAX];
    int i = 0;

    while (postfix[i].type != OPERATOR || postfix[i].value.operator != 'E') {
        if (postfix[i].type == OPERAND) {
            stack[++stackTop] = postfix[i].value.operand;
        } else if (postfix[i].type == VARIABLE) {
            stack[++stackTop] = x_value;
        } else if (postfix[i].type == OPERATOR) {
            if (stackTop < 1) {
                printf("Lỗi: Thiếu toán hạng!\n");
                return NAN;
            }
            float b = stack[stackTop--];
            float a = stack[stackTop--];
            switch (postfix[i].value.operator) {
                case '+': stack[++stackTop] = a + b; break;
                case '-': stack[++stackTop] = a - b; break;
                case '*': stack[++stackTop] = a * b; break;
                case '/': 
                    if (b == 0) {
                        printf("Lỗi: Chia cho 0!\n");
                        return NAN;
                    }
                    stack[++stackTop] = a / b; 
                    break;
                case '^': stack[++stackTop] = powf(a, b); break;
                default: 
                    printf("Lỗi: Toán tử không hợp lệ!\n");
                    return NAN;
            }
        }
        i++;
    }
    if (stackTop != 0) {
        printf("Lỗi: Biểu thức không hợp lệ!\n");
        return NAN;
    }
    return stack[stackTop];
}

void printTokens(Token *output) {
    int i = 0;
    printf("Biểu thức hậu tố: ");
    while (output[i].type != OPERATOR || output[i].value.operator != 'E') {
        if (output[i].type == OPERAND) {
            printf("%.2f ", output[i].value.operand);
        } else if (output[i].type == OPERATOR) {
            printf("%c ", output[i].value.operator);
        } else if (output[i].type == VARIABLE) {
            printf("x ");
        }
        i++;
    }
    printf("\n");
}