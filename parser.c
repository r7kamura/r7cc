#include "parser.h"
#include "tokenizer.h" // Token, TokenKind
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

bool at_kind(void) {
  return token->kind == TOKEN_KIND_INTEGER;
}

Token *consume(TokenKind kind) {
  Token *token_;
  if (token->kind == kind) {
    token_ = token;
    token = token->next;
    return token_;
  }
  return NULL;
}

Token *expect(TokenKind kind) {
  if (token->kind != kind) {
    error(token->string, "Unexpected token kind.");
  }
  Token *token_ = token;
  token = token->next;
  return token_;
}

int expect_number() {
  if (token->kind != TOKEN_KIND_NUMBER) {
    error(token->string, "Expected number token.");
  }
  int value = token->value;
  token = token->next;
  return value;
}

LocalVariable *new_local_variable(Type *type, char *name, int name_length, LocalVariable *next) {
  LocalVariable *local_variable = calloc(1, sizeof(LocalVariable));
  local_variable->type = type;
  local_variable->name = name;
  local_variable->name_length = name_length;
  local_variable->offset = (next == NULL ? 0 : next->offset) + type->size;
  local_variable->next = next;
  return local_variable;
}

LocalVariable *find_local_variable(Scope *scope, char *name, int name_length) {
  for (LocalVariable *local_variable = scope->local_variable; local_variable; local_variable = local_variable->next) {
    if (local_variable->name_length == name_length && !memcmp(local_variable->name, name, local_variable->name_length)) {
      return local_variable;
    }
  }
  if (scope->parent) {
    return find_local_variable(scope->parent, name, name_length);
  }
  return NULL;
}

LocalVariable *declare_local_variable(Type *type, char *name, int name_length) {
  LocalVariable *local_variable = find_local_variable(scope, name, name_length);
  if (local_variable != NULL) {
    fprintf(stderr, "Local variable `%.*s` is already defined.\n", name_length, name);
    exit(1);
  }

  local_variable = new_local_variable(type, name, name_length, scope->local_variable);
  scope->local_variable = local_variable;
  return local_variable;
}

Scope *new_scope(Scope *parent) {
  Scope *scope_ = calloc(1, sizeof(Scope));
  scope_->parent = parent;
  return scope_;
}

Node *new_node(NodeKind kind) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  return node;
}

Node *new_binary_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = new_node(kind);
  node->binary.lhs = lhs;
  node->binary.rhs = rhs;
  node->type = lhs->type;
  return node;
}

Node *new_add_node(Node *lhs, Node *rhs) {
  if (lhs->type->kind == TYPE_KIND_INTEGER && rhs->type->kind == TYPE_KIND_INTEGER) {
    return new_binary_node(NODE_KIND_ADD, lhs, rhs);
  }
  if (lhs->type->pointed_type && rhs->type->kind == TYPE_KIND_INTEGER) {
    return new_binary_node(NODE_KIND_ADD_POINTER, lhs, rhs);
  }
  if (lhs->type->kind == TYPE_KIND_INTEGER && rhs->type->pointed_type) {
    return new_binary_node(NODE_KIND_ADD_POINTER, rhs, lhs);
  }
  fprintf(stderr, "Unexpected operands on `+`.\n");
  exit(1);
}

Node *new_subtract_node(Node *lhs, Node *rhs) {
  if (lhs->type->kind == TYPE_KIND_INTEGER && rhs->type->kind == TYPE_KIND_INTEGER) {
    return new_binary_node(NODE_KIND_SUBTRACT, lhs, rhs);
  }
  if (lhs->type->pointed_type && rhs->type->kind == TYPE_KIND_INTEGER) {
    return new_binary_node(NODE_KIND_SUBTRACT_POINTER, lhs, rhs);
  }
  if (lhs->type->pointed_type && rhs->type->pointed_type) {
    return new_binary_node(NODE_KIND_DIFF_POINTER, lhs, rhs);
  }
  fprintf(stderr, "Unexpected operands on `-`.\n");
  exit(1);
}

Node *new_unary_node(NodeKind kind, Node *child) {
  Node *node = new_node(kind);
  node->node = child;
  return node;
}

