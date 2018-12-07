#!/bin/bash
try() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$expected expected, but got $actual"
    exit 1
  fi
}

try 0 'return 0;'
try 42 'return 42;'
try 21 'return 5+20-4;'
try 41 'return  12 + 34 - 5 ;'
try 47 "return 5+6*7;"
try 8 "return 5+6/2;"
try 13 "return 5+6*2-8/2;"
try 15 "return 5*(9-6);"
try 4 "a=b=2; return a+b;"
try 5 "a=2; b=3; return a+b;"
try 30 'a=3+2+1; b=5; return a*b;'
try 1 'return 1 == 1;'
try 0 'return 1 == 0;'
try 0 'return 1 != 1;'
try 1 'return 1 != 0;'
try 1 'a=1+2; b=3; return a==b;'
try 1 'a=1*2; b=3; return a!=b;'
echo OK