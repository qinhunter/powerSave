#!/bin/bash

gcc -o send.o send.c -lrt -lpthread -Wall -Wno-unused-function

gcc -o fwd.o fwd.c -lrt -lpthread -Wall -Wno-unused-function

gcc -o receive.o receive.c -lrt -lpthread -Wall -Wno-unused-function

gcc -o clear.o clear.c -lrt -lpthread -Wall
