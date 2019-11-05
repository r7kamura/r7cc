#include "code_generator.h" // generator
#include "parser.h"         // parse
#include <stdio.h>          // fprintf
#include <stdlib.h>         // exit

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Expected arguments count is 2, got %i\n", argc);
    exit(1);
  }

  generate(parse(argv[1]));

  return 0;
}
