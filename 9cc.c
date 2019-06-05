#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

char *user_input;

Vector *tokens;

int pos = 0;

// functions for handling error
// same arg as printf
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *msg) {
  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ %s\n", msg);
  exit(1);
}

Vector *new_vector();
void vec_push(Vector *vec, void *elem);

Vector *tokenize() {
  char *p = user_input;
  Vector *v = new_vector();
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (!strncmp(p, "==", 2)) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_EQ;
      t->input = p;
      vec_push(v, t);
      p += 2;
      continue;
    }
    if (!strncmp(p, "!=", 2)) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_NE;
      t->input = p;
      vec_push(v, t);
      p += 2;
      continue;
    }
    if (!strncmp(p, "<=", 2)) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_LE;
      t->input = p;
      vec_push(v, t);
      p += 2;
      continue;
    }
    if (!strncmp(p, ">=", 2)) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_GE;
      t->input = p;
      vec_push(v, t);
      p += 2;
      continue;
    }
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == '<' || *p == '>') {
      Token *t = malloc(sizeof(Token));
      t->ty = *p;
      t->input = p;
      vec_push(v, t);
      p++;
      continue;
    }
    if (isdigit(*p)) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_NUM;
      t->input = p;
      t->val = strtol(p, &p, 10);
      vec_push(v, t);
      continue;
    }
    error_at(p, "Could not tokenize");
  }
  Token *t = malloc(sizeof(Token));
  t->ty = TK_EOF;
  t->input = p;
  vec_push(v, t);
  return v;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

int consume(int ty) {
  Token *t = tokens->data[pos];
  if (t->ty != ty) {
    return 0;
  }
  pos++;
  return 1;
}

Node *expr();

Node *term() {
  if (consume('(')) {
    Node *node = expr();
    if (!consume(')')) {
      Token *t = tokens->data[pos];
      error_at(t->input, "No close parentheses found");
    }
    return node;
  }
  Token *t = tokens->data[pos];
  if (t->ty == TK_NUM) {
    pos++;
    return new_node_num(t->val);
  }
  error_at(t->input, "Not a number token or parenthesis");
}

Node *unary() {
  if (consume('+')) {
    return term();
  }
  if (consume('-')) {
    return new_node('-', new_node_num(0), term());
  }
  return term();
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume('*')) {
      node = new_node('*', node, unary());
    } else if (consume('/')) {
      node = new_node('/', node, unary());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node('+', node, mul());
    } else if (consume('-')) {
      node = new_node('-', node, mul());
    } else {
      return node;
    }
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume('<')) {
      node = new_node('<', node, add());
    } else if (consume('>')) {
      node = new_node('<', add(), node);
    } else if (consume(TK_LE)) {
      node = new_node(ND_LE, node, add());
    } else if (consume(TK_GE)) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

Node *equality() {
  Node *node = relational();
  for (;;) {
    if (consume(TK_EQ)) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume(TK_NE)) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

Node *expr() {
  Node *node = equality();
  return node;
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  imul rdi\n");
    break;
  case '/':
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case '<':
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

int expect(int line, int expected, int actual) {
  if (expected == actual) {
    return 0;
  }
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);
}

int runtest() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; i++) {
    vec_push(vec, (void *)i);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (long)vec->data[0]);
  expect(__LINE__, 50, (long)vec->data[50]);
  expect(__LINE__, 99, (long)vec->data[99]);

  printf("OK\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    error("引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  if (!strcmp(user_input, "-test")) {
    runtest();
    return 0;
  }
  tokens = tokenize();
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
