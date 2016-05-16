C = gcc
FLAGS = -Wall -g 

all: bpshell 

bpshell: bpshell.c shellcmds.c 
	$(C) $(FLAGS) -o bpshell bpshell.c shellcmds.c

clean: 
	rm bpshell
