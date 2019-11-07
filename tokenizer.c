#include "tokenizer.h"
#include <ctype.h>   // isdigit, isspace
#include <stdbool.h> // bool
#include <stdio.h>   // fprintf
#include <stdlib.h>  // exit, strtol
#include <string.h>  // memcmp, strlen

Token *new_token(TokenType type, char *begin, int length) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->string = begin;
  token->length = length;
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

bool starts_and_ends_with(char *string, char *segment) {
  int length = strlen(segment);
  return memcmp(string, segment, length) == 0 && !is_alnum(string[length]);
}

Token *tokenize(char *input) {
  char *p = input;

  Token head;
  head.next = NULL;
  Token *current = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
    } else if (starts_with(p, "==")) {
      current = current->next = new_token(TOKEN_TYPE_EQ, p, 2);
      p += 2;
    } else if (starts_with(p, "!=")) {
      current = current->next = new_token(TOKEN_TYPE_NE, p, 2);
      p += 2;
    } else if (starts_with(p, "<=")) {
      current = current->next = new_token(TOKEN_TYPE_LE, p, 2);
      p += 2;
    } else if (starts_with(p, ">=")) {
      current = current->next = new_token(TOKEN_TYPE_GE, p, 2);
      p += 2;
    } else if (*p == '+') {
      current = current->next = new_token(TOKEN_TYPE_ADD, p, 1);
      p++;
    } else if (*p == '-') {
      current = current->next = new_token(TOKEN_TYPE_SUBTRACT, p, 1);
      p++;
    } else if (*p == '*') {
      current = current->next = new_token(TOKEN_TYPE_MULTIPLY, p, 1);
      p++;
    } else if (*p == '/') {
      current = current->next = new_token(TOKEN_TYPE_DIVIDE, p, 1);
      p++;
    } else if (*p == ';') {
      current = current->next = new_token(TOKEN_TYPE_SEMICOLON, p, 1);
      p++;
    } else if (*p == '=') {
      current = current->next = new_token(TOKEN_TYPE_ASSIGN, p, 1);
      p++;
    } else if (*p == ',') {
      current = current->next = new_token(TOKEN_TYPE_COMMA, p, 1);
      p++;
    } else if (*p == '<') {
      current = current->next = new_token(TOKEN_TYPE_LT, p, 1);
      p++;
    } else if (*p == '>') {
      current = current->next = new_token(TOKEN_TYPE_GT, p, 1);
      p++;
    } else if (*p == '(') {
      current = current->next = new_token(TOKEN_TYPE_PARENTHESIS_LEFT, p, 1);
      p++;
    } else if (*p == ')') {
      current = current->next = new_token(TOKEN_TYPE_PARENTHESIS_RIGHT, p, 1);
      p++;
    } else if (*p == '{') {
      current = current->next = new_token(TOKEN_TYPE_BRACKET_LEFT, p, 1);
      p++;
    } else if (*p == '}') {
      current = current->next = new_token(TOKEN_TYPE_BRACKET_RIGHT, p, 1);
      p++;
    } else if (starts_and_ends_with(p, "if")) {
      current = current->next = new_token(TOKEN_TYPE_IF, p, 2);
      p += 2;
    } else if (starts_and_ends_with(p, "for")) {
      current = current->next = new_token(TOKEN_TYPE_FOR, p, 3);
      p += 3;
    } else if (starts_and_ends_with(p, "else")) {
      current = current->next = new_token(TOKEN_TYPE_ELSE, p, 4);
      p += 4;
    } else if (starts_and_ends_with(p, "while")) {
      current = current->next = new_token(TOKEN_TYPE_WHILE, p, 5);
      p += 5;
    } else if (starts_and_ends_with(p, "return")) {
      current = current->next = new_token(TOKEN_TYPE_RETURN, p, 6);
      p += 6;
    } else if (is_alpha(*p)) {
      char *q = p;
      p++;
      while (is_alnum(*p)) {
        p++;
      }
      current = current->next = new_token(TOKEN_TYPE_IDENTIFIER, q, p - q);
    } else if (isdigit(*p)) {
      char *q = p;
      int value = strtol(p, &p, 10);
      current = current->next = new_token(TOKEN_TYPE_NUMBER, p, p - q);
      current->value = value;
    } else {
      int position = p - input;
      fprintf(stderr, "%s\n", input);
      fprintf(stderr, "%*s^ Unexpected character.\n", position, "");
      exit(1);
    }
  }

  current->next = new_token(TOKEN_TYPE_EOF, p, 0);
  return head.next;
}
