# compile both programs
PROGRAMS = lz4gb

# compiler
CC  = gcc
CPP = g++

# compiler flags
# add -m32 if you want 32bit binaries (or add to EXTRAFLAGS via command-line)
CCFLAGS   = -O2 -Wall -pedantic -s -std=c99
CPPFLAGS  = -O2 -Wall -pedantic -s

# build only lz4gb
lz4gb: lz4gb.h lz4gb.cpp Makefile
	$(CPP) $(CPPFLAGS) $(EXTRAFLAGS) lz4gb.cpp  -o lz4gb

# delete binaries
clean:
	-rm -f $(PROGRAMS)
