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

Token *generate_token(TokenType type, Token *current, char *string) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->string = string;
  current->next = token;
  return token;
}

Token *tokenize(char *p) {
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
    raise_error("Expected TOKEN_TYPE_NUMBER, but not.");
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

  token = tokenize(argv[1]);

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
      raise_error("Unexpected character: '%c'\n", token->string[0]);
    }
  }

  printf("  ret\n");
  return 0;
}
