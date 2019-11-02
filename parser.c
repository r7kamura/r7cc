#include "cc7.h"
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

LocalVariable *local_variables;

Token *token;

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

LocalVariable *find_local_variable(Token *token) {
  for (LocalVariable *local_variable = local_variables; local_variable; local_variable = local_variable->next) {
    if (local_variable->length == token->length && !memcmp(local_variable->name, token->string, local_variable->length)) {
      return local_variable;
    }
  }
  return NULL;
}

Node *generate_local_variable_node(Token *token) {
  Node *node = calloc(1, sizeof(Node));

  node->type = NODE_TYPE_LOCAL_VARIABLE;

  LocalVariable *local_variable = find_local_variable(token);
  if (!local_variable) {
    local_variable = calloc(1, sizeof(LocalVariable));
    local_variable->name = token->string;
    local_variable->length = token->length;
    local_variable->offset = local_variables ? local_variables->offset + 8 : 8;
    local_variable->next = local_variables;
    local_variables = local_variable;
  }
  node->offset = local_variable->offset;

  return node;
}

// program = statements*
Statement *program() {
  Statement head;
  Statement *current = &head;

  while (!at_eof()) {
    current->next = calloc(1, sizeof(Statement));
    current = current->next;
    current->node = statement();
  }

  return head.next;
}

// statement = "return" expression ";"
//   | "if" "(" expression ")" statement ("else" statement)?
//   | expression ";"
Node *statement() {
  Node *node;
  if (token->type == TOKEN_TYPE_RETURN) {
    token = token->next;
    node = generate_branch_node(NODE_TYPE_RETURN, expression(), NULL);
    expect(";");
  } else if (token->type == TOKEN_TYPE_IF) {
    token = token->next;
    expect("(");
    Node *node_if_expression = expression();
    expect(")");
    Node *node_if_statement = statement();
    Node *node_if = generate_branch_node(NODE_TYPE_IF, node_if_expression, node_if_statement);
    Node *node_else = NULL;
    if (token->type == TOKEN_TYPE_ELSE) {
      token = token->next;
      node_else = statement();
    }
    node = generate_branch_node(NODE_TYPE_IF_ELSE, node_if, node_else);
  } else {
    node = expression();
    expect(";");
  }
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
    Node *node = generate_local_variable_node(token);
    token = token->next;
    return node;
  } else {
    return generate_leaf_node(expect_number());
  }
}

Statement *parse(char *string) {
  user_input = string;
  token = tokenize();
  return program();
}
