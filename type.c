#include "type.h"
#include <stdlib.h>

Type *int_type = &(Type){
  .kind = TYPE_KIND_INTEGER,
  .size = 8,
};

Type *new_pointer_type(Type *pointed_type) {
  Type *type = calloc(1, sizeof(Type));
  type->kind = TYPE_KIND_POINTER;
  type->pointed_type = pointed_type;
  type->size = 16;
  return type;
}
