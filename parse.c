#include "9cc.h"

static Vector *tokens;
static int pos;

static void expect(int ty)
{
  Token *t = tokens->data[pos];
  if (t->ty != ty)
    error("%c (%d) expected, but got %c (%d)", ty, ty, t->ty, t->ty);
  pos++;
}

// 再帰下降構文解析

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

Node *new_node_ident(char *name)
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

Node *expr();

Node *term()
{
  Token *t = tokens->data[pos];

  if (t->ty == TK_NUM)
  {
    pos++;
    return new_node_num(t->val);
  }
  if (t->ty == TK_IDENT)
  {
    pos++;
    return new_node_ident(t->name);
  }

  if (t->ty == '(')
  {
    pos++;
    Node *node = expr();
    expect(')');
    return node;
  }
  error("数値でも開きカッコでもないトークンです: %s\n", t->input);
}

Node *mul()
{
  Node *lhs = term();
  Token *t = tokens->data[pos];
  if (t->ty == '*')
  {
    pos++;
    return new_node('*', lhs, mul());
  }
  if (t->ty == '/')
  {
    pos++;
    return new_node('/', lhs, mul());
  }
  return lhs;
}

Node *expr()
{
  Node *lhs = mul();
  Token *t = tokens->data[pos];
  if (t->ty == '+')
  {
    pos++;
    return new_node('+', lhs, expr());
  }
  if (t->ty == '-')
  {
    pos++;
    return new_node('-', lhs, expr());
  }
  return lhs;
}

Node *equality()
{
  Node *lhs = expr();
  Token *t = tokens->data[pos];
  if (t->ty == TK_EQ)
  {
    pos++;
    return new_node(ND_EQ, lhs, expr());
  }
  if (t->ty == TK_NE)
  {
    pos++;
    return new_node(ND_NE, lhs, expr());
  }
  return lhs;
}

Node *assign()
{
  Node *lhs = equality();
  Token *t = tokens->data[pos];
  if (t->ty == '=')
  {
    pos++;
    return new_node('=', lhs, assign());
  }
  return lhs;
}

Node *program()
{
  Node *node = malloc(sizeof(Node));
  node->ty = ND_COMP_STMT;
  node->stmts = new_vector();

  for (;;)
  {
    Token *t = tokens->data[pos];
    if (t->ty == TK_EOF)
    {
      return node;
    }
    Node *e = assign();
    expect(';');
    vec_push(node->stmts, e);
  }
}

Node *parse(Vector *v)
{
  tokens = v;
  pos = 0;
  return program();
}