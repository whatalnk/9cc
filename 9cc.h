#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// container.c
typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
int runtest();

// parse.c
// Token
enum {
  TK_NUM = 256,
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
  ND_EQ,
  ND_NE,
  ND_LE,
};

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

Vector *tokenize();
Node *expr();

// codegen.c
void gen(Node *node);

// main.c
extern char *user_input;
extern Vector *tokens;
extern int pos;
void error(char *fmt, ...);
void error_at(char *loc, char *msg);
