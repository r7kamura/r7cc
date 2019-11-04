#include "cc7.h"
#include <stdio.h>
#include <stdlib.h>

int label_counter;

void generate(Node *node);

void generate_local_variable_address(Node *node) {
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->value);
  printf("  push rax\n");
}

void generate_add(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  add rax, rdi\n");
  printf("  push rax\n");
}

void generate_assign(Node *node) {
  if (node->lhs->type != NODE_TYPE_LOCAL_VARIABLE) {
    fprintf(stderr, "Left value in assignment must be a local variable.");
    exit(1);
  }
  generate_local_variable_address(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  mov [rax], rdi\n");
  printf("  push rdi\n");
}

void generate_block(Node *node) {
  generate(node->rhs);
}

void generate_divide(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cqo\n");
  printf("  idiv rdi\n");
  printf("  push rax\n");
}

void generate_eq(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  sete al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_for(Node *node) {
  int label_count = label_counter++;
  generate(node->lhs);
  printf(".Lbegin%i:\n", label_count);
  if (node->rhs->lhs) {
    generate(node->rhs->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%i\n", label_count);
  }
  generate(node->rhs->rhs->rhs);
  generate(node->rhs->rhs->lhs);
  printf("  jmp .Lbegin%i\n", label_count);
  printf(".Lend%i:\n", label_count);
}

void generate_function_call(Node *node) {
  int label_count = label_counter++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n");
  printf("  jnz .Lcall%i\n", label_count);
  printf("  mov rax, 0\n");
  printf("  call %.*s\n", node->name_length, node->name);
  printf("  jmp .Lend%i\n", label_count);
  printf(".Lcall%i:\n", label_count);
  printf("  sub rsp, 8\n");
  printf("  mov rax, 0\n");
  printf("  call %.*s\n", node->name_length, node->name);
  printf("  add rsp, 8\n");
  printf(".Lend%i:\n", label_count);
  printf("  push rax\n");
}

void generate_function_definition(Node *node) {
  printf("%.*s:\n", node->name_length, node->name);
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");
  generate(node->lhs);
  generate(node->rhs);
}

void generate_if_else(Node *node) {
  int label_count = label_counter++;
  if (node->rhs) {
    generate(node->lhs->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%i\n", label_count);
    generate(node->lhs->rhs);
    printf("  jmp .Lend%i\n", label_count);
    printf(".Lelse%i:\n", label_count);
    generate(node->rhs);
    printf(".Lend%i:\n", label_count);
  } else {
    generate(node->lhs->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%i\n", label_count);
    generate(node->lhs->rhs);
    printf(".Lend%i:\n", label_count);
  }
}

void generate_le(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  setle al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_local_variable(Node *node) {
  generate_local_variable_address(node);
  printf("  pop rax\n");
  printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void generate_lt(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  setl al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_multiply(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  imul rax, rdi\n");
  printf("  push rax\n");
}

void generate_ne(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  setne al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_number(Node *node) {
  printf("  push %d\n", node->value);
}

void generate_program(Node *node) {
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  generate(node->rhs);
}

void generate_return(Node *node) {
  generate(node->lhs);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void generate_statement(Node *node) {
  generate(node->lhs);
  if (node->rhs) {
    printf("  pop rax\n");
    generate(node->rhs);
  }
}

void generate_subtract(Node *node) {
  generate(node->lhs);
  generate(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  sub rax, rdi\n");
  printf("  push rax\n");
}

void generate_while(Node *node) {
  int label_count = label_counter++;
  printf(".Lbegin%i:\n", label_count);
  generate(node->lhs);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%i\n", label_count);
  generate(node->rhs);
  printf("  jmp .Lbegin%i\n", label_count);
  printf(".Lend%i:\n", label_count);
}

void generate(Node *node) {
  if (node == NULL) {
    return;
  }

  switch (node->type) {
  case NODE_TYPE_ADD:
    generate_add(node);
    break;
  case NODE_TYPE_ASSIGN:
    generate_assign(node);
    break;
  case NODE_TYPE_BLOCK:
    generate_block(node);
    break;
  case NODE_TYPE_DIVIDE:
    generate_divide(node);
    break;
  case NODE_TYPE_EQ:
    generate_eq(node);
    break;
  case NODE_TYPE_FOR:
    generate_for(node);
    break;
  case NODE_TYPE_FUNCTION_CALL:
    generate_function_call(node);
    break;
  case NODE_TYPE_FUNCTION_DEFINITION:
    generate_function_definition(node);
    break;
  case NODE_TYPE_IF_ELSE:
    generate_if_else(node);
    break;
  case NODE_TYPE_LE:
    generate_le(node);
    break;
  case NODE_TYPE_LOCAL_VARIABLE:
    generate_local_variable(node);
    break;
  case NODE_TYPE_LT:
    generate_lt(node);
    break;
  case NODE_TYPE_MULTIPLY:
    generate_multiply(node);
    break;
  case NODE_TYPE_NE:
    generate_ne(node);
    break;
  case NODE_TYPE_NUMBER:
    generate_number(node);
    break;
  case NODE_TYPE_PROGRAM:
    generate_program(node);
    break;
  case NODE_TYPE_RETURN:
    generate_return(node);
    break;
  case NODE_TYPE_STATEMENT:
    generate_statement(node);
    break;
  case NODE_TYPE_SUBTRACT:
    generate_subtract(node);
    break;
  case NODE_TYPE_WHILE:
    generate_while(node);
    break;
  default:
    fprintf(stderr, "Unexpected node.\n");
    exit(1);
  }
}
