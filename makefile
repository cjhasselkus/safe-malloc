CC = gcc
WARNING_FLAGS = -Wall -Wextra -g -O0
EXE = output
#SCAN_BUILD_DIR = <directory path for placement of CSA results>


all: main.o Safemalloc.o rangeTree.o
	$(CC) -o $(EXE) main.o Safemalloc.o rangeTree.o

# main.c is the testcase file name
main.o: main.c
	$(CC) $(WARNING_FLAGS) -c main.c

# Safemalloc .o files
obj: Safemalloc.o rangeTree.o

#individual targets
Safemalloc.o: Safemalloc.c Safemalloc.h
	$(CC) $(WARNING_FLAGS) -c Safemalloc.c
 
rangeTree.o: rangeTree.c rangeTree.h
	$(CC) $(WARNING_FLAGS) -c rangeTree.c
 
clean:
	rm -f $(EXE) *.o
	rm -rf $(SCAN_BUILD_DIR)

#
# Run the Clang Static Analyzer
#
#scan-build: clean
#	scan-build -o $(SCAN_BUILD_DIR) make