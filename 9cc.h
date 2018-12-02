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
  TK_EOF,
};

typedef struct
{
  int ty;
  int val;
  char *input;
} Token;

enum
{
  ND_NUM = 256,
  ND_IDENT,
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
