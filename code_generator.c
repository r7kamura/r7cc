#include "cc7.h"
#include <stdio.h>
#include <stdlib.h>

void generate_code_for_load() {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

void generate_code_for_store() {
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void generate_code_for_local_variable(Node *node) {
  if (node->type != NODE_TYPE_LOCAL_VARIABLE) {
    fprintf(stderr, "Left value in assignment must be a local variable.");
    exit(1);
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void generate_code_for_expression(Node *node) {
  switch (node->type) {
  case NODE_TYPE_NUMBER:
    printf("  push %d\n", node->value);
    return;
  case NODE_TYPE_ASSIGN:
    generate_code_for_local_variable(node->lhs);
    generate_code_for_expression(node->rhs);
    generate_code_for_load();
    return;
  case NODE_TYPE_LOCAL_VARIABLE:
    generate_code_for_local_variable(node);
    generate_code_for_store();
    return;
  }

  generate_code_for_expression(node->lhs);
  generate_code_for_expression(node->rhs);

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

void generate_code() {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; statements[i]; i++) {
    generate_code_for_expression(statements[i]);
  }
  printf("  pop rax\n");

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
