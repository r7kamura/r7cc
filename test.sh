#!/bin/sh
assert() {
  expected="$1"
  input="$2"

  ./r7cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

# minimal example
assert 2 "int main() { return 2; }"

# add
assert 3 "int main() { return 1 + 2; }"

# subtract
assert 1 "int main() { return 2 - 1; }"

# add and subtract
assert 0 "int main() { return 1 + 2 - 3; }"

# parentheses
assert 6 "int main() { return 1 + (2 + 3); }"

# multiply and divide
assert 1 "int main() { return (1 + 2 * 3) / 7; }"

# unary minus
assert 1 "int main() { return -1 + 2; }"

# eq
assert 0 "int main() { return 1 == 2; }"
assert 1 "int main() { return 2 == 2; }"

# ne
assert 1 "int main() { return 1 != 2; }"
assert 0 "int main() { return 2 != 2; }"

# lt
assert 1 "int main() { return 1 < 2; }"
assert 0 "int main() { return 1 < 1; }"

# gt
assert 1 "int main() { return 2 > 1; }"
assert 0 "int main() { return 1 > 1; }"

# le
assert 1 "int main() { return 1 <= 1; }"
assert 0 "int main() { return 2 <= 1; }"

# ge
assert 1 "int main() { return 1 >= 1; }"
assert 0 "int main() { return 1 >= 2; }"

# single-letter local variable
assert 1 "int main() { int a; a = 1; return a; }"
assert 2 "int main() { int a; int b; a = 1; b = 2; return b; }"
assert 3 "int main() { int a; int b; a = 1; b = 2; return a + b; }"

# multi-letter local variable
assert 1 "int main() { int foo; foo = 1; return foo; }"
assert 1 "int main() { int foo; int bar; foo = 1; bar = 2; return foo; }"
assert 2 "int main() { int foo; int bar; foo = 1; bar = 2; return foo * bar; }"

# if
assert 2 "int main() { if (1) return 2; }"
assert 3 "int main() { if (0) return 2; return 3; }"
assert 2 "int main() { int a; if (0) a = 1; else a = 2; return a; }"
assert 1 "int main() { int a; if (1) a = 1; else a = 2; return a; }"

# for
assert 6 "int main() { int a; int b; b = 0; for (a = 0; a < 3; a = a + 1) b = b + 2; return b; }"

# while
assert 3 "int main() { int a; a = 0; while (a < 3) a = a + 1; return a; }"

# function definition, function call
assert 2 "int a() { return 2; } int main() { return a(); }"

# function parameters
assert 6 "int multiply(int a, int b) { return a * b; } int main() { return multiply(2, 3); }"

# recursive function
assert 8 "int fib(int a) { if (a < 2) return a; return fib(a - 2) + fib(a - 1); } int main() { return fib(6); }"

# address and dereference
assert 2 "int main() { int a; int *b; a = 2; b = &a; return *b; }"

# initializer
assert 2 "int main() { int a = 2; return a; }"

# pointer return-value
assert 1 "int *f() { int a = 0; int *b = &a; return b; } int main() { return 1; }"

# pointer add
assert 1 "int main() { int a = 1; int b = 2; return *(&b + 1); }"
assert 1 "int main() { int a = 1; int b = 2; return *(1 + &b); }"

# pointer subtract
assert 2 "int main() { int a = 1; int b = 2; return *(&a - 1); }"
assert 1 "int main() { int a = 1; return *((&a + 1) - 1); }"

# pointer diff
assert 1 "int main() { int a = 10; return (&a + 1) - &a; }"

# sizeof operator
assert 8 "int main() { int a; return sizeof(a); }"
assert 16 "int main() { int a; return sizeof(&a); }"
assert 8 "int main() { int a; return sizeof(*&a); }"

# array declaration
assert 80 "int main() { int a[10]; return sizeof(a); }"

# array access via pointer
assert 1 "int main() { int a[2]; *a = 1; return *a; }"
assert 3 "int main() { int a[2]; *a = 1; *(a + 1) = 2; int *p = a; return *p + *(p + 1); }"

# array access via bracket
assert 1 "int main() { int a[2]; a[0] = 0; a[1] = 1; return a[1]; }"

# array of array
assert 1 "int main() { int a[2][3]; a[0][0] = 1; return a[0][0]; }"
assert 1 "int main() { int a[2][3]; a[0][1] = 1; return a[0][1]; }"
assert 1 "int main() { int a[2][3]; a[0][2] = 1; return a[0][2]; }"
assert 1 "int main() { int a[2][3]; a[1][0] = 1; return a[1][0]; }"

# global variable
assert 1 "int a; int main() { a = 1; return a; }"
assert 1 "int a[10]; int main() { a[0] = 1; return a[0]; }"

# char type
assert 1 "int main() { char a = 1; return 1; }"
assert 2 "int main() { char a = 1; char b = 2; return b; }"
assert 1 "int main() { char a; return sizeof(a); }"
assert 10 "int main() { char a[10]; return sizeof(a); }"

echo OK
