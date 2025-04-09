#ifndef POSTFIX_H
#define POSTFIX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 100

typedef enum {S_START, S_OPERAND, S_OPERATOR, S_OPEN, S_CLOSE, S_ERROR, S_END} state_t;

typedef enum {
    OPERAND,
    OPERATOR,
    VARIABLE
} TokenType;

typedef struct {
    TokenType type;
    union {
        float operand;
        char operator;
        float variable;
    } value;
} Token;

int isOperator(char c);
int precedence(char op);
Token *infixToPostfix(char* myFunction);
float evaluatePostfix(Token *postfix, float x_value);
void printTokens(Token *output);

#endif // POSTFIX_H
