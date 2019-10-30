#include "cc7.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *statement();

Node *assign();

Node *expression();

Node *equality();

Node *relational();

Node *add_or_subtract();

Node *multiply_or_devide();

Node *unary();

Node *primary();

// Global variables

Token *token;

char *user_input;

Node *statements[100];

// Utility functions

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

// Tokenizer functions

Token *generate_token(TokenType type, Token *current, char *string, int length) {
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->string = string;
  token->length = length;
  current->next = token;
  return token;
}

bool starts_with(char *string, char *segment) {
  return memcmp(string, segment, strlen(segment)) == 0;
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
    } else if ('a' <= *p && *p <= 'z') {
      current = generate_token(TOKEN_TYPE_IDENTIFIER, current, p, 1);
      p++;
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

// Token consumer functions

bool consume(char *string) {
  if (token->type == TOKEN_TYPE_RESERVED_SYMBOL && strlen(string) == token->length && !memcmp(token->string, string, token->length)) {
    token = token->next;
    return true;
  }
  return false;
}

bool at_eof() {
  return token->type == TOKEN_TYPE_EOF;
}

void expect(char *string) {
  if (token->type == TOKEN_TYPE_RESERVED_SYMBOL && strlen(string) == token->length && !memcmp(token->string, string, token->length)) {
    token = token->next;
  } else {
    report_error(token->string, "Expected '%s'", string);
  }
}

int expect_number() {
  if (token->type != TOKEN_TYPE_NUMBER) {
    report_error(token->string, "Expected a number.");
  }

  int value = token->value;
  token = token->next;
  return value;
}

// AST builder functions

Node *generate_branch_node(NodeType type, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->type = type;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *generate_leaf_node(int value) {
  Node *node = calloc(1, sizeof(Node));
  node->type = NODE_TYPE_NUMBER;
  node->value = value;
  return node;
}

Node *generate_local_variable_node(char *variable_name) {
  Node *node = calloc(1, sizeof(Node));
  node->type = NODE_TYPE_LOCAL_VARIABLE;
  node->offset = (variable_name[0] - 'a' + 1) * 8;
  return node;
}

// eBNF parts functions

// program = statements*
void *program() {
  int i = 0;
  while (!at_eof()) {
    statements[i] = statement();
    i++;
  }
  statements[i] = NULL;
}

// statement = expression ";"
Node *statement() {
  Node *node = expression();
  expect(";");
  return node;
}

// expression = assign
Node *expression() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    node = generate_branch_node(NODE_TYPE_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  while (true) {
    if (consume("==")) {
      node = generate_branch_node(NODE_TYPE_EQ, node, relational());
    } else if (consume("!=")) {
      node = generate_branch_node(NODE_TYPE_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add_or_subtract ("<" add_or_subtract | "<=" add_or_subtract | ">" add_or_subtract | ">=" add_or_subtract)*
Node *relational() {
  Node *node = add_or_subtract();

  while (true) {
    if (consume("<")) {
      node = generate_branch_node(NODE_TYPE_LT, node, add_or_subtract());
    } else if (consume("<=")) {
      node = generate_branch_node(NODE_TYPE_LE, node, add_or_subtract());
    } else if (consume(">")) {
      node = generate_branch_node(NODE_TYPE_LT, add_or_subtract(), node);
    } else if (consume(">=")) {
      node = generate_branch_node(NODE_TYPE_LE, add_or_subtract(), node);
    } else {
      return node;
    }
  }
}

// add_or_subtract = multiply_or_devide ("+" multiply_or_devide | "-" multiply_or_devide)*
Node *add_or_subtract() {
  Node *node = multiply_or_devide();

  while (true) {
    if (consume("+")) {
      node = generate_branch_node(NODE_TYPE_ADD, node, multiply_or_devide());
    } else if (consume("-")) {
      node = generate_branch_node(NODE_TYPE_SUBTRACT, node, multiply_or_devide());
    } else {
      return node;
    }
  }
}

// multiply_or_devide = unary ("*" unary | "/" unary)*
Node *multiply_or_devide() {
  Node *node = unary();

  while (true) {
    if (consume("*")) {
      node = generate_branch_node(NODE_TYPE_MULTIPLY, node, unary());
    } else if (consume("/")) {
      node = generate_branch_node(NODE_TYPE_DIVIDE, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume("-")) {
    return generate_branch_node(NODE_TYPE_SUBTRACT, generate_leaf_node(0), primary());
  } else {
    return primary();
  }
}

// primary = number | identifier | "(" expression ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expression();
    expect(")");
    return node;
  } else if (token->type == TOKEN_TYPE_IDENTIFIER) {
    Node *node = generate_local_variable_node(token->string);
    token = token->next;
    return node;
  } else {
    return generate_leaf_node(expect_number());
  }
}

void parse(char *string) {
  user_input = string;
  token = tokenize();
  program();
}