Node *new_address_node(Node *operand) {
  Node *node = new_unary_node(NODE_KIND_ADDRESS, operand);
  node->type = new_pointer_type(operand->type);
  return node;
}

Node *new_dereference_node(Node *operand) {
  Node *node = new_unary_node(NODE_KIND_DEREFERENCE, operand);
  node->type = operand->type->pointed_type;
  return node;
}

Node *new_number_node(int value) {
  Node *node = new_node(NODE_KIND_NUMBER);
  node->value = value;
  node->type = int_type;
  return node;
}

Node *new_sizeof_node(Node *operand) {
  return new_number_node(operand->type->size);
}

Node *new_local_variable_node(LocalVariable *local_variable) {
  Node *node = new_node(NODE_KIND_LOCAL_VARIABLE);
  node->offset = local_variable->offset;
  node->type = local_variable->type;
  return node;
}

Nodes *new_nodes() {
  return calloc(1, sizeof(Nodes));
}

// type = "int" "*"*
Type *type_part(void) {
  expect(TOKEN_KIND_INTEGER);
  Type *type = int_type;
  while (consume(TOKEN_KIND_ASTERISK)) {
    type = new_pointer_type(type);
  }
  return type;
}

// function_definition = type identifier "(" parameters? ")" statement_block
// parameters = parameter ("," parameter)*
// parameter = type identifier
Node *function_definition() {
  Type *type = type_part();
  Token *identifier = consume(TOKEN_KIND_IDENTIFIER);
  Node *node = new_node(NODE_KIND_FUNCTION_DEFINITION);
  node->function_definition.return_value_type = type;
  node->function_definition.name = identifier->string;
  node->function_definition.name_length = identifier->length;
  declare_local_variable(type, identifier->string, identifier->length);

  scope = new_scope(scope);
  expect(TOKEN_KIND_PARENTHESIS_LEFT);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (consume(TOKEN_KIND_COMMA) != NULL || consume(TOKEN_KIND_PARENTHESIS_RIGHT) == NULL) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    Type *type = type_part();
    Token *identifier_ = consume(TOKEN_KIND_IDENTIFIER);
    LocalVariable *local_variable = declare_local_variable(type, identifier_->string, identifier_->length);
    nodes->node = new_local_variable_node(local_variable);
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
  Node *node = new_node(NODE_KIND_PROGRAM);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (at_kind()) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = function_definition();
  }
  node->program.nodes = head->next;
  return node;
}

// statement_local_variable_declaration = type identifier ("[" number "]")* ("=" expression)? ";"
Node *statement_local_variable_declaration(void) {
  Type *type = type_part();
  Token *identifier = expect(TOKEN_KIND_IDENTIFIER);
  while (consume(TOKEN_KIND_BRACKET_LEFT)) {
    type = new_array_type(type, expect_number());
    expect(TOKEN_KIND_BRACKET_RIGHT);
  }
  LocalVariable *local_variable = declare_local_variable(type, identifier->string, identifier->length);

  Node *node;
  if (consume(TOKEN_KIND_ASSIGN)) {
    node = new_binary_node(NODE_KIND_ASSIGN, new_local_variable_node(local_variable), expression());
  } else {
    node = NULL;
  }

  expect(TOKEN_KIND_SEMICOLON);
  return node;
}

// statement
//   = statement_return
//   | statement_for
//   | statement_if
//   | statement_while
//   | statement_block
//   | statement_local_variable_declaration
//   | statement_expression
Node *statement() {
  switch (token->kind) {
  case TOKEN_KIND_RETURN:
    return statement_return();
  case TOKEN_KIND_FOR:
    return statement_for();
  case TOKEN_KIND_IF:
    return statement_if();
  case TOKEN_KIND_WHILE:
    return statement_while();
  case TOKEN_KIND_BRACE_LEFT:
    return statement_block();
  default:
    if (at_kind()) {
      return statement_local_variable_declaration();
    } else {
      return statement_expression();
    }
  }
}

