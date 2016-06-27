 
CC := arm-linux-gnueabihf-gcc 
INCLUDES := -I. -I../src/n_rf24l01_lib/src/
CCFLAGS := -g3 -o0 -Wall 

target := spidev_test
src_files := ${target}.c init_n_rf24l01.c ../src/n_rf24l01_lib/src/n_rf24l01.c
h_files := init_n_rf24l01.h

${target}: ${src_files} ${h_files} Makefile
	$(CC) ${src_files} $(CCFLAGS) $(INCLUDES) -o ${target}

clean: 
	rm -f ${target} 

