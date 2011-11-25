###################################################################
# @file Makefile
# 
# Makefile for server and client 
# 
# Created at Wed Nov 23 21:02:39 CST 2007
#
# Created by Stacies
###################################################################

SRC_DIR = ./src
OUT_DIR = ./OUTPUT
BIN_DIR = ./BIN
TEST_IN_DIR = $(SRC_DIR)/Test
TEST_OUT_DIR = ./Test
SHELL_DIR = ./shell
DOC_OUT_DIR = ./DOC_OUTPUT
CC = g++ -O2 -g

all : client

test : 

clean :
	rm -rf client
	@echo "-------- Clean All Done --------"

server : server.cpp
	${CC} $^ -o $@ -lpthread

client : client.cpp
	${CC} $^ -o $@ -lpthread

test : client
	./client localhost 9000 1 1 1 -proxy localhost 8000
