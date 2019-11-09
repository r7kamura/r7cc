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

Type *int_type = &(Type){.type = TYPE_TYPE_INTEGER};

void error(char *position, char *message) {
  int index = position - begin;
  fprintf(stderr, "%s\n", begin);
  fprintf(stderr, "%*s^ %s\n", index, "", message);
  exit(1);
}

bool at_type(void) {
  return token->type == TOKEN_TYPE_INTEGER;
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

Token *expect(TokenType type) {
  if (token->type != type) {
    error(token->string, "Unexpected token type.");
  }
  Token *token_ = token;
  token = token->next;
  return token_;
}

int expect_number() {
  if (token->type != TOKEN_TYPE_NUMBER) {
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
  local_variable->offset = next == NULL ? 8 : next->offset + 8;
  local_variable->next = next;
  return local_variable;
}

LocalVariable *find_local_variable(char *name, int name_length) {
  for (LocalVariable *local_variable = scope->local_variable; local_variable; local_variable = local_variable->next) {
    if (local_variable->name_length == name_length && !memcmp(local_variable->name, name, local_variable->name_length)) {
      return local_variable;
    }
  }
  return NULL;
}

LocalVariable *declare_local_variable(Type *type, char *name, int name_length) {
  LocalVariable *local_variable = find_local_variable(name, name_length);
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

Node *new_binary_node(NodeKind type, Node *lhs, Node *rhs) {
  Node *node = new_node(type);
  node->binary.lhs = lhs;
  node->binary.rhs = rhs;
  return node;
}

Node *new_unary_node(NodeKind type, Node *child) {
  Node *node = new_node(type);
  node->node = child;
  return node;
}

Node *new_number_node(int value) {
  Node *node = new_node(NODE_KIND_NUMBER);
  node->value = value;
  return node;
}

Node *new_local_variable_node(LocalVariable *local_variable) {
  Node *node = new_node(NODE_KIND_LOCAL_VARIABLE);
  node->offset = local_variable->offset;
  return node;
}

Nodes *new_nodes() {
  return calloc(1, sizeof(Nodes));
}

Type *new_pointer_type(Type *pointer) {
  Type *type = calloc(1, sizeof(Type));
  type->type = TYPE_TYPE_POINTER;
  type->pointer = pointer;
  return type;
}

// type = "int" "*"*
Type *type_part(void) {
  expect(TOKEN_TYPE_INTEGER);
  Type *type = int_type;
  while (consume(TOKEN_TYPE_ASTERISK)) {
    type = new_pointer_type(type);
  }
  return type;
}

// function_definition = type identifier "(" parameters? ")" statement_block
// parameters = parameter ("," parameter)*
// parameter = type identifier
Node *function_definition() {
  Type *type = type_part();
  Token *identifier = consume(TOKEN_TYPE_IDENTIFIER);
  Node *node = new_node(NODE_KIND_FUNCTION_DEFINITION);
  node->function_definition.return_value_type = type;
  node->function_definition.name = identifier->string;
  node->function_definition.name_length = identifier->length;

  scope = new_scope(scope);
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (consume(TOKEN_TYPE_COMMA) != NULL || consume(TOKEN_TYPE_PARENTHESIS_RIGHT) == NULL) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    Type *type = type_part();
    Token *identifier_ = consume(TOKEN_TYPE_IDENTIFIER);
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
  while (at_type()) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = function_definition();
  }
  node->program.nodes = head->next;
  return node;
}

// statement_local_variable_declaration = type identifier ("=" expression)? ";"
Node *statement_local_variable_declaration(void) {
  Type *type = type_part();

  Token *identifier = expect(TOKEN_TYPE_IDENTIFIER);
  LocalVariable *local_variable = declare_local_variable(type, identifier->string, identifier->length);

  Node *node;
  if (consume(TOKEN_TYPE_ASSIGN)) {
    node = new_binary_node(NODE_KIND_ASSIGN, new_local_variable_node(local_variable), expression());
  } else {
    node = NULL;
  }

  expect(TOKEN_TYPE_SEMICOLON);
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
    if (at_type()) {
      return statement_local_variable_declaration();
    } else {
      return statement_expression();
    }
  }
}

// statement_block = "{" statement* "}"
Node *statement_block() {
  expect(TOKEN_TYPE_BRACKET_LEFT);
  Node *node = new_node(NODE_KIND_BLOCK);
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
  Node *node = new_node(NODE_KIND_FOR);
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
  Node *node = new_node(NODE_KIND_IF);
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
  Node *node = new_node(NODE_KIND_RETURN);
  node->return_statement.expression = expression();
  expect(TOKEN_TYPE_SEMICOLON);
  return node;
}

// statement_while = "while" "(" expression ")" statement
Node *statement_while() {
  expect(TOKEN_TYPE_WHILE);
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Node *node = new_node(NODE_KIND_WHILE);
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
    if (node->kind != NODE_KIND_LOCAL_VARIABLE) {
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
    if (consume(TOKEN_TYPE_EQ)) {
      node = new_binary_node(NODE_KIND_EQ, node, relational());
    } else if (consume(TOKEN_TYPE_NE)) {
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
    if (consume(TOKEN_TYPE_LT)) {
      node = new_binary_node(NODE_KIND_LT, node, add_or_subtract());
    } else if (consume(TOKEN_TYPE_LE)) {
      node = new_binary_node(NODE_KIND_LE, node, add_or_subtract());
    } else if (consume(TOKEN_TYPE_GT)) {
      node = new_binary_node(NODE_KIND_LT, add_or_subtract(), node);
    } else if (consume(TOKEN_TYPE_GE)) {
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
    if (consume(TOKEN_TYPE_PLUS)) {
      node = new_binary_node(NODE_KIND_ADD, node, multiply_or_devide());
    } else if (consume(TOKEN_TYPE_MINUS)) {
      node = new_binary_node(NODE_KIND_SUBTRACT, node, multiply_or_devide());
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
      node = new_binary_node(NODE_KIND_MULTIPLY, node, unary());
    } else if (consume(TOKEN_TYPE_SLASH)) {
      node = new_binary_node(NODE_KIND_DIVIDE, node, unary());
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
    return new_unary_node(NODE_KIND_ADDRESS, unary());
  case TOKEN_TYPE_ASTERISK:
    token = token->next;
    return new_unary_node(NODE_KIND_DEREFERENCE, unary());
  case TOKEN_TYPE_MINUS:
    token = token->next;
    return new_binary_node(NODE_KIND_SUBTRACT, new_number_node(0), primary());
  case TOKEN_TYPE_PLUS:
    token = token->next;
  default:
    return primary();
  }
}

// function_call = identifier "(" (identifier ("," identifier)*)? ")"
Node *function_call(Token *identifier) {
  expect(TOKEN_TYPE_PARENTHESIS_LEFT);
  Nodes *head = new_nodes();
  Nodes *nodes = head;
  while (consume(TOKEN_TYPE_COMMA) != NULL || consume(TOKEN_TYPE_PARENTHESIS_RIGHT) == NULL) {
    nodes->next = new_nodes();
    nodes = nodes->next;
    nodes->node = expression();
  }
  Node *node = new_node(NODE_KIND_FUNCTION_CALL);
  node->function_call.name = identifier->string;
  node->function_call.name_length = identifier->length;
  node->function_call.parameters = head->next;
  return node;
}

// local_variable = identifier
Node *local_variable(Token *identifier) {
  LocalVariable *local_variable = find_local_variable(identifier->string, identifier->length);
  if (local_variable == NULL) {
    fprintf(stderr, "Undefined local variable: %.*s\n", identifier->length, identifier->string);
    exit(1);
  }
  return new_local_variable_node(local_variable);
}

// function_call_or_local_variable = function_call | local_variable
Node *function_call_or_local_variable() {
  Token *identifier = expect(TOKEN_TYPE_IDENTIFIER);
  if (token->type == TOKEN_TYPE_PARENTHESIS_LEFT) {
    return function_call(identifier);
  } else {
    return local_variable(identifier);
  }
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

// primary = expression_in_parentheses
//         | function_call_or_local_variable
//         | number
Node *primary() {
  switch (token->type) {
  case TOKEN_TYPE_PARENTHESIS_LEFT:
    return expression_in_parentheses();
  case TOKEN_TYPE_IDENTIFIER: {
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
