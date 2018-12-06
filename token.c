#include "9cc.h"

static Vector *tokens;
static int pos;

struct
{
  char *name;
  int ty;
} symbols[] = {
    {"==", TK_EQ},
    {"!=", TK_NE},
    {NULL, 0}};

Vector *tokenize(char *p)
{
  Vector *v = new_vector();
loop:
  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }

    // Multi-letter operator
    for (int j = 0; symbols[j].name; j++)
    {
      char *name = symbols[j].name;
      int len = strlen(name);
      if (strncmp(p, name, len))
      {
        continue;
      }
      Token *t = malloc(sizeof(Token));
      t->ty = symbols[j].ty;
      t->input = p;
      vec_push(v, t);
      p += len;
      goto loop;
    }

    // Single operatpr
    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';' || *p == '=')
    {
      Token *t = malloc(sizeof(Token));
      t->ty = *p;
      t->input = p;
      vec_push(v, t);
      p++;
      continue;
    }

    if (isdigit(*p))
    {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_NUM;
      t->input = p;
      t->val = strtol(p, &p, 10);
      vec_push(v, t);
      continue;
    }

    if ('a' <= *p && *p <= 'z')
    {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_IDENT;
      t->input = p;
      int len = 1;
      char *name = strndup(p, len);
      t->name = *name;
      vec_push(v, t);
      p++;
      continue;
    }

    fprintf(stderr, "トークナイズできません: %s\n", p);
    exit(1);
  }

  Token *t = malloc(sizeof(Token));
  t->ty = TK_EOF;
  t->input = p;
  vec_push(v, t);
  return v;
}