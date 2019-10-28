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

assert 0 0
assert 1 1
assert 42 42

assert 3 1+2
assert 1 2-1
assert 16 11+20-15
assert 16 "11 + 20 - 15"
assert 6 "1 + (2 + 3)"
assert 1 "(1 + 2 * 3) / 7"
assert 1 "-1 + 2"
assert 15 "-1 * (2 + 3) + 4 * 5"

assert 0 "1==2"
assert 1 "2==2"

assert 1 "1!=2"
assert 0 "2!=2"

assert 1 "1<2"
assert 0 "1<1"

assert 1 "2>1"
assert 0 "1>1"

assert 1 "1<=1"
assert 0 "2<=1"

assert 1 "1>=1"
assert 0 "1>=2"

echo OK
