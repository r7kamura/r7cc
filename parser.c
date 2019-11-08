#include "parser.h"
#include "tokenizer.h" // Token, TokenType
#include <stdarg.h>    // va_list
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

Token *token;

char *begin;

Scope *scope;

void error(char *position, char *message) {
  int index = position - begin;
  fprintf(stderr, "%s\n", begin);
  fprintf(stderr, "%*s^ %s\n", index, "", message);
  exit(1);
}

Token *consume(TokenType type) {
  Token *token_;
  if (token->type == type) {
    token_ = token;
    token = token->next;
    return token_;
  }
  return NULL;
}

void expect(TokenType type) {
  if (token->type != type) {
    error(token->string, "Unexpected token type.");
  }
  token = token->next;
}

int expect_number() {
  if (token->type != TOKEN_TYPE_NUMBER) {
    error(token->string, "Expected number token.");
  }
  int value = token->value;
  token = token->next;
  return value;
}

LocalVariable *find_local_variable(Token *token) {
  for (LocalVariable *local_variable = scope->local_variable; local_variable; local_variable = local_variable->next) {
    if (local_variable->length == token->length && !memcmp(local_variable->name, token->string, local_variable->length)) {
      return local_variable;
    }
  }
  return NULL;
}

Scope *new_scope(Scope *parent) {
  Scope *scope_ = calloc(1, sizeof(Scope));
  scope_->parent = parent;
  return scope_;
}

Node *new_node(NodeType type) {
  Node *node = calloc(1, sizeof(Node));
  node->type = type;
  return node;
}

Node *new_binary_node(NodeType type, Node *lhs, Node *rhs) {
  Node *node = new_node(type);
  node->binary.lhs = lhs;
  node->binary.rhs = rhs;
  return node;
}

Node *new_unary_node(NodeType type, Node *child) {
  Node *node = new_node(type);
  node->node = child;
  return node;
}

Node *new_number_node(int value) {
  Node *node = new_node(NODE_TYPE_NUMBER);
  node->value = value;
  return node;
}

Node *new_local_variable_node(Token *identifier) {
  Node *node = new_node(NODE_TYPE_LOCAL_VARIABLE);
  LocalVariable *local_variable = find_local_variable(identifier);
  if (local_variable == NULL) {
    local_variable = calloc(1, sizeof(LocalVariable));
    local_variable->name = identifier->string;
    local_variable->length = identifier->length;
    local_variable->offset = scope->local_variable == NULL ? 8 : scope->local_variable->offset + 8;
    local_variable->next = scope->local_variable;
    scope->local_variable = local_variable;
  }
  node->offset = local_variable->offset;
}

Nodes *new_nodes() {
  return calloc(1, sizeof(Nodes));
}

// function_definition = identifier "(" (identifier ("," identifier)*)? ")" statement_block
Node *function_definition() {
  Token *identifier = consume(TOKEN_TYPE_IDENTIFIER);
  Node *node = new_node(NODE_TYPE_FUNCTION_DEFINITION);
  node->function_definition.name = identifier->string;
  node->function_definition.name_length = identifier->length;

  scope = new_scope(scope);
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (consume(TOKEN_TYPE_COMMA) != NULL || consume(TOKEN_TYPE_PARENTHESIS_RIGHT) == NULL) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = new_local_variable_node(consume(TOKEN_TYPE_IDENTIFIER));
  }
  node->function_definition.parameters = head->next;
  node->function_definition.block = statement_block();
  node->function_definition.scope = scope;
  scope = scope->parent;

  return node;
}

// program = function_definition*
Node *program() {
  scope = new_scope(NULL);
  Node *node = new_node(NODE_TYPE_PROGRAM);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (token->type == TOKEN_TYPE_IDENTIFIER) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = function_definition();
  }
  node->program.nodes = head->next;
  return node;
}

// statement
//   = statement_return
//   | statement_for
//   | statement_if
//   | statement_while
//   | statement_block
//   | statement_expression
Node *statement() {
  switch (token->type) {
  case TOKEN_TYPE_RETURN:
    return statement_return();
  case TOKEN_TYPE_FOR:
    return statement_for();
  case TOKEN_TYPE_IF:
    return statement_if();
  case TOKEN_TYPE_WHILE:
    return statement_while();
  case TOKEN_TYPE_BRACKET_LEFT:
    return statement_block();
  default:
    return statement_expression();
  }
}

// statement_block = "{" statement* "}"
Node *statement_block() {
  expect(TOKEN_TYPE_BRACKET_LEFT);
  Node *node = new_node(NODE_TYPE_BLOCK);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (!consume(TOKEN_TYPE_BRACKET_RIGHT)) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = statement();
  }
  node->block.nodes = head->next;
  return node;
}

// statement_expression = expression ";"
Node *statement_expression() {
  Node *node = expression();
  expect(TOKEN_TYPE_SEMICOLON);
  return node;
}

// statement_for = "for" "(" expression? ";" expression? ";" expression? ")" statement
Node *statement_for() {
  Node *node = new_node(NODE_TYPE_FOR);
  expect(TOKEN_TYPE_FOR);
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);

  Node *initialization;
  if (consume(TOKEN_TYPE_SEMICOLON)) {
    initialization = NULL;
  } else {
    initialization = expression();
    expect(TOKEN_TYPE_SEMICOLON);
  }
  node->for_statement.initialization = initialization;

  Node *condition;
  if (consume(TOKEN_TYPE_SEMICOLON)) {
    condition = NULL;
  } else {
    condition = expression();
    expect(TOKEN_TYPE_SEMICOLON);
  }
  node->for_statement.condition = condition;

  Node *afterthrough;
  if (consume(TOKEN_TYPE_PARENTHESIS_RIGHT)) {
    afterthrough = NULL;
  } else {
    afterthrough = expression();
    expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  }
  node->for_statement.afterthrough = afterthrough;

  node->for_statement.statement = statement();

  return node;
}

