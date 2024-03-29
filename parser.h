#pragma once
#include "type.h"
#include <stdbool.h>

typedef struct LocalVariable LocalVariable;

struct LocalVariable {
  LocalVariable *next;
  Type *type;
  char *name;
  int name_length;
  bool is_global;

  // Offset from RBP. (e.g. 8, 16, 24)
  int offset;
};

typedef struct Scope Scope;

struct Scope {
  Scope *parent;
  LocalVariable *local_variable;
};

typedef enum {
  NODE_KIND_ADD,
  NODE_KIND_ADD_POINTER,
  NODE_KIND_ADDRESS,
  NODE_KIND_ASSIGN,
  NODE_KIND_BLOCK,
  NODE_KIND_DIFF_POINTER,
  NODE_KIND_DIVIDE,
  NODE_KIND_DEREFERENCE,
  NODE_KIND_EQ,
  NODE_KIND_FOR,
  NODE_KIND_FUNCTION_CALL,
  NODE_KIND_FUNCTION_DEFINITION,
  NODE_KIND_GLOBAL_VARIABLE_DEFINITION,
  NODE_KIND_IF,
  NODE_KIND_LE,
  NODE_KIND_LOCAL_VARIABLE,
  NODE_KIND_LT,
  NODE_KIND_MULTIPLY,
  NODE_KIND_NE,
  NODE_KIND_NUMBER,
  NODE_KIND_PROGRAM,
  NODE_KIND_RETURN,
  NODE_KIND_SUBTRACT,
  NODE_KIND_SUBTRACT_POINTER,
  NODE_KIND_TYPE,
  NODE_KIND_WHILE,
} NodeKind;

typedef struct Nodes Nodes;

typedef struct Node Node;

struct Node {
  NodeKind kind;

  Type *type;

  union {
    int value;

    Node *node;

    LocalVariable *local_variable;

    struct {
      Node *lhs;
      Node *rhs;
    } binary;

    struct {
      Nodes *nodes;
    } block;

    struct {
      Node *initialization;
      Node *condition;
      Node *afterthrough;
      Node *statement;
    } for_statement;

    struct {
      char *name;
      int name_length;
      Nodes *parameters;
    } function_call;

    struct {
      Type *return_value_type;
      char *name;
      int name_length;
      Nodes *parameters;
      Node *block;
      Scope *scope;
    } function_definition;

    struct {
      Node *condition;
      Node *true_statement;
      Node *false_statement;
    } if_statement;

    struct {
      Nodes *nodes;
    } program;

    struct {
      Node *expression;
    } return_statement;

    struct {
      Node *condition;
      Node *statement;
    } while_statement;
  };
};

struct Nodes {
  Node *node;
  Nodes *next;
};

Node *parse(char *string);