// statement_block = "{" statement* "}"
Node *statement_block() {
  expect(TOKEN_KIND_BRACE_LEFT);
  Node *node = new_node(NODE_KIND_BLOCK);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (!consume(TOKEN_KIND_BRACE_RIGHT)) {
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
  expect(TOKEN_KIND_SEMICOLON);
  return node;
}

// statement_for = "for" "(" expression? ";" expression? ";" expression? ")" statement
Node *statement_for() {
  Node *node = new_node(NODE_KIND_FOR);
  expect(TOKEN_KIND_FOR);
  expect(TOKEN_KIND_PARENTHESIS_LEFT);

  Node *initialization;
  if (consume(TOKEN_KIND_SEMICOLON)) {
    initialization = NULL;
  } else {
    initialization = expression();
    expect(TOKEN_KIND_SEMICOLON);
  }
  node->for_statement.initialization = initialization;

  Node *condition;
  if (consume(TOKEN_KIND_SEMICOLON)) {
    condition = NULL;
  } else {
    condition = expression();
    expect(TOKEN_KIND_SEMICOLON);
  }
  node->for_statement.condition = condition;

  Node *afterthrough;
  if (consume(TOKEN_KIND_PARENTHESIS_RIGHT)) {
    afterthrough = NULL;
  } else {
    afterthrough = expression();
    expect(TOKEN_KIND_PARENTHESIS_RIGHT);
  }
  node->for_statement.afterthrough = afterthrough;

  node->for_statement.statement = statement();

  return node;
}

// statement_if = "if" "(" expression ")" statement ("else" statement)?
Node *statement_if() {
  expect(TOKEN_KIND_IF);
  expect(TOKEN_KIND_PARENTHESIS_LEFT);
  Node *node = new_node(NODE_KIND_IF);
  node->if_statement.condition = expression();
  expect(TOKEN_KIND_PARENTHESIS_RIGHT);
  node->if_statement.true_statement = statement();
  if (consume(TOKEN_KIND_ELSE)) {
    node->if_statement.false_statement = statement();
  }
  return node;
}

// statement_return = "return" expression ";"
Node *statement_return() {
  expect(TOKEN_KIND_RETURN);
  Node *node = new_node(NODE_KIND_RETURN);
  node->return_statement.expression = expression();
  expect(TOKEN_KIND_SEMICOLON);
  return node;
}

// statement_while = "while" "(" expression ")" statement
Node *statement_while() {
  expect(TOKEN_KIND_WHILE);
  expect(TOKEN_KIND_PARENTHESIS_LEFT);
  Node *node = new_node(NODE_KIND_WHILE);
  node->while_statement.condition = expression();
  expect(TOKEN_KIND_PARENTHESIS_RIGHT);
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
  if (consume(TOKEN_KIND_ASSIGN)) {
    if (node->kind != NODE_KIND_LOCAL_VARIABLE && node->kind != NODE_KIND_DEREFERENCE) {
      fprintf(stderr, "Left value in assignment must be a local variable.");
      exit(1);
    }
    node = new_binary_node(NODE_KIND_ASSIGN, node, assign());
  }
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  while (true) {
    if (consume(TOKEN_KIND_EQ)) {
      node = new_binary_node(NODE_KIND_EQ, node, relational());
    } else if (consume(TOKEN_KIND_NE)) {
      node = new_binary_node(NODE_KIND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add_or_subtract ("<" add_or_subtract | "<=" add_or_subtract | ">" add_or_subtract | ">=" add_or_subtract)*
Node *relational() {
  Node *node = add_or_subtract();

  while (true) {
    if (consume(TOKEN_KIND_LT)) {
      node = new_binary_node(NODE_KIND_LT, node, add_or_subtract());
    } else if (consume(TOKEN_KIND_LE)) {
      node = new_binary_node(NODE_KIND_LE, node, add_or_subtract());
    } else if (consume(TOKEN_KIND_GT)) {
      node = new_binary_node(NODE_KIND_LT, add_or_subtract(), node);
    } else if (consume(TOKEN_KIND_GE)) {
      node = new_binary_node(NODE_KIND_LE, add_or_subtract(), node);
    } else {
      return node;
    }
  }
}

// add_or_subtract = multiply_or_devide ("+" multiply_or_devide | "-" multiply_or_devide)*
Node *add_or_subtract() {
  Node *node = multiply_or_devide();

  while (true) {
    if (consume(TOKEN_KIND_PLUS)) {
      node = new_add_node(node, multiply_or_devide());
    } else if (consume(TOKEN_KIND_MINUS)) {
      node = new_subtract_node(node, multiply_or_devide());
    } else {
      return node;
    }
  }
}

// multiply_or_devide = unary ("*" unary | "/" unary)*
Node *multiply_or_devide() {
  Node *node = unary();

  while (true) {
    if (consume(TOKEN_KIND_ASTERISK)) {
      node = new_binary_node(NODE_KIND_MULTIPLY, node, unary());
    } else if (consume(TOKEN_KIND_SLASH)) {
      node = new_binary_node(NODE_KIND_DIVIDE, node, unary());
    } else {
      return node;
    }
  }
}

// postfix = primary ("[" expression "]")*
Node *postfix(void) {
  Node *node = primary();

  while (consume(TOKEN_KIND_BRACKET_LEFT)) {
    Node *index_node = expression();
    expect(TOKEN_KIND_BRACKET_RIGHT);
    node = new_dereference_node(new_add_node(node, index_node));
  }

  return node;
}

// unary = "+"? primary
//       | "-"? primary
//       | "sizeof" unary
//       | "*" unary
//       | "&" unary
Node *unary() {
  switch (token->kind) {
  case TOKEN_KIND_SIZEOF:
    token = token->next;
    return new_sizeof_node(unary());
  case TOKEN_KIND_AMPERSAND:
    token = token->next;
    return new_address_node(unary());
  case TOKEN_KIND_ASTERISK:
    token = token->next;
    return new_dereference_node(unary());
  case TOKEN_KIND_MINUS:
    token = token->next;
    return new_binary_node(NODE_KIND_SUBTRACT, new_number_node(0), primary());
  case TOKEN_KIND_PLUS:
    token = token->next;
  default:
    return postfix();
  }
}

// function_call = identifier "(" (identifier ("," identifier)*)? ")"
Node *function_call(Token *identifier) {
  expect(TOKEN_KIND_PARENTHESIS_LEFT);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (consume(TOKEN_KIND_COMMA) != NULL || consume(TOKEN_KIND_PARENTHESIS_RIGHT) == NULL) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = expression();
  }
  Node *node = new_node(NODE_KIND_FUNCTION_CALL);
  node->function_call.name = identifier->string;
  node->function_call.name_length = identifier->length;
  node->function_call.parameters = head->next;
  node->type = find_local_variable(scope, identifier->string, identifier->length)->type;
  return node;
}

// local_variable = identifier
Node *local_variable(Token *identifier) {
  LocalVariable *local_variable = find_local_variable(scope, identifier->string, identifier->length);
  if (local_variable == NULL) {
    fprintf(stderr, "Undefined local variable: %.*s\n", identifier->length, identifier->string);
    exit(1);
  }
  return new_local_variable_node(local_variable);
}

// function_call_or_local_variable = function_call | local_variable
Node *function_call_or_local_variable() {
  Token *identifier = expect(TOKEN_KIND_IDENTIFIER);
  if (token->kind == TOKEN_KIND_PARENTHESIS_LEFT) {
    return function_call(identifier);
  } else {
    return local_variable(identifier);
  }
}

// expression_in_parentheses = "(" expression ")"
Node *expression_in_parentheses() {
  Node *node;
  expect(TOKEN_KIND_PARENTHESIS_LEFT);
  node = expression();
  expect(TOKEN_KIND_PARENTHESIS_RIGHT);
  return node;
}

Node *number() {
  return new_number_node(expect_number());
}

// primary = expression_in_parentheses
//         | function_call_or_local_variable
//         | number
Node *primary() {
  switch (token->kind) {
  case TOKEN_KIND_PARENTHESIS_LEFT:
    return expression_in_parentheses();
  case TOKEN_KIND_IDENTIFIER: {
    return function_call_or_local_variable();
  }
  default:
    return number();
  }
}

Node *parse(char *input) {
  begin = input;
  token = tokenize(input);
  return program();
}
