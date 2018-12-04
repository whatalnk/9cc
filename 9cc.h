#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

enum
{
  TK_NUM = 256,
  TK_IDENT,
  TK_EQ,
  TK_NE,
  TK_EOF,
};

typedef struct
{
  int ty;
  int val;
  char *name;
  char *input;
} Token;

enum
{
  ND_NUM = 256,
  ND_IDENT,
  ND_EQ,
  ND_NE,
};

typedef struct Node
{
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char name;
} Node;

// parse.c
extern Token tokens[];
extern Node *code[];

void tokenize(char *p);
void *program();

// codegen.c
void gen(Node *node);

// main.c
void error(char *fmt, ...);

// util.c

typedef struct
{
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

typedef struct
{
  Vector *keys;
  Vector *vals;
} Map;

Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

void runtest();