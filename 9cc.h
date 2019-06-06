#define _GNU_SOURCE
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

// container.c
typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void runtest();

// parse.c
// Token
enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_RETURN,
  TK_EQ,
  TK_NE,
  TK_LE,
  TK_GE,
  TK_EOF,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

// Node
enum {
  ND_NUM = 256,
  ND_IDENT,
  ND_RETURN,
  ND_EQ,
  ND_NE,
  ND_LE,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char *name;
} Node;

Vector *tokenize();
Node *expr();
void program();

extern Node *code[100];

// codegen.c
void gen(Node *node);

// main.c
extern char *user_input;
extern Vector *tokens;
extern int pos;
void error(char *fmt, ...);
noreturn void error_at(char *loc, char *msg);
