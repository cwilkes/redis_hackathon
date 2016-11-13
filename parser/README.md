# INSTALL
`make`

# RUN
`/path/to/redis-unstable/src/redis-server  --loadmodule ./src/rpn.so`

# EXAMPLES

```
TODO: fix this to call decode() instead of decodeOLD() that no longer work
redis-cli RPN.SOLVE 1 2 +

```

```$ redis-cli RPN.EVAL '{{a}} {{b}} + {{c}} *' 3 a b c 50 60 2
"220"```