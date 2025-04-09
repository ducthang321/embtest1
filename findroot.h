#ifndef FINDROOT_H
#define FINDROOT_H

#include "postfix.h"

float newtonRaphson(Token *postfix);
float bisectionMethod(Token *postfix);
float secantMethod(Token *postfix);

#endif