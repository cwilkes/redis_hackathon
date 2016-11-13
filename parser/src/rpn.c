#include "redismodule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// parsing taken from http://codereview.stackexchange.com/questions/79738/rpn-calculator-in-c
// str replace from http://stackoverflow.com/questions/779875/what-is-the-function-to-replace-string-in-c

char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig && !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

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

void decodeOLD(RedisModuleCtx *ctx, RedisModuleString  **instring, double *outval, int start, int size) {
}

void decode(RedisModuleCtx *ctx, char **instring, double *outval, int size) {
    RedisModule_Log(ctx, "notice", "decode: input size %d", size);
    int i=0, currStack=0;
    double stack[size/2];
    double temp;
    for (i=0;i<size;i++)
    {       
				RedisModule_Log(ctx, "notice", "index: %d", i);
        const char *val = instring[i];
				RedisModule_Log(ctx, "notice", "hi");
        RedisModule_Log(ctx, "notice", "Index %d value '%s'", i, val);

				if ( ! val[1] && is_operator(val[0] )) {
            RedisModule_Log(ctx, "warning", "doing operation '%s'", val);
						pop(stack, *val, &currStack);
				} else if ( (temp = atof(val)) || ! strcmp(val, "0.0") ) {
            RedisModule_Log(ctx, "warning", "doing push '%.1f'", temp);
						push(stack, temp, &currStack);
				} else {
						RedisModule_Log(ctx, "warning", "Invalid argument:  [%s].", val);
				}   
			  RedisModule_Log(ctx, "warning", "Done with:  [%s].", val);
    }
    *outval = stack[0];
}

char **split(char *input, char *splitter) {
				char **res = NULL;
				char *p = strtok(input, splitter);
				int n_spaces = 0;
				while (p) {
								res = realloc (res, sizeof (char*) * ++n_spaces);
								res[n_spaces-1] = p;
								p = strtok (NULL, splitter);
				}
				return res;
}

char **substitute(RedisModuleCtx *ctx, char *input_phrase, RedisModuleString **var_and_values, int start_pos, int number_keys) {
  RedisModule_Log(ctx, "notice", "input phrase '%s'", input_phrase);
	int i = 0;
	for (i=start_pos; i<start_pos+number_keys; i++) {
	  char key[80];
    sprintf(key, "{{%s}}", RedisModule_StringPtrLen(var_and_values[i], NULL));
		char *temp = RedisModule_StringPtrLen(var_and_values[i+number_keys], NULL);
		RedisModule_Log(ctx, "notice", "Going to swap '%s' with %s", key, temp);
		input_phrase = str_replace(input_phrase, key, temp);
	}
  // now split up that string '5 6 +' into an array of size 3
  RedisModule_Log(ctx, "notice", "input phrase after swap '%s'", input_phrase);
	char **ret = split(input_phrase, " ");
	return ret;
}

double RpnEval(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	double result;
	int i = 2;
	int num_keys = atoi(RedisModule_StringPtrLen(argv[i], NULL));
	RedisModule_Log(ctx, "notice", "number keys %d", num_keys);

  char **val = substitute(ctx, RedisModule_StringPtrLen(argv[1], NULL), argv, 3, num_keys);
  RedisModule_Log(ctx, "warning", "val 0: %s", val[0]);
  RedisModule_Log(ctx, "warning", "val 1: %s", val[1]);
  RedisModule_Log(ctx, "warning", "val 2: %s", val[2]);
	decode(ctx, val, &result, argc-4);
  return RedisModule_ReplyWithDouble(ctx, result);
}


/* RPN: solve RPN posted */
double RpnSolve(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
	double result;
	decodeOLD(ctx, argv, &result, 1, argc);
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
  if (RedisModule_CreateCommand(ctx, "rpn.eval", RpnEval, "readonly", 1,1,1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
	return 0;
}