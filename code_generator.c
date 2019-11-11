#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

static char *register_names_1byte[] = {
    "dil",
    "sil",
    "dl",
    "cl",
    "r8b",
    "r9b"};

static char *register_names_8byte[] = {
    "rdi",
    "rsi",
    "rdx",
    "rcx",
    "r8",
    "r9"};

int label_counter;

int align(int target, int unit) {
  return (target + unit) & ~(unit - 1);
}

void load(Type *type) {
  printf("  pop rax\n");
  if (type->size == 1) {
    printf("  movsx rax, BYTE PTR [rax]\n");
  } else {
    printf("  mov rax, [rax]\n");
  }
  printf("  push rax\n");
}

void store(Type *type) {
  printf("  pop rdi\n");
  printf("  pop rax\n");
  if (type->size == 1) {
    printf("  mov [rax], dil\n");
  } else {
    printf("  mov [rax], rdi\n");
  }
  printf("  push rdi\n");
}

void generate(Node *node);

void generate_add(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  add rax, rdi\n");
  printf("  push rax\n");
}

void generate_add_pointer(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  imul rdi, %i\n", node->binary.lhs->type->pointed_type->size);
  printf("  add rax, rdi\n");
  printf("  push rax\n");
}

void generate_address(Node *node) {
  switch (node->kind) {
  case NODE_KIND_ADDRESS:
    generate_address(node->node);
    break;
  case NODE_KIND_DEREFERENCE:
    generate(node->node);
    break;
  case NODE_KIND_LOCAL_VARIABLE:
    if (node->local_variable->is_global) {
      printf("  lea rax, %.*s[rip]\n", node->local_variable->name_length, node->local_variable->name);
    } else {
      printf("  mov rax, rbp\n");
      printf("  sub rax, %d\n", node->local_variable->offset);
    }
    printf("  push rax\n");
    break;
  }
}

void generate_assign(Node *node) {
  generate_address(node->binary.lhs);
  generate(node->binary.rhs);
  store(node->type);
}

void generate_block(Node *node) {
  for (Nodes *nodes = node->block.nodes; nodes != NULL; nodes = nodes->next) {
    generate(nodes->node);
  }
}

void generate_dereference(Node *node) {
  generate(node->node);
  if (node->type->kind != TYPE_KIND_ARRAY) {
    load(node->type);
  }
}

void generate_diff_pointer(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  sub rax, rdi\n");
  printf("  mov rdi, %i\n", node->binary.lhs->type->pointed_type->size);
  printf("  cqo\n");
  printf("  idiv rdi\n");
  printf("  push rax\n");
}

void generate_divide(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cqo\n");
  printf("  idiv rdi\n");
  printf("  push rax\n");
}

