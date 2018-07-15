CC := arm-linux-gnueabihf-g++
INCLUDES := -I.
LDFLAGS := -L./n_rf24l01_library/linux/build -ln_rf24l01
CCFLAGS := -g3 -o0 -Wall -std=c++17 

target := n_rf24l01_test
src_files := main.cpp 

${target}: ${src_files} Makefile
	$(CC) ${src_files} $(CCFLAGS) $(INCLUDES) $(LDFLAGS) -o ${target}

util : util.cpp Makefile
	$(CC) util.cpp $(CCFLAGS) $(INCLUDES) $(LDFLAGS) -o $@
	
clean: 
	rm -f ${target} 
	rm -f util
