#!/bin/bash

# Simple test of the command interpreter

# 1
in="abbabaq"
out="0,3,5;"
[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "Test 1: PASSED" || echo "Test 1: FAILED"

# 2
in="aaacbaq"  
out="0,1,5;"
[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "Test 2: PASSED" || echo "Test 2: FAILED"

# 3
in="aaaaccccq"  
out=";"
[[ $(./cmd_int <<< "$in") == "$out"* ]] && echo "Test 3: PASSED" || echo "Test 3: FAILED"



