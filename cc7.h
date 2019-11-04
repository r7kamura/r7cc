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
  NODE_TYPE_BLOCK,
  NODE_TYPE_DIVIDE,
  NODE_TYPE_EQ,
  NODE_TYPE_FOR,
  NODE_TYPE_FOR_AFTERTHROUGH,
  NODE_TYPE_FOR_CONDITION,
  NODE_TYPE_IF_ELSE,
  NODE_TYPE_IF,
  NODE_TYPE_LE,
  NODE_TYPE_LOCAL_VARIABLE,
  NODE_TYPE_LT,
  NODE_TYPE_MULTIPLY,
  NODE_TYPE_NE,
  NODE_TYPE_NUMBER,
  NODE_TYPE_RETURN,
  NODE_TYPE_STATEMENT,
  NODE_TYPE_SUBTRACT,
  NODE_TYPE_WHILE,
} NodeType;

typedef struct Node Node;

struct Node {
  NodeType type;
  Node *lhs;
  Node *rhs;
  int value;
};

typedef enum {
  TOKEN_TYPE_ADD,
  TOKEN_TYPE_ASSIGN,
  TOKEN_TYPE_BRACKET_LEFT,
  TOKEN_TYPE_BRACKET_RIGHT,
  TOKEN_TYPE_DIVIDE,
  TOKEN_TYPE_ELSE,
  TOKEN_TYPE_EOF,
  TOKEN_TYPE_EQ,
  TOKEN_TYPE_FOR,
  TOKEN_TYPE_GE,
  TOKEN_TYPE_GT,
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_IF,
  TOKEN_TYPE_LE,
  TOKEN_TYPE_LT,
  TOKEN_TYPE_MULTIPLY,
  TOKEN_TYPE_NE,
  TOKEN_TYPE_NUMBER,
  TOKEN_TYPE_PARENTHESIS_LEFT,
  TOKEN_TYPE_PARENTHESIS_RIGHT,
  TOKEN_TYPE_RETURN,
  TOKEN_TYPE_SEMICOLON,
  TOKEN_TYPE_SUBTRACT,
  TOKEN_TYPE_WHILE,
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

void generate_code(Node *node);

Token *tokenize();

Node *parse(char *string);

void report_error(char *location, char *fmt, ...);

char *user_input;
