#pragma once

typedef enum {
  TOKEN_KIND_AMPERSAND,
  TOKEN_KIND_ASSIGN,
  TOKEN_KIND_ASTERISK,
  TOKEN_KIND_BRACE_LEFT,
  TOKEN_KIND_BRACE_RIGHT,
  TOKEN_KIND_BRACKET_LEFT,
  TOKEN_KIND_BRACKET_RIGHT,
  TOKEN_KIND_CHAR,
  TOKEN_KIND_COMMA,
  TOKEN_KIND_ELSE,
  TOKEN_KIND_EOF,
  TOKEN_KIND_EQ,
  TOKEN_KIND_FOR,
  TOKEN_KIND_GE,
  TOKEN_KIND_GT,
  TOKEN_KIND_IDENTIFIER,
  TOKEN_KIND_IF,
  TOKEN_KIND_INTEGER,
  TOKEN_KIND_LE,
  TOKEN_KIND_LT,
  TOKEN_KIND_MINUS,
  TOKEN_KIND_NE,
  TOKEN_KIND_NUMBER,
  TOKEN_KIND_PARENTHESIS_LEFT,
  TOKEN_KIND_PARENTHESIS_RIGHT,
  TOKEN_KIND_PLUS,
  TOKEN_KIND_RETURN,
  TOKEN_KIND_SEMICOLON,
  TOKEN_KIND_SIZEOF,
  TOKEN_KIND_SLASH,
  TOKEN_KIND_WHILE,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  char *string;
  int length;
  int value;
};

Token *tokenize();
