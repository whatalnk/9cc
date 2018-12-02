#include "9cc.h"

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

void *program()
{
  int code_pos = 0;
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