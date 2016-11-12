#include "redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// taken from http://codereview.stackexchange.com/questions/79738/rpn-calculator-in-c

void push(double stack[], double value, int *currStack)
{
    int i = *currStack;

    while (i != 0)
    {
        stack[i] = stack[i-1];
        i--;
    }

    stack[0] = value;
    *currStack += 1;
}

void pop(double stack[], char operation, int *currStack)
{
    int i;

    switch (operation)
    {
        case '+':
            stack[0] = stack[1] + stack[0];
            break;
        case '-':
            stack[0] = stack[1] - stack[0];
            break;
        case '*':
            stack[0] = stack[1] * stack[0];
            break;
        case '/':
            stack[0] = stack[1] / stack[0];
            break;
        default:
            printf("Invalid character.");
    }

    double temp = stack[1];
    for (i=1; i < *currStack; i++) {
        stack[i] = stack[i+1];
    }
    stack[*currStack] = temp;

    *currStack -= 1;
}

int is_operator(char c) {
    switch (c) {
        case '+':
        case '-':
        case '*':
        case '/':
            return 1;
        default:
            return 0;
    }
}

void decode(RedisModuleCtx *ctx, RedisModuleString **instring, double *outval, int size)
{
    int i=0, currStack=0;
    double stack[size/2];
    double temp;
		size_t len;
    for (i=1;i<size;i++)
    {       
        const char *val = RedisModule_StringPtrLen(instring[i], &len);
				if ( ! val[1] && is_operator(val[0] )) {
						pop(stack, *val, &currStack);
				} else if ( (temp = atof(val)) || ! strcmp(val, "0.0") ) {
						push(stack, temp, &currStack);
				} else {
						RedisModule_Log(ctx, "warning", "Invalid argument:  [%s].", val);
				}   
    }
    *outval = stack[0];
}

/* RPN: solve RPN posted */
double RpnSolve(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	double result;
	size_t len;
	decode(ctx, argv, &result, argc);
  return RedisModule_ReplyWithDouble(ctx, result);
}

/* Registering the module */
int RedisModule_OnLoad(RedisModuleCtx *ctx) {
  if (RedisModule_Init(ctx, "rpn", 3, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
  if (RedisModule_CreateCommand(ctx, "rpn.solve", RpnSolve, "readonly", 1,1,1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
	return 0;
}