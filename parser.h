#pragma once

typedef enum {
  TYPE_TYPE_INTEGER,
  TYPE_TYPE_POINTER,
} TypeType;

typedef struct Type Type;

struct Type {
  TypeType type;
  Type *pointer;
};

typedef struct LocalVariable LocalVariable;

struct LocalVariable {
  LocalVariable *next;
  Type *type;
  char *name;
  int name_length;

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
  NODE_TYPE_ADDRESS,
  NODE_TYPE_ASSIGN,
  NODE_TYPE_BLOCK,
  NODE_TYPE_DIVIDE,
  NODE_TYPE_DEREFERENCE,
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
  NODE_TYPE_TYPE,
  NODE_TYPE_WHILE,
} NodeType;

typedef struct Nodes Nodes;

typedef struct Node Node;

struct Node {
  NodeType type;

  union {
    int value;

    int offset;

    Node *node;

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
