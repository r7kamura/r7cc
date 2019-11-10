#include "type.h"

int size_of_type(Type *type) {
  switch (type->kind) {
  case TYPE_KIND_INTEGER:
    return 8;
  default:
    return 16;
  }
}
