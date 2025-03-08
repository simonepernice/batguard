#!/bin/sh
clear
cppcheck --enable=all --std=c++11 -I ./include/ ./src/
