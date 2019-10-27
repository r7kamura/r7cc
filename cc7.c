#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum to categorize tokens.
typedef enum {
  TOKEN_TYPE_RESERVED_SYMBOL,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_EOF,
} TokenType;

typedef struct Token Token;

struct Token {
  TokenType type;
  Token *next;
  int value;
  char *string;
};

// Current target.
Token *token;

char *user_input;

void report_error(char *location, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int position = location - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", position, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Token *generate_token(TokenType type, Token *current, char *string) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->string = string;
  current->next = token;
  return token;
}

Token *tokenize() {
  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *current = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
    } else if (*p == '+' || *p == '-') {
      current = generate_token(TOKEN_TYPE_RESERVED_SYMBOL, current, p);
      p++;
    } else if (isdigit(*p)) {
      current = generate_token(TOKEN_TYPE_NUMBER, current, p);
      current->value = strtol(p, &p, 10);
    } else {
      report_error(p, "Expected a number.");
    }
  }

  generate_token(TOKEN_TYPE_EOF, current, p);
  return head.next;
}

void raise_error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool at_eof() {
  return token->type == TOKEN_TYPE_EOF;
}

int expect_number() {
  if (token->type != TOKEN_TYPE_NUMBER) {
    report_error(token->string, "Expected a number.");
  }

  int value = token->value;
  token = token->next;
  return value;
}

bool consume(char character) {
  if (token->type == TOKEN_TYPE_RESERVED_SYMBOL &&
      token->string[0] == character) {
    token = token->next;
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    raise_error("Invalid arguments count\n");
  }

  user_input = argv[1];
  token = tokenize();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
    } else if (consume('-')) {
      printf("  sub rax, %d\n", expect_number());
    } else {
      report_error(token->string, "Unexpected character.");
    }
  }

  printf("  ret\n");
  return 0;
}
