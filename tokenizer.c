#include "cc7.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

Token *generate_token(TokenType type, Token *current, char *string, int length) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->string = string;
  token->length = length;
  current->next = token;
  return token;
}

bool is_alpha(char character) {
  return 'a' <= character && character <= 'z' || 'A' <= character && character <= 'Z' || character == '_';
}

bool is_alnum(char character) {
  return is_alpha(character) || '0' <= character && character <= '9';
}

bool starts_with(char *string, char *segment) {
  return memcmp(string, segment, strlen(segment)) == 0;
}

bool starts_with_keyword(char *string, char *segment) {
  int length = strlen(segment);
  return memcmp(string, segment, length) == 0 && !is_alnum(string[length]);
}

Token *tokenize() {
  char *p = user_input;

  Token head;
  head.next = NULL;
  Token *current = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
    } else if (starts_with(p, "==") || starts_with(p, "!=") || starts_with(p, "<=") || starts_with(p, ">=")) {
      current = generate_token(TOKEN_TYPE_RESERVED_SYMBOL, current, p, 2);
      p += 2;
    } else if (strchr("+-*/()<>;=", *p)) {
      current = generate_token(TOKEN_TYPE_RESERVED_SYMBOL, current, p, 1);
      p++;
    } else if (starts_with_keyword(p, "if")) {
      current = generate_token(TOKEN_TYPE_IF, current, p, 2);
      p += 2;
    } else if (starts_with_keyword(p, "else")) {
      current = generate_token(TOKEN_TYPE_ELSE, current, p, 4);
      p += 4;
    } else if (starts_with_keyword(p, "return")) {
      current = generate_token(TOKEN_TYPE_RETURN, current, p, 6);
      p += 6;
    } else if (is_alpha(*p)) {
      char *q = p;
      p++;
      while (is_alnum(*p)) {
        p++;
      }
      current = generate_token(TOKEN_TYPE_IDENTIFIER, current, q, p - q);
    } else if (isdigit(*p)) {
      current = generate_token(TOKEN_TYPE_NUMBER, current, p, 0);
      char *q = p;
      current->value = strtol(p, &p, 10);
      current->length = p - q;
    } else {
      report_error(p, "Expected a number.");
    }
  }

  generate_token(TOKEN_TYPE_EOF, current, p, 0);
  return head.next;
}