// statement_if = "if" "(" expression ")" statement ("else" statement)?
Node *statement_if() {
  expect(TOKEN_TYPE_IF);
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Node *node = new_node(NODE_TYPE_IF);
  node->if_statement.condition = expression();
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  node->if_statement.true_statement = statement();
  if (consume(TOKEN_TYPE_ELSE)) {
    node->if_statement.false_statement = statement();
  }
  return node;
}

// statement_return = "return" expression ";"
Node *statement_return() {
  expect(TOKEN_TYPE_RETURN);
  Node *node = new_node(NODE_TYPE_RETURN);
  node->return_statement.expression = expression();
  expect(TOKEN_TYPE_SEMICOLON);
  return node;
}

// statement_while = "while" "(" expression ")" statement
Node *statement_while() {
  expect(TOKEN_TYPE_WHILE);
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Node *node = new_node(NODE_TYPE_WHILE);
  node->while_statement.condition = expression();
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  node->while_statement.statement = statement();
  return node;
}

// expression = assign
Node *expression() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume(TOKEN_TYPE_ASSIGN)) {
    if (node->type != NODE_TYPE_LOCAL_VARIABLE) {
      fprintf(stderr, "Left value in assignment must be a local variable.");
      exit(1);
    }
    node = new_binary_node(NODE_TYPE_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  while (true) {
    if (consume(TOKEN_TYPE_EQ)) {
      node = new_binary_node(NODE_TYPE_EQ, node, relational());
    } else if (consume(TOKEN_TYPE_NE)) {
      node = new_binary_node(NODE_TYPE_NE, node, relational());
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
      node = new_binary_node(NODE_TYPE_LT, node, add_or_subtract());
    } else if (consume(TOKEN_TYPE_LE)) {
      node = new_binary_node(NODE_TYPE_LE, node, add_or_subtract());
    } else if (consume(TOKEN_TYPE_GT)) {
      node = new_binary_node(NODE_TYPE_LT, add_or_subtract(), node);
    } else if (consume(TOKEN_TYPE_GE)) {
      node = new_binary_node(NODE_TYPE_LE, add_or_subtract(), node);
    } else {
      return node;
    }
  }
}

// add_or_subtract = multiply_or_devide ("+" multiply_or_devide | "-" multiply_or_devide)*
Node *add_or_subtract() {
  Node *node = multiply_or_devide();

  while (true) {
    if (consume(TOKEN_TYPE_PLUS)) {
      node = new_binary_node(NODE_TYPE_ADD, node, multiply_or_devide());
    } else if (consume(TOKEN_TYPE_MINUS)) {
      node = new_binary_node(NODE_TYPE_SUBTRACT, node, multiply_or_devide());
    } else {
      return node;
    }
  }
}

// multiply_or_devide = unary ("*" unary | "/" unary)*
Node *multiply_or_devide() {
  Node *node = unary();

  while (true) {
    if (consume(TOKEN_TYPE_ASTERISK)) {
      node = new_binary_node(NODE_TYPE_MULTIPLY, node, unary());
    } else if (consume(TOKEN_TYPE_SLASH)) {
      node = new_binary_node(NODE_TYPE_DIVIDE, node, unary());
    } else {
      return node;
    }
  }
}

// unary = "+"? primary
//       | "-"? primary
//       | "*" unary
//       | "&" unary
Node *unary() {
  switch (token->type) {
  case TOKEN_TYPE_AMPERSAND:
    token = token->next;
    return new_unary_node(NODE_TYPE_ADDRESS, unary());
  case TOKEN_TYPE_ASTERISK:
    token = token->next;
    return new_unary_node(NODE_TYPE_DEREFERENCE, unary());
  case TOKEN_TYPE_MINUS:
    token = token->next;
    return new_binary_node(NODE_TYPE_SUBTRACT, new_number_node(0), primary());
  case TOKEN_TYPE_PLUS:
    token = token->next;
  default:
    return primary();
  }
}

// function_call_or_local_variable = identifier ("(" (expression ("," expression)*)? ")")?
Node *function_call_or_local_variable() {
  Node *node;
  Token *identifier = consume(TOKEN_TYPE_IDENTIFIER);
  if (consume(TOKEN_TYPE_PARENTHESIS_LEFT)) {
    Nodes *head = new_nodes();
    Nodes *nodes = head;
    while (consume(TOKEN_TYPE_COMMA) != NULL || consume(TOKEN_TYPE_PARENTHESIS_RIGHT) == NULL) {
      nodes->next = new_nodes();
      nodes = nodes->next;
      nodes->node = expression();
    }
    node = new_node(NODE_TYPE_FUNCTION_CALL);
    node->function_call.name = identifier->string;
    node->function_call.name_length = identifier->length;
    node->function_call.parameters = head->next;
  } else {
    node = new_local_variable_node(identifier);
  }
  return node;
}

// expression_in_parentheses = "(" expression ")"
Node *expression_in_parentheses() {
  Node *node;
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  node = expression();
  expect(TOKEN_TYPE_PARENTHESIS_RIGHT);
  return node;
}

Node *number() {
  return new_number_node(expect_number());
}

// primary = expression_in_parentheses | function_call_or_local_variable | number
Node *primary() {
  switch (token->type) {
  case TOKEN_TYPE_PARENTHESIS_LEFT:
    return expression_in_parentheses();
  case TOKEN_TYPE_IDENTIFIER:
    return function_call_or_local_variable();
  default:
    return number();
  }
}

Node *parse(char *input) {
  begin = input;
  token = tokenize(input);
  return program();
}
