 
CC := arm-linux-gnueabihf-gcc 
INCLUDES := -I. -I../ 
CCFLAGS := -g3 -o0 -Wall 

target := tests
src_files := ${target}.c ../n_rf24l01.c
h_files := ../n_rf24l01.h

${target}: ${src_files} ${h_files} Makefile
	$(CC) ${src_files} $(CCFLAGS) $(INCLUDES) -o ${target}

clean: 
	rm -f ${target} 

