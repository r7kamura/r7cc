typedef enum {
  TYPE_KIND_INTEGER,
  TYPE_KIND_POINTER,
} TypeKind;

typedef struct Type Type;

struct Type {
  TypeKind kind;
  int size;
  Type *pointed_type;
};

Type *new_pointer_type(Type *pointed_type);

extern Type *int_type;
