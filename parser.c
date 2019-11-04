#include "cc7.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Node *statement();

Node *statement_block();

Node *statement_expression();

Node *statement_for();

Node *statement_if();

Node *statement_return();

Node *statement_while();

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

char *begin;

void report_error(char *location, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int position = location - begin;
  fprintf(stderr, "%s\n", begin);
  fprintf(stderr, "%*s", position, "");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume(TokenType type) {
  if (token->type == type) {
    token = token->next;
    return true;
  }
  return false;
}

bool at_eof() {
  return token->type == TOKEN_TYPE_EOF;
}

void expect(TokenType type) {
  if (token->type == type) {
    token = token->next;
  } else {
    report_error(token->string, "Not-expected token type.");
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
  node->value = local_variable->offset;

  return node;
}

// function_definition = identifier "(" ")" statement_block
Node *function_definition() {
  Node *function_definition = generate_branch_node(NODE_TYPE_FUNCTION_DEFINITION, NULL, NULL);
  function_definition->name = token->string;
  function_definition->name_length = token->length;
  token = token->next;
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  expect(TOKEN_TYPE_BRACKET_LEFT);
  function_definition->lhs = statement_block();
  return function_definition;
}

// program = function_definition*
Node *program() {
  Node *head = generate_branch_node(NODE_TYPE_FUNCTION_DEFINITION, NULL, NULL);
  Node *node = head;
  while (token->type == TOKEN_TYPE_IDENTIFIER) {
    node->rhs = function_definition();
    node = node->rhs;
  }
  return head->rhs;
}

// statement
//   = statement_return
//   | statement_for
//   | statement_if
//   | statement_while
//   | statement_block
//   | statement_expression
Node *statement() {
  if (consume(TOKEN_TYPE_RETURN)) {
    return statement_return();
  } else if (consume(TOKEN_TYPE_FOR)) {
    return statement_for();
  } else if (consume(TOKEN_TYPE_IF)) {
    return statement_if();
  } else if (consume(TOKEN_TYPE_WHILE)) {
    return statement_while();
  } else if (consume(TOKEN_TYPE_BRACKET_LEFT)) {
    return statement_block();
  } else {
    return statement_expression();
  }
}

// statement_block = "{" statement* "}"
Node *statement_block() {
  Node *node = generate_branch_node(NODE_TYPE_BLOCK, NULL, NULL);
  Node *head = node;
  while (!consume(TOKEN_TYPE_BRACKET_RIGHT)) {
    node->rhs = generate_branch_node(NODE_TYPE_STATEMENT, statement(), NULL);
    node = node->rhs;
  }
  return head;
}

// statement_expression = expression ";"
Node *statement_expression() {
  Node *node = expression();
  expect(TOKEN_TYPE_SEMICOLON);
  return node;
}

// statement_for = "for" "(" expression? ";" expression? ";" expression? ")" statement
Node *statement_for() {
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Node *initialization;
  if (consume(TOKEN_TYPE_SEMICOLON)) {
    initialization = NULL;
  } else {
    initialization = expression();
    expect(TOKEN_TYPE_SEMICOLON);
  }

  Node *condition;
  if (consume(TOKEN_TYPE_SEMICOLON)) {
    condition = NULL;
  } else {
    condition = expression();
    expect(TOKEN_TYPE_SEMICOLON);
  }

  Node *afterthrough;
  if (consume(TOKEN_TYPE_SEMICOLON)) {
    afterthrough = NULL;
  } else {
    afterthrough = expression();
  }
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);

  return generate_branch_node(
      NODE_TYPE_FOR,
      initialization,
      generate_branch_node(
          NODE_TYPE_FOR_CONDITION,
          condition,
          generate_branch_node(
              NODE_TYPE_FOR_AFTERTHROUGH,
              afterthrough,
              statement())));
}

// statement_if = "if" "(" expression ")" statement ("else" statement)?
Node *statement_if() {
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Node *node_if_expression = expression();
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  Node *node_if_statement = statement();
  Node *node_if = generate_branch_node(NODE_TYPE_IF, node_if_expression, node_if_statement);
  Node *node_else = NULL;
  if (token->type == TOKEN_TYPE_ELSE) {
    token = token->next;
    node_else = statement();
  }
  return generate_branch_node(NODE_TYPE_IF_ELSE, node_if, node_else);
}

// statement_return = "return" expression ";"
Node *statement_return() {
  Node *node = generate_branch_node(NODE_TYPE_RETURN, expression(), NULL);
  expect(TOKEN_TYPE_SEMICOLON);
  return node;
}

// statement_while = "while" "(" expression ")" statement
Node *statement_while() {
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Node *condition = expression();
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  return generate_branch_node(NODE_TYPE_WHILE, condition, statement());
}

// expression = assign
Node *expression() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume(TOKEN_TYPE_ASSIGN)) {
    node = generate_branch_node(NODE_TYPE_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  while (true) {
    if (consume(TOKEN_TYPE_EQ)) {
      node = generate_branch_node(NODE_TYPE_EQ, node, relational());
    } else if (consume(TOKEN_TYPE_NE)) {
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
    if (consume(TOKEN_TYPE_LT)) {
      node = generate_branch_node(NODE_TYPE_LT, node, add_or_subtract());
    } else if (consume(TOKEN_TYPE_LE)) {
      node = generate_branch_node(NODE_TYPE_LE, node, add_or_subtract());
    } else if (consume(TOKEN_TYPE_GT)) {
      node = generate_branch_node(NODE_TYPE_LT, add_or_subtract(), node);
    } else if (consume(TOKEN_TYPE_GE)) {
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
    if (consume(TOKEN_TYPE_ADD)) {
      node = generate_branch_node(NODE_TYPE_ADD, node, multiply_or_devide());
    } else if (consume(TOKEN_TYPE_SUBTRACT)) {
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
    if (consume(TOKEN_TYPE_MULTIPLY)) {
      node = generate_branch_node(NODE_TYPE_MULTIPLY, node, unary());
    } else if (consume(TOKEN_TYPE_DIVIDE)) {
      node = generate_branch_node(NODE_TYPE_DIVIDE, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume(TOKEN_TYPE_SUBTRACT)) {
    return generate_branch_node(NODE_TYPE_SUBTRACT, generate_leaf_node(0), primary());
  } else {
    return primary();
  }
}

Node *function_call_or_local_variable() {
  if (token->type == TOKEN_TYPE_IDENTIFIER) {
    Token *identifier = token;
    token = token->next;
    if (consume(TOKEN_TYPE_PARENTHESIS_LEFT)) {
      expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
      Node *function_call = generate_branch_node(NODE_TYPE_FUNCTION_CALL, NULL, NULL);
      function_call->name = identifier->string;
      function_call->name_length = identifier->length;
      return function_call;
    } else {
      return generate_local_variable_node(identifier);
    }
  }
  return NULL;
}

Node *expression_in_parentheses() {
  if (consume(TOKEN_TYPE_PARENTHESIS_LEFT)) {
    Node *node = expression();
    expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
    return node;
  }
  return NULL;
}

Node *number() {
  return generate_leaf_node(expect_number());
}

// primary = expression_in_parentheses | function_call_or_local_variable | number
Node *primary() {
  Node *node;
  if (node = expression_in_parentheses()) {
    return node;
  }
  if (node = function_call_or_local_variable()) {
    return node;
  }
  return number();
}

Node *parse(char *input) {
  begin = input;
  token = tokenize(input);
  return program();
}
