C   = gcc
CXX = g++

all : shell.c
	${C} $< -o hw3
