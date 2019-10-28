#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type definitions

typedef enum {
  NODE_TYPE_ADD,
  NODE_TYPE_SUBTRACT,
  NODE_TYPE_MULTIPLY,
  NODE_TYPE_DIVIDE,
  NODE_TYPE_NUMBER,
} NodeType;

typedef struct Node Node;

struct Node {
  NodeType type;
  Node *lhs;
  Node *rhs;
  int value;
};

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

// Proto-types for recursive functions

Node *expression();

Node *multiply_or_devide();

Node *unary();

Node *primary();

// Global variables

Token *token;

char *user_input;

// Utility functions

void raise_error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

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
    } else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
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

// Token consumer functions

bool consume(char character) {
  if (token->type == TOKEN_TYPE_RESERVED_SYMBOL &&
      token->string[0] == character) {
    token = token->next;
    return true;
  }
  return false;
}

bool at_eof() {
  return token->type == TOKEN_TYPE_EOF;
}

void expect(char expected_character) {
  if (token->type == TOKEN_TYPE_RESERVED_SYMBOL && token->string[0] == expected_character) {
    token = token->next;
  } else {
    report_error(token->string, "Expected '%c'", expected_character);
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

// eBNF parts functions

// expression = multiply_or_devide ("+" multiply_or_devide | "-" multiply_or_devide)*
Node *expression() {
  Node *node = multiply_or_devide();

  while (true) {
    if (consume('+')) {
      node = generate_branch_node(NODE_TYPE_ADD, node, multiply_or_devide());
    } else if (consume('-')) {
      node = generate_branch_node(NODE_TYPE_SUBTRACT, node, multiply_or_devide());
    } else {
      break;
    }
  }

  return node;
}

// multiply_or_devide = unary ("*" unary | "/" unary)*
Node *multiply_or_devide() {
  Node *node = unary();

  while (true) {
    if (consume('*')) {
      node = generate_branch_node(NODE_TYPE_MULTIPLY, node, unary());
    } else if (consume('/')) {
      node = generate_branch_node(NODE_TYPE_DIVIDE, node, unary());
    } else {
      break;
    }
  }

  return node;
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume('-')) {
    return generate_branch_node(NODE_TYPE_SUBTRACT, generate_leaf_node(0), primary());
  } else {
    return primary();
  }
}

// primary = number | "(" expression ")"
Node *primary() {
  if (consume('(')) {
    Node *node = expression();
    expect(')');
    return node;
  } else {
    return generate_leaf_node(expect_number());
  }
}

// Code generator functions

void generate_code(Node *node) {
  if (node->type == NODE_TYPE_NUMBER) {
    printf("  push %d\n", node->value);
    return;
  }

  generate_code(node->lhs);
  generate_code(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->type) {
  case NODE_TYPE_ADD:
    printf("  add rax, rdi\n");
    break;
  case NODE_TYPE_SUBTRACT:
    printf("  sub rax, rdi\n");
    break;
  case NODE_TYPE_MULTIPLY:
    printf("  imul rax, rdi\n");
    break;
  case NODE_TYPE_DIVIDE:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    raise_error("Invalid arguments count\n");
  }

  user_input = argv[1];
  token = tokenize();
  Node *node = expression();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  generate_code(node);
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}
