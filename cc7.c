#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Type definitions

typedef enum {
  NODE_TYPE_ADD,
  NODE_TYPE_DIVIDE,
  NODE_TYPE_EQ,
  NODE_TYPE_LE,
  NODE_TYPE_LT,
  NODE_TYPE_MULTIPLY,
  NODE_TYPE_NE,
  NODE_TYPE_NUMBER,
  NODE_TYPE_SUBTRACT,
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
  int length;
};

// Proto-types for recursive functions

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
    } else if (strchr("+-*/()<>", *p)) {
      current = generate_token(TOKEN_TYPE_RESERVED_SYMBOL, current, p, 1);
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

// eBNF parts functions

// expression = equality
Node *expression() {
  return equality();
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

// primary = number | "(" expression ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expression();
    expect(")");
    return node;
  } else {
    return generate_leaf_node(expect_number());
  }
}

// Code generator functions

void generate_fragment_code(Node *node) {
  if (node->type == NODE_TYPE_NUMBER) {
    printf("  push %d\n", node->value);
    return;
  }

  generate_fragment_code(node->lhs);
  generate_fragment_code(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->type) {
  case NODE_TYPE_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
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

void generate_entire_code(Node *node) {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  generate_fragment_code(node);
  printf("  pop rax\n");
  printf("  ret\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Expected arguments count is 2, got %i\n", argc);
    exit(1);
  }

  user_input = argv[1];
  token = tokenize();
  generate_entire_code(expression());

  return 0;
}
