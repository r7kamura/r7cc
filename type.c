#include "type.h"
#include <stdlib.h>

Type *new_pointer_type(Type *pointed_type) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_KIND_POINTER;
  type->pointed_type = pointed_type;
  return type;
}

int size_of_type(Type *type) {
  switch (type->kind) {
  case TYPE_KIND_INTEGER:
    return 8;
  default:
    return 16;
  }
}
