#include "redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// taken from http://codereview.stackexchange.com/questions/79738/rpn-calculator-in-c

/* RPN: solve RPN posted */
int RpnSolve(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2) return RedisModule_WrongArity(ctx);
  return RedisModule_ReplyWithString(ctx, argv[1]);
}

/* Registering the module */
int RedisModule_OnLoad(RedisModuleCtx *ctx) {
  if (RedisModule_Init(ctx, "rpn", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
  if (RedisModule_CreateCommand(ctx, "rpn.solve", RpnSolve, "readonly", 1,1,1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
	return 0;
}

void push(float stack[], float value, int *currStack)
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

void pop(float stack[], char operation, int *currStack)
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

    float temp = stack[1];
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

void decode(char **instring, float *outval, int size)
{
    int i=0, currStack=0;
    float stack[size/2];
    float temp;
    for (i=1;i<size;i++)
    {        
				if ( ! instring[i][1] && is_operator(instring[i][0] )) {
						pop(stack, *instring[i], &currStack);
				} else if ( (temp = atof(instring[i])) || ! strcmp(instring[i], "0.0") ) {
						push(stack, temp, &currStack);
				} else {
						printf("Invalid argument:  [%s].", instring[i]);
				}   
    }
    *outval = stack[0];
}