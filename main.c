#include "cc7.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Expected arguments count is 2, got %i\n", argc);
    exit(1);
  }

  Node *node = parse(argv[1]);
  generate_code(node);

  return 0;
}
