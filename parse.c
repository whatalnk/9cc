#include "9cc.h"

Map *vars;
int bpoff;

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

Vector *tokenize() {
  char *p = user_input;
  Vector *v = new_vector();
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }
    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      Token *t = malloc(sizeof(Token));
      t->ty = TK_RETURN;
      t->input = p;
      vec_push(v, t);
      p += 6;
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
        *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=') {
      Token *t = malloc(sizeof(Token));
      t->ty = *p;
      t->input = p;
      vec_push(v, t);
      p++;
      continue;
    }
    // variable
    if (isalpha(*p) || *p == '_') {
      int len = 1;
      while (isalpha(p[len]) || isdigit(p[len]) || p[len] == '_') {
        len++;
      }
      char *name = strndup(p, len);
      Token *t = malloc(sizeof(Token));
      t->ty = TK_IDENT;
      t->input = p;
      t->name = name;
      vec_push(v, t);
      p += len;
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
  if (t->ty == TK_IDENT) {
    pos++;
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = t->name;
    if (!map_exists(vars, t->name)) {
      map_put(vars, t->name, (void *)(intptr_t)bpoff);
      bpoff += 8;
    }
    return node;
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

Node *assign() {
  Node *node = equality();
  if (consume('=')) {
    node = new_node('=', node, assign());
  }
  return node;
}

Node *expr() {
  Node *node = assign();
  return node;
}

Node *stmt() {
  Node *node;
  if (consume(TK_RETURN)) {
    node = malloc(sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }
  if (!consume(';')) {
    Token *t = tokens->data[pos];
    error_at(t->input, "Token is not ';'");
  }
  return node;
}

void program() {
  int i = 0;
  bpoff = 0;
  vars = new_map();
  for (;;) {
    Token *t = tokens->data[pos];
    if (t->ty == TK_EOF) {
      code[i] = NULL;
      break;
    }
    code[i++] = stmt();
  }
}