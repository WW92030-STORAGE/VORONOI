#!/bin/bash
    
g++ main.cpp -o main -O3
./main > voronoi.out
rm main

python3 render.py