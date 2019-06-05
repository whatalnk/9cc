#include "9cc.h"

char *user_input;
Vector *tokens;
int pos;

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