void generate_eq(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  sete al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_for(Node *node) {
  int label_count = label_counter++;
  generate(node->for_statement.initialization);
  printf(".Lbegin%i:\n", label_count);
  if (node->for_statement.condition) {
    generate(node->for_statement.condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%i\n", label_count);
  }
  generate(node->for_statement.statement);
  generate(node->for_statement.afterthrough);
  printf("  jmp .Lbegin%i\n", label_count);
  printf(".Lend%i:\n", label_count);
}

void generate_function_call(Node *node) {
  int parameters_count = 0;
  for (Nodes *nodes = node->function_call.parameters; nodes != NULL; nodes = nodes->next) {
    generate(nodes->node);
    parameters_count++;
  }
  while (parameters_count--) {
    printf("  pop %s\n", register_names_8byte[parameters_count]);
  }
  int label_count = label_counter++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n");
  printf("  jnz .Lcall%i\n", label_count);
  printf("  mov rax, 0\n");
  printf("  call %.*s\n", node->function_call.name_length, node->function_call.name);
  printf("  jmp .Lend%i\n", label_count);
  printf(".Lcall%i:\n", label_count);
  printf("  sub rsp, 8\n");
  printf("  mov rax, 0\n");
  printf("  call %.*s\n", node->function_call.name_length, node->function_call.name);
  printf("  add rsp, 8\n");
  printf(".Lend%i:\n", label_count);
  printf("  push rax\n");
}

void generate_function_definition(Node *node) {
  printf(".global %.*s\n", node->function_definition.name_length, node->function_definition.name);
  printf("%.*s:\n", node->function_definition.name_length, node->function_definition.name);

  int offset = 0;
  for (LocalVariable *variable = node->function_definition.scope->local_variable; variable != NULL; variable = variable->next) {
    offset += variable->type->size;
  }
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %i\n", align(offset, 8));

  int i = 0;
  for (Nodes *nodes = node->function_definition.parameters; nodes != NULL; nodes = nodes->next) {
    char *register_name;
    if (nodes->node->local_variable->type->size == 1) {
      register_name = register_names_1byte[i];
    } else {
      register_name = register_names_8byte[i];
    }
    printf("  mov [rbp-%d], %s\n", nodes->node->local_variable->offset, register_name);
    i++;
  }

  generate(node->function_definition.block);
}

void generate_global_variable_definition(Node *node) {
  printf("%.*s:\n", node->local_variable->name_length, node->local_variable->name);
  printf("  .zero %d\n", node->local_variable->type->size);
}

void generate_if(Node *node) {
  int label_count = label_counter++;
  if (node->if_statement.false_statement) {
    generate(node->if_statement.condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lelse%i\n", label_count);
    generate(node->if_statement.true_statement);
    printf("  jmp .Lend%i\n", label_count);
    printf(".Lelse%i:\n", label_count);
    generate(node->if_statement.false_statement);
    printf(".Lend%i:\n", label_count);
  } else {
    generate(node->if_statement.condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%i\n", label_count);
    generate(node->if_statement.true_statement);
    printf(".Lend%i:\n", label_count);
  }
}

void generate_le(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  setle al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_local_variable(Node *node) {
  generate_address(node);
  if (node->type->kind != TYPE_KIND_ARRAY) {
    load(node->type);
  }
}

void generate_lt(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  cmp rax, rdi\n");
  printf("  setl al\n");
  printf("  movzb rax, al\n");
  printf("  push rax\n");
}

void generate_multiply(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  imul rax, rdi\n");
  printf("  push rax\n");
}

void generate_ne(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
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

  printf(".data\n");
  for (Nodes *nodes = node->program.nodes; nodes != NULL; nodes = nodes->next) {
    if (nodes->node->kind == NODE_KIND_GLOBAL_VARIABLE_DEFINITION) {
      generate(nodes->node);
    }
  }

  printf(".text\n");
  for (Nodes *nodes = node->program.nodes; nodes != NULL; nodes = nodes->next) {
    if (nodes->node->kind == NODE_KIND_FUNCTION_DEFINITION) {
      generate(nodes->node);
    }
  }
}

void generate_return(Node *node) {
  generate(node->return_statement.expression);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void generate_subtract(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  sub rax, rdi\n");
  printf("  push rax\n");
}

void generate_subtract_pointer(Node *node) {
  generate(node->binary.lhs);
  generate(node->binary.rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  imul rdi, %i\n", node->binary.lhs->type->pointed_type->size);
  printf("  sub rax, rdi\n");
  printf("  push rax\n");
}

void generate_while(Node *node) {
  int label_count = label_counter++;
  printf(".Lbegin%i:\n", label_count);
  generate(node->while_statement.condition);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%i\n", label_count);
  generate(node->while_statement.statement);
  printf("  jmp .Lbegin%i\n", label_count);
  printf(".Lend%i:\n", label_count);
}

void generate(Node *node) {
  if (node == NULL) {
    return;
  }

  switch (node->kind) {
  case NODE_KIND_ADD:
    generate_add(node);
    break;
  case NODE_KIND_ADD_POINTER:
    generate_add_pointer(node);
    break;
  case NODE_KIND_ADDRESS:
    generate_address(node);
    break;
  case NODE_KIND_ASSIGN:
    generate_assign(node);
    break;
  case NODE_KIND_BLOCK:
    generate_block(node);
    break;
  case NODE_KIND_DEREFERENCE:
    generate_dereference(node);
    break;
  case NODE_KIND_DIFF_POINTER:
    generate_diff_pointer(node);
    break;
  case NODE_KIND_DIVIDE:
    generate_divide(node);
    break;
  case NODE_KIND_EQ:
    generate_eq(node);
    break;
  case NODE_KIND_FOR:
    generate_for(node);
    break;
  case NODE_KIND_FUNCTION_CALL:
    generate_function_call(node);
    break;
  case NODE_KIND_FUNCTION_DEFINITION:
    generate_function_definition(node);
    break;
  case NODE_KIND_GLOBAL_VARIABLE_DEFINITION:
    generate_global_variable_definition(node);
    break;
  case NODE_KIND_IF:
    generate_if(node);
    break;
  case NODE_KIND_LE:
    generate_le(node);
    break;
  case NODE_KIND_LOCAL_VARIABLE:
    generate_local_variable(node);
    break;
  case NODE_KIND_LT:
    generate_lt(node);
    break;
  case NODE_KIND_MULTIPLY:
    generate_multiply(node);
    break;
  case NODE_KIND_NE:
    generate_ne(node);
    break;
  case NODE_KIND_NUMBER:
    generate_number(node);
    break;
  case NODE_KIND_PROGRAM:
    generate_program(node);
    break;
  case NODE_KIND_RETURN:
    generate_return(node);
    break;
  case NODE_KIND_SUBTRACT:
    generate_subtract(node);
    break;
  case NODE_KIND_SUBTRACT_POINTER:
    generate_subtract_pointer(node);
    break;
  case NODE_KIND_WHILE:
    generate_while(node);
    break;
  default:
    fprintf(stderr, "Unexpected node.\n");
    exit(1);
  }
}
