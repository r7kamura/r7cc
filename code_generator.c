#include "cc7.h"
#include <stdio.h>

void generate_fragment_code(Node *node) {
  if (node->type == NODE_TYPE_NUMBER) {
    printf("  push %d\n", node->value);
    return;
  }

  generate_fragment_code(node->lhs);
  generate_fragment_code(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->type) {
  case NODE_TYPE_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  case NODE_TYPE_ADD:
    printf("  add rax, rdi\n");
    break;
  case NODE_TYPE_SUBTRACT:
    printf("  sub rax, rdi\n");
    break;
  case NODE_TYPE_MULTIPLY:
    printf("  imul rax, rdi\n");
    break;
  case NODE_TYPE_DIVIDE:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");
}

void generate_code(Node *node) {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  generate_fragment_code(node);
  printf("  pop rax\n");
  printf("  ret\n");
}
