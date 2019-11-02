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

typedef enum {
  NODE_TYPE_ADD,
  NODE_TYPE_ASSIGN,
  NODE_TYPE_DIVIDE,
  NODE_TYPE_EQ,
  NODE_TYPE_IF,
  NODE_TYPE_LE,
  NODE_TYPE_LOCAL_VARIABLE,
  NODE_TYPE_LT,
  NODE_TYPE_MULTIPLY,
  NODE_TYPE_NE,
  NODE_TYPE_NUMBER,
  NODE_TYPE_RETURN,
  NODE_TYPE_SUBTRACT,
} NodeType;

typedef struct Node Node;

struct Node {
  NodeType type;
  Node *lhs;
  Node *rhs;
  int value;
  int offset;
};

typedef struct Statement Statement;

struct Statement {
  Statement *next;
  Node *node;
};

typedef enum {
  TOKEN_TYPE_EOF,
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_IF,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_RESERVED_SYMBOL,
  TOKEN_TYPE_RETURN,
} TokenType;

typedef struct Token Token;

struct Token {
  TokenType type;

  // For linked list.
  Token *next;

  // Token content on number. (e.g. 1)
  int value;

  // Token content on identifier or reserved symbol. (e.g. "foo")
  char *string;

  // Token Length. (e.g. 3)
  int length;
};

void generate_code(Statement *statement);

Token *tokenize();

Statement *parse(char *string);

void report_error(char *location, char *fmt, ...);

char *user_input;
