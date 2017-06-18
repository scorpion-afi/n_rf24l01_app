 
CC := arm-linux-gnueabihf-g++ 
CCFLAGS := -g3 -o0 -Wall --std=c++11

target := event_fd
src_files := src/spidev_test.cpp

${target}: ${src_files} Makefile
	$(CC) ${src_files} $(CCFLAGS) -o ${target}
clean: 
	rm -f ${target} 

