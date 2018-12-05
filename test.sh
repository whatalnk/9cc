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

try 0 '0;'
try 42 '42;'
try 21 '5+20-4;'
try 41 ' 12 + 34 - 5 ;'
try 47 "5+6*7;"
try 8 "5+6/2;"
try 13 "5+6*2-8/2;"
try 15 "5*(9-6);"
try 4 "a=b=2;a+b;"
try 5 "a=2;b=3;a+b;"
try 30 'a=3+2+1;b=5;a*b;'
try 1 '1 == 1;'
try 0 '1 == 0;'
try 0 '1 != 1;'
try 1 '1 != 0;'
try 1 'a=1+2;b=3;a==b;'
try 1 'a=1*2;b=3;a!=b;'
echo OK