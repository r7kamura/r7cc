typedef enum {
  TYPE_KIND_INTEGER,
  TYPE_KIND_POINTER,
} TypeKind;

typedef struct Type Type;

struct Type {
  TypeKind kind;
  Type *pointed_type;
};

int size_of_type(Type *type);
