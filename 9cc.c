#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

enum
{
  TK_NUM = 256,
  TK_EOF,
};

typedef struct
{
  int ty;
  int val;
  char *input;
} Token;

Token tokens[100];

void tokenize(char *p)
{
  int i = 0;
  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-')
    {
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if (isdigit(*p))
    {
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p, 10);
      i++;
      continue;
    }

    fprintf(stderr, "トークナイズできません: %s\n", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}

// 再帰下降構文解析
enum
{
  ND_NUM = 256,
};

typedef struct Node
{
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
} Node;

int pos = 0;

Node *new_node(int ty, Node *lhs, Node *rhs)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Node *number()
{
  if (tokens[pos].ty == TK_NUM)
  {
    return new_node_num(tokens[pos++].val);
  }
  error("数値ではないトークンです: %s\n", tokens[pos].input);
}

Node *expr()
{
  Node *lhs = number();
  while (tokens[pos].ty != TK_EOF)
  {
    if (tokens[pos].ty == '+')
    {
      pos++;
      return new_node('+', lhs, expr());
    }
    if (tokens[pos].ty == '-')
    {
      pos++;
      return new_node('-', lhs, expr());
    }
    break;
  }
  return lhs;
}

// コード生成
void gen(Node *node)
{
  if (node->ty == ND_NUM)
  {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty)
  {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  }
  printf("  push rax\n");
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  tokenize(argv[1]);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
