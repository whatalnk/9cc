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

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=')
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

    if ('a' <= *p && *p <= 'z')
    {
      tokens[i].ty = TK_IDENT;
      tokens[i].input = p;
      i++;
      p++;
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

Node *new_node_ident(char *input)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  // 変数は1文字
  int len = 1;
  char *name = strndup(input, len);
  node->name = *name;
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

Node *expr();

Node *term()
{
  if (tokens[pos].ty == TK_NUM)
  {
    return new_node_num(tokens[pos++].val);
  }
  if (tokens[pos].ty == TK_IDENT)
  {
    return new_node_ident(tokens[pos++].input);
  }
  if (tokens[pos].ty == '(')
  {
    pos++;
    Node *node = expr();
    if (tokens[pos].ty != ')')
    {
      error("開きカッコに対応する閉じカッコがありません: %s", tokens[pos].input);
    }
    pos++;
    return node;
  }
  error("数値でも開きカッコでもないトークンです: %s\n", tokens[pos].input);
}

Node *mul()
{
  Node *lhs = term();
  if (tokens[pos].ty == '*')
  {
    pos++;
    return new_node('*', lhs, mul());
  }
  if (tokens[pos].ty == '/')
  {
    pos++;
    return new_node('/', lhs, mul());
  }
  return lhs;
}

Node *expr()
{
  Node *lhs = mul();
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
  return lhs;
}

Node *assign()
{
  Node *lhs = expr();
  if (tokens[pos].ty == '=')
  {
    pos++;
    return new_node('=', lhs, assign());
  }
  if (tokens[pos].ty == ';')
  {
    pos++;
  }
  return lhs;
}

Node *code[100];

int code_pos = 0;

void *program()
{
  Node *node = assign();
  code[code_pos] = node;
  code_pos++;
  while (tokens[pos].ty != TK_EOF)
  {
    Node *node = assign();
    code[code_pos] = node;
    code_pos++;
  }
  code[code_pos] = NULL;
}

// コード生成
void gen_lval(Node *node)
{
  if (node->ty == ND_IDENT)
  {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", ('z' - node->name + 1) * 8);
    printf("  push rax\n");
    return;
  }
  error("代入の左辺値が変数ではありません");
}

void gen(Node *node)
{
  if (node->ty == ND_NUM)
  {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT)
  {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=')
  {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
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
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
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
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; code[i]; i++)
  {
    gen(code[i]);
    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
