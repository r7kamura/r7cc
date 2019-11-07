#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./cc7 "$input" > tmp.s
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
assert 2 "main() { return 2; }"

# add
assert 3 "main() { return 1 + 2; }"

# subtract
assert 1 "main() { return 2 - 1; }"

# add and subtract
assert 0 "main() { return 1 + 2 - 3; }"

# parentheses
assert 6 "main() { return 1 + (2 + 3); }"

# multiply and divide
assert 1 "main() { return (1 + 2 * 3) / 7; }"

# unary minus
assert 1 "main() { return -1 + 2; }"

# eq
assert 0 "main() { return 1 == 2; }"
assert 1 "main() { return 2 == 2; }"

# ne
assert 1 "main() { return 1 != 2; }"
assert 0 "main() { return 2 != 2; }"

# lt
assert 1 "main() { return 1 < 2; }"
assert 0 "main() { return 1 < 1; }"

# gt
assert 1 "main() { return 2 > 1; }"
assert 0 "main() { return 1 > 1; }"

# le
assert 1 "main() { return 1 <= 1; }"
assert 0 "main() { return 2 <= 1; }"

# ge
assert 1 "main() { return 1 >= 1; }"
assert 0 "main() { return 1 >= 2; }"

# single-letter local variable
assert 1 "main() { a = 1; return a; }"
assert 2 "main() { a = 1; b = 2; return b; }"
assert 3 "main() { a = 1; b = 2; return a + b; }"

# multi-letter local variable
assert 1 "main() { foo = 1; return foo; }"
assert 1 "main() { foo = 1; bar = 2; return foo; }"
assert 2 "main() { foo = 1; bar = 2; return foo * bar; }"

# if
assert 2 "main() { if (1) return 2; }"
assert 3 "main() { if (0) return 2; return 3; }"
assert 2 "main() { if (0) a = 1; else a = 2; return a; }"
assert 1 "main() { if (1) a = 1; else a = 2; return a; }"

# for
assert 6 "main() { b = 0; for (a = 0; a < 3; a = a + 1) b = b + 2; return b; }"

# while
assert 3 "main() { a = 0; while (a < 3) a = a + 1; return a; }"

# function definition, function call
assert 2 "a() { return 2; } main() { return a(); }"

# function parameters
assert 6 "multiply(a, b) { return a * b; } main() { return multiply(2, 3); }"

# recursive function
assert 8 "fib(a) { if (a < 2) return a; return fib(a - 2) + fib(a - 1); } main() { return fib(6); }"

echo OK
