#include <stddef.h>

typedef enum {
  TYPE_KIND_ARRAY,
  TYPE_KIND_CHAR,
  TYPE_KIND_INTEGER,
  TYPE_KIND_POINTER,
} TypeKind;

typedef struct Type Type;

struct Type {
  TypeKind kind;
  int size;
  Type *pointed_type;
  size_t array_length;
};

Type *new_array_type(Type *pointed_type, int array_length);
Type *new_pointer_type(Type *pointed_type);

extern Type *char_type;
extern Type *int_type;
