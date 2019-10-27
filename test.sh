#!/bin/bash
try() {
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

try 0 0
try 1 1
try 42 42
try 3 1+2
try 1 2-1
try 16 11+20-15

echo OK
