# Cache-Simulator

## Introduction

In this project I designed a cache simulator which simulates the direct mapped to fully associative cache. This cache is unified, which means that data and istructions are in the same cache. This sumulator also uses the writing policy of write back and write allocate. 

## How to run the code

For running this code, use the command "make -f Makefile" in Linux operating system. This command will compile and connect "cache.c" and "main.c" C code files and creates an executable "sim" file. For analysing the performance of this simulator, you can download and use one of the trace files in "Trace Files" folder. Each trace file contains the type of access to the cache and the address of the memory for that access.

## Cache Statistics

The following basic cache statistics can be measured in this simulator for instructions and data:

- Total number of cache accesses
- Cache misses
- Cache miss rate
- The number of replaces
- Traffic of the cache

