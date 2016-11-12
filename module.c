#include "redismodule.h"
/* ECHO <string> - Echo back a string sent from the client */
int EchoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
  if (argc < 2) return RedisModule_WrongArity(ctx);
  return RedisModule_ReplyWithString(ctx, argv[1]);
}

/* Registering the module */
int RedisModule_OnLoad(RedisModuleCtx *ctx) {
  if (RedisModule_Init(ctx, "example", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
  if (RedisModule_CreateCommand(ctx, "example.echo", EchoCommand, "readonly", 1,1,1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
}