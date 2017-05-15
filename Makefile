C   = gcc
CXX = g++

all : shell.cpp
	${CXX} $< -o hw3 -std=c++11
