#!/bin/bash

gcc -c -o server.o server.c
g++ -c -o tinyxml2.o tinyxml2.cpp
g++ -c -o weatherReport.o weatherReport.cpp
g++ -o server4 *.o -lpthread
rm *.o

