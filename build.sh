#!/bin/sh

set +xe

LIB="/home/zaraki/libs/cpp"
SFML=$LIB/"SFML-2.6.1"

clang++ *.cpp -o debug/main -Wall -I./include -I$SFML/include -L$SFML/lib -lsfml-graphics -lsfml-window -lsfml-system
