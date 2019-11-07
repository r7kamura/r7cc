#pragma once

typedef struct LocalVariable LocalVariable;

struct LocalVariable {
  // For linked list.
  LocalVariable *next;

  // Variable name. (e.g. "foo")
  char *name;

  // Variable name length. (e.g. 3)
  int length;

  // Offset from RBP. (e.g. 8, 16, 24)
  int offset;
};

typedef struct Scope Scope;

struct Scope {
  Scope *parent;
  LocalVariable *local_variable;
};

typedef enum {
  NODE_TYPE_ADD,
  NODE_TYPE_ASSIGN,
  NODE_TYPE_BLOCK,
  NODE_TYPE_DIVIDE,
  NODE_TYPE_EQ,
  NODE_TYPE_FOR,
  NODE_TYPE_FUNCTION_CALL,
  NODE_TYPE_FUNCTION_DEFINITION,
  NODE_TYPE_IF,
  NODE_TYPE_LE,
  NODE_TYPE_LOCAL_VARIABLE,
  NODE_TYPE_LT,
  NODE_TYPE_MULTIPLY,
  NODE_TYPE_NE,
  NODE_TYPE_NUMBER,
  NODE_TYPE_PROGRAM,
  NODE_TYPE_RETURN,
  NODE_TYPE_SUBTRACT,
  NODE_TYPE_WHILE,
} NodeType;

typedef struct Nodes Nodes;

typedef struct Node Node;

struct Node {
  NodeType type;

  union {
    int value;

    int offset;

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
    } function_call;

    struct {
      char *name;
      int name_length;
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