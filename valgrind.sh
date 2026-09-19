#!/bin/bash
    
g++ main.cpp -o main -O0 -pg
valgrind ./main